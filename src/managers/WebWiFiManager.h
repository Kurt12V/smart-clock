#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "WiFiManager.h"

class WebWiFiManager
{
public:

    explicit WebWiFiManager(
        WiFiManager& wifiManager
    );

    void setupRoutes(
        WebServer& server
    );

    bool handleDynamicRequest(
        WebServer& server
    );
        bool isSetupMode() const;
        String getIP() const;

String getAPIP() const;

private:

    void handleStatus(
        WebServer& server
    );

    void handleScan(
        WebServer& server
    );

    void handleConnect(
        WebServer& server
    );

    void handleForget(
        WebServer& server
    );

    void handleSetup(
        WebServer& server
    );

    void sendError(
        WebServer& server,
        int code,
        const char* message
    );

private:

    WiFiManager& _wifiManager;
};