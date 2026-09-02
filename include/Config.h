#pragma once

#include <Arduino.h>
#include "Pins.h"   // GPIO определения

// ============================================================
// SYSTEM
// ============================================================

namespace Config
{
    constexpr const char* DEVICE_NAME = "SmartClock";
    constexpr const char* DEVICE_DESCRIPTION = "ESP32-S3 Smart Clock";
    constexpr bool DEBUG = true;
}


// ============================================================
// SERIAL
// ============================================================

namespace Config
{
    constexpr unsigned long SERIAL_BAUD_RATE = 115200;
}


// ============================================================
// I2C
// ============================================================

namespace Config
{
    constexpr uint32_t I2C_FREQUENCY = 400000;   // 400 кГц
}


// ============================================================
// SENSORS (адреса и интервалы)
// ============================================================

namespace Config
{
    constexpr uint32_t SENSOR_UPDATE_INTERVAL_MS = 1000;

    constexpr uint8_t SHT45_I2C_ADDRESS   = 0x44;
    constexpr uint8_t VEML7700_I2C_ADDRESS = 0x10;
    constexpr uint8_t VL53L8CX_I2C_ADDRESS = 0x29;
}


// ============================================================
// RTC
// ============================================================

namespace Config
{
    constexpr uint8_t RTC_I2C_ADDRESS = 0x68;
    constexpr uint32_t TIME_UPDATE_INTERVAL_MS = 1000;
}


// ============================================================
// SD CARD
// ============================================================

namespace Config
{
    constexpr uint32_t SD_SPI_FREQUENCY = 20000000;   // 20 МГц
    constexpr const char* SD_ROOT = "/";
    constexpr uint32_t SD_CARD_TIMEOUT_MS = 5000;     // 5 секунд
}


// ============================================================
// DISPLAYS (ST7789)
// ============================================================

namespace Config
{
    constexpr uint16_t DISPLAY_WIDTH  = 320;
    constexpr uint16_t DISPLAY_HEIGHT = 240;
    constexpr uint8_t  DISPLAY_COUNT  = 4;

    constexpr uint32_t DISPLAY_SPI_FREQUENCY = 40000000;   // 40 МГц
    constexpr uint8_t  DISPLAY_ROTATION = 0;

    // Яркость (в процентах)
    constexpr uint8_t DISPLAY_MIN_BRIGHTNESS     = 0;
    constexpr uint8_t DISPLAY_MAX_BRIGHTNESS     = 100;
    constexpr uint8_t DISPLAY_DEFAULT_BRIGHTNESS = 80;

    constexpr uint32_t DISPLAY_REFRESH_INTERVAL_MS = 50;   // 20 Гц
}


// ============================================================
// AUDIO (MAX98357 + INMP441)
// ============================================================

namespace Config
{
    constexpr uint32_t AUDIO_SAMPLE_RATE = 16000;
    constexpr uint8_t  AUDIO_VOLUME_DEFAULT = 60;
    constexpr uint8_t  AUDIO_MIN_VOLUME = 0;
    constexpr uint8_t  AUDIO_MAX_VOLUME = 100;

    constexpr uint32_t MIC_SAMPLE_RATE = 16000;
    constexpr uint16_t MIC_BITS = 16;
    constexpr uint16_t MIC_BUFFER_SIZE = 256;

    constexpr uint16_t AUDIO_BITS = 16;
    constexpr uint16_t AUDIO_BUFFER_SIZE = 512;
}


// ============================================================
// LED MATRIX (16x16)
// ============================================================

namespace Config
{
    constexpr uint8_t  MATRIX_WIDTH  = 16;
    constexpr uint8_t  MATRIX_HEIGHT = 16;
    constexpr uint16_t MATRIX_LED_COUNT = MATRIX_WIDTH * MATRIX_HEIGHT; // 256
    constexpr uint8_t  MATRIX_MAX_BRIGHTNESS = 255;
    constexpr uint32_t LED_UPDATE_INTERVAL_MS = 30;
}


// ============================================================
// COB LED (через LD1500SB)
// ============================================================

namespace Config
{
    constexpr uint8_t  COB_PWM_RESOLUTION = 8;   // бит, диапазон 0-255
    constexpr uint32_t COB_PWM_FREQUENCY = 5000; // Гц
    constexpr uint8_t  COB_COUNT = 4;
}


// ============================================================
// 74HCT245 BUFFER
// ============================================================

namespace Config
{
    constexpr uint8_t BUFFER_DIR_OUTPUT = 1;
    constexpr uint8_t BUFFER_DIR_INPUT = 0;
}


// ============================================================
// TIMERS
// ============================================================

namespace Config
{
    constexpr uint8_t  MAX_TIMERS = 8;
    constexpr uint32_t TIMER_UPDATE_INTERVAL_MS = 100;

    // Минимальный и максимальный таймер (в секундах)
    constexpr uint32_t MIN_TIMER_SECONDS = 1;
    constexpr uint32_t MAX_TIMER_SECONDS = 24 * 60 * 60;   // 24 часа
}


// ============================================================
// ALARMS
// ============================================================

namespace Config
{
    constexpr uint8_t  MAX_ALARMS = 16;
    constexpr uint32_t ALARM_CHECK_INTERVAL_MS = 500;
}


// ============================================================
// TASKS
// ============================================================

namespace Config
{
    constexpr uint8_t MAX_TASKS = 100;
}


// ============================================================
// SLEEP / ENERGY SAVING
// ============================================================

namespace Config
{
    constexpr uint32_t SLEEP_SENSOR_INTERVAL_MS = 1000;   // опрос датчиков во сне
    constexpr uint32_t SLEEP_SAVE_INTERVAL_MS = 60000;    // сохранение состояния каждую минуту
}