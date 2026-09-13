#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include "Pins.h"

class LVGLManager
{
public:
    static constexpr uint8_t DISPLAY_COUNT = 4;

    static constexpr uint16_t WIDTH  = 172;
    static constexpr uint16_t HEIGHT = 320;

    static constexpr uint16_t BUFFER_LINES = 32;

public:
    LVGLManager();

    bool begin();
    void update();

    lv_display_t* display(uint8_t index);

    bool isReady() const;

private:

    struct DisplayContext
    {
        Adafruit_ST7789* tft = nullptr;
        lv_display_t* lvDisplay = nullptr;

        lv_color_t* buffer = nullptr;

        bool initialized = false;
    };

    static void flushCallback(
        lv_display_t* display,
        const lv_area_t* area,
        uint8_t* pxMap
    );

    bool initDisplay(uint8_t index);

private:

    Adafruit_ST7789 _tft1;
    Adafruit_ST7789 _tft2;
    Adafruit_ST7789 _tft3;
    Adafruit_ST7789 _tft4;

    DisplayContext _contexts[DISPLAY_COUNT];

    lv_color_t _buffers[
        DISPLAY_COUNT
    ][WIDTH * BUFFER_LINES];

    bool _initialized;
    uint32_t _lastTick;
};