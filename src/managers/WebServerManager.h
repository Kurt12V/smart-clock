#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#include "SettingsManager.h"
#include "SDManager.h"
#include "SoundManager.h"
#include "AlarmManager.h"
#include "AlarmController.h"

class WebServerManager
{
public:

    WebServerManager();

    bool begin(
        SettingsManager& settings,
        SDManager& sd,
        SoundManager& sound,
        const char* ssid,
        const char* password
    );

    void setAlarmManager(
        AlarmManager& alarmManager
    );

    void setAlarmController(
        AlarmController& alarmController
    );

    void update();

    bool isConnected() const;

    String getIP() const;


private:

    // ========================================================
    // ROUTES
    // ========================================================

    void setupRoutes();

    void handleRoot();

    void handleNotFound();


    // ========================================================
    // SETTINGS
    // ========================================================

    void handleGetParam();

    void handleSetParam();

    void handleGetAllParams();

    void handleReset();


    // ========================================================
    // SENSORS
    // ========================================================

    void handleSensors();


    // ========================================================
    // SD
    // ========================================================

    void handleSD();


    // ========================================================
    // AUDIO
    // ========================================================

    void handleAudioPlay();

    void handleAudioPause();

    void handleAudioResume();

    void handleAudioStop();

    void handleAudioStatus();


    // ========================================================
    // ALARMS
    // ========================================================

    void handleGetAlarms();

    void handleGetAlarm();

    void handleCreateAlarm();

    void handleUpdateAlarm();

    void handleDeleteAlarm();

    void handleEnableAlarm();

    void handleDisableAlarm();

    void handleAlarmRuntime();

    void handleAlarmDismiss();

    void handleAlarmSnooze();


    // ========================================================
    // ALARM HELPERS
    // ========================================================

    bool parseAlarmFromRequest(
        Alarm& alarm
    );

    void sendAlarm(
        const Alarm& alarm
    );

    void sendAlarmList();


    // ========================================================
    // JSON
    // ========================================================

    bool parseJson(
        JsonDocument& document
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


    // ========================================================
    // REQUEST HELPERS
    // ========================================================

    bool isValidAlarmId(
        const String& id
    ) const;

    String getAlarmIdFromRequest();

    bool getBoolean(
        JsonObjectConst object,
        const char* key,
        bool& value
    ) const;

    bool getUnsigned32(
        JsonObjectConst object,
        const char* key,
        uint32_t& value
    ) const;


private:

    WebServer _server;

    SettingsManager* _settings;

    SDManager* _sd;

    SoundManager* _sound;

    AlarmManager* _alarmManager;

    AlarmController* _alarmController;

    bool _initialized;
};