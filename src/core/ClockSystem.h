#pragma once

#include <Arduino.h>

#include "RTC.h"
#include "ClockManager.h"
#include "Pins.h"

class ClockSystem
{
public:

```
ClockSystem();


// ========================================
// INITIALIZATION
// ========================================

bool begin(
    const char* timeZone = "Europe/Moscow"
);


// ========================================
// SYNCHRONIZATION
// ========================================

// Синхронизация времени RTC через NTP
bool syncFromNTP(
    const char* ntpServer = "pool.ntp.org",
    uint8_t maxAttempts = 10
);


// ========================================
// TIMEZONE
// ========================================

// Установить временную зону
bool setTimeZone(const char* timeZone);

// Получить текущую временную зону
String getTimeZone() const;


// ========================================
// SET LOCAL DATE AND TIME
// ========================================

// Установить локальные дату и время
bool setLocalDateTime(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second = 0
);


// ========================================
// SET UTC DATE AND TIME
// ========================================

// Установить UTC дату и время
bool setUTCDateTime(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second = 0
);


// ========================================
// SET TIME ONLY
// ========================================

// Изменить только локальное время
bool setLocalTime(
    int hour,
    int minute,
    int second = 0
);


// Изменить только UTC время
bool setUTCTime(
    int hour,
    int minute,
    int second = 0
);


// ========================================
// SET DATE ONLY
// ========================================

// Изменить только локальную дату
bool setLocalDate(
    int year,
    int month,
    int day
);


// Изменить только UTC дату
bool setUTCDate(
    int year,
    int month,
    int day
);


// ========================================
// GET DATA
// ========================================

TimeData getTimeData();

DateData getDateData();

Constants::DayOfWeek getDayOfWeek();

String getTimeHHMM();


// ========================================
// STATUS
// ========================================

bool isTimeValid();
```

private:

```
RTC rtc;

ClockManager clockManager;
```

};
