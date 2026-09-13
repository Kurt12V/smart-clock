#include "DisplaySystem.h"


DisplaySystem::DisplaySystem(
    ClockSystem& clock,
    SensorManager& sensors
)
    : _clock(clock),
      _sensors(sensors),
      _lvgl(),
      _screens(
          clock,
          sensors
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

    // --------------------------------------------------------
    // LVGL + TFT
    // --------------------------------------------------------

    if (!_lvgl.begin())
    {
        return false;
    }

    // --------------------------------------------------------
    // SCREENS
    // --------------------------------------------------------

    if (!_screens.begin())
    {
        return false;
    }

    _initialized = true;

    return true;
}


// ============================================================
// UPDATE
// ============================================================

void DisplaySystem::update()
{
    if (!_initialized)
        return;

    // Обновляем данные UI.
    _screens.update();

    // Обрабатываем LVGL.
    _lvgl.update();
}


// ============================================================
// READY
// ============================================================

bool DisplaySystem::isReady() const
{
    return _initialized;
}


// ============================================================
// LVGL
// ============================================================

LVGLManager& DisplaySystem::lvgl()
{
    return _lvgl;
}


// ============================================================
// SCREENS
// ============================================================

ScreenManager& DisplaySystem::screens()
{
    return _screens;
}