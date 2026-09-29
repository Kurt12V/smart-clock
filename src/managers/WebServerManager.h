#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "SettingsManager.h"
#include "SDManager.h"
#include "SoundManager.h"


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
        SDManager&       sd,
        SoundManager&    sound,
        const char*      ssid,
        const char*      password
    );


    // ========================================================
    // UPDATE
    // ========================================================

    void update();


    // ========================================================
    // STATUS
    // ========================================================

    bool isConnected() const;

    String getIP() const;


private:

    // ========================================================
    // SERVER
    // ========================================================

    WebServer _server;


    // ========================================================
    // MANAGERS
    // ========================================================

    SettingsManager* _settings;
    SDManager*       _sd;
    SoundManager*    _sound;


    // ========================================================
    // STATE
    // ========================================================

    bool _initialized;


    // ========================================================
    // ROUTES
    // ========================================================

    void setupRoutes();


    // ========================================================
    // GENERAL
    // ========================================================

    void handleRoot();
    void handleNotFound();


    // ========================================================
    // SETTINGS API
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
    // JSON
    // ========================================================

    bool parseJson(
        JsonDocument& doc
    );


    // ========================================================
    // RESPONSE
    // ========================================================

    void sendJson(
        int code,
        const String& body
    );


    void sendOk();


    void sendError(
        int code,
        const char* message
    );
};