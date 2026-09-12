#pragma once

#include <Arduino.h>
#include <time.h>

#include "Constants.h"
#include "Settings.h"
#include "hardware/rtc/RTC.h"

class ClockManager
{
public:
    ClockManager(
        RTC& rtc,
        const Settings::Clock& settings
    );

    bool begin();

    void update();

    // ========================================================
    // TIME
    // ========================================================

    uint8_t hour() const;
    uint8_t minute() const;
    uint8_t second() const;

    // ========================================================
    // TIME DIGITS
    // ========================================================

    uint8_t hour1() const;
    uint8_t hour2() const;

    uint8_t minute1() const;
    uint8_t minute2() const;

    uint8_t second1() const;
    uint8_t second2() const;

    // ========================================================
    // DATE
    // ========================================================

    uint8_t day() const;
    uint8_t month() const;
    uint16_t year() const;

    Constants::DayOfWeek dayOfWeek() const;

    // ========================================================
    // TIMESTAMP
    // ========================================================

    time_t utcTime() const;
    time_t localTime() const;

    // ========================================================
    // RTC
    // ========================================================

    RTC& rtc();

    // ========================================================
    // STATUS
    // ========================================================

    bool isValid() const;

private:
    time_t calculateLocalTime(
        time_t utc
    ) const;

    void updateDateTime(
        time_t localTime
    );

private:
    RTC& _rtc;

    const Settings::Clock& _settings;

    time_t _utcTime = 0;
    time_t _localTime = 0;

    uint8_t _hour = 0;
    uint8_t _minute = 0;
    uint8_t _second = 0;

    uint8_t _day = 1;
    uint8_t _month = 1;
    uint16_t _year = 2000;

    Constants::DayOfWeek _dayOfWeek =
        Constants::DayOfWeek::Sunday;

    bool _valid = false;
};