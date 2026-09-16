
#include "DisplaySystem.h"

#include "./utils/Logger.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

DisplaySystem::DisplaySystem(
    ClockSystem& clock,
    SensorManager& sensors
)
    : _clock(clock),
      _sensors(sensors),

      _spi(),

      _lvgl(_spi),

      _screens(
          clock,
          sensors,
          _lvgl
      ),

      _initialized(false)
{
}

// ============================================================
// BEGIN
// ============================================================

bool DisplaySystem::begin()
{
    if (_initialized)
        return true;

    Logger::info(
        "DISPLAY",
        "Starting DisplaySystem..."
    );

    // ========================================================
    // SPI
    // ========================================================

    if (!_spi.begin())
    {
        Logger::info(
            "DISPLAY",
            "SPI initialization failed"
        );

        return false;
    }

    // ========================================================
    // LVGL + TFT
    // ========================================================

    if (!_lvgl.begin())
    {
        Logger::info(
            "DISPLAY",
            "LVGL initialization failed"
        );

        return false;
    }

    // ========================================================
    // SCREENS
    // ========================================================

    if (!_screens.begin())
    {
        Logger::info(
            "DISPLAY",
            "ScreenManager initialization failed"
        );

        return false;
    }

    _initialized = true;

    Logger::info(
        "DISPLAY",
        "DisplaySystem ready"
    );

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void DisplaySystem::update()
{
    if (!_initialized)
        return;

    // --------------------------------------------------------
    // Обновление данных экранов
    // --------------------------------------------------------

    _screens.update();

    // --------------------------------------------------------
    // Обработка LVGL
    // --------------------------------------------------------

    _lvgl.update();
}

// ============================================================
// IS READY
// ============================================================

bool DisplaySystem::isReady() const
{
    return _initialized;
}

// ============================================================
// SPI
// ============================================================

SPIManager&
DisplaySystem::spi()
{
    return _spi;
}

// ============================================================
// LVGL
// ============================================================

LVGLManager&
DisplaySystem::lvgl()
{
    return _lvgl;
}

// ============================================================
// SCREENS
// ============================================================

ScreenManager&
DisplaySystem::screens()
{
    return _screens;
}

