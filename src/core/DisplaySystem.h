#pragma once

#include "./managers/SPIManager.h"
#include "./managers/LVGLManager.h"
#include "./managers/ScreenManager.h"

#include "./core/ClockSystem.h"
#include "./managers/SensorsManager.h"

#include "./managers/LVGLManager.h"
#include "./managers/ScreenManager.h"


class DisplaySystem
{
public:

    DisplaySystem(
        ClockSystem& clock,
        SensorManager& sensors
    );

    bool begin();
    void update();

    bool isReady() const;

    LVGLManager& lvgl();
    ScreenManager& screens();

private:

    ClockSystem& _clock;
    SensorManager& _sensors;

    // --------------------------------------------------------
    // SPI
    // --------------------------------------------------------

    SPIManager _spi;

    // --------------------------------------------------------
    // LVGL
    // --------------------------------------------------------

    LVGLManager _lvgl;

    ScreenManager _screens;

    // --------------------------------------------------------
    // State
    // --------------------------------------------------------

    bool _initialized;
};