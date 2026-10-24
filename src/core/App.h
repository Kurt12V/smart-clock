#pragma once

#include <Arduino.h>
#include <WebServer.h>

// ============================================================
// MANAGERS
// ============================================================

#include "./managers/Setting/SettingsManager.h"
#include "./managers/SPI/SPIManager.h"
#include "./managers/I2S/I2SManager.h"
#include "./managers/SD/SDManager.h"
#include "./managers/Sound/SoundManager.h"
#include "./managers/Sensors/SensorsManager.h"
#include "./core/ClockSystem.h"
#include "./managers/Led/LightingManager.h"
#include "./managers/Alarm/AlarmManager.h"
#include "./managers/Alarm/AlarmController.h"
#include "./managers/WiFi/WiFiManager.h"

// ============================================================
// SYSTEMS
// ============================================================

#include "./core/DisplaySystem.h"

// ============================================================
// WEB
// ============================================================

#include "./managers/WEB/WebWiFiManager.h"


// ============================================================
// APP
// ============================================================

class App
{
public:

    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    App();


    // ========================================================
    // LIFECYCLE
    // ========================================================

    bool begin();

    void update();


private:

    // ========================================================
    // INITIALIZATION
    // ========================================================

    bool initSettings();

    bool initSPI();

    bool initI2S();

    bool initSD();

    bool initSound();

    bool initClock();

    bool initSensors();

    bool initLighting();

    bool initDisplay();

    bool initAlarm();

    bool initWiFi();

    bool initWebWiFi();


    // ========================================================
    // CORE MANAGERS
    // ========================================================

    SettingsManager _settings;

    SPIManager _spiManager;

    I2SManager _i2sManager;

    SDManager _sdManager;

    SoundManager _soundManager;

    SensorManager _sensorManager;

    ClockSystem _clockSystem;

    LightingManager _lighting;

    DisplaySystem _displaySystem;


    // ========================================================
    // ALARM
    // ========================================================

    AlarmManager _alarmManager;

    AlarmController _alarmController;


    // ========================================================
    // NETWORK
    // ========================================================

    WiFiManager _wifiManager;

    // HTTP server on port 80.
    WebServer _webServer;

    // Wi-Fi HTTP API.
    WebWiFiManager _webWiFiManager;


    // ========================================================
    // STATE
    // ========================================================

    bool _initialized;
};