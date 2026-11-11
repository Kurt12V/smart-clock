#include "MqttCommandHandler.h"

#include <Arduino.h>
#include <ESP.h>
#include <WiFi.h>

#include <cstdio>
#include <cstring>
#include <ctime>

MqttCommandHandler* MqttCommandHandler::_instance = nullptr;

namespace {

constexpr uint32_t DEFAULT_RESTART_DELAY_MS = 500;
constexpr uint32_t MAX_RESTART_DELAY_MS = 10000;

bool readInt(
    JsonVariantConst params,
    const char* key,
    int& value
) {
    if (params.isNull() || !params[key].is<int>()) {
        return false;
    }

    value = params[key].as<int>();
    return true;
}

bool readBool(
    JsonVariantConst params,
    const char* key,
    bool& value
) {
    if (params.isNull() || !params[key].is<bool>()) {
        return false;
    }

    value = params[key].as<bool>();
    return true;
}

} // namespace


// ============================================================
// CONSTRUCTOR
// ============================================================

MqttCommandHandler::MqttCommandHandler()
    : _mqtt(nullptr),
      _settings(nullptr),
      _clock(nullptr),
      _wifi(nullptr),
      _initialized(false) {
}


// ============================================================
// INITIALIZATION
// ============================================================

bool MqttCommandHandler::begin(
    SmartClockMqttClient& mqtt,
    SettingsManager& settings,
    ClockSystem& clock,
    WiFiManager& wifi
) {
    if (_initialized) {
        return true;
    }

    if (_instance != nullptr && _instance != this) {
        return false;
    }

    _mqtt = &mqtt;
    _settings = &settings;
    _clock = &clock;
    _wifi = &wifi;

    _instance = this;

    _mqtt->setEventCallback(
        &MqttCommandHandler::onMqttEvent
    );

    _initialized = true;

    return true;
}


// ============================================================
// MQTT EVENT
// ============================================================

void MqttCommandHandler::onMqttEvent(
    SmartClockMqttClient::Event event,
    const char* topic,
    const char* payload
) {
    (void)topic;

    if (_instance == nullptr ||
        !_instance->_initialized) {
        return;
    }

    if (event ==
        SmartClockMqttClient::Event::COMMAND_RECEIVED) {

        _instance->handleCommand(payload);
    }
}


// ============================================================
// COMMAND DISPATCHER
// ============================================================

void MqttCommandHandler::handleCommand(
    const char* payload
) {
    if (payload == nullptr || _mqtt == nullptr) {
        return;
    }

    JsonDocument document;

    DeserializationError error =
        deserializeJson(document, payload);

    if (error) {
        sendAck(
            "",
            "",
            false,
            "Invalid JSON payload"
        );

        return;
    }

    const char* requestId =
        document["request_id"] | "";

    const char* command =
        document["command"] | "";

    JsonVariantConst params =
        document["params"];

    if (requestId[0] == '\0' ||
        command[0] == '\0') {

        sendAck(
            requestId,
            command,
            false,
            "Required fields: request_id, command"
        );

        return;
    }

    if (strncmp(command, "system.", 7) == 0) {
        systemCommand(
            requestId,
            command,
            params
        );
    }
    else if (strncmp(command, "clock.", 6) == 0) {
        clockCommand(
            requestId,
            command,
            params
        );
    }
    else if (strncmp(command, "network.", 8) == 0) {
        networkCommand(
            requestId,
            command,
            params
        );
    }
    else if (strncmp(command, "display.", 8) == 0) {
        displayCommand(
            requestId,
            command,
            params
        );
    }
    else {
        unknownCommand(
            requestId,
            command
        );
    }
}


// ============================================================
// SYSTEM COMMANDS
// ============================================================

void MqttCommandHandler::systemCommand(
    const char* requestId,
    const char* command,
    JsonVariantConst params
) {
    // --------------------------------------------------------
    // system.ping
    // --------------------------------------------------------

    if (strcmp(command, "system.ping") == 0) {
        JsonDocument data;

        data["pong"] = true;
        data["device_id"] = _mqtt->getDeviceId();
        data["uptime_ms"] = millis();

        sendResult(
            requestId,
            command,
            true,
            "pong",
            data
        );

        return;
    }

    // --------------------------------------------------------
    // system.get_info
    // --------------------------------------------------------

    if (strcmp(command, "system.get_info") == 0) {
        JsonDocument data;

        data["device_id"] = _mqtt->getDeviceId();
        data["chip_model"] = ESP.getChipModel();
        data["chip_revision"] = ESP.getChipRevision();
        data["cpu_mhz"] = ESP.getCpuFreqMHz();
        data["flash_size"] = ESP.getFlashChipSize();
        data["free_heap"] = ESP.getFreeHeap();
        data["min_free_heap"] = ESP.getMinFreeHeap();
        data["sdk_version"] = ESP.getSdkVersion();
        data["uptime_ms"] = millis();

        sendResult(
            requestId,
            command,
            true,
            "Device information",
            data
        );

        return;
    }

    // --------------------------------------------------------
    // system.get_status
    // --------------------------------------------------------

    if (strcmp(command, "system.get_status") == 0) {
        JsonDocument data;

        data["wifi_connected"] =
            _wifi->isConnected();

        data["mqtt_connected"] =
            _mqtt->isConnected();

        data["ip"] = _wifi->getIP();
        data["rssi"] = _wifi->getRSSI();

        data["uptime_ms"] = millis();

        data["time_valid"] =
            _clock->isTimeValid();

        JsonObject settings =
            data["settings"].to<JsonObject>();

        settings["brightness"] =
            _settings->get(Param::BRIGHTNESS);

        settings["brightness_mode"] =
            _settings->get(Param::BRIGHTNESS_MODE);

        settings["matrix_enabled"] =
            _settings->get(Param::MATRIX_ENABLED) != 0;

        settings["matrix_brightness"] =
            _settings->get(Param::MATRIX_BRIGHTNESS);

        settings["cob_enabled"] =
            _settings->get(Param::COB_ENABLED) != 0;

        settings["cob_brightness"] =
            _settings->get(Param::COB_BRIGHTNESS);

        settings["volume_media"] =
            _settings->get(Param::VOLUME_MEDIA);

        settings["volume_alarm"] =
            _settings->get(Param::VOLUME_ALARM);

        settings["volume_system"] =
            _settings->get(Param::VOLUME_SYSTEM);

        settings["microphone_enabled"] =
            _settings->get(Param::MICROPHONE_ENABLED) != 0;

        settings["utc_offset"] =
            _settings->get(Param::UTC_OFFSET);

        sendResult(
            requestId,
            command,
            true,
            "Device status",
            data
        );

        return;
    }

    // --------------------------------------------------------
    // system.sync
    // --------------------------------------------------------

    if (strcmp(command, "system.sync") == 0) {
        publishCurrentState();

        sendAck(
            requestId,
            command,
            true,
            "Current state published"
        );

        return;
    }

    // --------------------------------------------------------
    // system.restart
    // --------------------------------------------------------

    if (strcmp(command, "system.restart") == 0) {
        uint32_t delayMs =
            DEFAULT_RESTART_DELAY_MS;

        if (!params.isNull() &&
            params["delay_ms"].is<uint32_t>()) {

            delayMs =
                params["delay_ms"].as<uint32_t>();
        }

        if (delayMs > MAX_RESTART_DELAY_MS) {
            sendAck(
                requestId,
                command,
                false,
                "delay_ms must be between 0 and 10000"
            );

            return;
        }

        if (sendAck(
                requestId,
                command,
                true,
                "Restart scheduled")) {

            delay(delayMs);
            ESP.restart();
        }

        return;
    }

    unknownCommand(requestId, command);
}


// ============================================================
// CLOCK COMMANDS
// ============================================================

void MqttCommandHandler::clockCommand(
    const char* requestId,
    const char* command,
    JsonVariantConst params
) {
    // --------------------------------------------------------
    // clock.get_time
    // --------------------------------------------------------

    if (strcmp(command, "clock.get_time") == 0) {
        JsonDocument data;

        char local[24] = {};
        char utc[24] = {};

        formatTime(
            _clock->getLocalTime(),
            local,
            sizeof(local)
        );

        formatTime(
            _clock->getUTCTime(),
            utc,
            sizeof(utc)
        );

        data["valid"] = _clock->isTimeValid();
        data["datetime"] = local;
        data["utc_datetime"] = utc;

        data["timezone_offset"] =
            _settings->get(Param::UTC_OFFSET);

        data["hour"] = _clock->hour();
        data["minute"] = _clock->minute();
        data["second"] = _clock->second();

        data["day"] = _clock->day();
        data["month"] = _clock->month();
        data["year"] = _clock->year();

        sendResult(
            requestId,
            command,
            true,
            "Current clock time",
            data
        );

        return;
    }

    // --------------------------------------------------------
    // clock.sync_ntp
    // --------------------------------------------------------

    if (strcmp(command, "clock.sync_ntp") == 0) {
        const char* server =
            params["server"] | "pool.ntp.org";

        bool success =
            _clock->syncFromNTP(server, 10);

        sendAck(
            requestId,
            command,
            success,
            success
                ? "NTP synchronization successful"
                : "NTP synchronization failed"
        );

        return;
    }

    // --------------------------------------------------------
    // clock.set_utc / clock.set_datetime
    // --------------------------------------------------------

    if (strcmp(command, "clock.set_utc") == 0 ||
        strcmp(command, "clock.set_datetime") == 0) {

        const char* text =
            params["datetime"] | "";

        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;

        if (!parseDateTime(
                text,
                year,
                month,
                day,
                hour,
                minute,
                second)) {

            sendAck(
                requestId,
                command,
                false,
                "datetime must use YYYY-MM-DDTHH:MM:SS"
            );

            return;
        }

        bool success = false;

        if (strcmp(command, "clock.set_utc") == 0) {
            success = _clock->setUTCDateTime(
                year,
                month,
                day,
                hour,
                minute,
                second
            );
        }
        else {
            if (params["timezone"].is<const char*>()) {
                int offsetHours = 0;

                if (!parseTimezone(
                        params["timezone"].as<const char*>(),
                        offsetHours)) {

                    sendAck(
                        requestId,
                        command,
                        false,
                        "Invalid timezone format"
                    );

                    return;
                }

                _settings->set(
                    Param::UTC_OFFSET,
                    offsetHours
                );

                _settings->save(
                    Param::UTC_OFFSET
                );
            }

            success = _clock->setLocalDateTime(
                year,
                month,
                day,
                hour,
                minute,
                second
            );
        }

        sendAck(
            requestId,
            command,
            success,
            success
                ? "Clock updated"
                : "Clock update failed"
        );

        return;
    }

    // --------------------------------------------------------
    // clock.set_timezone
    // --------------------------------------------------------

    if (strcmp(command, "clock.set_timezone") == 0) {
        const char* timezone =
            params["timezone"] | "";

        int offsetHours = 0;

        if (!parseTimezone(timezone, offsetHours)) {
            sendAck(
                requestId,
                command,
                false,
                "Use UTC, UTC+04:00 or UTC-05:00 format"
            );

            return;
        }

        _settings->set(
            Param::UTC_OFFSET,
            offsetHours
        );

        bool success =
            _settings->save(Param::UTC_OFFSET);

        JsonDocument data;

        data["timezone"] = timezone;
        data["offset_hours"] = offsetHours;

        sendResult(
            requestId,
            command,
            success,
            success ? "Timezone saved" : "Could not save timezone",
            data
        );

        return;
    }

    // --------------------------------------------------------
    // clock.get_timezone
    // --------------------------------------------------------

    if (strcmp(command, "clock.get_timezone") == 0) {
        int offset =
            _settings->get(Param::UTC_OFFSET);

        char timezone[16];

        snprintf(
            timezone,
            sizeof(timezone),
            "UTC%c%02d:00",
            offset < 0 ? '-' : '+',
            abs(offset)
        );

        JsonDocument data;

        data["timezone"] = timezone;
        data["offset_hours"] = offset;

        sendResult(
            requestId,
            command,
            true,
            "Current timezone",
            data
        );

        return;
    }

    unknownCommand(requestId, command);
}


// ============================================================
// NETWORK COMMANDS
// ============================================================

void MqttCommandHandler::networkCommand(
    const char* requestId,
    const char* command,
    JsonVariantConst params
) {
    // --------------------------------------------------------
    // network.get_status
    // --------------------------------------------------------

    if (strcmp(command, "network.get_status") == 0) {
        JsonDocument data;

        data["wifi_connected"] =
            _wifi->isConnected();

        data["wifi_mode"] =
            static_cast<int>(_wifi->getMode());

        data["ssid"] = _wifi->getSSID();
        data["ip"] = _wifi->getIP();
        data["rssi"] = _wifi->getRSSI();

        data["mqtt_connected"] =
            _mqtt->isConnected();

        sendResult(
            requestId,
            command,
            true,
            "Network status",
            data
        );

        return;
    }

    // --------------------------------------------------------
    // network.get_ip
    // --------------------------------------------------------

    if (strcmp(command, "network.get_ip") == 0) {
        JsonDocument data;

        data["ip"] = _wifi->getIP();
        data["connected"] = _wifi->isConnected();

        sendResult(
            requestId,
            command,
            true,
            "Network IP",
            data
        );

        return;
    }

    // --------------------------------------------------------
    // network.scan_wifi
    // --------------------------------------------------------

    if (strcmp(command, "network.scan_wifi") == 0) {
        int count = _wifi->scanNetworks();

        JsonDocument data;

        data["count"] = count;

        JsonArray networks =
            data["networks"].to<JsonArray>();

        for (int i = 0; i < count; ++i) {
            JsonObject network =
                networks.add<JsonObject>();

            network["ssid"] =
                _wifi->getScanSSID(i);

            network["rssi"] =
                _wifi->getScanRSSI(i);

            network["encrypted"] =
                _wifi->getScanEncrypted(i);
        }

        sendResult(
            requestId,
            command,
            count >= 0,
            count >= 0
                ? "Wi-Fi scan complete"
                : "Wi-Fi scan failed",
            data
        );

        return;
    }

    // --------------------------------------------------------
    // network.connect_wifi
    // --------------------------------------------------------

    if (strcmp(command, "network.connect_wifi") == 0) {
        const char* ssid =
            params["ssid"] | "";

        const char* password =
            params["password"] | "";

        if (ssid[0] == '\0') {
            sendAck(
                requestId,
                command,
                false,
                "ssid is required"
            );

            return;
        }

        // Пароль не включается в ACK или логи.
        if (!_wifi->saveCredentials(
                String(ssid),
                String(password))) {

            sendAck(
                requestId,
                command,
                false,
                "Could not save Wi-Fi credentials"
            );

            return;
        }

        bool success = _wifi->connect(
            String(ssid),
            String(password)
        );

        sendAck(
            requestId,
            command,
            success,
            success
                ? "Wi-Fi connection requested"
                : "Could not start Wi-Fi connection"
        );

        return;
    }

    // --------------------------------------------------------
    // network.disconnect_wifi
    // --------------------------------------------------------

    if (strcmp(command, "network.disconnect_wifi") == 0) {
        _wifi->disconnect();

        sendAck(
            requestId,
            command,
            true,
            "Wi-Fi disconnect requested"
        );

        return;
    }

    // --------------------------------------------------------
    // network.mqtt_reconnect
    // --------------------------------------------------------

    if (strcmp(command, "network.mqtt_reconnect") == 0) {
        if (_mqtt->isConnected()) {
            sendAck(
                requestId,
                command,
                true,
                "MQTT is already connected"
            );
        }
        else {
            sendAck(
                requestId,
                command,
                false,
                "MQTT client retries automatically; force reconnect API is unavailable"
            );
        }

        return;
    }

    unknownCommand(requestId, command);
}


// ============================================================
// DISPLAY AND LIGHTING COMMANDS
// ============================================================


void MqttCommandHandler::displayCommand(
    const char* requestId,
    const char* command,
    JsonVariantConst params
) {
    // --------------------------------------------------------
    // display.get_status
    // --------------------------------------------------------

    if (strcmp(command, "display.get_status") == 0) {
        JsonDocument data;

        data["brightness"] =
            _settings->get(Param::BRIGHTNESS);

        data["brightness_mode"] =
            _settings->get(Param::BRIGHTNESS_MODE);

        data["matrix_enabled"] =
            _settings->get(Param::MATRIX_ENABLED) != 0;

        data["matrix_brightness"] =
            _settings->get(Param::MATRIX_BRIGHTNESS);

        data["matrix_effect"] =
            _settings->get(Param::MATRIX_EFFECT);

        data["matrix_speed"] =
            _settings->get(Param::MATRIX_SPEED);

        data["cob_enabled"] =
            _settings->get(Param::COB_ENABLED) != 0;

        data["cob_brightness"] =
            _settings->get(Param::COB_BRIGHTNESS);

        data["cob_effect"] =
            _settings->get(Param::COB_EFFECT);

        data["cob_speed"] =
            _settings->get(Param::COB_SPEED);

        sendResult(
            requestId,
            command,
            true,
            "Display and lighting settings",
            data
        );

        return;
    }

    // --------------------------------------------------------
    // display.get_matrix_status
    // --------------------------------------------------------

    if (strcmp(command, "display.get_matrix_status") == 0) {
        JsonDocument data;

        data["enabled"] =
            _settings->get(Param::MATRIX_ENABLED) != 0;

        data["brightness"] =
            _settings->get(Param::MATRIX_BRIGHTNESS);

        data["effect"] =
            _settings->get(Param::MATRIX_EFFECT);

        data["speed"] =
            _settings->get(Param::MATRIX_SPEED);

        sendResult(
            requestId,
            command,
            true,
            "Matrix settings",
            data
        );

        return;
    }

    // --------------------------------------------------------
    // display.get_cob_status
    // --------------------------------------------------------

    if (strcmp(command, "display.get_cob_status") == 0) {
        JsonDocument data;

        data["enabled"] =
            _settings->get(Param::COB_ENABLED) != 0;

        data["brightness"] =
            _settings->get(Param::COB_BRIGHTNESS);

        data["effect"] =
            _settings->get(Param::COB_EFFECT);

        data["speed"] =
            _settings->get(Param::COB_SPEED);

        sendResult(
            requestId,
            command,
            true,
            "COB settings",
            data
        );

        return;
    }

    // --------------------------------------------------------
    // display.get_brightness
    // --------------------------------------------------------

    if (strcmp(command, "display.get_brightness") == 0) {
        JsonDocument data;

        data["value"] =
            _settings->get(Param::BRIGHTNESS);

        data["mode"] =
            _settings->get(Param::BRIGHTNESS_MODE);

        sendResult(
            requestId,
            command,
            true,
            "Brightness settings",
            data
        );

        return;
    }

    // --------------------------------------------------------
    // display.set_brightness
    // --------------------------------------------------------

    if (strcmp(command, "display.set_brightness") == 0) {
        int value;

        const Param parameter = Param::BRIGHTNESS;
        const Param modeParameter = Param::BRIGHTNESS_MODE;

        const auto& desc = _settings->getDesc(parameter);

        if (!readInt(params, "value", value) ||
            value < desc.minValue ||
            value > desc.maxValue) {

            sendAck(
                requestId,
                command,
                false,
                "Brightness value is outside allowed range"
            );

            return;
        }

        _settings->set(parameter, value);
        _settings->set(modeParameter, 0);

        bool success =
            _settings->save(parameter) &&
            _settings->save(modeParameter);

        sendAck(
            requestId,
            command,
            success,
            success
                ? "Manual brightness updated"
                : "Could not save brightness"
        );

        return;
    }

    // --------------------------------------------------------
    // display.set_enabled
    // --------------------------------------------------------

    if (strcmp(command, "display.set_enabled") == 0) {
        bool enabled;

        if (!readBool(params, "enabled", enabled)) {
            sendAck(
                requestId,
                command,
                false,
                "enabled must be boolean"
            );

            return;
        }

        int value = enabled
            ? _settings->getDesc(Param::BRIGHTNESS).defaultValue
            : 0;

        _settings->set(Param::BRIGHTNESS, value);
        _settings->set(Param::BRIGHTNESS_MODE, 0);

        bool success =
            _settings->save(Param::BRIGHTNESS) &&
            _settings->save(Param::BRIGHTNESS_MODE);

        sendAck(
            requestId,
            command,
            success,
            success
                ? (enabled
                    ? "Brightness restored to default"
                    : "Brightness set to zero")
                : "Could not save display settings"
        );

        return;
    }

    // --------------------------------------------------------
    // display.set_mode
    // --------------------------------------------------------

    if (strcmp(command, "display.set_mode") == 0) {
        int value;

        if (!readInt(params, "value", value) ||
            value < 0 || value > 1) {

            sendAck(
                requestId,
                command,
                false,
                "value must be 0 (manual) or 1 (automatic)"
            );

            return;
        }

        _settings->set(
            Param::BRIGHTNESS_MODE,
            value
        );

        bool success =
            _settings->save(Param::BRIGHTNESS_MODE);

        sendAck(
            requestId,
            command,
            success,
            success
                ? "Brightness mode updated"
                : "Could not save brightness mode"
        );

        return;
    }

    // --------------------------------------------------------
    // MATRIX / COB ENABLED
    // --------------------------------------------------------

    if (strcmp(command, "display.set_matrix_enabled") == 0 ||
        strcmp(command, "display.set_cob_enabled") == 0) {

        bool enabled;

        if (!readBool(params, "enabled", enabled)) {
            sendAck(
                requestId,
                command,
                false,
                "enabled must be boolean"
            );

            return;
        }

        bool matrix =
            strcmp(command, "display.set_matrix_enabled") == 0;

        Param parameter = matrix
            ? Param::MATRIX_ENABLED
            : Param::COB_ENABLED;

        _settings->set(
            parameter,
            enabled ? 1 : 0
        );

        bool success = _settings->save(parameter);

        sendAck(
            requestId,
            command,
            success,
            success
                ? (matrix
                    ? "Matrix enabled state updated"
                    : "COB enabled state updated")
                : "Could not save enabled state"
        );

        return;
    }

    // --------------------------------------------------------
    // MATRIX / COB BRIGHTNESS
    // --------------------------------------------------------

    if (strcmp(command, "display.set_matrix_brightness") == 0 ||
        strcmp(command, "display.set_cob_brightness") == 0) {

        int value;

        if (!readInt(params, "value", value)) {
            sendAck(
                requestId,
                command,
                false,
                "value must be an integer"
            );

            return;
        }

        bool matrix =
            strcmp(command, "display.set_matrix_brightness") == 0;

        Param parameter = matrix
            ? Param::MATRIX_BRIGHTNESS
            : Param::COB_BRIGHTNESS;

        const auto& desc = _settings->getDesc(parameter);

        if (value < desc.minValue ||
            value > desc.maxValue) {

            sendAck(
                requestId,
                command,
                false,
                "Brightness value is outside allowed range"
            );

            return;
        }

        _settings->set(parameter, value);

        bool success = _settings->save(parameter);

        sendAck(
            requestId,
            command,
            success,
            success
                ? (matrix
                    ? "Matrix brightness updated"
                    : "COB brightness updated")
                : "Could not save brightness"
        );

        return;
    }

    // --------------------------------------------------------
    // MATRIX / COB EFFECT
    // --------------------------------------------------------

    if (strcmp(command, "display.set_matrix_effect") == 0 ||
        strcmp(command, "display.set_cob_effect") == 0) {

        int value;

        if (!readInt(params, "value", value)) {
            sendAck(
                requestId,
                command,
                false,
                "value must be an integer"
            );

            return;
        }

        bool matrix =
            strcmp(command, "display.set_matrix_effect") == 0;

        Param parameter = matrix
            ? Param::MATRIX_EFFECT
            : Param::COB_EFFECT;

        const auto& desc = _settings->getDesc(parameter);

        if (value < desc.minValue ||
            value > desc.maxValue) {

            sendAck(
                requestId,
                command,
                false,
                "Effect value is outside allowed range"
            );

            return;
        }

        _settings->set(parameter, value);

        bool success = _settings->save(parameter);

        sendAck(
            requestId,
            command,
            success,
            success
                ? (matrix
                    ? "Matrix effect updated"
                    : "COB effect updated")
                : "Could not save effect"
        );

        return;
    }

    // --------------------------------------------------------
    // MATRIX / COB SPEED
    // --------------------------------------------------------

    if (strcmp(command, "display.set_matrix_speed") == 0 ||
        strcmp(command, "display.set_cob_speed") == 0) {

        int value;

        if (!readInt(params, "value", value)) {
            sendAck(
                requestId,
                command,
                false,
                "value must be an integer"
            );

            return;
        }

        bool matrix =
            strcmp(command, "display.set_matrix_speed") == 0;

        Param parameter = matrix
            ? Param::MATRIX_SPEED
            : Param::COB_SPEED;

        const auto& desc = _settings->getDesc(parameter);

        if (value < desc.minValue ||
            value > desc.maxValue) {

            sendAck(
                requestId,
                command,
                false,
                "Speed value is outside allowed range"
            );

            return;
        }

        _settings->set(parameter, value);

        bool success = _settings->save(parameter);

        sendAck(
            requestId,
            command,
            success,
            success
                ? (matrix
                    ? "Matrix speed updated"
                    : "COB speed updated")
                : "Could not save speed"
        );

        return;
    }

    // --------------------------------------------------------
    // UNKNOWN COMMAND
    // --------------------------------------------------------

    unknownCommand(requestId, command);
}



// ============================================================
// ACK / RESULT
// ============================================================

bool MqttCommandHandler::sendAck(
    const char* requestId,
    const char* command,
    bool success,
    const char* message
) {
    if (_mqtt == nullptr) {
        return false;
    }

    return _mqtt->publishAck(
        requestId != nullptr ? requestId : "",
        command != nullptr ? command : "",
        success,
        message
    );
}


bool MqttCommandHandler::sendResult(
    const char* requestId,
    const char* command,
    bool success,
    const char* message,
    JsonDocument& data
) {
    if (_mqtt == nullptr) {
        return false;
    }

    JsonDocument response;

    response["device_id"] =
        _mqtt->getDeviceId();

    response["request_id"] =
        requestId != nullptr ? requestId : "";

    response["command"] =
        command != nullptr ? command : "";

    response["status"] =
        success ? "success" : "error";

    response["uptime_ms"] = millis();

    if (message != nullptr && message[0] != '\0') {
        response["message"] = message;
    }

    response["result"].set(
        data.as<JsonVariantConst>()
    );

    return _mqtt->publishAck(response);
}


// ============================================================
// PUBLISH CURRENT STATE
// ============================================================

void MqttCommandHandler::publishCurrentState() {
    if (_mqtt == nullptr ||
        _settings == nullptr ||
        _clock == nullptr ||
        _wifi == nullptr) {
        return;
    }

    JsonDocument state;

    state["device_id"] =
        _mqtt->getDeviceId();

    state["wifi_connected"] =
        _wifi->isConnected();

    state["mqtt_connected"] =
        _mqtt->isConnected();

    state["ip"] = _wifi->getIP();
    state["rssi"] = _wifi->getRSSI();

    state["uptime_ms"] = millis();

    state["time_valid"] =
        _clock->isTimeValid();

    char local[24] = {};
    char utc[24] = {};

    formatTime(
        _clock->getLocalTime(),
        local,
        sizeof(local)
    );

    formatTime(
        _clock->getUTCTime(),
        utc,
        sizeof(utc)
    );

    state["datetime"] = local;
    state["utc_datetime"] = utc;

    state["utc_offset"] =
        _settings->get(Param::UTC_OFFSET);

    JsonObject settings =
        state["settings"].to<JsonObject>();

    settings["brightness"] =
        _settings->get(Param::BRIGHTNESS);

    settings["brightness_mode"] =
        _settings->get(Param::BRIGHTNESS_MODE);

    settings["matrix_enabled"] =
        _settings->get(Param::MATRIX_ENABLED) != 0;

    settings["matrix_brightness"] =
        _settings->get(Param::MATRIX_BRIGHTNESS);

    settings["cob_enabled"] =
        _settings->get(Param::COB_ENABLED) != 0;

    settings["cob_brightness"] =
        _settings->get(Param::COB_BRIGHTNESS);

    settings["volume_media"] =
        _settings->get(Param::VOLUME_MEDIA);

    settings["volume_alarm"] =
        _settings->get(Param::VOLUME_ALARM);

    settings["volume_system"] =
        _settings->get(Param::VOLUME_SYSTEM);

    settings["microphone_enabled"] =
        _settings->get(Param::MICROPHONE_ENABLED) != 0;

    _mqtt->publishState(state, true);
}


// ============================================================
// UNKNOWN COMMAND
// ============================================================

void MqttCommandHandler::unknownCommand(
    const char* requestId,
    const char* command
) {
    sendAck(
        requestId,
        command,
        false,
        "Unknown command"
    );
}


// ============================================================
// DATE/TIME PARSING
// ============================================================

bool MqttCommandHandler::parseDateTime(
    const char* text,
    uint16_t& year,
    uint8_t& month,
    uint8_t& day,
    uint8_t& hour,
    uint8_t& minute,
    uint8_t& second
) {
    if (text == nullptr) {
        return false;
    }

    int y, mo, d, h, mi, s;
    char trailing;

    int fields = sscanf(
        text,
        "%4d-%2d-%2dT%2d:%2d:%2d%c",
        &y,
        &mo,
        &d,
        &h,
        &mi,
        &s,
        &trailing
    );

    if (fields != 6 ||
        y < 2000 || y > 2099 ||
        mo < 1 || mo > 12 ||
        d < 1 || d > 31 ||
        h < 0 || h > 23 ||
        mi < 0 || mi > 59 ||
        s < 0 || s > 59) {

        return false;
    }

    year = static_cast<uint16_t>(y);
    month = static_cast<uint8_t>(mo);
    day = static_cast<uint8_t>(d);
    hour = static_cast<uint8_t>(h);
    minute = static_cast<uint8_t>(mi);
    second = static_cast<uint8_t>(s);

    return true;
}


// ============================================================
// TIMEZONE PARSING
// ============================================================

bool MqttCommandHandler::parseTimezone(
    const char* text,
    int& offsetHours
) {
    if (text == nullptr) {
        return false;
    }

    if (strcmp(text, "UTC") == 0 ||
        strcmp(text, "UTC+00:00") == 0 ||
        strcmp(text, "UTC-00:00") == 0) {

        offsetHours = 0;
        return true;
    }

    if (strlen(text) != 9 ||
        strncmp(text, "UTC", 3) != 0 ||
        (text[3] != '+' && text[3] != '-') ||
        text[4] < '0' || text[4] > '9' ||
        text[5] < '0' || text[5] > '9' ||
        text[6] != ':' ||
        text[7] != '0' ||
        text[8] != '0') {

        return false;
    }

    int hours =
        (text[4] - '0') * 10 +
        (text[5] - '0');

    if (hours > 14) {
        return false;
    }

    offsetHours =
        text[3] == '-' ? -hours : hours;

    return true;
}


// ============================================================
// TIME FORMATTING
// ============================================================

void MqttCommandHandler::formatTime(
    time_t timestamp,
    char* output,
    size_t outputSize
) {
    if (output == nullptr || outputSize == 0) {
        return;
    }

    struct tm timeInfo {};

    if (timestamp <= 0 ||
        gmtime_r(&timestamp, &timeInfo) == nullptr) {

        snprintf(
            output,
            outputSize,
            "invalid"
        );

        return;
    }

    snprintf(
        output,
        outputSize,
        "%04d-%02d-%02dT%02d:%02d:%02d",
        timeInfo.tm_year + 1900,
        timeInfo.tm_mon + 1,
        timeInfo.tm_mday,
        timeInfo.tm_hour,
        timeInfo.tm_min,
        timeInfo.tm_sec
    );
}