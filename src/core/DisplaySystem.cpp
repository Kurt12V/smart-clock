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

    Serial.println("DisplaySystem: ScreenManager OK");


    // ========================================================
    // CENTER MODES
    // ========================================================

    // Screen 0:
    // одна цифра
    _screens.setCenterMode(
        0,
        ScreenManager::CenterMode::ONE_DIGIT
    );

    // Screen 1:
    // две цифры вертикально
    _screens.setCenterMode(
        1,
        ScreenManager::CenterMode::TWO_DIGITS_VERTICAL
    );

    // Screen 2:
    // одна цифра
    _screens.setCenterMode(
        2,
        ScreenManager::CenterMode::ONE_DIGIT
    );

    // Screen 3:
    // одна цифра
    _screens.setCenterMode(
        3,
        ScreenManager::CenterMode::ONE_DIGIT
    );


    // ========================================================
    // INITIAL TOP BARS
    // ========================================================

    _screens.setTopText(0, "");
    _screens.setTopText(1, "");
    _screens.setTopText(2, "");
    _screens.setTopText(3, "");


    // ========================================================
    // INITIAL DATA
    // ========================================================

    updateScreens();


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

void DisplaySystem::updateSensors()
{
    // ========================================================
    // TEMPERATURE
    // ========================================================

    String temperature =
        _sensors.getTemperatureC();

    _screens.setBottomText(
        0,
        temperature.c_str()
    );


    // ========================================================
    // HUMIDITY
    // ========================================================

    String humidity =
        _sensors.getHumidity();

    _screens.setBottomText(
        1,
        humidity.c_str()
    );


    // ========================================================
    // LIGHT
    // ========================================================

    String light =
        _sensors.getLight();

    _screens.setBottomText(
        2,
        light.c_str()
    );
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
// DATE
// ============================================================

LVGLManager& DisplaySystem::lvgl()
{
    char dateText[32];

    snprintf(
        dateText,
        sizeof(dateText),
        "%02u.%02u.%04u",
        _clock.day(),
        _clock.month(),
        _clock.year()
    );


    _screens.setBottomText(
        3,
        dateText
    );
}

// ============================================================
// SCREENS ACCESS
// ============================================================

ScreenManager&
DisplaySystem::screens()
{
    return _screens;
}
