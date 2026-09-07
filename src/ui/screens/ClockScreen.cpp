#include "ui/screens/ClockScreen.h"

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

namespace
{
    constexpr uint16_t COLOR_BLACK = ST77XX_BLACK;
    constexpr uint16_t COLOR_WHITE = ST77XX_WHITE;
    constexpr uint16_t COLOR_ACCENT = 0x05FF;
    constexpr uint16_t COLOR_ACCENT_DIM = 0x0330;
    constexpr uint16_t COLOR_MUTED = 0x7BEF;
}

ClockScreen::ClockScreen(DisplayManager& display)
    : _display(display)
{
}

void ClockScreen::setTime(
    uint8_t hour1,
    uint8_t hour2,
    uint8_t minute1,
    uint8_t minute2
)
{
    _hour1 = hour1 % 10;
    _hour2 = hour2 % 10;
    _minute1 = minute1 % 10;
    _minute2 = minute2 % 10;
}

void ClockScreen::setDate(
    const char* day,
    uint8_t date,
    uint8_t month,
    uint16_t year
)
{
    if (day)
    {
        strncpy(_day, day, sizeof(_day) - 1);
        _day[sizeof(_day) - 1] = '\0';
    }

    _date = date;
    _month = month;
    _year = year;
}

void ClockScreen::setSensors(
    const char* temperature,
    const char* humidity,
    const char* light,
    const char* battery
)
{
    if (temperature)
    {
        strncpy(_temperature, temperature, sizeof(_temperature) - 1);
        _temperature[sizeof(_temperature) - 1] = '\0';
    }

    if (humidity)
    {
        strncpy(_humidity, humidity, sizeof(_humidity) - 1);
        _humidity[sizeof(_humidity) - 1] = '\0';
    }

    if (light)
    {
        strncpy(_light, light, sizeof(_light) - 1);
        _light[sizeof(_light) - 1] = '\0';
    }

    if (battery)
    {
        strncpy(_battery, battery, sizeof(_battery) - 1);
        _battery[sizeof(_battery) - 1] = '\0';
    }
}

void ClockScreen::notify(const char* message)
{
    if (!message)
    {
        _notification[0] = '\0';
        _hasNotification = false;
        return;
    }

    strncpy(_notification, message, sizeof(_notification) - 1);
    _notification[sizeof(_notification) - 1] = '\0';
    _hasNotification = true;
}

void ClockScreen::draw()
{
    char date[12];
    snprintf(date, sizeof(date), "%02u.%02u", _date, _month);

    drawDigitPanel(0, _hour1, "HOUR", date, false);
    drawDigitPanel(1, _hour2, "HOUR", _day[0] ? _day : "--", true);
    drawDigitPanel(2, _minute1, "MIN", _temperature[0] ? _temperature : "--", false);
    drawDigitPanel(3, _minute2, "MIN", _humidity[0] ? _humidity : "--", false);

    if (_hasNotification)
    {
        drawNotification();
    }
}

void ClockScreen::drawDigitPanel(
    uint8_t index,
    uint8_t digit,
    const char* title,
    const char* footer,
    bool drawSeparator
)
{
    ST7789_172x320& tft = _display.get(index);
    const char digitText[] = {static_cast<char>('0' + digit), '\0'};

    tft.fillScreen(COLOR_BLACK);
    tft.fillRect(0, 0, 172, 4, COLOR_ACCENT);
    tft.drawFastHLine(12, 54, 148, COLOR_ACCENT_DIM);
    tft.drawFastHLine(12, 252, 148, COLOR_ACCENT_DIM);

    tft.setFont(&FreeMono9pt7b);
    tft.setTextSize(1);
    tft.setTextColor(COLOR_MUTED);
    tft.setCursor(14, 34);
    tft.print(title);

    tft.setFont(&FreeMonoBold24pt7b);
    tft.setTextSize(4);
    tft.setTextColor(COLOR_WHITE);
    tft.setCursor(25, 214);
    tft.print(digitText);
    tft.setTextSize(1);

    if (drawSeparator)
    {
        tft.fillCircle(158, 128, 5, COLOR_ACCENT);
        tft.fillCircle(158, 184, 5, COLOR_ACCENT);
    }

    tft.setFont(&FreeMono9pt7b);
    tft.setTextColor(COLOR_MUTED);
    tft.setCursor(14, 292);
    tft.print(footer);
}

void ClockScreen::drawNotification()
{
    _display.fillRect(20, 115, 648, 90, COLOR_BLACK);
    _display.drawRect(20, 115, 648, 90, COLOR_ACCENT);
    _display.drawText(
        _notification,
        35,
        170,
        COLOR_WHITE,
        &FreeMonoBold18pt7b
    );
}
