#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ============================================================
// MQTT CONFIGURATION
// ============================================================

namespace SmartClockMqttConfig {

    constexpr char BROKER_HOST[] = "192.168.1.93";
    constexpr uint16_t BROKER_PORT = 1883;

    constexpr char DEVICE_ID[] = "smartclock-01";

    // Оставь пустыми, если Mosquitto не требует авторизации.
    constexpr char USERNAME[] = "";
    constexpr char PASSWORD[] = "";
}

// ============================================================
// MQTT CONSTANTS
// ============================================================

namespace SmartClockMqttConstants {

    constexpr uint16_t BUFFER_SIZE = 512;

    constexpr uint32_t RECONNECT_INTERVAL_MS = 5000;

    constexpr uint32_t STATE_INTERVAL_MS = 30000;

    constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 10000;

    constexpr uint32_t TCP_TIMEOUT_MS = 3000;

    constexpr size_t MAX_PAYLOAD_SIZE = 512;

    constexpr size_t MAX_TOPIC_SIZE = 96;

    constexpr size_t MAX_DEVICE_ID_SIZE = 48;

    constexpr size_t MAX_USERNAME_SIZE = 64;

    constexpr size_t MAX_PASSWORD_SIZE = 96;
}

// ============================================================
// MQTT CLIENT
// ============================================================

class SmartClockMqttClient {

public:

    // --------------------------------------------------------
    // EVENTS
    // --------------------------------------------------------

    enum class Event {

        CONNECTED,

        DISCONNECTED,

        COMMAND_RECEIVED,

        ERROR
    };

    using EventCallback = void (*)(
        Event event,
        const char* topic,
        const char* payload
    );

    // --------------------------------------------------------
    // CONSTRUCTOR
    // --------------------------------------------------------

    SmartClockMqttClient();

    ~SmartClockMqttClient();

    SmartClockMqttClient(
        const SmartClockMqttClient&
    ) = delete;

    SmartClockMqttClient& operator=(
        const SmartClockMqttClient&
    ) = delete;

    // --------------------------------------------------------
    // LIFECYCLE
    // --------------------------------------------------------

    bool begin();

    void update();

    bool isConnected();

    // --------------------------------------------------------
    // PUBLISH
    // --------------------------------------------------------

    bool publish(
        const char* topic,
        const char* payload,
        bool retained = false
    );

    bool publishJson(
        const char* topic,
        JsonDocument& document,
        bool retained = false
    );

    bool publishTelemetry(
        JsonDocument& document
    );

    bool publishState(
        JsonDocument& document,
        bool retained = true
    );

    bool publishEvent(
        const char* eventName
    );

    // --------------------------------------------------------
    // ACK
    // --------------------------------------------------------

    bool publishAck(
        JsonDocument& document
    );

    bool publishAck(
        const char* requestId,
        const char* command,
        bool success,
        const char* message = nullptr
    );

    // --------------------------------------------------------
    // CALLBACK
    // --------------------------------------------------------

    void setEventCallback(
        EventCallback callback
    );

    // --------------------------------------------------------
    // GETTERS
    // --------------------------------------------------------

    const char* getDeviceId() const;

    const char* getCommandTopic() const;

    const char* getStateTopic() const;

    const char* getTelemetryTopic() const;

    const char* getEventTopic() const;

    const char* getAckTopic() const;

private:

    // --------------------------------------------------------
    // CONNECTION
    // --------------------------------------------------------

    bool connect();

    bool probeBrokerTcp();

    void disconnect(
        const char* reason
    );

    // --------------------------------------------------------
    // MQTT CALLBACK
    // --------------------------------------------------------

    static void mqttCallback(
        char* topic,
        uint8_t* payload,
        unsigned int length
    );

    void handleMessage(
        const char* topic,
        const uint8_t* payload,
        unsigned int length
    );

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    void publishConnectionState(
        bool connected
    );

    // --------------------------------------------------------
    // EVENTS
    // --------------------------------------------------------

    void notify(
        Event event,
        const char* topic,
        const char* payload
    );

    // --------------------------------------------------------
    // CLIENTS
    // --------------------------------------------------------

    WiFiClient _wifiClient;

    PubSubClient _mqttClient;

    // --------------------------------------------------------
    // CONFIGURATION
    // --------------------------------------------------------

    char _broker[64];

    uint16_t _port;

    char _deviceId[
        SmartClockMqttConstants::MAX_DEVICE_ID_SIZE
    ];

    char _username[
        SmartClockMqttConstants::MAX_USERNAME_SIZE
    ];

    char _password[
        SmartClockMqttConstants::MAX_PASSWORD_SIZE
    ];

    // --------------------------------------------------------
    // TOPICS
    // --------------------------------------------------------

    char _commandTopic[
        SmartClockMqttConstants::MAX_TOPIC_SIZE
    ];

    char _stateTopic[
        SmartClockMqttConstants::MAX_TOPIC_SIZE
    ];

    char _telemetryTopic[
        SmartClockMqttConstants::MAX_TOPIC_SIZE
    ];

    char _eventTopic[
        SmartClockMqttConstants::MAX_TOPIC_SIZE
    ];

    char _ackTopic[
        SmartClockMqttConstants::MAX_TOPIC_SIZE
    ];

    // --------------------------------------------------------
    // STATUS
    // --------------------------------------------------------

    bool _initialized;

    bool _wasConnected;

    bool _hasReconnectAttempted;

    bool _wifiWasConnected;

    bool _tcpProbePassed;

    // --------------------------------------------------------
    // TIMERS
    // --------------------------------------------------------

    uint32_t _lastReconnectAttempt;

    uint32_t _lastStatePublish;

    uint32_t _lastWifiReconnectAttempt;

    // --------------------------------------------------------
    // CALLBACK
    // --------------------------------------------------------

    EventCallback _eventCallback;

    // --------------------------------------------------------
    // INSTANCE
    // --------------------------------------------------------

    static SmartClockMqttClient* _instance;
};