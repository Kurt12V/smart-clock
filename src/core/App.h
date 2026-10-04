#pragma once

#include <Arduino.h>

#include "./managers/SettingsManager.h"
#include "./managers/SPIManager.h"
#include "./managers/I2SManager.h"
#include "./managers/SDManager.h"
#include "./managers/SoundManager.h"
#include "./managers/SensorsManager.h"
#include "./managers/LightingManager.h"
#include "./managers/WebServerManager.h"

#include "./core/ClockSystem.h"
#include "./core/DisplaySystem.h"


class App
{
public:
    App();

    bool begin();
    void update();

private:
    bool initSettings();
    bool initSPI();
    bool initI2S();
    bool initSD();
    bool initSound();
    bool initClock();
    bool initSensors();
    bool initLighting();
    bool initDisplay();
    bool initWebServer();

private:
    SettingsManager _settings;

    SPIManager _spiManager;

    I2SManager _i2sManager;

    SDManager _sdManager;

    SoundManager _soundManager;

    SensorManager _sensorManager;

    ClockSystem _clockSystem;

    LightingManager _lighting;

    DisplaySystem _displaySystem;

    WebServerManager _webServer;

    bool _initialized;
};

