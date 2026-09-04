#include "ClockSystem.h"

ClockSystem::ClockSystem()
: clockManager(rtc)
{
}

// ========================================
// INITIALIZATION
// ========================================

bool ClockSystem::begin(const char* timeZone)
{
Serial.println();
Serial.println("================================");
Serial.println("         CLOCK SYSTEM");
Serial.println("================================");

```
// RTC INITIALIZATION

if (!rtc.begin(PIN_I2C_SDA, PIN_I2C_SCL))
{
    Serial.println("[ClockSystem] RTC ERROR");

    return false;
}

Serial.println("[ClockSystem] RTC READY");


// CLOCK MANAGER INITIALIZATION

if (!clockManager.begin(timeZone))
{
    Serial.println("[ClockSystem] CLOCK MANAGER ERROR");

    return false;
}

Serial.println("[ClockSystem] CLOCK MANAGER READY");

Serial.println("================================");

return true;
```

}

// ========================================
// NTP SYNCHRONIZATION
// ========================================

bool ClockSystem::syncFromNTP(
const char* ntpServer,
uint8_t maxAttempts
)
{
return clockManager.syncFromNTP(
ntpServer,
maxAttempts
);
}

// ========================================
// TIMEZONE
// ========================================

bool ClockSystem::setTimeZone(
const char* timeZone
)
{
return clockManager.setTimeZone(timeZone);
}

String ClockSystem::getTimeZone() const
{
return clockManager.getTimeZone();
}

// ========================================
// SET LOCAL DATE AND TIME
// ========================================

bool ClockSystem::setLocalDateTime(
int year,
int month,
int day,
int hour,
int minute,
int second
)
{
return clockManager.setLocalTime(
year,
month,
day,
hour,
minute,
second
);
}

// ========================================
// SET UTC DATE AND TIME
// ========================================

bool ClockSystem::setUTCDateTime(
int year,
int month,
int day,
int hour,
int minute,
int second
)
{
return clockManager.setUTCTime(
year,
month,
day,
hour,
minute,
second
);
}

// ========================================
// SET LOCAL TIME ONLY
// ========================================

bool ClockSystem::setLocalTime(
int hour,
int minute,
int second
)
{
DateData date = getDateData();

```
if (!date.valid)
{
    return false;
}

return setLocalDateTime(
    date.year,
    date.month,
    date.day,
    hour,
    minute,
    second
);
```

}

// ========================================
// SET UTC TIME ONLY
// ========================================

bool ClockSystem::setUTCTime(
int hour,
int minute,
int second
)
{
struct tm timeinfo;

```
if (!clockManager.getUTC(timeinfo))
{
    return false;
}

return setUTCDateTime(
    timeinfo.tm_year + 1900,
    timeinfo.tm_mon + 1,
    timeinfo.tm_mday,
    hour,
    minute,
    second
);
```

}

// ========================================
// SET LOCAL DATE ONLY
// ========================================

bool ClockSystem::setLocalDate(
int year,
int month,
int day
)
{
TimeData time = getTimeData();

```
if (!time.valid)
{
    return false;
}

return setLocalDateTime(
    year,
    month,
    day,
    time.hour,
    time.minute,
    time.second
);
```

}

// ========================================
// SET UTC DATE ONLY
// ========================================

bool ClockSystem::setUTCDate(
int year,
int month,
int day
)
{
struct tm timeinfo;

```
if (!clockManager.getUTC(timeinfo))
{
    return false;
}

return setUTCDateTime(
    year,
    month,
    day,
    timeinfo.tm_hour,
    timeinfo.tm_min,
    timeinfo.tm_sec
);
```

}

// ========================================
// GET TIME
// ========================================

TimeData ClockSystem::getTimeData()
{
return clockManager.getTimeData();
}

// ========================================
// GET DATE
// ========================================

DateData ClockSystem::getDateData()
{
return clockManager.getDateData();
}

// ========================================
// DAY OF WEEK
// ========================================

Constants::DayOfWeek ClockSystem::getDayOfWeek()
{
return clockManager.getDayOfWeek();
}

// ========================================
// HH:MM
// ========================================

String ClockSystem::getTimeHHMM()
{
return clockManager.getTimeHHMM();
}

// ========================================
// STATUS
// ========================================

bool ClockSystem::isTimeValid()
{
return clockManager.isTimeValid();
}
