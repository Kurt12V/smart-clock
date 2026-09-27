#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

class WebServerManager
{
public:

    WebServerManager();

    bool begin(
        const char* ssid,
        const char* password
    );

    void update();

    bool isConnected() const;

    String getIP() const;

private:

    WebServer _server;

    bool _initialized;

    void setupRoutes();
    void handleRoot();
    void handleNotFound();
};