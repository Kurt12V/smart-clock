#pragma once

#include <Arduino.h>
#include "Pins.h"

// ============================================================
// SETTINGS DEFAULTS / LIMITS
// ============================================================

namespace Config
{
    // ========================================================
    // DISPLAY
    // ========================================================

    constexpr uint8_t DISPLAY_MIN_BRIGHTNESS     = 0;
    constexpr uint8_t DISPLAY_MAX_BRIGHTNESS     = 100;
    constexpr uint8_t DISPLAY_DEFAULT_BRIGHTNESS = 80;


    // ========================================================
    // MATRIX
    // ========================================================

    constexpr bool MATRIX_ENABLED_DEFAULT = true;

    constexpr uint8_t MATRIX_BRIGHTNESS_MIN     = 0;
    constexpr uint8_t MATRIX_BRIGHTNESS_MAX     = 100;
    constexpr uint8_t MATRIX_BRIGHTNESS_DEFAULT = 50;

    constexpr uint8_t MATRIX_EFFECT_MIN     = 0;
    constexpr uint8_t MATRIX_EFFECT_MAX     = 9;
    constexpr uint8_t MATRIX_EFFECT_DEFAULT = 0;

    constexpr uint8_t MATRIX_SPEED_MIN     = 0;
    constexpr uint8_t MATRIX_SPEED_MAX     = 100;
    constexpr uint8_t MATRIX_SPEED_DEFAULT = 50;


    // ========================================================
    // COB
    // ========================================================

    constexpr bool COB_ENABLED_DEFAULT = true;

    constexpr uint8_t COB_BRIGHTNESS_MIN     = 0;
    constexpr uint8_t COB_BRIGHTNESS_MAX     = 100;
    constexpr uint8_t COB_BRIGHTNESS_DEFAULT = 100;

    constexpr uint8_t COB_EFFECT_MIN     = 0;
    constexpr uint8_t COB_EFFECT_MAX     = 9;
    constexpr uint8_t COB_EFFECT_DEFAULT = 0;

    constexpr uint8_t COB_SPEED_MIN     = 0;
    constexpr uint8_t COB_SPEED_MAX     = 100;
    constexpr uint8_t COB_SPEED_DEFAULT = 50;


    // ========================================================
    // AUDIO
    // ========================================================

    constexpr uint8_t AUDIO_MIN_VOLUME     = 0;
    constexpr uint8_t AUDIO_MAX_VOLUME     = 100;
    constexpr uint8_t AUDIO_VOLUME_DEFAULT = 60;

    constexpr uint8_t MEDIA_VOLUME_DEFAULT  = 80;
    constexpr uint8_t ALARM_VOLUME_DEFAULT  = 100;
    constexpr uint8_t SYSTEM_VOLUME_DEFAULT = 80;


    // ========================================================
    // MICROPHONE
    // ========================================================

    constexpr bool MIC_ENABLED_DEFAULT = true;


    // ========================================================
    // CLOCK / TIMEZONE
    // ========================================================

    constexpr int8_t UTC_OFFSET_MIN     = -12;
    constexpr int8_t UTC_OFFSET_MAX     = 14;
    constexpr int8_t UTC_OFFSET_DEFAULT = 3;
}


// ============================================================
// SYSTEM
// ============================================================

namespace Config
{
    constexpr const char* DEVICE_NAME =
        "SmartClock";

    constexpr const char* DEVICE_DESCRIPTION =
        "ESP32-S3 Smart Clock";

    constexpr bool DEBUG =
        true;
}


// ============================================================
// SERIAL
// ============================================================

namespace Config
{
    constexpr size_t LVGL_BUFFER_SIZE =
        320 * 40;

    constexpr unsigned long SERIAL_BAUD_RATE =
        115200;
}


// ============================================================
// I2C
// ============================================================

namespace Config
{
    constexpr uint32_t I2C_FREQUENCY =
        400000;
}


// ============================================================
// SENSORS
// ============================================================

namespace Config
{
    constexpr uint32_t SENSOR_UPDATE_INTERVAL_MS =
        1000;

    constexpr uint8_t SHT45_I2C_ADDRESS =
        0x44;

    constexpr uint8_t VEML7700_I2C_ADDRESS =
        0x10;

    constexpr uint8_t VL53L8CX_I2C_ADDRESS =
        0x29;
}


// ============================================================
// RTC
// ============================================================

namespace Config
{
    constexpr uint8_t RTC_I2C_ADDRESS =
        0x68;

    constexpr uint32_t TIME_UPDATE_INTERVAL_MS =
        1000;
}


// ============================================================
// SD CARD
// ============================================================

namespace Config
{
    constexpr uint32_t SD_SPI_FREQUENCY =
        20000000;

    constexpr const char* SD_ROOT =
        "/";

    constexpr uint32_t SD_CARD_TIMEOUT_MS =
        5000;
}


// ============================================================
// DISPLAYS
// ============================================================

namespace Config
{
    constexpr uint16_t DISPLAY_WIDTH  = 320;
    constexpr uint16_t DISPLAY_HEIGHT = 240;
    constexpr uint8_t  DISPLAY_COUNT  = 4;

    constexpr uint16_t TOTAL_WIDTH =
        DISPLAY_WIDTH * DISPLAY_COUNT;

    constexpr uint32_t DISPLAY_SPI_FREQUENCY =
        40000000;

    constexpr uint8_t DISPLAY_ROTATION =
        0;

    constexpr uint32_t DISPLAY_REFRESH_INTERVAL_MS =
        50;
}


// ============================================================
// AUDIO
// ============================================================

namespace Config
{
    constexpr uint32_t AUDIO_SAMPLE_RATE =
        16000;

    constexpr uint16_t AUDIO_BITS =
        16;

    constexpr uint16_t AUDIO_BUFFER_SIZE =
        512;


    // --------------------------------------------------------
    // MICROPHONE
    // --------------------------------------------------------

    constexpr uint32_t MIC_SAMPLE_RATE =
        16000;

    constexpr uint16_t MIC_BITS =
        16;

    constexpr uint16_t MIC_BUFFER_SIZE =
        256;
}


// ============================================================
// LED MATRIX
// ============================================================

namespace Config
{
    constexpr uint8_t MATRIX_WIDTH =
        16;

    constexpr uint8_t MATRIX_HEIGHT =
        16;

    constexpr uint16_t MATRIX_LED_COUNT =
        MATRIX_WIDTH * MATRIX_HEIGHT;

    // Physical WS2812 brightness range.
    constexpr uint8_t MATRIX_MAX_BRIGHTNESS =
        255;

    constexpr uint32_t LED_UPDATE_INTERVAL_MS =
        30;
}


// ============================================================
// COB LED
// ============================================================

namespace Config
{
    constexpr uint8_t COB_PWM_RESOLUTION =
        8;

    constexpr uint32_t COB_PWM_FREQUENCY =
        5000;

    constexpr uint8_t COB_COUNT =
        4;
}


// ============================================================
// 74HCT245 BUFFER
// ============================================================

namespace Config
{
    constexpr uint8_t BUFFER_DIR_OUTPUT =
        1;

    constexpr uint8_t BUFFER_DIR_INPUT =
        0;
}


// ============================================================
// TIMERS
// ============================================================

namespace Config
{
    constexpr uint8_t MAX_TIMERS =
        8;

    constexpr uint32_t TIMER_UPDATE_INTERVAL_MS =
        100;

    constexpr uint32_t MIN_TIMER_SECONDS =
        1;

    constexpr uint32_t MAX_TIMER_SECONDS =
        24 * 60 * 60;
}


// ============================================================
// ALARMS
// ============================================================

namespace Config
{
    constexpr uint8_t MAX_ALARMS =
        16;

    constexpr uint32_t ALARM_CHECK_INTERVAL_MS =
        500;
}


// ============================================================
// TASKS
// ============================================================

namespace Config
{
    constexpr uint8_t MAX_TASKS =
        100;
}


// ============================================================
// SLEEP / ENERGY SAVING
// ============================================================

namespace Config
{
    constexpr uint32_t SLEEP_SENSOR_INTERVAL_MS =
        1000;

    constexpr uint32_t SLEEP_SAVE_INTERVAL_MS =
        60000;
}
