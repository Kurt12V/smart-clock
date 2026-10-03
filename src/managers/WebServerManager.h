#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "./managers/SettingsManager.h"
#include "./managers/SDManager.h"
#include "./managers/SoundManager.h"
#include "./managers/AlarmManager.h"
#include "./core/ClockSystem.h"


class WebServerManager
{
public:

    WebServerManager();


    // =========================================================
    // BEGIN
    // =========================================================

    bool begin(
        SettingsManager& settings,
        SDManager& sd,
        SoundManager& sound,
        ClockSystem& clockSystem,
        AlarmManager& alarmManager,
        const char* ssid,
        const char* password
    );


    // =========================================================
    // UPDATE
    // =========================================================

    void update();


    // =========================================================
    // STATUS
    // =========================================================

    bool isRunning() const;

    bool isConnected() const;

    String getIP() const;


private:

    // =========================================================
    // ROUTES
    // =========================================================

    void setupRoutes();


    // =========================================================
    // WEB
    // =========================================================

    void handleRoot();

    void handleNotFound();


    // =========================================================
    // SETTINGS
    // =========================================================

    void handleGetParam();

    void handleSetParam();

    void handleGetAllParams();

    void handleReset();


    // =========================================================
    // TIME
    // =========================================================

    void handleTime();

    void handleTimeSync();


    // =========================================================
    // LEGACY ALARM
    // =========================================================

    void handleGetAlarm();

    void handleSetAlarm();


    // =========================================================
    // ALARM MANAGER
    // =========================================================

    void handleGetAlarms();

    void handleCreateAlarm();

    void handleGetAlarmById();

    void handleUpdateAlarmById();

    void handleDeleteAlarmById();


    // =========================================================
    // ALARM JSON
    // =========================================================

    bool parseAlarmJson(
        JsonDocument& doc,
        AlarmData& alarm
    );

    void serializeAlarm(
        JsonDocument& doc,
        const AlarmData& alarm
    );

    void serializeAlarmSchedule(
        JsonDocument& doc,
        const AlarmScheduleData& schedule
    );


    bool getAlarmIdFromUri(
        String& id
    );


    // =========================================================
    // TIMER
    // =========================================================

    void handleGetTimer();

    void handleSetTimer();

    void updateTimer();


    // =========================================================
    // AUDIO
    // =========================================================

    void handleAudioPlay();

    void handleAudioPause();

    void handleAudioResume();

    void handleAudioStop();

    void handleAudioStatus();


    // =========================================================
    // SD
    // =========================================================

    void handleSD();


    // =========================================================
    // SENSORS
    // =========================================================

    void handleSensors();


    // =========================================================
    // JSON
    // =========================================================

    bool parseJson(
        JsonDocument& doc
    );

    void sendJson(
        int code,
        const String& body
    );

    void sendOk();

    void sendError(
        int code,
        const char* message
    );


    // =========================================================
    // LEGACY ALARM STATE
    // =========================================================

    struct AlarmDataLegacy
    {
        bool enabled = false;

        uint8_t hour = 7;

        uint8_t minute = 30;

        uint8_t sound = 0;

        uint8_t volume = 80;

        bool repeat[7] =
        {
            false,
            false,
            false,
            false,
            false,
            false,
            false
        };
    };


    // =========================================================
    // TIMER
    // =========================================================

    enum class TimerStatus : uint8_t
    {
        READY = 0,
        RUNNING,
        PAUSED,
        FINISHED
    };


    struct TimerData
    {
        TimerStatus status =
            TimerStatus::READY;

        uint32_t durationSeconds =
            0;

        uint32_t remainingSeconds =
            0;

        uint32_t lastUpdate =
            0;
    };


    // =========================================================
    // MEMBERS
    // =========================================================

    WebServer _server;


    SettingsManager* _settings;

    SDManager* _sd;

    SoundManager* _sound;

    ClockSystem* _clockSystem;

    AlarmManager* _alarmManager;


    AlarmDataLegacy _alarm;

    TimerData _timer;


    bool _initialized;
};
