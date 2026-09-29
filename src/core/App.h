#pragma once

#include <Arduino.h>

#include "managers/SettingsManager.h"
#include "managers/SPIManager.h"
#include "managers/I2SManager.h"
#include "managers/SDManager.h"
#include "managers/SoundManager.h"
#include "managers/InputManager.h"
#include "managers/SensorsManager.h"
#include "managers/WebServerManager.h"

#include "hardware/cob/CobLed.h"
#include "hardware/cob/CobLedManager.h"
#include "hardware/cob/CobEffects.h"

#include "core/ClockSystem.h"
#include "core/DisplaySystem.h"

class App
{
public:

    App();

    bool begin();

    void update();

    bool isReady() const;

private:

    // ========================================================
    // INITIALIZATION STEPS
    // ========================================================

    bool initSettings();
    bool initSPI();
    bool initI2S();
    bool initSD();
    bool initSound();
    bool initClock();
    bool initSensors();
    bool initCob();
    bool initDisplay();
    bool initInput();
    bool initWebServer();

    // ========================================================
    // RUNTIME POLLING
    // ========================================================

    void updateCob();

    // ========================================================
    // STATE
    // ========================================================

    bool _ready;

    // ========================================================
    // SETTINGS (первым — от него зависят остальные)
    // ========================================================

    SettingsManager _settings;

    // ========================================================
    // MANAGERS
    // ========================================================

    SPIManager _spiManager;
    I2SManager _i2sManager;
    SDManager  _sdManager;

    SoundManager _soundManager;

    InputManager  _inputManager;
    SensorManager _sensorManager;

    // ========================================================
    // COB LED (4 штуки + менеджер + эффекты)
    //
    // Порядок: сначала сами CobLed, потом CobLedManager,
    // потом CobEffects (принимает ссылку на менеджер).
    // ========================================================

    CobLed _cob1;
    CobLed _cob2;
    CobLed _cob3;
    CobLed _cob4;

    CobLedManager _cobManager;
    CobEffects    _cobEffects;

    // ========================================================
    // CORE SYSTEMS
    // ========================================================

    ClockSystem   _clockSystem;
    DisplaySystem _displaySystem;

    // ========================================================
    // WEB SERVER
    // ========================================================

    WebServerManager _webServer;
};