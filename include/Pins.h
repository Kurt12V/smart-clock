#pragma once

// ============================================================
// SPI — ДИСПЛЕИ
// ============================================================
#define PIN_SCLK    12      // SPI Clock
#define PIN_MOSI    11      // SPI MOSI
#define PIN_TFT_DC       2      // Data / Command
#define PIN_TFT_CS1      4      // Дисплей №1
#define PIN_TFT_CS2      5      // Дисплей №2
#define PIN_TFT_CS3      6      // Дисплей №3
#define PIN_TFT_CS4      7      // Дисплей №4
#define PIN_TFT_RST1     3      // Дисплей №1
#define PIN_TFT_RST2    10      // Дисплей №2
#define PIN_TFT_RST3    13      // Дисплей №3
#define PIN_TFT_RST4    42      // Дисплей №4
#define PIN_TFT_BL       1      // Общая подсветка, PWM

// ============================================================
// I2C
// SHT45 / VEML7700 / VL53L8CX / DS3231
// ============================================================
#define PIN_I2C_SDA      8
#define PIN_I2C_SCL      9

// ============================================================
// SD CARD
// ============================================================
#define PIN_SD_CS      35
#define PIN_SD_MISO   36
// SCLK = PIN_SCLK
// MOSI = PIN_MOSI



// ============================================================
// AUDIO — MAX98357A
// ============================================================
// Два MAX98357A работают в СТЕРЕО.
// ESP32-S3 передаёт один стерео I2S поток:
//                 ┌─> MAX98357A #1 → LEFT
// ESP32-S3 I2S ───┤
//                 └─> MAX98357A #2 → RIGHT
// BCLK / LRC / DIN являются ОБЩИМИ.
// ============================================================
#define PIN_I2S_BCLK    40      // I2S Bit Clock
#define PIN_I2S_LRCLK   41      // I2S Left / Right Clock
#define PIN_I2S_DIN    48      // I2S Data Out


// ============================================================
// INMP441 — MICROPHONE
// ============================================================

#define PIN_INMP_SCK   15      // I2S Bit Clock
#define PIN_INMP_WS    16      // I2S Word Select / LRCLK
#define PIN_INMP_SD    17      // I2S Data In from microphone



// ============================================================
// LD1500SB — COB LED
// PWM
// ============================================================

#define PIN_PWM_LD1     18
#define PIN_PWM_LD2     37
#define PIN_PWM_LD3     38
#define PIN_PWM_LD4     39


// ============================================================
// LED MATRIX 16×16
// WS2812 / SK6812
// ============================================================

#define PIN_LED_MATRIX   14   // Data

// ============================================================
// ENCODER
// ============================================================

#define PIN_ENCODER_CLK  20
#define PIN_ENCODER_DT   21
#define PIN_ENCODER_SW   47