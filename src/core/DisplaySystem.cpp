#include "DisplaySystem.h"

#include <cstdio>

DisplaySystem::DisplaySystem(
    ClockSystem& clock,
    SensorManager& sensors
)
    : _clock(clock),
      _sensors(sensors),
      _lvgl(),
      _screens(clock, sensors, _lvgl),
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

    Serial.println("DisplaySystem: starting LVGL...");

    if (!_lvgl.begin())
    {
        Serial.println("DisplaySystem: LVGL ERROR");
        return false;
    }

    Serial.println("DisplaySystem: LVGL OK");


    // ========================================================
    // CREATE FOUR CLOCK SCREENS
    // ========================================================

    if (!_screens.begin())
    {
        Serial.println("DisplaySystem: ScreenManager ERROR");
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

    Serial.println("DisplaySystem: ready");

    return true;
}


// ============================================================
// UPDATE
// ============================================================

void DisplaySystem::update()
{
    if (!_initialized)
        return;


    // ========================================================
    // UPDATE ALL SCREEN DATA
    // ========================================================

    updateScreens();


    // ========================================================
    // SCREEN MANAGER
    // ========================================================

    _screens.update();


    // ========================================================
    // LVGL
    // ========================================================

    _lvgl.update();
}


// ============================================================
// UPDATE SCREENS
// ============================================================

void DisplaySystem::updateScreens()
{
    updateTime();
    updateSensors();
    updateDate();
}


// ============================================================
// TIME
// ============================================================

void DisplaySystem::updateTime()
{
    // ========================================================
    // SCREEN 0
    // ========================================================
    //
    // десятки часов
    //
    // 14
    // ^
    // 1
    //

    _screens.setCenterText(
        0,
        _clock.getHourTens()
    );


    // ========================================================
    // SCREEN 1
    // ========================================================
    //
    // часы вертикально
    //
    // 1
    // 4
    //

    _screens.setCenterText(
        1,
        _clock.getHourTens(),
        _clock.getHourOnes()
    );


    // ========================================================
    // SCREEN 2
    // ========================================================
    //
    // десятки минут
    //
    // 25
    // ^
    // 2
    //

    _screens.setCenterText(
        2,
        _clock.getMinuteTens()
    );


    // ========================================================
    // SCREEN 3
    // ========================================================
    //
    // единицы минут
    //
    // 5
    //

    _screens.setCenterText(
        3,
        _clock.getMinuteOnes()
    );
}


// ============================================================
// SENSOR DATA
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
// DATE
// ============================================================

void DisplaySystem::updateDate()
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

ScreenManager& DisplaySystem::screens()
{
    return _screens;
}
