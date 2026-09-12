#include "ClockSystem.h"

#include <WiFi.h>
#include <time.h>

// ============================================================
// CONSTRUCTOR
// ============================================================

ClockSystem::ClockSystem(
    const Settings::Clock& settings
)
    : _rtc(),
      _clockManager(_rtc, settings),
      _settings(settings)
{
}

// ============================================================
// BEGIN
// ============================================================

bool ClockSystem::begin()
{
    return _clockManager.begin();
}

// ============================================================
// UPDATE
// ============================================================

void ClockSystem::update()
{
    _clockManager.update();
}

// ============================================================
// NTP SYNC
// ============================================================

bool ClockSystem::syncFromNTP(
    const char* ntpServer,
    uint8_t maxAttempts
)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    // Получаем именно UTC
    configTime(
        0,
        0,
        ntpServer
    );

    struct tm timeInfo;

    for (
        uint8_t attempt = 0;
        attempt < maxAttempts;
        ++attempt
    )
    {
        // Важно:
        // ::getLocalTime — функция Arduino,
        // а не ClockSystem::getLocalTime()
        if (::getLocalTime(
                &timeInfo,
                1000
            ))
        {
            DateTime utc(
                timeInfo.tm_year + 1900,
                timeInfo.tm_mon + 1,
                timeInfo.tm_mday,
                timeInfo.tm_hour,
                timeInfo.tm_min,
                timeInfo.tm_sec
            );

            _rtc.setDateTime(
                utc.year(),
                utc.month(),
                utc.day(),
                utc.hour(),
                utc.minute(),
                utc.second()
            );

            updateManager();

            return true;
        }

        delay(100);
    }

    return false;
}

// ============================================================
// SET LOCAL DATE + TIME
// ============================================================

bool ClockSystem::setLocalDateTime(
    uint16_t year,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second
)
{
    DateTime local(
        year,
        month,
        day,
        hour,
        minute,
        second
    );

    const time_t utc =
        localToUTC(
            local.unixtime()
        );

    _rtc.setDateTime(utc);

    updateManager();

    return true;
}

// ============================================================
// SET UTC DATE + TIME
// ============================================================

bool ClockSystem::setUTCDateTime(
    uint16_t year,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second
)
{
    _rtc.setDateTime(
        year,
        month,
        day,
        hour,
        minute,
        second
    );

    updateManager();

    return true;
}

// ============================================================
// SET LOCAL TIME
// ============================================================

bool ClockSystem::setLocalTime(
    uint8_t hour,
    uint8_t minute,
    uint8_t second
)
{
    // Берём именно локальное время
    // чтобы корректно работать около полуночи.
    DateTime currentLocal(
        getLocalTime()
    );

    DateTime local(
        currentLocal.year(),
        currentLocal.month(),
        currentLocal.day(),
        hour,
        minute,
        second
    );

    const time_t utc =
        localToUTC(
            local.unixtime()
        );

    _rtc.setDateTime(utc);

    updateManager();

    return true;
}

// ============================================================
// SET UTC TIME
// ============================================================

bool ClockSystem::setUTCTime(
    uint8_t hour,
    uint8_t minute,
    uint8_t second
)
{
    const DateTime currentUTC =
        _rtc.getDateTime();

    _rtc.setDateTime(
        currentUTC.year(),
        currentUTC.month(),
        currentUTC.day(),
        hour,
        minute,
        second
    );

    updateManager();

    return true;
}

// ============================================================
// SET LOCAL DATE
// ============================================================

bool ClockSystem::setLocalDate(
    uint16_t year,
    uint8_t month,
    uint8_t day
)
{
    DateTime currentLocal(
        getLocalTime()
    );

    DateTime local(
        year,
        month,
        day,
        currentLocal.hour(),
        currentLocal.minute(),
        currentLocal.second()
    );

    const time_t utc =
        localToUTC(
            local.unixtime()
        );

    _rtc.setDateTime(utc);

    updateManager();

    return true;
}

// ============================================================
// SET UTC DATE
// ============================================================

bool ClockSystem::setUTCDate(
    uint16_t year,
    uint8_t month,
    uint8_t day
)
{
    const DateTime currentUTC =
        _rtc.getDateTime();

    _rtc.setDateTime(
        year,
        month,
        day,
        currentUTC.hour(),
        currentUTC.minute(),
        currentUTC.second()
    );

    updateManager();

    return true;
}

// ============================================================
// TIME
// ============================================================

uint8_t ClockSystem::hour() const
{
    return _clockManager.hour();
}

uint8_t ClockSystem::minute() const
{
    return _clockManager.minute();
}

uint8_t ClockSystem::second() const
{
    return _clockManager.second();
}

// ============================================================
// TIME DIGITS
// ============================================================

uint8_t ClockSystem::getHourTens() const
{
    return _clockManager.hour1();
}

uint8_t ClockSystem::getHourOnes() const
{
    return _clockManager.hour2();
}

uint8_t ClockSystem::getMinuteTens() const
{
    return _clockManager.minute1();
}

uint8_t ClockSystem::getMinuteOnes() const
{
    return _clockManager.minute2();
}

// ============================================================
// DATE
// ============================================================

uint8_t ClockSystem::day() const
{
    return _clockManager.day();
}

uint8_t ClockSystem::month() const
{
    return _clockManager.month();
}

uint16_t ClockSystem::year() const
{
    return _clockManager.year();
}

Constants::DayOfWeek ClockSystem::getDayOfWeek() const
{
    return _clockManager.dayOfWeek();
}

// ============================================================
// TIMESTAMP
// ============================================================

time_t ClockSystem::getUTCTime() const
{
    return _clockManager.utcTime();
}

time_t ClockSystem::getLocalTime() const
{
    return _clockManager.localTime();
}

// ============================================================
// RTC
// ============================================================

DateTime ClockSystem::getRTCDateTime()
{
    return _rtc.getDateTime();
}

RTC& ClockSystem::getRTC()
{
    return _rtc;
}

// ============================================================
// STATUS
// ============================================================

bool ClockSystem::isTimeValid() const
{
    return _clockManager.isValid();
}

// ============================================================
// LOCAL -> UTC
// ============================================================

time_t ClockSystem::localToUTC(
    time_t local
) const
{
    const int16_t offsetHours =
        static_cast<int16_t>(
            _settings.utcOffset
        );

    return local -
        (
            static_cast<time_t>(
                offsetHours
            ) * 3600
        );
}

// ============================================================
// UTC -> LOCAL
// ============================================================

time_t ClockSystem::utcToLocal(
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
// UPDATE MANAGER
// ============================================================

void ClockSystem::updateManager()
{
    _clockManager.update();
}