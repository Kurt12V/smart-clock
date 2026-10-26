#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>

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
        return false;
    }

    Serial0.println("[MQTT] Connecting to broker...");

    Serial0.print("[MQTT] Broker: ");
    Serial0.print(MQTTConfig::BROKER_HOST);
    Serial0.print(':');
    Serial0.println(MQTTConfig::BROKER_PORT);

    Serial0.print("[MQTT] Client ID: ");
    Serial0.println(MQTTConfig::CLIENT_ID);

    if (
        mqttClient.connect(
            MQTTConfig::CLIENT_ID
        )
    )
    {
        Serial0.println("[MQTT] Connected successfully");

        if (
            mqttClient.subscribe(
                MQTTConfig::COMMAND_TOPIC
            )
        )
        {
            Serial0.print("[MQTT] Subscribed: ");
            Serial0.println(MQTTConfig::COMMAND_TOPIC);
        }
        else
        {
            Serial0.println("[MQTT] Subscribe failed");
        }

        mqttClient.publish(
            MQTTConfig::STATE_TOPIC,
            "{\"status\":\"online\"}"
        );

        return true;
    }

    Serial0.print("[MQTT] Connection failed. State: ");
    Serial0.println(mqttClient.state());

    return false;
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
        Serial0.print("[MQTT] Published: ");
        Serial0.println(payload);
    }
    else
    {
        Serial0.println("[MQTT] Publish failed");
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

            Serial0.print("[WiFi] ESP32 IP: ");
            Serial0.println(WiFi.localIP());

            Serial0.print("[WiFi] Gateway: ");
            Serial0.println(WiFi.gatewayIP());

            Serial0.print("[WiFi] RSSI: ");
            Serial0.println(WiFi.RSSI());

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