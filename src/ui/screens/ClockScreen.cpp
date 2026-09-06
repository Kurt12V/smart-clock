#include "./ui/screens/ClockScreen.h"

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>

ClockScreen::ClockScreen(DisplayManager& display)
    : _display(display)
{
}

// ============================================================
// TIME
// ============================================================

void ClockScreen::setTime(
    uint8_t hour1,
    uint8_t hour2,
    uint8_t minute1,
    uint8_t minute2
)
{
    _hour1 = hour1;
    _hour2 = hour2;
    _minute1 = minute1;
    _minute2 = minute2;
}

// ============================================================
// DATE
// ============================================================

void ClockScreen::setDate(
    const char* day,
    uint8_t date,
    uint8_t month,
    uint16_t year
)
{
    if (day != nullptr)
    {
        strncpy(
            _day,
            day,
            sizeof(_day) - 1
        );

        _day[sizeof(_day) - 1] = '\0';
    }

    _date = date;
    _month = month;
    _year = year;
}

// ============================================================
// SENSORS
// ============================================================

void ClockScreen::setSensors(
    const char* temperature,
    const char* humidity,
    const char* light,
    const char* battery
)
{
    if (temperature != nullptr)
    {
        strncpy(
            _temperature,
            temperature,
            sizeof(_temperature) - 1
        );

        _temperature[sizeof(_temperature) - 1] = '\0';
    }

    if (humidity != nullptr)
    {
        strncpy(
            _humidity,
            humidity,
            sizeof(_humidity) - 1
        );

        _humidity[sizeof(_humidity) - 1] = '\0';
    }

    if (light != nullptr)
    {
        strncpy(
            _light,
            light,
            sizeof(_light) - 1
        );

        _light[sizeof(_light) - 1] = '\0';
    }

    if (battery != nullptr)
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
    if (message == nullptr)
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
    auto& tft = _display.get(0);

    tft.fillScreen(0x0000);

    // Верхняя полоска
    tft.fillRect(
        0,
        -6,
        175,
        19,
        0xFFFF
    );

    // Нижняя полоска
    tft.fillRect(
        0,
        304,
        175,
        22,
        0xFFFF
    );

    // Основное значение
    tft.setTextColor(0xE8EC);
    tft.setTextSize(7);
    tft.setFreeFont(&FreeMono24pt7b);

    tft.drawString(
        _temperature,
        -12,
        45
    );

    // Нижние данные
    tft.setTextColor(0x9936);
    tft.setTextSize(1);
    tft.setFreeFont(&FreeMonoBold9pt7b);

    char text[64];

    snprintf(
        text,
        sizeof(text),
        "%slx %sC %s%%",
        _light,
        _temperature,
        _humidity
    );

    tft.drawString(
        text,
        20,
        305
    );
}

// ============================================================
// SCREEN 2
// HUMIDITY
// ============================================================

void ClockScreen::drawScreen2()
{
    auto& tft = _display.get(1);

    tft.fillScreen(0x0000);

    tft.fillRect(
        0,
        -6,
        175,
        19,
        0xFFFF
    );

    tft.fillRect(
        0,
        304,
        175,
        22,
        0xFFFF
    );

    tft.setTextColor(0xE8EC);
    tft.setTextSize(7);
    tft.setFreeFont(&FreeMono24pt7b);

    tft.drawString(
        _humidity,
        -12,
        45
    );

    tft.setTextColor(0x9936);
    tft.setTextSize(1);
    tft.setFreeFont(&FreeMonoBold9pt7b);

    char text[64];

    snprintf(
        text,
        sizeof(text),
        "%sC %slx",
        _temperature,
        _light
    );

    tft.drawString(
        text,
        20,
        305
    );
}

// ============================================================
// SCREEN 3
// LIGHT
// ============================================================

void ClockScreen::drawScreen3()
{
    auto& tft = _display.get(2);

    tft.fillScreen(0x0000);

    tft.fillRect(
        0,
        -6,
        175,
        19,
        0xFFFF
    );

    tft.fillRect(
        0,
        304,
        175,
        22,
        0xFFFF
    );

    tft.setTextColor(0xE8EC);
    tft.setTextSize(7);
    tft.setFont(&FreeMono24pt7b);

    tft.drawString(
        _light,
        -12,
        45
    );

    tft.setTextColor(0x9936);
    tft.setTextSize(1);
    tft.setFreeFont(&FreeMonoBold9pt7b);

    char text[64];

    snprintf(
        text,
        sizeof(text),
        "%sC %s%%",
        _temperature,
        _humidity
    );

    tft.drawString(
        text,
        20,
        305
    );
}

// ============================================================
// SCREEN 4
// BATTERY
// ============================================================

void ClockScreen::drawScreen4()
{
    auto& tft = _display.get(3);

    tft.fillScreen(0x0000);

    tft.fillRect(
        0,
        -6,
        175,
        19,
        0xFFFF
    );

    tft.fillRect(
        0,
        304,
        175,
        22,
        0xFFFF
    );

    tft.setTextColor(0xE8EC);
    tft.setTextSize(7);
    tft.setFreeFont(&FreeMono24pt7b);

    tft.drawString(
        _battery,
        -12,
        45
    );

    tft.setTextColor(0x9936);
    tft.setTextSize(1);
    tft.setFreeFont(&FreeMonoBold9pt7b);

    char text[64];

    snprintf(
        text,
        sizeof(text),
        "%slx %s%%",
        _light,
        _humidity
    );

    tft.drawString(
        text,
        20,
        305
    );
}

// ============================================================
// NOTIFICATION
// ============================================================

void ClockScreen::drawNotification()
{
    // Здесь позже сделаем адаптивное уведомление:
    //
    // короткий текст  -> одна строка
    // длинный текст   -> несколько строк
    // очень длинный   -> автоматический перенос
    //
    // Размер области и шрифт будут подбираться автоматически.

    auto& tft = _display.get(0);

    tft.setTextColor(0xFFFF);
    tft.setTextSize(2);
    tft.setFreeFont(nullptr);

    tft.drawString(
        _notification,
        10,
        140
    );
}