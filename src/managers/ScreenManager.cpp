#include "ScreenManager.h"


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
    if (
        _initialized
    )
    {
        return true;
    }


    Serial.println(
        "ScreenManager::begin()"
    );


    // ========================================================
    // CREATE FOUR CLOCK SCREENS
    // ========================================================

    for (
        uint8_t i = 0;
        i < SCREEN_COUNT;
        ++i
    )
    {
        lv_display_t* display =
            _lvgl.display(i);


        if (
            display == nullptr
        )
        {
            Serial.print(
                "Screen "
            );

            Serial.print(
                i
            );

            Serial.println(
                ": display NULL"
            );

            return false;
        }


        _screens[i].begin(
            display
        );


        Serial.print(
            "ClockScreen "
        );

        Serial.print(
            i
        );

        Serial.println(
            ": OK"
        );
    }


    // ========================================================
    // CENTER MODES
    // ========================================================

    // TFT0:
    // одна цифра

    setCenterMode(
        0,
        ClockScreen::CenterMode::ONE_DIGIT
    );


    // TFT1:
    // две цифры вертикально

    setCenterMode(
        1,
        ClockScreen::CenterMode::TWO_DIGITS_VERTICAL
    );


    // TFT2:
    // одна цифра

    setCenterMode(
        2,
        ClockScreen::CenterMode::ONE_DIGIT
    );


    // TFT3:
    // одна цифра

    setCenterMode(
        3,
        ClockScreen::CenterMode::ONE_DIGIT
    );


    // ========================================================
    // STATE
    // ========================================================

    _lastUpdate =
        0;


    _initialized =
        true;


    // ========================================================
    // INITIAL DATA
    // ========================================================

    updateClock();

    updateSensors();

    updateDate();


    Serial.println(
        "ScreenManager: READY"
    );


    return true;
}


// ============================================================
// UPDATE
// ============================================================

void ScreenManager::update()
{
    if (
        !_initialized
    )
    {
        return;
    }


    const uint32_t now =
        millis();


    // ========================================================
    // UPDATE EVERY SECOND
    // ========================================================

    if (
        now - _lastUpdate < 1000
    )
    {
        return;
    }


    _lastUpdate =
        now;


    updateClock();

    updateSensors();

    updateDate();
}


// ============================================================
// CLOCK
// ============================================================

void ScreenManager::updateClock()
{
    // ========================================================
    // CURRENT TIME
    // ========================================================
    //
    // Например:
    //
    // 14:25
    //
    // hour tens    = 1
    // hour ones    = 4
    // minute tens  = 2
    // minute ones  = 5
    //
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

    setCenterText(
        0,
        hourTens
    );


    // ========================================================
    // SCREEN 1
    //
    //      1
    //      4
    // ========================================================

    setCenterText(
        1,
        hourTens,
        hourOnes
    );


    // ========================================================
    // SCREEN 2
    // ========================================================

    setCenterText(
        2,
        minuteTens
    );


    // ========================================================
    // SCREEN 3
    // ========================================================

    setCenterText(
        3,
        minuteOnes
    );
}


// ============================================================
// SENSORS
// ============================================================

void ScreenManager::updateSensors()
{
    // ========================================================
    // TEMPERATURE
    // ========================================================

    String temperature =
        _sensors.getTemperatureC();


    String temperatureText =
        temperature +
        "°C";


    setBottomText(
        0,
        temperatureText.c_str()
    );


    // ========================================================
    // HUMIDITY
    // ========================================================

    String humidity =
        _sensors.getHumidity();


    String humidityText =
        humidity +
        "%";


    setBottomText(
        1,
        humidityText.c_str()
    );


    // ========================================================
    // LIGHT
    // ========================================================

    String light =
        _sensors.getLight();


    String lightText =
        light +
        "lx";


    setBottomText(
        2,
        lightText.c_str()
    );
}


// ============================================================
// DATE
// ============================================================

void ScreenManager::updateDate()
{
    const char* dayName =
        "---";


    switch (
        _clock.getDayOfWeek()
    )
    {
        case Constants::DayOfWeek::Sunday:

            dayName = "SUN";

            break;


        case Constants::DayOfWeek::Monday:

            dayName = "MON";

            break;


        case Constants::DayOfWeek::Tuesday:

            dayName = "TUE";

            break;


        case Constants::DayOfWeek::Wednesday:

            dayName = "WED";

            break;


        case Constants::DayOfWeek::Thursday:

            dayName = "THU";

            break;


        case Constants::DayOfWeek::Friday:

            dayName = "FRI";

            break;


        case Constants::DayOfWeek::Saturday:

            dayName = "SAT";

            break;


        default:

            dayName = "---";

            break;
    }


    char dateText[64];


    snprintf(
        dateText,
        sizeof(dateText),

        "%s %02u.%02u.%04u",

        dayName,

        _clock.day(),

        _clock.month(),

        _clock.year()
    );


    setBottomText(
        3,
        dateText
    );
}


// ============================================================
// SET CENTER MODE
// ============================================================

void ScreenManager::setCenterMode(
    uint8_t screen,
    ClockScreen::CenterMode mode
)
{
    if (
        screen >= SCREEN_COUNT
    )
    {
        return;
    }


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
    if (
        screen >= SCREEN_COUNT
    )
    {
        return;
    }


    _screens[screen].setCenterText(
        text
    );
}


// ============================================================
// ONE DIGIT
// ============================================================

void ScreenManager::setCenterText(
    uint8_t screen,
    uint8_t digit
)
{
    if (
        screen >= SCREEN_COUNT
    )
    {
        return;
    }


    _screens[screen].setCenterText(
        digit
    );
}


// ============================================================
// TWO DIGITS
// ============================================================

void ScreenManager::setCenterText(
    uint8_t screen,
    uint8_t topDigit,
    uint8_t bottomDigit
)
{
    if (
        screen >= SCREEN_COUNT
    )
    {
        return;
    }


    _screens[screen].setCenterText(
        topDigit,
        bottomDigit
    );
}


// ============================================================
// TOP TEXT
// ============================================================

void ScreenManager::setTopText(
    uint8_t screen,
    const char* text
)
{
    if (
        screen >= SCREEN_COUNT
    )
    {
        return;
    }


    _screens[screen].setTopText(
        text
    );
}


// ============================================================
// BOTTOM TEXT
// ============================================================

void ScreenManager::setBottomText(
    uint8_t screen,
    const char* text
)
{
    if (
        screen >= SCREEN_COUNT
    )
    {
        return;
    }


    _screens[screen].setBottomText(
        text
    );
}


// ============================================================
// VISIBLE
// ============================================================

void ScreenManager::setVisible(
    uint8_t screen,
    bool visible
)
{
    if (
        screen >= SCREEN_COUNT
    )
    {
        return;
    }


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
    return _screens[index];
}


// ============================================================
// READY
// ============================================================

bool ScreenManager::isReady() const
{
    return _initialized;
}
