#pragma once

#include <Arduino.h>
#include <RTClib.h>
#include <time.h>

#include "Constants.h"
#include "Settings.h"

#include "hardware/rtc/RTC.h"
#include "managers/ClockManager.h"

class ClockSystem
{
public:
    explicit ClockSystem(
        const Settings::Clock& settings
    );

    // ========================================================
    // SYSTEM
    // ========================================================

    bool begin();

    void update();

    // ========================================================
    // NTP
    // ========================================================

    bool syncFromNTP(
        const char* ntpServer = "pool.ntp.org",
        uint8_t maxAttempts = 10
    );

    // ========================================================
    // DATE + TIME
    // ========================================================

    bool setLocalDateTime(
        uint16_t year,
        uint8_t month,
        uint8_t day,
        uint8_t hour,
        uint8_t minute,
        uint8_t second
    );

    bool setUTCDateTime(
        uint16_t year,
        uint8_t month,
        uint8_t day,
        uint8_t hour,
        uint8_t minute,
        uint8_t second
    );

    // ========================================================
    // TIME
    // ========================================================

    bool setLocalTime(
        uint8_t hour,
        uint8_t minute,
        uint8_t second = 0
    );

    bool setUTCTime(
        uint8_t hour,
        uint8_t minute,
        uint8_t second = 0
    );

    // ========================================================
    // DATE
    // ========================================================

    bool setLocalDate(
        uint16_t year,
        uint8_t month,
        uint8_t day
    );

    bool setUTCDate(
        uint16_t year,
        uint8_t month,
        uint8_t day
    );

    // ========================================================
    // CURRENT TIME
    // ========================================================

    uint8_t hour() const;
    uint8_t minute() const;
    uint8_t second() const;

    // ========================================================
    // TIME DIGITS
    // ========================================================

    uint8_t getHourTens() const;
    uint8_t getHourOnes() const;

    uint8_t getMinuteTens() const;
    uint8_t getMinuteOnes() const;

    // ========================================================
    // DATE
    // ========================================================

    uint8_t day() const;
    uint8_t month() const;
    uint16_t year() const;

    Constants::DayOfWeek getDayOfWeek() const;

    // ========================================================
    // TIMESTAMP
    // ========================================================

    time_t getUTCTime() const;
    time_t getLocalTime() const;

    // ========================================================
    // RTC
    // ========================================================

    DateTime getRTCDateTime();

    RTC& getRTC();

    // ========================================================
    // STATUS
    // ========================================================

    bool isTimeValid() const;

private:
    time_t localToUTC(
        time_t local
    ) const;

    time_t utcToLocal(
        time_t utc
    ) const;

    void updateManager();

private:
    RTC _rtc;

    ClockManager _clockManager;

    const Settings::Clock& _settings;
};