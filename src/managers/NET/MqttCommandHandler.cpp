#include "MqttCommandHandler.h"

#include <cstring>
#include <cstdlib>


// ============================================================
// STATIC INSTANCE
// ============================================================

MqttCommandHandler*
    MqttCommandHandler::_instance = nullptr;


// ============================================================
// CONSTRUCTOR
// ============================================================

MqttCommandHandler::MqttCommandHandler()
    : _mqttClient(nullptr) {

    Serial0.printf(
        "[COMMAND] Обработчик команд создан\n"
    );
}


// ============================================================
// BEGIN
// ============================================================

bool MqttCommandHandler::begin(
    SmartClockMqttClient& mqttClient
) {
    if (_instance != nullptr &&
        _instance != this) {

        Serial0.printf(
            "[COMMAND] [ERROR] Другой обработчик уже зарегистрирован\n"
        );

        return false;
    }

    _mqttClient = &mqttClient;

    _instance = this;

    _mqttClient->setEventCallback(
        MqttCommandHandler::onMqttEvent
    );

    Serial0.printf(
        "[COMMAND] Обработчик команд инициализирован\n"
    );

    return true;
}


// ============================================================
// MQTT EVENT CALLBACK
// ============================================================

void MqttCommandHandler::onMqttEvent(
    SmartClockMqttClient::Event event,
    const char* topic,
    const char* payload
) {
    if (_instance == nullptr) {
        return;
    }

    switch (event) {

        case SmartClockMqttClient::Event::CONNECTED:

            Serial0.printf(
                "[COMMAND] MQTT подключён\n"
            );

            break;


        case SmartClockMqttClient::Event::DISCONNECTED:

            Serial0.printf(
                "[COMMAND] MQTT отключён\n"
            );

            break;


        case SmartClockMqttClient::Event::COMMAND_RECEIVED:

            Serial0.printf(
                "[COMMAND] Получена команда из MQTT\n"
            );

            _instance->handleCommand(payload);

            break;


        case SmartClockMqttClient::Event::ERROR:

            Serial0.printf(
                "[COMMAND] [ERROR] Событие ошибки MQTT: %s\n",
                topic != nullptr ? topic : ""
            );

            break;
    }
}


// ============================================================
// HANDLE COMMAND
// ============================================================

void MqttCommandHandler::handleCommand(
    const char* payload
) {
    if (payload == nullptr ||
        payload[0] == '\0') {

        Serial0.printf(
            "[COMMAND] [ERROR] Пустая команда\n"
        );

        return;
    }

    // --------------------------------------------------------
    // PARSE JSON
    // --------------------------------------------------------

    JsonDocument document;

    const DeserializationError error =
        deserializeJson(
            document,
            payload
        );

    if (error) {

        Serial0.printf(
            "[COMMAND] [ERROR] Ошибка JSON: %s\n",
            error.c_str()
        );

        return;
    }

    // --------------------------------------------------------
    // READ REQUEST ID
    // --------------------------------------------------------

    const char* requestId =
        document["request_id"] | "";

    // --------------------------------------------------------
    // READ COMMAND
    // --------------------------------------------------------

    const char* command =
        document["command"] | "";

    if (command[0] == '\0') {

        Serial0.printf(
            "[COMMAND] [ERROR] Отсутствует поле command\n"
        );

        sendAck(
            requestId,
            "unknown",
            false,
            "Missing command field"
        );

        return;
    }

    Serial0.printf(
        "[COMMAND] Выполнение: %s\n",
        command
    );

    // --------------------------------------------------------
    // DISPATCH COMMAND
    // --------------------------------------------------------

    if (strcmp(command, "ping") == 0) {

        executePing(requestId);

    } else if (strcmp(command, "get_status") == 0) {

        executeGetStatus(requestId);

    } else if (strcmp(command, "restart") == 0) {

        executeRestart(requestId);

    } else {

        executeUnknownCommand(
            requestId,
            command
        );
    }
}


// ============================================================
// PING
// ============================================================

void MqttCommandHandler::executePing(
    const char* requestId
) {
    Serial0.printf(
        "[COMMAND] ping: проверка связи\n"
    );

    sendAck(
        requestId,
        "ping",
        true,
        "pong"
    );
}


// ============================================================
// GET STATUS
// ============================================================

void MqttCommandHandler::executeGetStatus(
    const char* requestId
) {
    Serial0.printf(
        "[COMMAND] Запрос состояния устройства\n"
    );

    if (_mqttClient == nullptr) {

        Serial0.printf(
            "[COMMAND] [ERROR] MQTT-клиент недоступен\n"
        );

        return;
    }

    JsonDocument response;

    response["device_id"] =
        _mqttClient->getDeviceId();

    response["request_id"] =
        requestId;

    response["command"] =
        "get_status";

    response["status"] =
        "success";

    response["message"] =
        "Device status retrieved";

    JsonObject result =
        response["result"].to<JsonObject>();

    result["mqtt_connected"] =
        _mqttClient->isConnected();

    result["wifi_connected"] =
        WiFi.status() == WL_CONNECTED;

    result["uptime_ms"] =
        millis();

    if (WiFi.status() == WL_CONNECTED) {

        result["ip"] =
            WiFi.localIP().toString();

        result["rssi"] =
            WiFi.RSSI();

    } else {

        result["ip"] = "";

        result["rssi"] = 0;
    }

    Serial0.printf(
        "[COMMAND] Отправка состояния устройства\n"
    );

    if (!_mqttClient->publishAck(response)) {

        Serial0.printf(
            "[COMMAND] [ERROR] Не удалось отправить состояние\n"
        );
    }
}


// ============================================================
// RESTART
// ============================================================

void MqttCommandHandler::executeRestart(
    const char* requestId
) {
    Serial0.printf(
        "[COMMAND] Запрошена перезагрузка устройства\n"
    );

    const bool ackSent = sendAck(
        requestId,
        "restart",
        true,
        "Device is restarting"
    );

    if (!ackSent) {

        Serial0.printf(
            "[COMMAND] [ERROR] ACK не отправлен; перезагрузка отменена\n"
        );

        return;
    }

    Serial0.printf(
        "[COMMAND] Перезагрузка через 500 мс\n"
    );

    delay(500);

    ESP.restart();
}


// ============================================================
// SEND ACK
// ============================================================

bool MqttCommandHandler::sendAck(
    const char* requestId,
    const char* command,
    bool success,
    const char* message
) {
    if (_mqttClient == nullptr) {

        Serial0.printf(
            "[COMMAND] [ERROR] Невозможно отправить ACK: MQTT-клиент отсутствует\n"
        );

        return false;
    }

    if (requestId == nullptr ||
        requestId[0] == '\0') {

        Serial0.printf(
            "[COMMAND] [WARNING] У команды отсутствует request_id\n"
        );

        return false;
    }

    Serial0.printf(
        "[COMMAND] ACK: request_id=%s, command=%s, status=%s\n",
        requestId,
        command,
        success ? "success" : "error"
    );

    return _mqttClient->publishAck(
        requestId,
        command,
        success,
        message
    );
}


// ============================================================
// UNKNOWN COMMAND
// ============================================================

void MqttCommandHandler::executeUnknownCommand(
    const char* requestId,
    const char* command
) {
    Serial0.printf(
        "[COMMAND] [WARNING] Неизвестная команда: %s\n",
        command
    );

    sendAck(
        requestId,
        command,
        false,
        "Unknown command"
    );
}