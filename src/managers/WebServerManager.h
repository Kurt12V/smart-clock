#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "SettingsManager.h"

class WebServerManager
{
public:

    WebServerManager();

    bool begin(
        SettingsManager& settings,
        const char* ssid,
        const char* password
    );

    void update();

    bool   isConnected() const;
    String getIP() const;

private:

    WebServer        _server;
    SettingsManager* _settings;
    bool             _initialized;

    // --------------------------------------------------------
    // routes
    // --------------------------------------------------------

    void setupRoutes();

    void handleRoot();
    void handleNotFound();

    void handleGetParam();
    void handleSetParam();
    void handleGetAllParams();
    void handleReset();

    void handleSensors();

    // --------------------------------------------------------
    // helpers
    // --------------------------------------------------------

    bool parseJson(JsonDocument& doc);

    void sendJson(int code, const String& body);
    void sendOk();
    void sendError(int code, const char* message);
};