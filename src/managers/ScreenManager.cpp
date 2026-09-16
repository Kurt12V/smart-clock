#include "ScreenManager.h"

#include <Arduino.h>
#include <cstdio>
#include <cstring>

#include "./core/ClockSystem.h"
#include "./managers/SensorsManager.h"
#include "./managers/LVGLManager.h"

// ============================================================
// CONSTANTS
// ============================================================

namespace
{
    constexpr uint8_t SCREEN_COUNT = 4;

    constexpr uint32_t UPDATE_INTERVAL_MS = 1000;

    // --------------------------------------------------------
    // Day names
    // --------------------------------------------------------

    const char* DAY_NAMES[] =
    {
        "SUN",
        "MON",
        "TUE",
        "WED",
        "THU",
        "FRI",
        "SAT"
    };
}


// ============================================================
// CONSTRUCTOR
// ============================================================

ScreenManager::ScreenManager(
    ClockSystem& clock,
    SensorManager& sensors,
    LVGLManager& lvgl
)
    : _clock(clock),
      _sensors(sensors),
      _lvgl(lvgl),
      _initialized(false),
      _lastUpdate(0)
{
}


// ============================================================
// BEGIN
// ============================================================

bool ScreenManager::begin()
{
    if (_initialized)
        return true;

    // --------------------------------------------------------
    // Проверяем LVGL
    // --------------------------------------------------------

    if (!_lvgl.isReady())
        return false;


    // ========================================================
    // CREATE FOUR CLOCK SCREENS
    // ========================================================

    for (uint8_t i = 0; i < SCREEN_COUNT; i++)
    {
        lv_display_t* display =
            _lvgl.display(i);

        if (display == nullptr)
        {
            return false;
        }

        if (!_screens[i].begin(display))
        {
            return false;
        }
    }


    // ========================================================
    // CENTER MODES
    // ========================================================

    // --------------------------------------------------------
    // Screen 0
    //
    // Hour tens
    //
    // Example:
    // 14:25
    // screen 0 = 1
    // --------------------------------------------------------

    _screens[0].setCenterMode(
        CenterMode::ONE_DIGIT
    );


    // --------------------------------------------------------
    // Screen 1
    //
    // Hour:
    //
    // 1
    // 4
    // --------------------------------------------------------

    _screens[1].setCenterMode(
        CenterMode::TWO_DIGITS_VERTICAL
    );


    // --------------------------------------------------------
    // Screen 2
    //
    // Minute tens
    //
    // Example:
    // 14:25
    // screen 2 = 2
    // --------------------------------------------------------

    _screens[2].setCenterMode(
        CenterMode::ONE_DIGIT
    );


    // --------------------------------------------------------
    // Screen 3
    //
    // Minute ones
    //
    // Example:
    // 14:25
    // screen 3 = 5
    // --------------------------------------------------------

    _screens[3].setCenterMode(
        CenterMode::ONE_DIGIT
    );


    // ========================================================
    // INITIAL STATE
    // ========================================================

    for (uint8_t i = 0; i < SCREEN_COUNT; i++)
    {
        _screens[i].setVisible(true);
    }


    // ========================================================
    // INITIAL DATA
    // ========================================================

    updateClock();

    updateSensors();

    updateDate();


    // ========================================================
    // READY
    // ========================================================

    _lastUpdate = millis();

    _initialized = true;

    // --------------------------------------------------------
    // После _initialized=true обновляем ещё раз,
    // чтобы ClockScreen применил данные через свои методы.
    // --------------------------------------------------------

    updateClock();

    updateSensors();

    updateDate();

    return true;
}


// ============================================================
// UPDATE
// ============================================================

void ScreenManager::update()
{
    if (!_initialized)
        return;

    const uint32_t now = millis();

    if (now - _lastUpdate < UPDATE_INTERVAL_MS)
        return;

    _lastUpdate = now;


    // ========================================================
    // CLOCK
    // ========================================================

    updateClock();


    // ========================================================
    // SENSORS
    // ========================================================

    updateSensors();


    // ========================================================
    // DATE
    // ========================================================

    updateDate();
}


// ============================================================
// UPDATE CLOCK
// ============================================================

void ScreenManager::updateClock()
{
    if (!_initialized && _screens[0].isVisible() == false)
    {
        // Ничего не делаем.
        // Эта проверка не обязательна, но защищает от
        // случайного вызова до инициализации.
    }

    // ========================================================
    // GET TIME DIGITS
    // ========================================================

    const uint8_t hourTens =
        _clock.getHourTens();

    const uint8_t hourOnes =
        _clock.getHourOnes();

    const uint8_t minuteTens =
        _clock.getMinuteTens();

    const uint8_t minuteOnes =
        _clock.getMinuteOnes();


    // ========================================================
    // SCREEN 0
    // ========================================================

    _screens[0].setCenterText(
        hourTens
    );


    // ========================================================
    // SCREEN 1
    // ========================================================

    _screens[1].setCenterText(
        hourTens,
        hourOnes
    );


    // ========================================================
    // SCREEN 2
    // ========================================================

    _screens[2].setCenterText(
        minuteTens
    );


    // ========================================================
    // SCREEN 3
    // ========================================================

    _screens[3].setCenterText(
        minuteOnes
    );
}


// ============================================================
// UPDATE SENSORS
// ============================================================

void ScreenManager::updateSensors()
{
    // ========================================================
    // TEMPERATURE
    // ========================================================

    String temperature =
        _sensors.getTemperatureC();

    if (temperature == "--")
    {
        _screens[0].setBottomText(
            "--"
        );
    }
    else
    {
        String text =
            temperature + "°C";

        _screens[0].setBottomText(
            text.c_str()
        );
    }


    // ========================================================
    // HUMIDITY
    // ========================================================

    String humidity =
        _sensors.getHumidity();

    if (humidity == "--")
    {
        _screens[1].setBottomText(
            "--"
        );
    }
    else
    {
        String text =
            humidity + "%";

        _screens[1].setBottomText(
            text.c_str()
        );
    }


    // ========================================================
    // LIGHT
    // ========================================================

    String light =
        _sensors.getLight();

    if (light == "--")
    {
        _screens[2].setBottomText(
            "--"
        );
    }
    else
    {
        String text =
            light + "lx";

        _screens[2].setBottomText(
            text.c_str()
        );
    }
}


// ============================================================
// UPDATE DATE
// ============================================================

void ScreenManager::updateDate()
{
    // ========================================================
    // DATE
    // ========================================================

    const uint8_t day =
        _clock.day();

    const uint8_t month =
        _clock.month();

    const uint16_t year =
        _clock.year();


    // ========================================================
    // DAY OF WEEK
    // ========================================================

    Constants::DayOfWeek dayOfWeek =
        _clock.getDayOfWeek();


    uint8_t dayIndex =
        static_cast<uint8_t>(dayOfWeek);


    if (dayIndex > 6)
        dayIndex = 0;


    const char* dayName =
        DAY_NAMES[dayIndex];


    // ========================================================
    // FORMAT
    //
    // Example:
    //
    // MON 16.09.2026
    // ========================================================

    char dateText[32];

    snprintf(
        dateText,
        sizeof(dateText),
        "%s %02u.%02u.%04u",
        dayName,
        day,
        month,
        year
    );


    // ========================================================
    // SCREEN 3
    // ========================================================

    _screens[3].setBottomText(
        dateText
    );
}


// ============================================================
// SET CENTER MODE
// ============================================================

void ScreenManager::setCenterMode(
    uint8_t screen,
    CenterMode mode
)
{
    if (screen >= SCREEN_COUNT)
        return;

    _screens[screen].setCenterMode(
        mode
    );
}


// ============================================================
// SET CENTER TEXT
// ============================================================

void ScreenManager::setCenterText(
    uint8_t screen,
    const char* text
)
{
    if (screen >= SCREEN_COUNT)
        return;

    _screens[screen].setCenterText(
        text
    );
}


// ============================================================
// SET CENTER DIGIT
// ============================================================

void ScreenManager::setCenterText(
    uint8_t screen,
    uint8_t digit
)
{
    if (screen >= SCREEN_COUNT)
        return;

    _screens[screen].setCenterText(
        digit
    );
}


// ============================================================
// SET CENTER TWO DIGITS
// ============================================================

void ScreenManager::setCenterText(
    uint8_t screen,
    uint8_t topDigit,
    uint8_t bottomDigit
)
{
    if (screen >= SCREEN_COUNT)
        return;

    _screens[screen].setCenterText(
        topDigit,
        bottomDigit
    );
}


// ============================================================
// SET TOP TEXT
// ============================================================

void ScreenManager::setTopText(
    uint8_t screen,
    const char* text
)
{
    if (screen >= SCREEN_COUNT)
        return;

    _screens[screen].setTopText(
        text
    );
}


// ============================================================
// SET BOTTOM TEXT
// ============================================================

void ScreenManager::setBottomText(
    uint8_t screen,
    const char* text
)
{
    if (screen >= SCREEN_COUNT)
        return;

    _screens[screen].setBottomText(
        text
    );
}


// ============================================================
// SET VISIBLE
// ============================================================

void ScreenManager::setVisible(
    uint8_t screen,
    bool visible
)
{
    if (screen >= SCREEN_COUNT)
        return;

    _screens[screen].setVisible(
        visible
    );
}


// ============================================================
// SCREEN ACCESS
// ============================================================

ClockScreen& ScreenManager::screen(
    uint8_t index
)
{
    // --------------------------------------------------------
    // Для корректного использования index должен быть 0..3.
    //
    // При неправильном индексе возвращаем screen 0,
    // чтобы не получить выход за границы массива.
    // --------------------------------------------------------

    if (index >= SCREEN_COUNT)
        index = 0;

    return _screens[index];
}


// ============================================================
// IS READY
// ============================================================

bool ScreenManager::isReady() const
{
    return _initialized;
}

