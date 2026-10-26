#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ============================================================
// WI-FI CONFIGURATION
// ============================================================

namespace WiFiConfig
{
    constexpr char SSID[] = "tpl45";
    constexpr char PASSWORD[] = "12713714";
}

// ============================================================
// MQTT CONFIGURATION
// ============================================================

namespace MQTTConfig
{
    constexpr char BROKER_HOST[] = "192.168.1.93";
    constexpr uint16_t BROKER_PORT = 1883;

    constexpr char CLIENT_ID[] = "smartclock-01";

    constexpr char COMMAND_TOPIC[] =
        "smartclock/smartclock-01/command";

    constexpr char TEST_TOPIC[] =
        "smartclock/smartclock-01/test";

    constexpr char STATE_TOPIC[] =
        "smartclock/smartclock-01/state";
}

// ============================================================
// MQTT CLIENT
// ============================================================

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// ============================================================
// TIMERS
// ============================================================

constexpr uint32_t MQTT_RECONNECT_INTERVAL_MS = 5000;
constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 10000;
constexpr uint32_t PUBLISH_INTERVAL_MS = 5000;

uint32_t lastMqttReconnectAttempt = 0;
uint32_t lastWifiReconnectAttempt = 0;
uint32_t lastPublishTime = 0;

uint32_t messageCounter = 0;

// ============================================================
// HELPERS
// ============================================================

static void printMqttStateName(int state)
{
    Serial0.printf("[MQTT][STATE] %d -> ", state);

    switch (state)
    {
        case -4: Serial0.println("MQTT_CONNECTION_TIMEOUT"); break;
        case -3: Serial0.println("MQTT_CONNECTION_LOST"); break;
        case -2: Serial0.println("MQTT_CONNECT_FAILED (TCP unreachable)"); break;
        case -1: Serial0.println("MQTT_DISCONNECTED"); break;
        case  0: Serial0.println("MQTT_CONNECTED"); break;
        case  1: Serial0.println("MQTT_CONNECT_BAD_PROTOCOL"); break;
        case  2: Serial0.println("MQTT_CONNECT_BAD_CLIENT_ID"); break;
        case  3: Serial0.println("MQTT_CONNECT_UNAVAILABLE"); break;
        case  4: Serial0.println("MQTT_CONNECT_BAD_CREDENTIALS"); break;
        case  5: Serial0.println("MQTT_CONNECT_UNAUTHORIZED"); break;
        default: Serial0.println("UNKNOWN"); break;
    }
}

// Проверка TCP-доступности брокера (до MQTT)
static bool probeBrokerTcp()
{
    Serial0.println("[NET][PROBE] Checking TCP reachability...");
    Serial0.printf("[NET][PROBE] Target: %s:%u\n",
                   MQTTConfig::BROKER_HOST,
                   MQTTConfig::BROKER_PORT);

    WiFiClient probe;

    // Таймаут на connect — 3 секунды
    const uint32_t start = millis();
    const bool ok = probe.connect(
        MQTTConfig::BROKER_HOST,
        MQTTConfig::BROKER_PORT
    );
    const uint32_t elapsed = millis() - start;

    if (!ok)
    {
        Serial0.printf(
            "[NET][PROBE][ERROR] TCP connect FAILED after %lu ms\n",
            static_cast<unsigned long>(elapsed)
        );
        Serial0.println("[NET][PROBE][ERROR] Возможные причины:");
        Serial0.println("  1. Брокер слушает только 127.0.0.1 (Mosquitto 2.0+)");
        Serial0.println("  2. Брандмауэр блокирует порт 1883");
        Serial0.println("  3. ESP32 и брокер в разных подсетях");
        Serial0.println("  4. Брокер не запущен");
        probe.stop();
        return false;
    }

    Serial0.printf(
        "[NET][PROBE] TCP connect OK (%lu ms)\n",
        static_cast<unsigned long>(elapsed)
    );

    probe.stop();
    return true;
}

// ============================================================
// MQTT CALLBACK
// ============================================================

void mqttCallback(
    char* topic,
    byte* payload,
    unsigned int length
)
{
    Serial0.println();
    Serial0.println("========== MQTT MESSAGE ==========");

    Serial0.print("Topic: ");
    Serial0.println(topic);

    Serial0.print("Payload: ");

    for (unsigned int i = 0; i < length; ++i)
    {
        Serial0.print(static_cast<char>(payload[i]));
    }

    Serial0.println();
    Serial0.println("==================================");
    Serial0.println();

    // Test command: ping
    if (
        strcmp(topic, MQTTConfig::COMMAND_TOPIC) == 0 &&
        length == 4 &&
        memcmp(payload, "ping", 4) == 0
    )
    {
        mqttClient.publish(
            MQTTConfig::STATE_TOPIC,
            "{\"status\":\"pong\"}"
        );

        Serial0.println("[MQTT] Sent pong");
    }
}

// ============================================================
// WIFI
// ============================================================

void connectWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    Serial0.println("[WiFi] Connecting...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(
        WiFiConfig::SSID,
        WiFiConfig::PASSWORD
    );
}

void printNetworkInfo()
{
    Serial0.println();
    Serial0.println("========== NETWORK INFO ==========");

    Serial0.printf("[NET] WiFi status: %d\n", WiFi.status());
    Serial0.printf("[NET] ESP32 IP:    %s\n",
                   WiFi.localIP().toString().c_str());
    Serial0.printf("[NET] Subnet mask: %s\n",
                   WiFi.subnetMask().toString().c_str());
    Serial0.printf("[NET] Gateway:     %s\n",
                   WiFi.gatewayIP().toString().c_str());
    Serial0.printf("[NET] DNS:         %s\n",
                   WiFi.dnsIP().toString().c_str());
    Serial0.printf("[NET] RSSI:        %d dBm\n", WiFi.RSSI());
    Serial0.printf("[NET] Broker:      %s:%u\n",
                   MQTTConfig::BROKER_HOST,
                   MQTTConfig::BROKER_PORT);

    // Сравнение подсетей
    const IPAddress ip   = WiFi.localIP();
    const IPAddress mask = WiFi.subnetMask();

    IPAddress broker;
    broker.fromString(MQTTConfig::BROKER_HOST);

    const bool sameSubnet =
        ((ip[0] & mask[0]) == (broker[0] & mask[0])) &&
        ((ip[1] & mask[1]) == (broker[1] & mask[1])) &&
        ((ip[2] & mask[2]) == (broker[2] & mask[2])) &&
        ((ip[3] & mask[3]) == (broker[3] & mask[3]));

    Serial0.printf("[NET] Same subnet as broker: %s\n",
                   sameSubnet ? "YES" : "NO (!!!)");

    if (!sameSubnet)
    {
        Serial0.println("[NET][WARN] ESP32 and broker are in different subnets.");
        Serial0.println("[NET][WARN] Direct connection may not work.");
    }

    Serial0.println("==================================");
    Serial0.println();
}

void updateWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    const uint32_t now = millis();

    if (
        now - lastWifiReconnectAttempt <
        WIFI_RECONNECT_INTERVAL_MS
    )
    {
        return;
    }

    lastWifiReconnectAttempt = now;

    Serial0.println("[WiFi] Reconnecting...");

    WiFi.disconnect();
    WiFi.begin(
        WiFiConfig::SSID,
        WiFiConfig::PASSWORD
    );
}

// ============================================================
// MQTT CONNECTION
// ============================================================

bool connectMQTT()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial0.println("[MQTT] Skipped: WiFi not connected");
        return false;
    }

    Serial0.println();
    Serial0.println("[MQTT] ========== CONNECT ATTEMPT ==========");
    Serial0.printf("[MQTT] Broker:    %s:%u\n",
                   MQTTConfig::BROKER_HOST,
                   MQTTConfig::BROKER_PORT);
    Serial0.printf("[MQTT] Client ID: %s\n",
                   MQTTConfig::CLIENT_ID);

    // Шаг 1: TCP-проверка
    if (!probeBrokerTcp())
    {
        Serial0.println("[MQTT] Aborting connect: TCP unreachable");
        Serial0.println("[MQTT] =====================================");
        return false;
    }

    // Шаг 2: MQTT-подключение
    Serial0.println("[MQTT] Calling mqttClient.connect()...");

    const uint32_t start = millis();
    const bool connected = mqttClient.connect(
        MQTTConfig::CLIENT_ID
    );
    const uint32_t elapsed = millis() - start;

    Serial0.printf(
        "[MQTT] connect() returned %s after %lu ms\n",
        connected ? "true" : "false",
        static_cast<unsigned long>(elapsed)
    );

    if (!connected)
    {
        const int state = mqttClient.state();
        printMqttStateName(state);

        Serial0.println("[MQTT] Подсказки:");
        Serial0.println("  -2  : TCP не установлен (см. [NET][PROBE])");
        Serial0.println("  -3  : TCP был, но соединение разорвано");
        Serial0.println("  -4  : таймаут (брокер медленный/фильтр)");
        Serial0.println("   4  : неверные логин/пароль");
        Serial0.println("   5  : нет прав (ACL)");
        Serial0.println("[MQTT] =====================================");
        return false;
    }

    Serial0.println("[MQTT] Connected successfully");

    if (
        mqttClient.subscribe(
            MQTTConfig::COMMAND_TOPIC
        )
    )
    {
        Serial0.printf("[MQTT] Subscribed: %s\n",
                       MQTTConfig::COMMAND_TOPIC);
    }
    else
    {
        Serial0.println("[MQTT] Subscribe failed");
    }

    const bool published = mqttClient.publish(
        MQTTConfig::STATE_TOPIC,
        "{\"status\":\"online\"}"
    );

    Serial0.printf("[MQTT] Publish online: %s\n",
                   published ? "OK" : "FAILED");

    Serial0.println("[MQTT] =====================================");
    return true;
}

void updateMQTT()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }

    if (!mqttClient.connected())
    {
        const uint32_t now = millis();

        if (
            now - lastMqttReconnectAttempt <
            MQTT_RECONNECT_INTERVAL_MS
        )
        {
            return;
        }

        lastMqttReconnectAttempt = now;

        connectMQTT();

        return;
    }

    mqttClient.loop();
}

// ============================================================
// TEST MESSAGE PUBLISHING
// ============================================================

void publishTestMessage()
{
    if (!mqttClient.connected())
    {
        return;
    }

    const uint32_t now = millis();

    if (
        now - lastPublishTime <
        PUBLISH_INTERVAL_MS
    )
    {
        return;
    }

    lastPublishTime = now;

    char payload[128];

    ++messageCounter;

    snprintf(
        payload,
        sizeof(payload),
        "{\"device\":\"smartclock-01\",\"counter\":%lu,\"uptime_ms\":%lu}",
        static_cast<unsigned long>(messageCounter),
        static_cast<unsigned long>(millis())
    );

    const bool published = mqttClient.publish(
        MQTTConfig::TEST_TOPIC,
        payload
    );

    if (published)
    {
        Serial0.printf("[MQTT] Published to %s: %s\n",
                       MQTTConfig::TEST_TOPIC,
                       payload);
    }
    else
    {
        Serial0.println("[MQTT] Publish failed (buffer full or disconnected)");
    }
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);

    delay(1000);

    Serial0.println();
    Serial0.println("==================================");
    Serial0.println(" SmartClock MQTT Connection Test");
    Serial0.println("==================================");

    mqttClient.setServer(
        MQTTConfig::BROKER_HOST,
        MQTTConfig::BROKER_PORT
    );

    mqttClient.setCallback(mqttCallback);

    mqttClient.setKeepAlive(30);
    mqttClient.setSocketTimeout(5);

    // Увеличиваем буфер для JSON
    mqttClient.setBufferSize(512);

    connectWiFi();
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    updateWiFi();

    if (WiFi.status() == WL_CONNECTED)
    {
        static bool wifiConnectedMessagePrinted = false;

        if (!wifiConnectedMessagePrinted)
        {
            wifiConnectedMessagePrinted = true;

            Serial0.println("[WiFi] Connected");
            printNetworkInfo();

            lastMqttReconnectAttempt = 0;
        }

        updateMQTT();

        publishTestMessage();
    }
    else
    {
        static bool wifiDisconnectedMessagePrinted = false;

        if (!wifiDisconnectedMessagePrinted)
        {
            wifiDisconnectedMessagePrinted = true;

            Serial0.println("[WiFi] Disconnected");
        }
    }

    delay(2);
}