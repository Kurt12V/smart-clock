#include "managers/ClockManager.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

ClockManager::ClockManager(
    RTC& rtc,
    const Settings::Clock& settings
)
    : _rtc(rtc),
      _settings(settings)
{
}

// ============================================================
// BEGIN
// ============================================================

bool ClockManager::begin()
{
    if (!_rtc.begin())
    {
        _valid = false;
        return false;
    }

    // if (!_settings.useRTC)
    // {
    //     _valid = false;
    //     return true;
    // }

    update();

    return _valid;
}

// ============================================================
// UPDATE
// ============================================================

void ClockManager::update()
{
    // if (!_settings.useRTC)
    // {
    //     _valid = false;
    //     return;
    // }

    const DateTime now = _rtc.getDateTime();

    _utcTime = now.unixtime();

    _localTime =
        calculateLocalTime(_utcTime);

    updateDateTime(_localTime);

    _valid = true;
}

// ============================================================
// UTC -> LOCAL
// ============================================================

time_t ClockManager::calculateLocalTime(
    time_t utc
) const
{
    const int16_t offsetHours =
        static_cast<int16_t>(
            _settings.utcOffset
        );

    return utc +
        (
            static_cast<time_t>(
                offsetHours
            ) * 3600
        );
}

// ============================================================
// UPDATE DATE/TIME
// ============================================================

void ClockManager::updateDateTime(
    time_t localTime
)
{
    const DateTime local(localTime);

    _hour = local.hour();
    _minute = local.minute();
    _second = local.second();

    _day = local.day();
    _month = local.month();
    _year = local.year();

    _dayOfWeek =
        static_cast<Constants::DayOfWeek>(
            local.dayOfTheWeek()
        );
}

// ============================================================
// TIME
// ============================================================

uint8_t ClockManager::hour() const
{
    return _hour;
}

uint8_t ClockManager::minute() const
{
    return _minute;
}

uint8_t ClockManager::second() const
{
    return _second;
}

// ============================================================
// TIME DIGITS
// ============================================================

uint8_t ClockManager::hour1() const
{
    return _hour / 10;
}

uint8_t ClockManager::hour2() const
{
    return _hour % 10;
}

uint8_t ClockManager::minute1() const
{
    return _minute / 10;
}

uint8_t ClockManager::minute2() const
{
    return _minute % 10;
}

uint8_t ClockManager::second1() const
{
    return _second / 10;
}

uint8_t ClockManager::second2() const
{
    return _second % 10;
}

// ============================================================
// DATE
// ============================================================

uint8_t ClockManager::day() const
{
    return _day;
}

uint8_t ClockManager::month() const
{
    return _month;
}

uint16_t ClockManager::year() const
{
    return _year;
}

Constants::DayOfWeek ClockManager::dayOfWeek() const
{
    return _dayOfWeek;
}

// ============================================================
// TIMESTAMP
// ============================================================

time_t ClockManager::utcTime() const
{
    return _utcTime;
}

time_t ClockManager::localTime() const
{
    return _localTime;
}

// ============================================================
// RTC
// ============================================================

RTC& ClockManager::rtc()
{
    return _rtc;
}

// ============================================================
// STATUS
// ============================================================

bool ClockManager::isValid() const
{
    return _valid;
}