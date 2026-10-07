#pragma once

#include <Arduino.h>

// ============================================================
// CORE MANAGERS
// ============================================================

#include "./managers/SettingsManager.h"
#include "./managers/SPIManager.h"
#include "./managers/I2SManager.h"
#include "./managers/SDManager.h"
#include "./managers/SoundManager.h"
#include "./managers/SensorsManager.h"
#include "./managers/LightingManager.h"

// ============================================================
// CORE SYSTEMS
// ============================================================

#include "./core/ClockSystem.h"
#include "./core/DisplaySystem.h"

// ============================================================
// ALARM
// ============================================================

#include "./managers/AlarmManager.h"
#include "./managers/AlarmController.h"

// ============================================================
// WEB
// ============================================================

#include "./managers/WebServerManager.h"
#include "./managers/WebPageManager.h"
#include "./managers/WebSettingsManager.h"
#include "./managers/WebSDManager.h"
#include "./managers/WebAudioManager.h"
#include "./managers/WebAlarmManager.h"


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

    bool initWebServer();


private:

    // ========================================================
    // CORE
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
    // WEB MODULES
    // ========================================================

    WebPageManager _webPageManager;

    WebSettingsManager _webSettingsManager;

    WebSDManager _webSDManager;

    WebAudioManager _webAudioManager;

    WebAlarmManager _webAlarmManager;

    WebServerManager _webServer;


    // ========================================================
    // STATE
    // ========================================================

    bool _initialized;
};