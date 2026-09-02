#pragma once

// ======================== SPI для дисплеев (общие линии) ========================
#define PIN_TFT_SCLK   12    // SPI Clock для всех дисплеев
#define PIN_TFT_MOSI   11    // SPI Data (MOSI) для всех дисплеев
#define PIN_TFT_DC      2    // Data/Command – общий для всех

// ======================== CS для каждого дисплея ========================
#define PIN_TFT_CS1     4    // Дисплей №1
#define PIN_TFT_CS2     5    // Дисплей №2
#define PIN_TFT_CS3     6    // Дисплей №3
#define PIN_TFT_CS4     7    // Дисплей №4

// ======================== RST для каждого дисплея ========================
#define PIN_TFT_RST1 3
#define PIN_TFT_RST2 10
#define PIN_TFT_RST3 13 
#define PIN_TFT_RST4 42

// ======================== Подсветка дисплеев (общая) ========================
#define PIN_TFT_BL      1    // Управление яркостью (ШИМ)

// ======================== I2C (датчики, RTC, VL53L8CX) ========================
#define PIN_I2C_SDA     8
#define PIN_I2C_SCL     9

// // ======================== SD-карта (SPI) ========================
// #define PIN_SD_CS      47
// #define PIN_SD_SCK     46
// #define PIN_SD_MOSI    44
// #define PIN_SD_MISO    45

// // ======================== Аудио MAX98357 (первый) ========================
// #define PIN_MAX1_BCLK  14
// #define PIN_MAX1_LRC   15
// #define PIN_MAX1_DIN   16

// // ======================== Аудио MAX98357 (второй) ========================
// #define PIN_MAX2_BCLK  17
// #define PIN_MAX2_LRC   18
// #define PIN_MAX2_DIN   21

// // ======================== Микрофон INMP441 (I2S) ========================
// #define PIN_INMP_DOUT  38
// #define PIN_INMP_BCLK  39
// #define PIN_INMP_LRCLK 41

// // ======================== 74HCT245 (буфер/уровень) ========================
// #define PIN_245_DIR    37   // Направление передачи
// #define PIN_245_OE     36   // Включение буфера (активный LOW)

// // ======================== Драйверы LD1500SB для COB LED (ШИМ) ========================
// #define PIN_PWM_LD1    33   // COB LED №1
// #define PIN_PWM_LD2    34   // COB LED №2
// #define PIN_PWM_LD3    35   // COB LED №3
// #define PIN_PWM_LD4    13   // COB LED №4 (используем GPIO13, т.к. 48 может отсутствовать)

// // ======================== LED-матрица 16x16 (WS2812/SK6812) ========================
// #define PIN_LED_MATRIX 40   // Data line (NeoPixel)

// // ======================== VL53L8CX (ToF-камера) ========================
// #define PIN_VL53_INT   42   // Прерывание (опционально)

// // ======================== Дополнительные пины (свободные) ========================
// // GPIO 10, 19, 20, 43 (USB D-), 44-47 уже заняты, 48 не используется.