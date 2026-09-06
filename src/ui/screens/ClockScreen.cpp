#include "./ui/screens/ClockScreen.h"

#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMono9pt7b.h>

static constexpr uint16_t COLOR_BLACK = 0x0000;
static constexpr uint16_t COLOR_WHITE = 0xFFFF;

static constexpr uint16_t COLOR_VALUE = 0xE8EC;
static constexpr uint16_t COLOR_INFO  = 0x9936;


// ============================================================
// Constructor
// ============================================================

ClockScreen::ClockScreen(DisplayManager& display)
    : _display(display)
{
}


// ============================================================
// SET TIME
// ============================================================

void ClockScreen::setTime(
    uint8_t hour1,
    uint8_t hour2,
    uint8_t minute1,
    uint8_t minute2
)
{
    _hour1   = hour1;
    _hour2   = hour2;
    _minute1 = minute1;
    _minute2 = minute2;
}


// ============================================================
// SET DATE
// ============================================================

void ClockScreen::setDate(
    const char* day,
    uint8_t date,
    uint8_t month,
    uint16_t year
)
{
    if (day)
    {
        strncpy(
            _day,
            day,
            sizeof(_day) - 1
        );

        _day[sizeof(_day) - 1] = '\0';
    }

    _date  = date;
    _month = month;
    _year  = year;
}


// ============================================================
// SET SENSORS
// ============================================================

void ClockScreen::setSensors(
    const char* temperature,
    const char* humidity,
    const char* light,
    const char* battery
)
{
    if (temperature)
    {
        strncpy(
            _temperature,
            temperature,
            sizeof(_temperature) - 1
        );

        _temperature[sizeof(_temperature) - 1] = '\0';
    }

    if (humidity)
    {
        strncpy(
            _humidity,
            humidity,
            sizeof(_humidity) - 1
        );

        _humidity[sizeof(_humidity) - 1] = '\0';
    }

    if (light)
    {
        strncpy(
            _light,
            light,
            sizeof(_light) - 1
        );

        _light[sizeof(_light) - 1] = '\0';
    }

    if (battery)
    {
        strncpy(
            _battery,
            battery,
            sizeof(_battery) - 1
        );

        _battery[sizeof(_battery) - 1] = '\0';
    }
}


// ============================================================
// NOTIFICATION
// ============================================================

void ClockScreen::notify(
    const char* message
)
{
    if (!message)
    {
        _notification[0] = '\0';
        _hasNotification = false;
        return;
    }

    strncpy(
        _notification,
        message,
        sizeof(_notification) - 1
    );

    _notification[sizeof(_notification) - 1] = '\0';

    _hasNotification = true;
}


// ============================================================
// DRAW
// ============================================================

void ClockScreen::draw()
{
    drawScreen1();
    drawScreen2();
    drawScreen3();
    drawScreen4();

    if (_hasNotification)
    {
        drawNotification();
    }
}


// ============================================================
// SCREEN 1
// TEMPERATURE
// ============================================================

void ClockScreen::drawScreen1()
{
    ST7789_172x320& tft = _display.get(0);

    tft.fillScreen(COLOR_BLACK);

    // Верхняя белая полоса
    tft.fillRect(
        0,
        0,
        172,
        19,
        COLOR_WHITE
    );

    // Нижняя белая полоса
    tft.fillRect(
        0,
        304,
        172,
        16,
        COLOR_WHITE
    );

    // Большое значение
    tft.setFont(&FreeMonoBold24pt7b);
    tft.setTextColor(COLOR_VALUE);

    tft.setCursor(
        -12,
        95
    );

    tft.print(_temperature);

    // Нижняя информация
    tft.setFont(&FreeMono9pt7b);
    tft.setTextColor(COLOR_INFO);

    tft.setCursor(
        20,
        318
    );

    tft.print("TEMP");
}


// ============================================================
// SCREEN 2
// HUMIDITY
// ============================================================

void ClockScreen::drawScreen2()
{
    ST7789_172x320& tft = _display.get(1);

    tft.fillScreen(COLOR_BLACK);

    tft.fillRect(
        0,
        0,
        172,
        19,
        COLOR_WHITE
    );

    tft.fillRect(
        0,
        304,
        172,
        16,
        COLOR_WHITE
    );

    tft.setFont(&FreeMonoBold24pt7b);
    tft.setTextColor(COLOR_VALUE);

    tft.setCursor(
        -12,
        95
    );

    tft.print(_humidity);

    tft.setFont(&FreeMono9pt7b);
    tft.setTextColor(COLOR_INFO);

    tft.setCursor(
        20,
        318
    );

    tft.print("HUM");
}


// ============================================================
// SCREEN 3
// LIGHT
// ============================================================

void ClockScreen::drawScreen3()
{
    ST7789_172x320& tft = _display.get(2);

    tft.fillScreen(COLOR_BLACK);

    tft.fillRect(
        0,
        0,
        172,
        19,
        COLOR_WHITE
    );

    tft.fillRect(
        0,
        304,
        172,
        16,
        COLOR_WHITE
    );

    tft.setFont(&FreeMonoBold24pt7b);
    tft.setTextColor(COLOR_VALUE);

    tft.setCursor(
        -12,
        95
    );

    tft.print(_light);

    tft.setFont(&FreeMono9pt7b);
    tft.setTextColor(COLOR_INFO);

    tft.setCursor(
        20,
        318
    );

    tft.print("LIGHT");
}


// ============================================================
// SCREEN 4
// BATTERY
// ============================================================

void ClockScreen::drawScreen4()
{
    ST7789_172x320& tft = _display.get(3);

    tft.fillScreen(COLOR_BLACK);

    tft.fillRect(
        0,
        0,
        172,
        19,
        COLOR_WHITE
    );

    tft.fillRect(
        0,
        304,
        172,
        16,
        COLOR_WHITE
    );

    tft.setFont(&FreeMonoBold24pt7b);
    tft.setTextColor(COLOR_VALUE);

    tft.setCursor(
        -12,
        95
    );

    tft.print(_battery);

    tft.setFont(&FreeMono9pt7b);
    tft.setTextColor(COLOR_INFO);

    tft.setCursor(
        20,
        318
    );

    tft.print("BAT");
}


// ============================================================
// NOTIFICATION
// ============================================================

void ClockScreen::drawNotification()
{
    // Уведомление выводим поверх всех 4 экранов
    // как единую область 688x320.

    // Затемнение / фон
    _display.fillRect(
        20,
        115,
        648,
        90,
        COLOR_BLACK
    );

    _display.drawRect(
        20,
        115,
        648,
        90,
        COLOR_WHITE
    );

    _display.drawText(
        _notification,
        35,
        170,
        COLOR_WHITE,
        &FreeMonoBold18pt7b
    );
}