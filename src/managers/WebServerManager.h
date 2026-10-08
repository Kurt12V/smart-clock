#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <WiFi.h>

#include "SettingsManager.h"

class WebPageManager;
class WebSettingsManager;
class WebSDManager;
class WebAudioManager;
class WebAlarmManager;
class WebWiFiManager;

class WebServerManager
{
public:

    WebServerManager();

    bool begin(
        SettingsManager& settings
    );

    void update();

    bool isConnected() const;

    String getIP() const;


    // ========================================================
    // MODULES
    // ========================================================

    void setPageManager(
        WebPageManager& manager
    );

    void setSettingsManager(
        WebSettingsManager& manager
    );

    void setSDManager(
        WebSDManager& manager
    );

    void setAudioManager(
        WebAudioManager& manager
    );

    void setAlarmManager(
        WebAlarmManager& manager
    );

    void setWiFiManager(
        WebWiFiManager& manager
    );

private:

    // ========================================================
    // ROUTES
    // ========================================================

    void setupRoutes();

    void handleRoot();

    void handleNotFound();


    // ========================================================
    // SETUP PAGE
    // ========================================================

    bool serveWiFiSetupPage();


    // ========================================================
    // STATIC FILE
    // ========================================================

    bool serveFile(
        const char* path,
        const char* contentType
    );


private:

    WebServer _server;

    SettingsManager* _settings;

    WebPageManager* _pageManager;

    WebSettingsManager* _settingsManager;

    WebSDManager* _sdManager;

    WebAudioManager* _audioManager;

    WebAlarmManager* _alarmManager;

    WebWiFiManager* _webWiFiManager;

    bool _initialized;
};
