#include "DisplaySystem.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

DisplaySystem::DisplaySystem(
    SettingsManager& settings,
    ClockSystem& clock,
    SensorManager& sensors
)
    : _settings(settings),

      _clock(clock),
      _sensors(sensors),

      _spi(),

      _lvgl(_spi),

      _screens(
          clock,
          sensors,
          _lvgl
      ),

      _initialized(false),
      _appliedBrightness(255)
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

    // ========================================================
    // APPLY BRIGHTNESS FROM SETTINGS
    // ========================================================

    pollBrightness();

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
    // Brightness (from SettingsManager, live)
    // --------------------------------------------------------

    pollBrightness();

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
// POLL BRIGHTNESS
// ============================================================

void DisplaySystem::pollBrightness()
{
    // --------------------------------------------------------
    // Физически подсветка одна (PIN_TFT_BL),
    // поэтому используется параметр disp1.
    //
    // Если в будущем появятся 4 отдельных BL-пина —
    // расширить здесь для disp2..disp4.
    // --------------------------------------------------------

    uint8_t want =
        static_cast<uint8_t>(
            _settings.get(
                Param::DisplayBrightness
            )
        );

    if (want == _appliedBrightness)
        return;

    _appliedBrightness = want;

    _lvgl.setBrightness(want);

    Serial0.print(
        "[DISPLAY] Applied brightness: "
    );

    Serial0.print(want);
    Serial0.println("%");
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

SPIManager& DisplaySystem::spi()
{
    return _spi;
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