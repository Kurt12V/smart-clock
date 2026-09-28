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

    WebServerManager();

    bool begin(
        SettingsManager& settings,
        SDManager&       sd,
        SoundManager&    sound,
        const char*      ssid,
        const char*      password
    );

    void update();

    bool   isConnected() const;
    String getIP() const;

private:

    WebServer        _server;
    SettingsManager* _settings;
    SDManager*       _sd;
    SoundManager*    _sound;
    bool             _initialized;

    void setupRoutes();

    void handleRoot();
    void handleNotFound();

    void handleGetParam();
    void handleSetParam();
    void handleGetAllParams();
    void handleReset();

    void handleSensors();
    void handleSD();

    // audio
    void handleAudioPlay();
    void handleAudioPause();
    void handleAudioResume();
    void handleAudioStop();
    void handleAudioStatus();

    bool parseJson(JsonDocument& doc);

    void sendJson(int code, const String& body);
    void sendOk();
    void sendError(int code, const char* message);
};