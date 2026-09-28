#pragma once

#include <Arduino.h>

#include "./managers/SPIManager.h"
#include "./managers/LVGLManager.h"
#include "./managers/ScreenManager.h"
#include "./managers/SettingsManager.h"

class ClockSystem;
class SensorManager;

class DisplaySystem
{
public:

    // ========================================================
    // LIFECYCLE
    // ========================================================

    DisplaySystem(
        SettingsManager& settings,
        ClockSystem& clock,
        SensorManager& sensors
    );

    bool begin();

    void update();

    bool isReady() const;

    // ========================================================
    // ACCESSORS
    // ========================================================

    SPIManager&    spi();
    LVGLManager&   lvgl();
    ScreenManager& screens();

private:

    // ========================================================
    // BRIGHTNESS
    // ========================================================

    void pollBrightness();

    // ========================================================
    // MEMBERS
    // ========================================================

    SettingsManager& _settings;

    ClockSystem&   _clock;
    SensorManager& _sensors;

    SPIManager    _spi;
    LVGLManager   _lvgl;
    ScreenManager _screens;

    bool    _initialized;
    uint8_t _appliedBrightness;
};