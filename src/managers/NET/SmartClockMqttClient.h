#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

class SmartClockMqttClient {
public:

    // ========================================================
    // EVENTS
    // ========================================================

    enum class Event : uint8_t {
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


    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    SmartClockMqttClient();

    SmartClockMqttClient(
        const SmartClockMqttClient&
    ) = delete;

    SmartClockMqttClient& operator=(
        const SmartClockMqttClient&
    ) = delete;


    // ========================================================
    // INITIALIZATION
    // ========================================================

    bool begin(
        const char* broker,
        uint16_t port,
        const char* deviceId,
        const char* username = nullptr,
        const char* password = nullptr
    );


    // ========================================================
    // UPDATE
    // ========================================================

    void update();


    // ========================================================
    // CONNECTION
    // ========================================================

    bool isConnected();


    // ========================================================
    // PUBLISH TEXT
    // ========================================================

    bool publish(
        const char* topic,
        const char* payload,
        bool retained = false
    );


    // ========================================================
    // PUBLISH JSON
    // ========================================================

    bool publishJson(
        const char* topic,
        JsonDocument& document,
        bool retained = false
    );


    // ========================================================
    // TELEMETRY
    // ========================================================

    bool publishTelemetry(
        JsonDocument& document
    );


    // ========================================================
    // STATE
    // ========================================================

    bool publishState(
        JsonDocument& document,
        bool retained = true
    );


    // ========================================================
    // EVENTS
    // ========================================================

    bool publishEvent(
        const char* eventName
    );


    // ========================================================
    // ACK
    // ========================================================

    bool publishAck(
        JsonDocument& document
    );

    bool publishAck(
        const char* requestId,
        const char* command,
        bool success,
        const char* message = nullptr
    );


    // ========================================================
    // CALLBACK
    // ========================================================

    void setEventCallback(
        EventCallback callback
    );


    // ========================================================
    // GETTERS
    // ========================================================

    const char* getDeviceId() const;

    const char* getCommandTopic() const;

    const char* getStateTopic() const;

    const char* getTelemetryTopic() const;

    const char* getEventTopic() const;

    const char* getAckTopic() const;


private:

    // ========================================================
    // INSTANCE
    // ========================================================

    static SmartClockMqttClient* _instance;


    // ========================================================
    // MQTT
    // ========================================================

    WiFiClient _wifiClient;

    PubSubClient _mqttClient;


    // ========================================================
    // CONFIGURATION
    // ========================================================

    char _broker[128];

    char _deviceId[48];

    char _username[64];

    char _password[64];


    // ========================================================
    // TOPICS
    // ========================================================

    char _commandTopic[96];

    char _stateTopic[96];

    char _telemetryTopic[96];

    char _eventTopic[96];

    char _ackTopic[96];


    // ========================================================
    // FLAGS
    // ========================================================

    bool _initialized;

    bool _wasConnected;

    bool _hasReconnectAttempted;


    // ========================================================
    // TIMERS
    // ========================================================

    uint32_t _lastReconnectAttempt;

    uint32_t _lastStatePublish;


    // ========================================================
    // CONSTANTS
    // ========================================================

    static constexpr uint32_t RECONNECT_INTERVAL_MS = 5000;

    static constexpr uint32_t STATE_INTERVAL_MS = 30000;

    static constexpr size_t MQTT_BUFFER_SIZE = 1024;

    static constexpr size_t MAX_PAYLOAD_SIZE = 768;


    // ========================================================
    // CALLBACK
    // ========================================================

    EventCallback _eventCallback;


    // ========================================================
    // MQTT INTERNAL
    // ========================================================

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

    bool connect();

    void publishConnectionState(
        bool connected
    );


    // ========================================================
    // EVENT NOTIFICATION
    // ========================================================

    void notify(
        Event event,
        const char* topic = "",
        const char* payload = ""
    );
};