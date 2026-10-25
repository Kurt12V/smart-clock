#pragma once

#include <Arduino.h>
#include <WebServer.h>

// ============================================================
// MANAGERS
// ============================================================

#include "./managers/Setting/SettingsManager.h"
#include "./managers/SPI/SPIManager.h"
#include "./managers/I2S/I2SManager.h"
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
    App();

    // Initialize all application components.
    bool begin();

    // Update all active components.
    void update();

    // Application status.
    bool isInitialized() const
    {
        return _initialized;
    }

private:
    // ========================================================
    // INITIALIZATION
    // ========================================================

    bool initSettings();
    bool initSPI();
    bool initI2S();
    bool initClock();
    bool initSensors();
    bool initLighting();
    bool initDisplay();
    bool initSound();
    bool initAlarm();
    bool initWiFi();
    bool initWebWiFi();

    // ========================================================
    // CORE MANAGERS
    // ========================================================

    SettingsManager _settings;

    SPIManager _spiManager;

    I2SManager _i2sManager;

    SoundManager _soundManager;

    SensorManager _sensorManager;

    ClockSystem _clockSystem;

    LightingManager _lighting;

    DisplaySystem _displaySystem;

    // ========================================================
    // ALARMS
    // ========================================================

    AlarmManager _alarmManager;

    AlarmController _alarmController;

    // ========================================================
    // NETWORK
    // ========================================================

    WiFiManager _wifiManager;

    WebServer _webServer;

    WebWiFiManager _webWiFiManager;

    // ========================================================
    // STATE
    // ========================================================

    bool _initialized = false;
};