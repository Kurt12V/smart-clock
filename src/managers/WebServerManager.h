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


private:

    // ========================================================
    // ROUTES
    // ========================================================

    void setupRoutes();

    void handleRoot();

    void handleNotFound();


private:

    WebServer _server;

    SettingsManager* _settings;

    WebPageManager* _pageManager;

    WebSettingsManager* _settingsManager;

    WebSDManager* _sdManager;

    WebAudioManager* _audioManager;

    WebAlarmManager* _alarmManager;

    bool _initialized;
};