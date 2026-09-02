#pragma once

#include <Arduino.h>

// ============================================================
// TIME CONSTANTS
// ============================================================

namespace Constants
{
    constexpr uint32_t MS_PER_SECOND = 1000;
    constexpr uint32_t MS_PER_MINUTE = 60 * MS_PER_SECOND;
    constexpr uint32_t MS_PER_HOUR   = 60 * MS_PER_MINUTE;

    constexpr uint32_t SECONDS_PER_MINUTE = 60;
    constexpr uint32_t MINUTES_PER_HOUR   = 60;
    constexpr uint32_t HOURS_PER_DAY      = 24;
}


// ============================================================
// DAYS OF WEEK
// ============================================================

namespace Constants
{
    enum class DayOfWeek : uint8_t
    {
        Sunday = 0,
        Monday,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday
    };
}


// ============================================================
// TIMER (общий)
// ============================================================

namespace Constants
{
    constexpr uint32_t MIN_TIMER_SECONDS = 1;
    constexpr uint32_t MAX_TIMER_SECONDS = 24 * 60 * 60;  // 24 часа
}


// ============================================================
// POMODORO
// ============================================================

namespace Constants
{
    constexpr uint32_t POMODORO_WORK_MINUTES              = 25;
    constexpr uint32_t POMODORO_SHORT_BREAK_MINUTES       = 5;
    constexpr uint32_t POMODORO_LONG_BREAK_MINUTES        = 15;
    constexpr uint8_t  POMODORO_CYCLES_BEFORE_LONG_BREAK  = 4;
}



// ============================================================
// FILE PATHS (SD-карта)
// ============================================================

namespace Constants
{
    constexpr const char* SETTINGS_DIR  = "/settings";
    constexpr const char* CLOCK_DIR     = "/clock";
    constexpr const char* TASKS_DIR     = "/tasks";
    constexpr const char* SLEEP_DIR     = "/sleep";
    constexpr const char* SENSOR_DIR    = "/sensors";
    constexpr const char* LOG_DIR       = "/logs";
    constexpr const char* AUDIO_DIR     = "/audio";
}


// ============================================================
// ДОПОЛНИТЕЛЬНЫЕ КОНСТАНТЫ (добавлены из вашего проекта)
// ============================================================

namespace Constants
{
    // ------ Строковые сообщения для дисплеев ------
    constexpr const char* MSG_WELCOME   = "System Ready";
    constexpr const char* MSG_ERROR     = "ERROR";
    constexpr const char* MSG_LOADING   = "Loading...";
    constexpr const char* MSG_NO_SD     = "SD Card not found";
    constexpr const char* MSG_WIFI_CONN = "WiFi connected";
    constexpr const char* MSG_WIFI_FAIL = "WiFi failed";

    // ------ Названия датчиков ------
    constexpr const char* SENSOR_SHT45  = "SHT45";
    constexpr const char* SENSOR_VEML   = "VEML7700";
    constexpr const char* SENSOR_VL53   = "VL53L8CX";
    constexpr const char* SENSOR_RTC    = "DS3231";

    // ------ Форматы времени и даты ------
    constexpr const char* TIME_FORMAT   = "%02d:%02d:%02d";
    constexpr const char* DATE_FORMAT   = "%02d.%02d.%04d";

    // ------ Единицы измерения ------
    constexpr const char* UNIT_TEMP_C   = "°C";
    constexpr const char* UNIT_TEMP_F   = "°F";
    constexpr const char* UNIT_HUMID    = "%";
    constexpr const char* UNIT_LUX      = "lx";
    constexpr const char* UNIT_DIST_MM  = "mm";

    // ------ Таймауты обновления (в миллисекундах) ------
    constexpr unsigned long SENSOR_UPDATE_INTERVAL  = 2000;  // 2 с
    constexpr unsigned long DISPLAY_REFRESH_RATE    = 50;    // 50 мс (20 Гц)
    constexpr unsigned long LED_UPDATE_INTERVAL     = 30;    // 30 мс
    constexpr unsigned long SD_CARD_TIMEOUT         = 5000;  // 5 с
}