#include "ClockManager.h"

ClockManager::ClockManager(RTC& rtcModule)
: _rtc(rtcModule) {
}

bool ClockManager::begin(const char* timeZone) {

```
_isInitialized = true;

Serial.println("[ClockManager] Initializing...");

return setTimeZone(timeZone);
```

}

// ========================================
// TIMEZONE
// ========================================

bool ClockManager::setTimeZone(
const char* timeZone
) {

```
if (!_isInitialized) {

    Serial.println(
        "[ClockManager] ERROR: Not initialized"
    );

    return false;
}


_timeZone = timeZone;


configTzTime(
    _timeZone.c_str(),
    "pool.ntp.org"
);


Serial.printf(
    "[ClockManager] Timezone: %s\n",
    _timeZone.c_str()
);


return true;
```

}

// ========================================
// NTP SYNC
// ========================================

bool ClockManager::syncFromNTP(
const char* ntpServer,
uint8_t maxAttempts
) {

```
if (!_isInitialized) {

    return false;
}


Serial.println(
    "[ClockManager] Synchronizing with NTP..."
);


configTzTime(
    _timeZone.c_str(),
    ntpServer
);


time_t timestamp = 0;


for (
    uint8_t i = 0;
    i < maxAttempts;
    i++
) {

    timestamp = time(nullptr);


    // Проверяем, получено ли реальное время
    if (timestamp > 1700000000) {

        // Сохраняем UTC в RTC
        _rtc.setDateTime(timestamp);


        Serial.println(
            "[ClockManager] RTC synchronized"
        );


        return true;
    }


    delay(500);
}


Serial.println(
    "[ClockManager] ERROR: NTP sync failed"
);


return false;
```

}

// ========================================
// SET LOCAL TIME
// ========================================

bool ClockManager::setLocalTime(
int year,
int month,
int day,
int hour,
int minute,
int second
) {


if (!_isInitialized) {
    return false;
}


time_t utc = localToUnix(
    year,
    month,
    day,
    hour,
    minute,
    second
);


if (utc == -1) {
    return false;
}


_rtc.setDateTime(utc);

return true;


}

// ========================================
// SET UTC TIME
// ========================================

bool ClockManager::setUTCTime(
int year,
int month,
int day,
int hour,
int minute,
int second
) {


if (!_isInitialized) {
    return false;
}


if (
    year < 2024 ||
    year > 2099 ||
    month < 1 ||
    month > 12 ||
    day < 1 ||
    day > 31 ||
    hour < 0 ||
    hour > 23 ||
    minute < 0 ||
    minute > 59 ||
    second < 0 ||
    second > 59
) {

    return false;
}


_rtc.setDateTime(
    year,
    month,
    day,
    hour,
    minute,
    second
);


return true;

}

// ========================================
// GET LOCAL TIME
// ========================================

bool ClockManager::getLocalTime(
struct tm& timeinfo
) {


if (!_isInitialized) {
    return false;
}


time_t utc = getRtcUnixTime();


if (utc < 1700000000) {
    return false;
}


localtime_r(
    &utc,
    &timeinfo
);


return true;


}

// ========================================
// GET UTC
// ========================================

bool ClockManager::getUTC(
struct tm& timeinfo
) {


if (!_isInitialized) {
    return false;
}


time_t utc = getRtcUnixTime();


if (utc < 1700000000) {
    return false;
}


gmtime_r(
    &utc,
    &timeinfo
);


return true;


}

// ========================================
// STRING: LOCAL TIME
// ========================================

String ClockManager::getLocalTimeString() {

struct tm timeinfo;


if (!getLocalTime(timeinfo)) {

    return "--";
}


char buffer[32];


strftime(
    buffer,
    sizeof(buffer),
    "%Y-%m-%d %H:%M:%S",
    &timeinfo
);


return String(buffer);


}

// ========================================
// STRING: UTC
// ========================================

String ClockManager::getUTCTimeString() {


struct tm timeinfo;


if (!getUTC(timeinfo)) {

    return "--";
}


char buffer[32];


strftime(
    buffer,
    sizeof(buffer),
    "%Y-%m-%d %H:%M:%S",
    &timeinfo
);


return String(buffer);


}

// ========================================
// FORMAT TIME
// ========================================

String ClockManager::formatTime(
const struct tm& timeinfo
) {

char buffer[16];


snprintf(
    buffer,
    sizeof(buffer),
    Constants::TIME_FORMAT,
    timeinfo.tm_hour,
    timeinfo.tm_min,
    timeinfo.tm_sec
);


return String(buffer);


}

// ========================================
// FORMAT DATE
// ========================================

String ClockManager::formatDate(
const struct tm& timeinfo
) {


char buffer[16];


snprintf(
    buffer,
    sizeof(buffer),
    Constants::DATE_FORMAT,
    timeinfo.tm_mday,
    timeinfo.tm_mon + 1,
    timeinfo.tm_year + 1900
);


return String(buffer);


}

// ========================================
// DAY OF WEEK
// ========================================

Constants::DayOfWeek ClockManager::getDayOfWeek() {

struct tm timeinfo;


if (!getLocalTime(timeinfo)) {

    return Constants::DayOfWeek::Sunday;
}


return static_cast<Constants::DayOfWeek>(
    timeinfo.tm_wday
);


}

// ========================================
// VALIDATION
// ========================================

bool ClockManager::isTimeValid() {

if (!_isInitialized) {
    return false;
}


DateTime now = _rtc.getDateTime();


return (
    now.year() >= 2024 &&
    now.year() <= 2099 &&
    now.month() >= 1 &&
    now.month() <= 12 &&
    now.day() >= 1 &&
    now.day() <= 31
);


}

// ========================================
// DEBUG
// ========================================

void ClockManager::printLocalTime() {


struct tm timeinfo;


if (getLocalTime(timeinfo)) {

    Serial.printf(
        "Local: %s %s\n",
        formatDate(timeinfo).c_str(),
        formatTime(timeinfo).c_str()
    );

} else {

    Serial.println(
        "Local: --"
    );
}


}

void ClockManager::printUTCTime() {


struct tm timeinfo;


if (getUTC(timeinfo)) {

    Serial.printf(
        "UTC: %s %s\n",
        formatDate(timeinfo).c_str(),
        formatTime(timeinfo).c_str()
    );

} else {

    Serial.println(
        "UTC: --"
    );
}


}

// ========================================
// RTC UNIX TIME
// ========================================

time_t ClockManager::getRtcUnixTime() {


return _rtc.getUnixTime();


}

// ========================================
// LOCAL → UTC
// ========================================

time_t ClockManager::localToUnix(
int year,
int month,
int day,
int hour,
int minute,
int second
) {


struct tm t = {};


t.tm_year = year - 1900;

t.tm_mon = month - 1;

t.tm_mday = day;

t.tm_hour = hour;

t.tm_min = minute;

t.tm_sec = second;

// Автоматическое определение DST
t.tm_isdst = -1;


return mktime(&t);


}
TimeData ClockManager::getTimeData() {

TimeData data;

struct tm timeinfo;

if (!getLocalTime(timeinfo)) {

    data.valid = false;

    return data;
}


data.hour = timeinfo.tm_hour;

data.minute = timeinfo.tm_min;

data.second = timeinfo.tm_sec;

data.valid = true;


return data;

}
DDateData ClockManager::getDateData()
{
DateData data;

```
struct tm timeinfo;

if (!getLocalTime(timeinfo))
{
    data.valid = false;
    return data;
}

data.year = timeinfo.tm_year + 1900;

data.month = timeinfo.tm_mon + 1;

data.day = timeinfo.tm_mday;

data.dayOfWeek =
    static_cast<Constants::DayOfWeek>(
        timeinfo.tm_wday
    );

data.valid = true;

return data;
```

}

Constants::DayOfWeek ClockManager::getDayOfWeek() {

    struct tm timeinfo;

    if (!getLocalTime(timeinfo)) {

        return Constants::DayOfWeek::Sunday;
    }

    return static_cast<Constants::DayOfWeek>(
        timeinfo.tm_wday
    );
}



