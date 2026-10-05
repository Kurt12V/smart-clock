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

    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    WebServerManager();


    // ========================================================
    // BEGIN
    // ========================================================

    bool begin(
        SettingsManager& settings,
        SDManager& sd,
        SoundManager& sound,
        const char* ssid,
        const char* password
    );


    // ========================================================
    // ALARM CONNECTION
    // ========================================================

    void setAlarmManager(
        AlarmManager& alarmManager
    );

    void setAlarmController(
        AlarmController& alarmController
    );


    // ========================================================
    // UPDATE
    // ========================================================

    void update();


    // ========================================================
    // STATE
    // ========================================================

    bool isConnected() const;

    String getIP() const;


private:

    // ========================================================
    // ROUTES
    // ========================================================

    void setupRoutes();


    // ========================================================
    // ROOT
    // ========================================================

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
    // ALARM JSON
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


private:

    // ========================================================
    // SERVER
    // ========================================================

    WebServer _server;


    // ========================================================
    // MANAGERS
    // ========================================================

    SettingsManager* _settings;

    SDManager* _sd;

    SoundManager* _sound;

    AlarmManager* _alarmManager;

    AlarmController* _alarmController;


    // ========================================================
    // STATE
    // ========================================================

    bool _initialized;
};