#pragma once

#include <Arduino.h>
#include <Adafruit_ST7789.h>
#include <lvgl.h>

#include "./managers/SPIManager.h"
#include "Pins.h"
#include "Config.h"

class LVGLManager
{
public:

    // ========================================================
    // LIFECYCLE
    // ========================================================

    LVGLManager(SPIManager& spi);

    bool begin();

    void update();

    bool isReady() const;

    // ========================================================
    // BRIGHTNESS
    // ========================================================

    void    setBrightness(uint8_t percent);   // 0..100
    uint8_t brightness() const;

    // ========================================================
    // DISPLAYS
    // ========================================================

    void refresh();

    void clearDisplays();

    bool testDisplays();

    lv_display_t* display(uint8_t index);

private:

    // ========================================================
    // DISPLAY CONTEXT
    // ========================================================
static constexpr uint8_t BL_CHANNEL = 0;
    struct DisplayContext
    {
        uint8_t          index;
        Adafruit_ST7789* tft;
        lv_display_t*    lvDisplay;
        lv_color_t*      buffer;
        bool             initialized;
    };

    // ========================================================
    // INTERNAL
    // ========================================================

    bool initDisplay(uint8_t index);

    void applyBrightness();

    static void flushCallback(
        lv_display_t* display,
        const lv_area_t* area,
        uint8_t* pxMap
    );

    // ========================================================
    // MEMBERS
    // ========================================================

    SPIManager& _spi;

    Adafruit_ST7789 _tft1;
    Adafruit_ST7789 _tft2;
    Adafruit_ST7789 _tft3;
    Adafruit_ST7789 _tft4;

    DisplayContext _contexts[Config::DISPLAY_COUNT];

    lv_color_t _buffers[Config::DISPLAY_COUNT][Config::LVGL_BUFFER_SIZE];

    bool     _initialized;
    uint32_t _lastUpdate;

    uint8_t _brightness;   // желаемая яркость 0..100
    uint8_t _applied;      // последняя применённая 0..100, 255 = не применена
};