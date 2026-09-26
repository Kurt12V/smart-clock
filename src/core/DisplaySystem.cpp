#include "DisplaySystem.h"

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

    Serial0.println();
    Serial0.println("============================================");
    Serial0.println("[DISPLAY] Starting DisplaySystem");
    Serial0.println("============================================");

    // ========================================================
    // SPI
    // ========================================================

    Serial0.println("[DISPLAY] Initializing SPI...");

    if (!_spi.begin())
    {
        Serial0.println("[DISPLAY] SPI initialization FAILED");
        return false;
    }

    Serial0.println("[DISPLAY] SPI OK");

    // ========================================================
    // LVGL + TFT
    // ========================================================

    Serial0.println("[DISPLAY] Initializing LVGL + TFT...");

    if (!_lvgl.begin())
    {
        Serial0.println("[DISPLAY] LVGL initialization FAILED");
        return false;
    }

    Serial0.println("[DISPLAY] LVGL OK");

    // ========================================================
    // CLEAR DISPLAYS
    // ========================================================

    Serial0.println("[DISPLAY] Clearing physical displays...");

    _lvgl.clearDisplays();

    Serial0.println("[DISPLAY] Displays cleared");

    // ========================================================
    // SCREENS
    // ========================================================

    Serial0.println("[DISPLAY] Initializing ScreenManager...");

    if (!_screens.begin())
    {
        Serial0.println("[DISPLAY] ScreenManager initialization FAILED");
        return false;
    }

    Serial0.println("[DISPLAY] ScreenManager OK");

    // ========================================================
    // FIRST FRAME
    // ========================================================

    Serial0.println("[DISPLAY] Rendering first frame...");

    _lvgl.refresh();

    Serial0.println("[DISPLAY] First frame rendered");

    // ========================================================
    // READY
    // ========================================================

    _initialized = true;

    Serial0.println();
    Serial0.println("============================================");
    Serial0.println("[DISPLAY] DisplaySystem READY");
    Serial0.println("============================================");
    Serial0.println();

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
    // Update screen data
    // --------------------------------------------------------

    _screens.update();

    // --------------------------------------------------------
    // LVGL
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