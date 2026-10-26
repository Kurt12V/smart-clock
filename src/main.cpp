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
    Serial.println();
    Serial.println("========== MQTT MESSAGE ==========");

    Serial.print("Topic: ");
    Serial.println(topic);

    Serial.print("Payload: ");

    for (unsigned int i = 0; i < length; ++i)
    {
        Serial.print(static_cast<char>(payload[i]));
    }

    Serial.println();
    Serial.println("==================================");
    Serial.println();

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

        Serial.println("[MQTT] Sent pong");
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

    Serial.println("[WiFi] Connecting...");

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

    Serial.println("[WiFi] Reconnecting...");

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

    Serial.println("[MQTT] Connecting to broker...");

    Serial.print("[MQTT] Broker: ");
    Serial.print(MQTTConfig::BROKER_HOST);
    Serial.print(':');
    Serial.println(MQTTConfig::BROKER_PORT);

    Serial.print("[MQTT] Client ID: ");
    Serial.println(MQTTConfig::CLIENT_ID);

    if (
        mqttClient.connect(
            MQTTConfig::CLIENT_ID
        )
    )
    {
        Serial.println("[MQTT] Connected successfully");

        if (
            mqttClient.subscribe(
                MQTTConfig::COMMAND_TOPIC
            )
        )
        {
            Serial.print("[MQTT] Subscribed: ");
            Serial.println(MQTTConfig::COMMAND_TOPIC);
        }
        else
        {
            Serial.println("[MQTT] Subscribe failed");
        }

        mqttClient.publish(
            MQTTConfig::STATE_TOPIC,
            "{\"status\":\"online\"}"
        );

        return true;
    }

    Serial.print("[MQTT] Connection failed. State: ");
    Serial.println(mqttClient.state());

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
        Serial.print("[MQTT] Published: ");
        Serial.println(payload);
    }
    else
    {
        Serial.println("[MQTT] Publish failed");
    }
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("==================================");
    Serial.println(" SmartClock MQTT Connection Test");
    Serial.println("==================================");

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

            Serial.println("[WiFi] Connected");

            Serial.print("[WiFi] ESP32 IP: ");
            Serial.println(WiFi.localIP());

            Serial.print("[WiFi] Gateway: ");
            Serial.println(WiFi.gatewayIP());

            Serial.print("[WiFi] RSSI: ");
            Serial.println(WiFi.RSSI());

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

            Serial.println("[WiFi] Disconnected");
        }
    }

    delay(2);
}