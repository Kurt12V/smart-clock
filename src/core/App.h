
#pragma once

#include <Arduino.h>

// ============================================================
// MANAGERS
// ============================================================

#include "./managers/SettingsManager.h"
#include "./managers/SPIManager.h"
#include "./managers/I2SManager.h"
#include "./managers/SDManager.h"
#include "./managers/SoundManager.h"
#include "./managers/InputManager.h"
#include "./managers/SensorsManager.h"

#include "./managers/CobLedManager.h"
#include "./managers/LedMatrixManager.h"

#include "./managers/AlarmManager.h"
#include "./managers/WebServerManager.h"

// ============================================================
// HARDWARE
// ============================================================

#include "./hardware/light/CobLed.h"

// ============================================================
// CORE
// ============================================================

#include "./core/ClockSystem.h"
#include "./core/DisplaySystem.h"
#include "./core/LightSystem.h"


// ============================================================
// APP
// ============================================================

class App
{
public:

    App();

    bool begin();
    void update();

    bool isReady() const;


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

    bool initAlarm();

    bool initLight();

    bool initDisplay();
    bool initInput();
    bool initWebServer();


    // ========================================================
    // STATE
    // ========================================================

    bool _ready;


    // ========================================================
    // CORE MANAGERS
    // ========================================================

    SettingsManager _settings;

    SPIManager _spiManager;

    I2SManager _i2sManager;

    SDManager _sdManager;

    SoundManager _soundManager;

    InputManager _inputManager;

    SensorManager _sensorManager;


    // ========================================================
    // CLOCK
    // ========================================================

    ClockSystem _clockSystem;


    // ========================================================
    // COB LED HARDWARE
    // ========================================================

    CobLed _cob1;
    CobLed _cob2;
    CobLed _cob3;
    CobLed _cob4;


    // ========================================================
    // LIGHT MANAGERS
    // ========================================================

    CobLedManager _cobManager;

    LedMatrixManager _matrixManager;

    LightSystem _lightSystem;


    // ========================================================
    // ALARM
    // ========================================================

    AlarmManager _alarmManager;


    // ========================================================
    // DISPLAY
    // ========================================================

    DisplaySystem _displaySystem;


    // ========================================================
    // WEB
    // ========================================================

    WebServerManager _webServer;
};
