#pragma once
#include "TimeData.h"
#include <Arduino.h>
#include <time.h>

#include "RTC.h"
#include "Constants.h"

class ClockManager {
public:


ClockManager(RTC& rtcModule);


// ================================
// INITIALIZATION
// ================================

bool begin(
    const char* timeZone = "Europe/Moscow"
);
// ========================================
// STRUCTURED TIME
// ========================================

// Получить время отдельными значениями
TimeData getTimeData();

// Получить дату отдельными значениями
DateData getDateData();

// ========================================
// DAY OF WEEK
// ========================================

// Получить день недели
Constants::DayOfWeek getDayOfWeek();



// ================================
// TIMEZONE
// ================================

bool setTimeZone(
    const char* timeZone
);


// ================================
// NTP
// ================================

bool syncFromNTP(
    const char* ntpServer = "pool.ntp.org",
    uint8_t maxAttempts = 10
);


// ================================
// SET TIME
// ================================

bool setLocalTime(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second
);


bool setUTCTime(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second
);


// ================================
// GET TIME
// ================================

bool getLocalTime(
    struct tm& timeinfo
);


bool getUTC(
    struct tm& timeinfo
);


// ================================
// STRINGS
// ================================

String getLocalTimeString();

String getUTCTimeString();


// ================================
// FORMATTING
// ================================

static String formatTime(
    const struct tm& timeinfo
);


static String formatDate(
    const struct tm& timeinfo
);


// ================================
// DAY OF WEEK
// ================================

Constants::DayOfWeek getDayOfWeek();


// ================================
// STATUS
// ================================

bool isTimeValid();


// ================================
// DEBUG
// ================================

void printLocalTime();

void printUTCTime();


private:


RTC& _rtc;

String _timeZone;

bool _isInitialized = false;


time_t getRtcUnixTime();


time_t localToUnix(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second
);


};
