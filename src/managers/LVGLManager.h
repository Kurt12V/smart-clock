#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <lvgl.h>

#include "Pins.h"

class LVGLManager
{
public:
    static constexpr uint8_t DISPLAY_COUNT = 4;

    static constexpr uint16_t WIDTH  = 172;
    static constexpr uint16_t HEIGHT = 320;

    // Количество строк в частичном LVGL-буфере
    static constexpr uint16_t BUFFER_LINES = 32;

    // Размер одного буфера в пикселях
    static constexpr uint32_t BUFFER_SIZE =
        static_cast<uint32_t>(WIDTH) *
        static_cast<uint32_t>(BUFFER_LINES);


public:

    // ========================================================
    // LIFECYCLE
    // ========================================================

    bool begin();

    void update();

    lv_display_t* display(uint8_t index);

    bool isReady() const;


private:

    struct DisplayContext
    {
        // Номер дисплея:
        // 0 = TFT1
        // 1 = TFT2
        // 2 = TFT3
        // 3 = TFT4
        uint8_t index = 0;

        // Физический TFT
        Adafruit_ST7789* tft = nullptr;

        // LVGL display instance
        lv_display_t* lvDisplay = nullptr;

        // LVGL draw buffer
        lv_color_t* buffer = nullptr;

        // Состояние
        bool initialized = false;
    };


private:

    static void flushCallback(
        lv_display_t* display,
        const lv_area_t* area,
        uint8_t* pxMap
    );

    bool initDisplay(uint8_t index);


private:

    // =========================================================
    // TFT
    // =========================================================

    Adafruit_ST7789 _tft1;
    Adafruit_ST7789 _tft2;
    Adafruit_ST7789 _tft3;
    Adafruit_ST7789 _tft4;


    // =========================================================
    // CONTEXT
    // =========================================================

    DisplayContext _contexts[
        DISPLAY_COUNT
    ];


    // =========================================================
    // LVGL BUFFERS
    // =========================================================

    /*
        Отдельный буфер для каждого дисплея.

        _buffers[0] -> TFT1
        _buffers[1] -> TFT2
        _buffers[2] -> TFT3
        _buffers[3] -> TFT4

        Размер каждого:
        172 × 32 пикселя
    */

    lv_color_t _buffers[
        DISPLAY_COUNT
    ][BUFFER_SIZE];


    // =========================================================
    // STATE
    // =========================================================

    bool _initialized;

    uint32_t _lastTick;
};
