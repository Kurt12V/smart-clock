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

      _lastReconnectAttempt(0),
      _lastStatePublish(0),

      _eventCallback(nullptr) {

    _mqttClient.setKeepAlive(30);

    _mqttClient.setSocketTimeout(3);

    Serial0.printf(
        "[MQTT] [INFO] MQTT-клиент создан\n"
    );
}


// ============================================================
// BEGIN
// ============================================================

bool SmartClockMqttClient::begin() {
    // Адрес MQTT-брокера и параметры устройства заданы в SmartClockMqttClient.h.
    const char* broker = SmartClockMqttConfig::BROKER_HOST;
    const uint16_t port = SmartClockMqttConfig::BROKER_PORT;
    const char* deviceId = SmartClockMqttConfig::DEVICE_ID;
    const char* username = SmartClockMqttConfig::USERNAME;
    const char* password = SmartClockMqttConfig::PASSWORD;
    Serial0.printf(
        "[MQTT] [INFO] Инициализация MQTT-клиента\n"
    );

    // --------------------------------------------------------
    // VALIDATION
    // --------------------------------------------------------

    if (broker == nullptr ||
        deviceId == nullptr ||
        broker[0] == '\0' ||
        deviceId[0] == '\0' ||
        port == 0) {

        Serial0.printf(
            "[MQTT] [ERROR] Некорректная конфигурация\n"
        );

        return false;
    }

    // --------------------------------------------------------
    // SINGLE INSTANCE
    // --------------------------------------------------------

    if (_instance != nullptr &&
        _instance != this) {

        Serial0.printf(
            "[MQTT] [ERROR] Другой MQTT-клиент уже существует\n"
        );

        return false;
    }

    // --------------------------------------------------------
    // VALIDATE LENGTH
    // --------------------------------------------------------

    if (strlen(broker) >= sizeof(_broker)) {

        Serial0.printf(
            "[MQTT] [ERROR] Адрес брокера слишком длинный\n"
        );

        return false;
    }

    if (strlen(deviceId) >= sizeof(_deviceId)) {

        Serial0.printf(
            "[MQTT] [ERROR] Device ID слишком длинный\n"
        );

        return false;
    }

    if (username != nullptr &&
        strlen(username) >= sizeof(_username)) {

        Serial0.printf(
            "[MQTT] [ERROR] Имя пользователя слишком длинное\n"
        );

        return false;
    }

    if (password != nullptr &&
        strlen(password) >= sizeof(_password)) {

        Serial0.printf(
            "[MQTT] [ERROR] Пароль слишком длинный\n"
        );

        return false;
    }

    if (password != nullptr &&
        password[0] != '\0' &&
        (username == nullptr || username[0] == '\0')) {

        Serial0.printf(
            "[MQTT] [ERROR] Пароль задан без имени пользователя\n"
        );

        return false;
    }

    // --------------------------------------------------------
    // COPY CONFIGURATION
    // --------------------------------------------------------

    snprintf(
        _broker,
        sizeof(_broker),
        "%s",
        broker
    );

    snprintf(
        _deviceId,
        sizeof(_deviceId),
        "%s",
        deviceId
    );

    if (username != nullptr) {

        snprintf(
            _username,
            sizeof(_username),
            "%s",
            username
        );
    }

    if (password != nullptr) {

        snprintf(
            _password,
            sizeof(_password),
            "%s",
            password
        );
    }

    // --------------------------------------------------------
    // CONFIGURATION LOG
    // --------------------------------------------------------

    Serial0.printf(
        "[MQTT] [INFO] Broker: %s:%u\n",
        _broker,
        port
    );

    Serial0.printf(
        "[MQTT] [INFO] Device ID: %s\n",
        _deviceId
    );

    Serial0.printf(
        "[MQTT] [INFO] Авторизация: %s\n",
        _username[0] != '\0'
            ? "включена"
            : "отключена"
    );

    // --------------------------------------------------------
    // CREATE TOPICS
    // --------------------------------------------------------

    int result = snprintf(
        _commandTopic,
        sizeof(_commandTopic),
        "smartclock/%s/command",
        _deviceId
    );

    if (result < 0 ||
        static_cast<size_t>(result) >= sizeof(_commandTopic)) {

        Serial0.printf(
            "[MQTT] [ERROR] Ошибка создания command topic\n"
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

        Serial0.printf(
            "[MQTT] [ERROR] Ошибка создания state topic\n"
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

        Serial0.printf(
            "[MQTT] [ERROR] Ошибка создания telemetry topic\n"
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

        Serial0.printf(
            "[MQTT] [ERROR] Ошибка создания events topic\n"
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

        Serial0.printf(
            "[MQTT] [ERROR] Ошибка создания ack topic\n"
        );

        return false;
    }

    // --------------------------------------------------------
    // TOPIC LOG
    // --------------------------------------------------------

    Serial0.printf(
        "[MQTT] [INFO] Command: %s\n",
        _commandTopic
    );

    Serial0.printf(
        "[MQTT] [INFO] State: %s\n",
        _stateTopic
    );

    Serial0.printf(
        "[MQTT] [INFO] Telemetry: %s\n",
        _telemetryTopic
    );

    Serial0.printf(
        "[MQTT] [INFO] Events: %s\n",
        _eventTopic
    );

    Serial0.printf(
        "[MQTT] [INFO] ACK: %s\n",
        _ackTopic
    );

    // --------------------------------------------------------
    // MQTT BUFFER
    // --------------------------------------------------------

    if (!_mqttClient.setBufferSize(MQTT_BUFFER_SIZE)) {

        Serial0.printf(
            "[MQTT] [ERROR] Не удалось выделить MQTT-буфер\n"
        );

        return false;
    }

    // --------------------------------------------------------
    // MQTT CONFIGURATION
    // --------------------------------------------------------

    _mqttClient.setServer(
        _broker,
        port
    );

    _mqttClient.setCallback(
        SmartClockMqttClient::mqttCallback
    );

    // --------------------------------------------------------
    // INITIALIZE
    // --------------------------------------------------------

    _instance = this;

    _initialized = true;

    _wasConnected = false;

    _hasReconnectAttempted = false;

    _lastReconnectAttempt = 0;

    _lastStatePublish = 0;

    Serial0.printf(
        "[MQTT] [INFO] Инициализация завершена\n"
    );

    // --------------------------------------------------------
    // WIFI STATUS
    // --------------------------------------------------------

    if (WiFi.status() == WL_CONNECTED) {

        Serial0.printf(
            "[MQTT] [INFO] Wi-Fi подключён: IP=%s, RSSI=%d dBm\n",
            WiFi.localIP().toString().c_str(),
            WiFi.RSSI()
        );

    } else {

        Serial0.printf(
            "[MQTT] [WARNING] Wi-Fi не подключён; ожидание подключения\n"
        );
    }

    return true;
}


// ============================================================
// UPDATE
// ============================================================

void SmartClockMqttClient::update() {
    if (!_initialized) {
        return;
    }

    // --------------------------------------------------------
    // WIFI STATUS
    // --------------------------------------------------------

    if (WiFi.status() != WL_CONNECTED) {

        if (_mqttClient.connected()) {

            Serial0.printf(
                "[MQTT] [WARNING] Wi-Fi потерян\n"
            );

            _mqttClient.disconnect();
        }

        if (_wasConnected) {

            _wasConnected = false;

            Serial0.printf(
                "[MQTT] [WARNING] MQTT отключён из-за потери Wi-Fi\n"
            );

            notify(
                Event::DISCONNECTED,
                "mqtt",
                "wifi_disconnected"
            );
        }

        return;
    }

    // --------------------------------------------------------
    // PROCESS MQTT
    // --------------------------------------------------------

    if (_mqttClient.connected()) {

        _mqttClient.loop();
    }

    // --------------------------------------------------------
    // RECONNECT
    // --------------------------------------------------------

    if (!_mqttClient.connected()) {

        if (_wasConnected) {

            _wasConnected = false;

            Serial0.printf(
                "[MQTT] [WARNING] Соединение с брокером потеряно\n"
            );

            notify(
                Event::DISCONNECTED,
                "mqtt",
                "broker_disconnected"
            );
        }

        const uint32_t now = millis();

        const bool firstAttempt =
            !_hasReconnectAttempted;

        const bool intervalElapsed =
            now - _lastReconnectAttempt >=
            RECONNECT_INTERVAL_MS;

        if (firstAttempt || intervalElapsed) {

            _lastReconnectAttempt = now;

            _hasReconnectAttempted = true;

            Serial0.printf(
                "[MQTT] [INFO] Попытка подключения к брокеру\n"
            );

            connect();
        }

        return;
    }

    // --------------------------------------------------------
    // PERIODIC STATE
    // --------------------------------------------------------

    const uint32_t now = millis();

    if (now - _lastStatePublish >= STATE_INTERVAL_MS) {

        _lastStatePublish = now;

        Serial0.printf(
            "[MQTT] [INFO] Публикация периодического состояния\n"
        );

        publishConnectionState(true);
    }
}


// ============================================================
// CONNECT
// ============================================================

bool SmartClockMqttClient::connect() {
    if (!_initialized) {

        Serial0.printf(
            "[MQTT] [ERROR] Клиент не инициализирован\n"
        );

        return false;
    }

    if (WiFi.status() != WL_CONNECTED) {

        Serial0.printf(
            "[MQTT] [WARNING] Нет Wi-Fi; подключение отложено\n"
        );

        return false;
    }

    // --------------------------------------------------------
    // CLIENT ID
    // --------------------------------------------------------

    char clientId[80];

    const int result = snprintf(
        clientId,
        sizeof(clientId),
        "smartclock-%s",
        _deviceId
    );

    if (result < 0 ||
        static_cast<size_t>(result) >= sizeof(clientId)) {

        Serial0.printf(
            "[MQTT] [ERROR] MQTT Client ID слишком длинный\n"
        );

        return false;
    }

    Serial0.printf(
        "[MQTT] [INFO] Подключение: client_id=%s\n",
        clientId
    );

    // --------------------------------------------------------
    // CONNECT WITH LWT
    // --------------------------------------------------------

    bool connected = false;

    if (_username[0] != '\0') {

        connected = _mqttClient.connect(
            clientId,
            _username,
            _password,
            _stateTopic,
            1,
            true,
            "{\"connected\":false}"
        );

    } else {

        connected = _mqttClient.connect(
            clientId,
            _stateTopic,
            1,
            true,
            "{\"connected\":false}"
        );
    }

    // --------------------------------------------------------
    // CONNECTION RESULT
    // --------------------------------------------------------

    if (!connected) {

        Serial0.printf(
            "[MQTT] [ERROR] Ошибка подключения, код PubSubClient=%d\n",
            _mqttClient.state()
        );

        notify(
            Event::ERROR,
            "mqtt",
            "connection_failed"
        );

        return false;
    }

    _wasConnected = true;

    Serial0.printf(
        "[MQTT] [INFO] Подключение к MQTT успешно\n"
    );

    // --------------------------------------------------------
    // SUBSCRIBE
    // --------------------------------------------------------

    if (_mqttClient.subscribe(
            _commandTopic,
            1
        )) {

        Serial0.printf(
            "[MQTT] [INFO] Подписка: %s\n",
            _commandTopic
        );

    } else {

        Serial0.printf(
            "[MQTT] [ERROR] Не удалось подписаться: %s\n",
            _commandTopic
        );

        notify(
            Event::ERROR,
            _commandTopic,
            "subscription_failed"
        );
    }

    // --------------------------------------------------------
    // PUBLISH STATE
    // --------------------------------------------------------

    publishConnectionState(true);

    _lastStatePublish = millis();

    notify(
        Event::CONNECTED,
        "mqtt",
        "connected"
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

        Serial0.printf(
            "[MQTT] [ERROR] Получено некорректное сообщение\n"
        );

        notify(
            Event::ERROR,
            "mqtt",
            "invalid_message"
        );

        return;
    }

    // --------------------------------------------------------
    // VALIDATE PAYLOAD SIZE
    // --------------------------------------------------------

    if (length >= MAX_PAYLOAD_SIZE) {

        Serial0.printf(
            "[MQTT] [WARNING] Сообщение слишком большое: %u байт\n",
            length
        );

        notify(
            Event::ERROR,
            topic,
            "payload_too_large"
        );

        return;
    }

    // --------------------------------------------------------
    // COPY PAYLOAD
    // --------------------------------------------------------

    char message[MAX_PAYLOAD_SIZE];

    if (length > 0) {

        memcpy(
            message,
            payload,
            length
        );
    }

    message[length] = '\0';

    Serial0.printf(
        "[MQTT] [INFO] Получено сообщение: topic=%s, size=%u\n",
        topic,
        length
    );

    // --------------------------------------------------------
    // VALIDATE TOPIC
    // --------------------------------------------------------

    if (strcmp(topic, _commandTopic) != 0) {

        Serial0.printf(
            "[MQTT] [WARNING] Неожиданный топик: %s\n",
            topic
        );

        notify(
            Event::ERROR,
            topic,
            "unexpected_topic"
        );

        return;
    }

    // --------------------------------------------------------
    // COMMAND RECEIVED
    // --------------------------------------------------------

    Serial0.printf(
        "[MQTT] [INFO] Получена команда: %s\n",
        message
    );

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

        Serial0.printf(
            "[MQTT] [ERROR] Публикация отменена: пустой указатель\n"
        );

        return false;
    }

    if (!_mqttClient.connected()) {

        Serial0.printf(
            "[MQTT] [WARNING] MQTT отключён; публикация отменена: %s\n",
            topic
        );

        return false;
    }

    const size_t length = strlen(payload);

    Serial0.printf(
        "[MQTT] [INFO] Публикация: topic=%s, size=%u\n",
        topic,
        static_cast<unsigned int>(length)
    );

    const bool success = _mqttClient.publish(
        topic,
        payload,
        retained
    );

    if (success) {

        Serial0.printf(
            "[MQTT] [INFO] Публикация успешна: %s\n",
            topic
        );

    } else {

        Serial0.printf(
            "[MQTT] [ERROR] Ошибка публикации: %s\n",
            topic
        );
    }

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

        Serial0.printf(
            "[MQTT] [ERROR] Не задан topic для JSON\n"
        );

        return false;
    }

    if (!_mqttClient.connected()) {

        Serial0.printf(
            "[MQTT] [WARNING] MQTT отключён; JSON не отправлен\n"
        );

        return false;
    }

    // --------------------------------------------------------
    // MEASURE JSON
    // --------------------------------------------------------

    const size_t requiredSize = measureJson(document);

    char buffer[MAX_PAYLOAD_SIZE];

    if (requiredSize >= sizeof(buffer)) {

        Serial0.printf(
            "[MQTT] [ERROR] JSON слишком большой: %u байт\n",
            static_cast<unsigned int>(requiredSize)
        );

        return false;
    }

    // --------------------------------------------------------
    // SERIALIZE JSON
    // --------------------------------------------------------

    const size_t written = serializeJson(
        document,
        buffer,
        sizeof(buffer)
    );

    if (written != requiredSize) {

        Serial0.printf(
            "[MQTT] [ERROR] Ошибка сериализации JSON\n"
        );

        return false;
    }

    // --------------------------------------------------------
    // PUBLISH JSON
    // --------------------------------------------------------

    Serial0.printf(
        "[MQTT] [INFO] Отправка JSON: topic=%s, size=%u\n",
        topic,
        static_cast<unsigned int>(written)
    );

    const bool success = _mqttClient.publish(
        topic,
        reinterpret_cast<const uint8_t*>(buffer),
        static_cast<unsigned int>(written),
        retained
    );

    if (success) {

        Serial0.printf(
            "[MQTT] [INFO] JSON отправлен: %s\n",
            topic
        );

    } else {

        Serial0.printf(
            "[MQTT] [ERROR] Не удалось отправить JSON: %s\n",
            topic
        );
    }

    return success;
}


// ============================================================
// PUBLISH TELEMETRY
// ============================================================

bool SmartClockMqttClient::publishTelemetry(
    JsonDocument& document
) {
    Serial0.printf(
        "[MQTT] [INFO] Отправка телеметрии\n"
    );

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
    Serial0.printf(
        "[MQTT] [INFO] Отправка состояния устройства\n"
    );

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

        Serial0.printf(
            "[MQTT] [ERROR] Не задано имя события\n"
        );

        return false;
    }

    JsonDocument document;

    document["device_id"] = _deviceId;

    document["event"] = eventName;

    document["uptime_ms"] = millis();

    Serial0.printf(
        "[MQTT] [INFO] Событие: %s\n",
        eventName
    );

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

        Serial0.printf(
            "[MQTT] [WARNING] ACK не отправлен: MQTT отключён\n"
        );

        return false;
    }

    if (document["device_id"].isNull()) {

        document["device_id"] = _deviceId;
    }

    const char* requestId =
        document["request_id"] | "";

    const char* command =
        document["command"] | "";

    const char* status =
        document["status"] | "";

    Serial0.printf(
        "[MQTT] [INFO] ACK: request_id=%s, command=%s, status=%s\n",
        requestId,
        command,
        status
    );

    const bool success = publishJson(
        _ackTopic,
        document,
        false
    );

    if (success) {

        Serial0.printf(
            "[MQTT] [INFO] ACK отправлен: %s\n",
            requestId
        );

    } else {

        Serial0.printf(
            "[MQTT] [ERROR] Ошибка отправки ACK: %s\n",
            requestId
        );
    }

    return success;
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

        Serial0.printf(
            "[MQTT] [ERROR] ACK: не задан request_id или command\n"
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

    const String ipAddress =
        WiFi.localIP().toString();

    document["ip"] = ipAddress;

    document["rssi"] =
        WiFi.status() == WL_CONNECTED
            ? WiFi.RSSI()
            : 0;

    document["uptime_ms"] = millis();

    Serial0.printf(
        "[MQTT] [INFO] Состояние: MQTT=%s, Wi-Fi=%s\n",
        connected ? "connected" : "disconnected",
        WiFi.status() == WL_CONNECTED
            ? "connected"
            : "disconnected"
    );

    publishState(
        document,
        true
    );
}


// ============================================================
// EVENT CALLBACK
// ============================================================

void SmartClockMqttClient::setEventCallback(
    EventCallback callback
) {
    _eventCallback = callback;

    Serial0.printf(
        "[MQTT] [INFO] Callback событий %s\n",
        callback != nullptr
            ? "установлен"
            : "отключён"
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