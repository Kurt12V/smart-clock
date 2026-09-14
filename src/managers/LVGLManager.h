#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <lvgl.h>

#include "./ui/LVGLTFTPanel.h"

class LVGLManager
{
public:
    static constexpr uint8_t DISPLAY_COUNT = 4;

    LVGLManager();

    bool begin();

    void update();

    bool isReady() const;

    lv_display_t* display(
        uint8_t index
    ) const;

private:
    Adafruit_ST7789 _tft[
        DISPLAY_COUNT
    ];

    LVGLTFTPanel _panels[
        DISPLAY_COUNT
    ];

    bool _initialized;
};
