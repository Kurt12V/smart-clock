#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include <lvgl.h>

#include "managers/SPIManager.h"

// ============================================================
// LVGL MANAGER
// ============================================================

class LVGLManager
{
public:

    static constexpr uint8_t DISPLAY_COUNT = 4;

    // --------------------------------------------------------
    // Logical display size
    // --------------------------------------------------------

    static constexpr uint16_t WIDTH  = 172;
    static constexpr uint16_t HEIGHT = 320;

    // --------------------------------------------------------
    // Physical ST7789 size
    // --------------------------------------------------------

    static constexpr uint16_t TFT_WIDTH  = 240;
    static constexpr uint16_t TFT_HEIGHT = 320;

    // --------------------------------------------------------
    // Panel offset
    // --------------------------------------------------------

    static constexpr int16_t X_OFFSET = 34;
    static constexpr int16_t Y_OFFSET = 0;

    // --------------------------------------------------------
    // LVGL partial buffer
    // --------------------------------------------------------

    static constexpr uint16_t BUFFER_LINES = 32;

    static constexpr uint32_t BUFFER_SIZE =
        WIDTH * BUFFER_LINES;

    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    explicit LVGLManager(SPIManager& spi);

    // ========================================================
    // LIFECYCLE
    // ========================================================

    bool begin();

    void update();

    bool isReady() const;

    // ========================================================
    // DISPLAY CONTROL
    // ========================================================

    void clearDisplays();

    void refresh();

    // ========================================================
    // TEST
    // ========================================================

    bool testDisplays();

    // ========================================================
    // DISPLAY
    // ========================================================

    lv_display_t* display(
        uint8_t index
    );

private:

    // ========================================================
    // DISPLAY CONTEXT
    // ========================================================

    struct DisplayContext
    {
        uint8_t index;

        Adafruit_ST7789* tft;

        lv_display_t* lvDisplay;

        lv_color_t* buffer;

        bool initialized;
    };

    // ========================================================
    // INTERNAL
    // ========================================================

    bool initDisplay(
        uint8_t index
    );

    static void flushCallback(
        lv_display_t* display,
        const lv_area_t* area,
        uint8_t* pxMap
    );

    void flush(
        const lv_area_t* area,
        uint8_t* pxMap
    );

    // ========================================================
    // SPI
    // ========================================================

    SPIManager& _spi;

    // ========================================================
    // TFT
    // ========================================================

    Adafruit_ST7789 _tft1;
    Adafruit_ST7789 _tft2;
    Adafruit_ST7789 _tft3;
    Adafruit_ST7789 _tft4;

    // ========================================================
    // CONTEXTS
    // ========================================================

    DisplayContext _contexts[DISPLAY_COUNT];

    // ========================================================
    // LVGL BUFFERS
    // ========================================================

    lv_color_t _buffers[
        DISPLAY_COUNT
    ][
        BUFFER_SIZE
    ];

    // ========================================================
    // STATE
    // ========================================================

    bool _initialized;

    uint32_t _lastUpdate;
};