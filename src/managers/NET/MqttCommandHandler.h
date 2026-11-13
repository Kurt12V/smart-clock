
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "SmartClockMqttClient.h"
#include "./managers/Setting/SettingsManager.h"
#include "./core/ClockSystem.h"
#include "./managers/WiFi/WiFiManager.h"
#include "./managers/Alarm/AlarmManager.h"


class MqttCommandHandler
{
public:
    MqttCommandHandler();

    MqttCommandHandler(const MqttCommandHandler&) = delete;
    MqttCommandHandler& operator=(const MqttCommandHandler&) = delete;

    bool begin(
        SmartClockMqttClient& mqtt,
        SettingsManager& settings,
        ClockSystem& clock,
        WiFiManager& wifi,
        AlarmManager& alarms
    );

    static void onMqttEvent(
        SmartClockMqttClient::Event event,
        const char* topic,
        const char* payload
    );

private:
    static MqttCommandHandler* _instance;

    SmartClockMqttClient* _mqtt;
    SettingsManager* _settings;
    ClockSystem* _clock;
    WiFiManager* _wifi;
    AlarmManager* _alarms;

    bool _initialized;

    void handleCommand(const char* payload);

    void systemCommand(
        const char* requestId,
        const char* command,
        JsonVariantConst params
    );

    void clockCommand(
        const char* requestId,
        const char* command,
        JsonVariantConst params
    );

    void networkCommand(
        const char* requestId,
        const char* command,
        JsonVariantConst params
    );

    void displayCommand(
        const char* requestId,
        const char* command,
        JsonVariantConst params
    );

    void alarmCommand(
        const char* requestId,
        const char* command,
        JsonVariantConst params
    );

    bool sendAck(
        const char* requestId,
        const char* command,
        bool success,
        const char* message
    );

    bool sendResult(
        const char* requestId,
        const char* command,
        bool success,
        const char* message,
        JsonDocument& data
    );

    void publishCurrentState();

    void unknownCommand(
        const char* requestId,
        const char* command
    );

    static bool parseDateTime(
        const char* text,
        uint16_t& year,
        uint8_t& month,
        uint8_t& day,
        uint8_t& hour,
        uint8_t& minute,
        uint8_t& second
    );

    static bool parseTimezone(
        const char* text,
        int& offsetHours
    );

    static void formatTime(
        time_t timestamp,
        char* output,
        size_t outputSize
    );
};
