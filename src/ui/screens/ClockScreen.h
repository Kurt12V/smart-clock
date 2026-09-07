#pragma once

#include <Arduino.h>
#include "managers/DisplayManager.h"

class ClockScreen
{
public:
    explicit ClockScreen(DisplayManager& display);

    void setTime(uint8_t hour1, uint8_t hour2, uint8_t minute1, uint8_t minute2);
    void setDate(const char* day, uint8_t date, uint8_t month, uint16_t year);
    void setSensors(
        const char* temperature,
        const char* humidity,
        const char* light,
        const char* battery
    );
    void notify(const char* message);
    void draw();

private:
    DisplayManager& _display;

    uint8_t _hour1 = 0;
    uint8_t _hour2 = 0;
    uint8_t _minute1 = 0;
    uint8_t _minute2 = 0;

    char _day[12] = {};
    uint8_t _date = 1;
    uint8_t _month = 1;
    uint16_t _year = 2026;

    char _temperature[16] = {};
    char _humidity[16] = {};
    char _light[16] = {};
    char _battery[16] = {};

    char _notification[96] = {};
    bool _hasNotification = false;

    void drawDigitPanel(
        uint8_t index,
        uint8_t digit,
        const char* title,
        const char* footer,
        bool drawSeparator
    );
    void drawNotification();
};
