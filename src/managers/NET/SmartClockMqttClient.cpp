#include "SmartClockMqttClient.h"

#include <cstring>
#include <cstdio>

// ============================================================
// STATIC INSTANCE
// ============================================================

SmartClockMqttClient*
    SmartClockMqttClient::_instance = nullptr;


// ============================================================
// CONSTRUCTOR
// ============================================================

SmartClockMqttClient::SmartClockMqttClient()
    : _wifiClient(),
      _mqttClient(_wifiClient),

      _broker{},
      _port(0),

      _deviceId{},
      _username{},
      _password{},

      _commandTopic{},
      _stateTopic{},
      _telemetryTopic{},
      _eventTopic{},
      _ackTopic{},

      _initialized(false),
      _wasConnected(false),
      _hasReconnectAttempted(false),
      _wifiWasConnected(false),
      _tcpProbePassed(false),

      _lastReconnectAttempt(0),
      _lastStatePublish(0),
      _lastWifiReconnectAttempt(0),

      _eventCallback(nullptr) {

    _mqttClient.setKeepAlive(30);

    _mqttClient.setSocketTimeout(5);

    Serial0.println(
        "[MQTT] Клиент создан"
    );
}


// ============================================================
// DESTRUCTOR
// ============================================================

SmartClockMqttClient::~SmartClockMqttClient() {

    if (_mqttClient.connected()) {
        _mqttClient.disconnect();
    }

    if (_instance == this) {
        _instance = nullptr;
    }
}


// ============================================================
// BEGIN
// ============================================================

bool SmartClockMqttClient::begin() {

    Serial0.println();
    Serial0.println(
        "========================================"
    );
    Serial0.println(
        "[MQTT] Инициализация SmartClockMqttClient"
    );
    Serial0.println(
        "========================================"
    );

    // --------------------------------------------------------
    // SINGLE INSTANCE
    // --------------------------------------------------------

    if (_instance != nullptr && _instance != this) {

        Serial0.println(
            "[MQTT][ERROR] Другой экземпляр MQTT уже существует"
        );

        return false;
    }

    // --------------------------------------------------------
    // CONFIGURATION
    // --------------------------------------------------------

    const char* broker =
        SmartClockMqttConfig::BROKER_HOST;

    const uint16_t port =
        SmartClockMqttConfig::BROKER_PORT;

    const char* deviceId =
        SmartClockMqttConfig::DEVICE_ID;

    const char* username =
        SmartClockMqttConfig::USERNAME;

    const char* password =
        SmartClockMqttConfig::PASSWORD;

    if (broker == nullptr ||
        broker[0] == '\0' ||
        deviceId == nullptr ||
        deviceId[0] == '\0' ||
        port == 0) {

        Serial0.println(
            "[MQTT][ERROR] Некорректная конфигурация"
        );

        return false;
    }

    if (strlen(broker) >= sizeof(_broker) ||
        strlen(deviceId) >= sizeof(_deviceId)) {

        Serial0.println(
            "[MQTT][ERROR] Адрес брокера или Device ID слишком длинный"
        );

        return false;
    }

    if (username != nullptr &&
        strlen(username) >= sizeof(_username)) {

        Serial0.println(
            "[MQTT][ERROR] Username слишком длинный"
        );

        return false;
    }

    if (password != nullptr &&
        strlen(password) >= sizeof(_password)) {

        Serial0.println(
            "[MQTT][ERROR] Password слишком длинный"
        );

        return false;
    }

    if (password != nullptr &&
        password[0] != '\0' &&
        (username == nullptr || username[0] == '\0')) {

        Serial0.println(
            "[MQTT][ERROR] Password задан без Username"
        );

        return false;
    }

    snprintf(
        _broker,
        sizeof(_broker),
        "%s",
        broker
    );

    _port = port;

    snprintf(
        _deviceId,
        sizeof(_deviceId),
        "%s",
        deviceId
    );

    snprintf(
        _username,
        sizeof(_username),
        "%s",
        username != nullptr ? username : ""
    );

    snprintf(
        _password,
        sizeof(_password),
        "%s",
        password != nullptr ? password : ""
    );

    // --------------------------------------------------------
    // TOPICS
    // --------------------------------------------------------

    int result = snprintf(
        _commandTopic,
        sizeof(_commandTopic),
        "smartclock/%s/command",
        _deviceId
    );

    if (result < 0 ||
        static_cast<size_t>(result) >= sizeof(_commandTopic)) {

        Serial0.println(
            "[MQTT][ERROR] Ошибка создания command topic"
        );

        return false;
    }

    result = snprintf(
        _stateTopic,
        sizeof(_stateTopic),
        "smartclock/%s/state",
        _deviceId
    );

    if (result < 0 ||
        static_cast<size_t>(result) >= sizeof(_stateTopic)) {

        Serial0.println(
            "[MQTT][ERROR] Ошибка создания state topic"
        );

        return false;
    }

    result = snprintf(
        _telemetryTopic,
        sizeof(_telemetryTopic),
        "smartclock/%s/telemetry",
        _deviceId
    );

    if (result < 0 ||
        static_cast<size_t>(result) >= sizeof(_telemetryTopic)) {

        Serial0.println(
            "[MQTT][ERROR] Ошибка создания telemetry topic"
        );

        return false;
    }

    result = snprintf(
        _eventTopic,
        sizeof(_eventTopic),
        "smartclock/%s/events",
        _deviceId
    );

    if (result < 0 ||
        static_cast<size_t>(result) >= sizeof(_eventTopic)) {

        Serial0.println(
            "[MQTT][ERROR] Ошибка создания events topic"
        );

        return false;
    }

    result = snprintf(
        _ackTopic,
        sizeof(_ackTopic),
        "smartclock/%s/ack",
        _deviceId
    );

    if (result < 0 ||
        static_cast<size_t>(result) >= sizeof(_ackTopic)) {

        Serial0.println(
            "[MQTT][ERROR] Ошибка создания ack topic"
        );

        return false;
    }

    // --------------------------------------------------------
    // LOG CONFIGURATION
    // --------------------------------------------------------

    Serial0.printf(
        "[MQTT] Broker: %s:%u\n",
        _broker,
        _port
    );

    Serial0.printf(
        "[MQTT] Client ID: %s\n",
        _deviceId
    );

    Serial0.printf(
        "[MQTT] Authorization: %s\n",
        _username[0] != '\0'
            ? "enabled"
            : "disabled"
    );

    Serial0.printf(
        "[MQTT] Command topic: %s\n",
        _commandTopic
    );

    Serial0.printf(
        "[MQTT] State topic: %s\n",
        _stateTopic
    );

    Serial0.printf(
        "[MQTT] Telemetry topic: %s\n",
        _telemetryTopic
    );

    Serial0.printf(
        "[MQTT] Events topic: %s\n",
        _eventTopic
    );

    Serial0.printf(
        "[MQTT] ACK topic: %s\n",
        _ackTopic
    );

    // --------------------------------------------------------
    // MQTT BUFFER
    // --------------------------------------------------------

    if (!_mqttClient.setBufferSize(
            SmartClockMqttConstants::BUFFER_SIZE
        )) {

        Serial0.println(
            "[MQTT][ERROR] Не удалось установить буфер 512 байт"
        );

        return false;
    }

    // --------------------------------------------------------
    // MQTT CONFIGURATION
    // --------------------------------------------------------

    _mqttClient.setServer(
        _broker,
        _port
    );

    _mqttClient.setCallback(
        SmartClockMqttClient::mqttCallback
    );

    // --------------------------------------------------------
    // INITIAL STATE
    // --------------------------------------------------------

    _instance = this;

    _initialized = true;

    _wasConnected = false;

    _hasReconnectAttempted = false;

    _wifiWasConnected = false;

    _tcpProbePassed = false;

    _lastReconnectAttempt = 0;

    _lastStatePublish = 0;

    _lastWifiReconnectAttempt = 0;

    Serial0.println(
        "[MQTT] Инициализация завершена"
    );

    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    if (WiFi.status() == WL_CONNECTED) {

        Serial0.printf(
            "[MQTT] Wi-Fi connected: IP=%s, RSSI=%d dBm\n",
            WiFi.localIP().toString().c_str(),
            WiFi.RSSI()
        );

        _wifiWasConnected = true;

    } else {

        Serial0.println(
            "[MQTT] Wi-Fi не подключён. Ожидание подключения."
        );
    }

    Serial0.println(
        "========================================"
    );

    return true;
}


// ============================================================
// UPDATE
// ============================================================

void SmartClockMqttClient::update() {

    if (!_initialized) {
        return;
    }

    const uint32_t now = millis();

    // --------------------------------------------------------
    // WIFI DISCONNECTED
    // --------------------------------------------------------

    if (WiFi.status() != WL_CONNECTED) {

        if (_mqttClient.connected()) {

            Serial0.println(
                "[MQTT] Wi-Fi потерян. Отключение MQTT."
            );

            _mqttClient.disconnect();
        }

        if (_wasConnected) {

            _wasConnected = false;

            notify(
                Event::DISCONNECTED,
                "mqtt",
                "wifi_disconnected"
            );
        }

        _wifiWasConnected = false;

        return;
    }

    // --------------------------------------------------------
    // WIFI RECONNECTED
    // --------------------------------------------------------

    if (!_wifiWasConnected) {

        _wifiWasConnected = true;

        _hasReconnectAttempted = false;

        _lastReconnectAttempt = 0;

        Serial0.printf(
            "[MQTT] Wi-Fi восстановлен: %s\n",
            WiFi.localIP().toString().c_str()
        );
    }

    // --------------------------------------------------------
    // MQTT LOOP
    // --------------------------------------------------------

    if (_mqttClient.connected()) {

        _mqttClient.loop();
    }

    // --------------------------------------------------------
    // MQTT DISCONNECTED
    // --------------------------------------------------------

    if (!_mqttClient.connected()) {

        if (_wasConnected) {

            _wasConnected = false;

            Serial0.printf(
                "[MQTT] Соединение потеряно, state=%d\n",
                _mqttClient.state()
            );

            notify(
                Event::DISCONNECTED,
                "mqtt",
                "broker_disconnected"
            );
        }

        // ----------------------------------------------------
        // RECONNECT TIMER
        // ----------------------------------------------------

        const bool firstAttempt =
            !_hasReconnectAttempted;

        const bool intervalElapsed =
            now - _lastReconnectAttempt >=
            SmartClockMqttConstants::RECONNECT_INTERVAL_MS;

        if (firstAttempt || intervalElapsed) {

            _lastReconnectAttempt = now;

            _hasReconnectAttempted = true;

            connect();
        }

        return;
    }

    // --------------------------------------------------------
    // PERIODIC STATE
    // --------------------------------------------------------

    if (now - _lastStatePublish >=
        SmartClockMqttConstants::STATE_INTERVAL_MS) {

        _lastStatePublish = now;

        publishConnectionState(true);
    }
}


// ============================================================
// TCP PROBE
// ============================================================

bool SmartClockMqttClient::probeBrokerTcp() {

    Serial0.println(
        "[NET][PROBE] Проверка TCP-доступности брокера"
    );

    Serial0.printf(
        "[NET][PROBE] Target: %s:%u\n",
        _broker,
        _port
    );

    WiFiClient probe;

    probe.setTimeout(
        SmartClockMqttConstants::TCP_TIMEOUT_MS
    );

    const uint32_t start = millis();

    const bool success = probe.connect(
        _broker,
        _port
    );

    const uint32_t elapsed = millis() - start;

    if (!success) {

        Serial0.printf(
            "[NET][PROBE][ERROR] TCP connect FAILED after %lu ms\n",
            static_cast<unsigned long>(elapsed)
        );

        Serial0.println(
            "[NET][PROBE] Проверь IP, порт, Mosquitto и firewall."
        );

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
// CONNECT
// ============================================================

bool SmartClockMqttClient::connect() {

    if (!_initialized) {

        Serial0.println(
            "[MQTT][ERROR] Клиент не инициализирован"
        );

        return false;
    }

    if (WiFi.status() != WL_CONNECTED) {

        Serial0.println(
            "[MQTT] Подключение пропущено: Wi-Fi недоступен"
        );

        return false;
    }

    Serial0.println();
    Serial0.println(
        "[MQTT] ========== CONNECT ATTEMPT =========="
    );

    Serial0.printf(
        "[MQTT] Broker: %s:%u\n",
        _broker,
        _port
    );

    // --------------------------------------------------------
    // TCP CHECK
    // --------------------------------------------------------

    if (!probeBrokerTcp()) {

        Serial0.println(
            "[MQTT] Подключение отменено: TCP недоступен"
        );

        Serial0.println(
            "[MQTT] ===================================="
        );

        notify(
            Event::ERROR,
            "mqtt",
            "tcp_unreachable"
        );

        return false;
    }

    _tcpProbePassed = true;

    // --------------------------------------------------------
    // CLIENT ID
    // --------------------------------------------------------

    // ВАЖНО:
    // Не добавляем префикс "smartclock-".
    // DEVICE_ID уже равен "smartclock-01".

    const char* clientId = _deviceId;

    Serial0.printf(
        "[MQTT] Client ID: %s\n",
        clientId
    );

    // --------------------------------------------------------
    // MQTT CONNECT
    // --------------------------------------------------------

    Serial0.println(
        "[MQTT] Calling mqttClient.connect()..."
    );

    const uint32_t start = millis();

    bool connected = false;

    // --------------------------------------------------------
    // NO AUTHENTICATION
    // --------------------------------------------------------

    if (_username[0] == '\0') {

        connected = _mqttClient.connect(
            clientId
        );
    }

    // --------------------------------------------------------
    // AUTHENTICATION
    // --------------------------------------------------------

    else {

        connected = _mqttClient.connect(
            clientId,
            _username,
            _password
        );
    }

    const uint32_t elapsed = millis() - start;

    Serial0.printf(
        "[MQTT] connect() returned %s after %lu ms\n",
        connected ? "true" : "false",
        static_cast<unsigned long>(elapsed)
    );

    // --------------------------------------------------------
    // CONNECTION FAILED
    // --------------------------------------------------------

    if (!connected) {

        const int state = _mqttClient.state();

        Serial0.printf(
            "[MQTT][ERROR] MQTT state: %d\n",
            state
        );

        switch (state) {

            case -4:
                Serial0.println(
                    "[MQTT] MQTT_CONNECTION_TIMEOUT"
                );
                break;

            case -3:
                Serial0.println(
                    "[MQTT] MQTT_CONNECTION_LOST"
                );
                break;

            case -2:
                Serial0.println(
                    "[MQTT] MQTT_CONNECT_FAILED"
                );
                break;

            case -1:
                Serial0.println(
                    "[MQTT] MQTT_DISCONNECTED"
                );
                break;

            case 1:
                Serial0.println(
                    "[MQTT] MQTT_CONNECT_BAD_PROTOCOL"
                );
                break;

            case 2:
                Serial0.println(
                    "[MQTT] MQTT_CONNECT_BAD_CLIENT_ID"
                );
                break;

            case 3:
                Serial0.println(
                    "[MQTT] MQTT_CONNECT_UNAVAILABLE"
                );
                break;

            case 4:
                Serial0.println(
                    "[MQTT] MQTT_CONNECT_BAD_CREDENTIALS"
                );
                break;

            case 5:
                Serial0.println(
                    "[MQTT] MQTT_CONNECT_UNAUTHORIZED"
                );
                break;

            default:
                Serial0.println(
                    "[MQTT] UNKNOWN MQTT ERROR"
                );
                break;
        }

        _wasConnected = false;

        notify(
            Event::ERROR,
            "mqtt",
            "connection_failed"
        );

        Serial0.println(
            "[MQTT] ===================================="
        );

        return false;
    }

    // --------------------------------------------------------
    // CONNECTED
    // --------------------------------------------------------

    _wasConnected = true;

    Serial0.println(
        "[MQTT] Connected successfully"
    );

    // --------------------------------------------------------
    // SUBSCRIBE
    // --------------------------------------------------------

    const bool subscribed = _mqttClient.subscribe(
        _commandTopic
    );

    if (subscribed) {

        Serial0.printf(
            "[MQTT] Subscribed: %s\n",
            _commandTopic
        );

    } else {

        Serial0.printf(
            "[MQTT][ERROR] Subscribe failed: %s\n",
            _commandTopic
        );

        notify(
            Event::ERROR,
            _commandTopic,
            "subscription_failed"
        );
    }

    // --------------------------------------------------------
    // PUBLISH ONLINE STATE
    // --------------------------------------------------------

    publishConnectionState(true);

    _lastStatePublish = millis();

    notify(
        Event::CONNECTED,
        "mqtt",
        "connected"
    );

    Serial0.println(
        "[MQTT] ===================================="
    );

    return true;
}


// ============================================================
// MQTT CALLBACK
// ============================================================

void SmartClockMqttClient::mqttCallback(
    char* topic,
    uint8_t* payload,
    unsigned int length
) {

    if (_instance == nullptr) {
        return;
    }

    _instance->handleMessage(
        topic,
        payload,
        length
    );
}


// ============================================================
// HANDLE MESSAGE
// ============================================================

void SmartClockMqttClient::handleMessage(
    const char* topic,
    const uint8_t* payload,
    unsigned int length
) {

    if (topic == nullptr ||
        (payload == nullptr && length > 0)) {

        Serial0.println(
            "[MQTT][ERROR] Некорректное входящее сообщение"
        );

        notify(
            Event::ERROR,
            "mqtt",
            "invalid_message"
        );

        return;
    }

    if (length >=
        SmartClockMqttConstants::MAX_PAYLOAD_SIZE) {

        Serial0.printf(
            "[MQTT][ERROR] Payload слишком большой: %u байт\n",
            length
        );

        notify(
            Event::ERROR,
            topic,
            "payload_too_large"
        );

        return;
    }

    char message[
        SmartClockMqttConstants::MAX_PAYLOAD_SIZE
    ];

    if (length > 0) {

        memcpy(
            message,
            payload,
            length
        );
    }

    message[length] = '\0';

    Serial0.println();
    Serial0.println(
        "========== MQTT MESSAGE =========="
    );

    Serial0.printf(
        "[MQTT] Topic: %s\n",
        topic
    );

    Serial0.printf(
        "[MQTT] Payload: %s\n",
        message
    );

    Serial0.println(
        "=================================="
    );

    // --------------------------------------------------------
    // TOPIC VALIDATION
    // --------------------------------------------------------

    if (strcmp(topic, _commandTopic) != 0) {

        Serial0.println(
            "[MQTT][WARNING] Неожиданный топик"
        );

        return;
    }

    // --------------------------------------------------------
    // NOTIFY COMMAND HANDLER
    // --------------------------------------------------------

    notify(
        Event::COMMAND_RECEIVED,
        topic,
        message
    );
}


// ============================================================
// PUBLISH TEXT
// ============================================================

bool SmartClockMqttClient::publish(
    const char* topic,
    const char* payload,
    bool retained
) {

    if (topic == nullptr ||
        payload == nullptr) {

        Serial0.println(
            "[MQTT][ERROR] Публикация: пустой указатель"
        );

        return false;
    }

    if (!_mqttClient.connected()) {

        Serial0.printf(
            "[MQTT][ERROR] MQTT отключён: %s\n",
            topic
        );

        return false;
    }

    const size_t length = strlen(payload);

    Serial0.printf(
        "[MQTT] Publish: topic=%s, size=%u\n",
        topic,
        static_cast<unsigned int>(length)
    );

    const bool success = _mqttClient.publish(
        topic,
        payload,
        retained
    );

    Serial0.printf(
        "[MQTT] Publish result: %s\n",
        success ? "OK" : "FAILED"
    );

    return success;
}


// ============================================================
// PUBLISH JSON
// ============================================================

bool SmartClockMqttClient::publishJson(
    const char* topic,
    JsonDocument& document,
    bool retained
) {

    if (topic == nullptr) {

        Serial0.println(
            "[MQTT][ERROR] JSON topic не задан"
        );

        return false;
    }

    if (!_mqttClient.connected()) {

        Serial0.println(
            "[MQTT][ERROR] MQTT отключён; JSON не отправлен"
        );

        return false;
    }

    const size_t requiredSize = measureJson(
        document
    );

    if (requiredSize >=
        SmartClockMqttConstants::MAX_PAYLOAD_SIZE) {

        Serial0.printf(
            "[MQTT][ERROR] JSON слишком большой: %u байт\n",
            static_cast<unsigned int>(requiredSize)
        );

        return false;
    }

    char buffer[
        SmartClockMqttConstants::MAX_PAYLOAD_SIZE
    ];

    const size_t written = serializeJson(
        document,
        buffer,
        sizeof(buffer)
    );

    if (written != requiredSize) {

        Serial0.println(
            "[MQTT][ERROR] Ошибка сериализации JSON"
        );

        return false;
    }

    Serial0.printf(
        "[MQTT] Publish JSON: topic=%s, size=%u\n",
        topic,
        static_cast<unsigned int>(written)
    );

    const bool success = _mqttClient.publish(
        topic,
        reinterpret_cast<const uint8_t*>(buffer),
        static_cast<unsigned int>(written),
        retained
    );

    Serial0.printf(
        "[MQTT] JSON result: %s\n",
        success ? "OK" : "FAILED"
    );

    return success;
}


// ============================================================
// PUBLISH TELEMETRY
// ============================================================

bool SmartClockMqttClient::publishTelemetry(
    JsonDocument& document
) {

    return publishJson(
        _telemetryTopic,
        document,
        false
    );
}


// ============================================================
// PUBLISH STATE
// ============================================================

bool SmartClockMqttClient::publishState(
    JsonDocument& document,
    bool retained
) {

    return publishJson(
        _stateTopic,
        document,
        retained
    );
}


// ============================================================
// PUBLISH EVENT
// ============================================================

bool SmartClockMqttClient::publishEvent(
    const char* eventName
) {

    if (eventName == nullptr ||
        eventName[0] == '\0') {

        Serial0.println(
            "[MQTT][ERROR] Имя события не задано"
        );

        return false;
    }

    JsonDocument document;

    document["device_id"] = _deviceId;

    document["event"] = eventName;

    document["uptime_ms"] = millis();

    return publishJson(
        _eventTopic,
        document,
        false
    );
}


// ============================================================
// PUBLISH ACK JSON
// ============================================================

bool SmartClockMqttClient::publishAck(
    JsonDocument& document
) {

    if (!_mqttClient.connected()) {

        Serial0.println(
            "[MQTT][ERROR] ACK не отправлен: MQTT отключён"
        );

        return false;
    }

    if (document["device_id"].isNull()) {

        document["device_id"] = _deviceId;
    }

    if (document["uptime_ms"].isNull()) {

        document["uptime_ms"] = millis();
    }

    const char* requestId =
        document["request_id"] | "";

    const char* command =
        document["command"] | "";

    const char* status =
        document["status"] | "";

    Serial0.printf(
        "[MQTT] ACK: request_id=%s, command=%s, status=%s\n",
        requestId,
        command,
        status
    );

    return publishJson(
        _ackTopic,
        document,
        false
    );
}


// ============================================================
// PUBLISH ACK SIMPLE
// ============================================================

bool SmartClockMqttClient::publishAck(
    const char* requestId,
    const char* command,
    bool success,
    const char* message
) {

    if (requestId == nullptr ||
        command == nullptr ||
        requestId[0] == '\0' ||
        command[0] == '\0') {

        Serial0.println(
            "[MQTT][ERROR] ACK: request_id или command не задан"
        );

        return false;
    }

    JsonDocument document;

    document["device_id"] = _deviceId;

    document["request_id"] = requestId;

    document["command"] = command;

    document["status"] = success
        ? "success"
        : "error";

    document["uptime_ms"] = millis();

    if (message != nullptr) {

        document["message"] = message;
    }

    return publishAck(document);
}


// ============================================================
// CONNECTION STATE
// ============================================================

void SmartClockMqttClient::publishConnectionState(
    bool connected
) {

    JsonDocument document;

    document["device_id"] = _deviceId;

    document["connected"] = connected;

    document["wifi_connected"] =
        WiFi.status() == WL_CONNECTED;

    if (WiFi.status() == WL_CONNECTED) {

        document["ip"] =
            WiFi.localIP().toString();

        document["rssi"] = WiFi.RSSI();

    } else {

        document["ip"] = "";

        document["rssi"] = 0;
    }

    document["uptime_ms"] = millis();

    publishState(
        document,
        true
    );
}


// ============================================================
// SET EVENT CALLBACK
// ============================================================

void SmartClockMqttClient::setEventCallback(
    EventCallback callback
) {

    _eventCallback = callback;

    Serial0.printf(
        "[MQTT] Event callback: %s\n",
        callback != nullptr ? "set" : "cleared"
    );
}


// ============================================================
// NOTIFY
// ============================================================

void SmartClockMqttClient::notify(
    Event event,
    const char* topic,
    const char* payload
) {

    if (_eventCallback == nullptr) {
        return;
    }

    _eventCallback(
        event,
        topic != nullptr ? topic : "",
        payload != nullptr ? payload : ""
    );
}


// ============================================================
// DISCONNECT
// ============================================================

void SmartClockMqttClient::disconnect(
    const char* reason
) {

    if (_mqttClient.connected()) {

        _mqttClient.disconnect();
    }

    if (_wasConnected) {

        _wasConnected = false;

        notify(
            Event::DISCONNECTED,
            "mqtt",
            reason != nullptr ? reason : "disconnected"
        );
    }
}


// ============================================================
// GETTERS
// ============================================================

bool SmartClockMqttClient::isConnected() {

    return _mqttClient.connected();
}


const char* SmartClockMqttClient::getDeviceId() const {

    return _deviceId;
}


const char* SmartClockMqttClient::getCommandTopic() const {

    return _commandTopic;
}


const char* SmartClockMqttClient::getStateTopic() const {

    return _stateTopic;
}


const char* SmartClockMqttClient::getTelemetryTopic() const {

    return _telemetryTopic;
}


const char* SmartClockMqttClient::getEventTopic() const {

    return _eventTopic;
}


const char* SmartClockMqttClient::getAckTopic() const {

    return _ackTopic;
}