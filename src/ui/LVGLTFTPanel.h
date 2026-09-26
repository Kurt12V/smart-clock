#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <lvgl.h>

class LVGLTFTPanel
{
public:
    static constexpr uint16_t WIDTH = 172;
    static constexpr uint16_t HEIGHT = 320;
    static constexpr uint16_t BUFFER_LINES = 32;

    LVGLTFTPanel(
        Adafruit_ST7789& tft,
        int16_t xOffset = 34,
        int16_t yOffset = 0
    );

    bool begin();

    lv_display_t* display() const;

    bool isReady() const;

    void setBacklight(bool state);

private:
    Adafruit_ST7789& _tft;

    int16_t _xOffset;
    int16_t _yOffset;

    lv_display_t* _display;

    lv_color_t _buffer[
        WIDTH * BUFFER_LINES
    ];

    bool _ready;

    static void flushCallback(
        lv_display_t* display,
        const lv_area_t* area,
        uint8_t* pxMap
    );

    void flush(
        const lv_area_t* area,
        uint8_t* pxMap
    );
};