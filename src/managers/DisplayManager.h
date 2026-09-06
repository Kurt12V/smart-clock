#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>

#include "Display.h"


class DisplayManager
{
public:

    // ========================================================
    // Размер одного дисплея
    // ========================================================

    static constexpr int16_t DISPLAY_WIDTH  = 172;
    static constexpr int16_t DISPLAY_HEIGHT = 320;

    static constexpr uint8_t DISPLAY_COUNT = 4;

    static constexpr int16_t TOTAL_WIDTH =
        DISPLAY_WIDTH * DISPLAY_COUNT;

    static constexpr int16_t TOTAL_HEIGHT =
        DISPLAY_HEIGHT;


    explicit DisplayManager(
        Display& display
    );


    // ========================================================
    // Инициализация
    // ========================================================

    bool begin();

    bool isInitialized() const;


    // ========================================================
    // Физические дисплеи
    // ========================================================

    ST7789_172x320& get(
        uint8_t index
    );

    ST7789_172x320* getPtr(
        uint8_t index
    );


    // ========================================================
    // Очистка
    // ========================================================

    void clear(
        uint16_t color = ST77XX_BLACK
    );

    void clearDisplay(
        uint8_t index,
        uint16_t color = ST77XX_BLACK
    );


    // ========================================================
    // Общий виртуальный Canvas
    //
    // X:
    //
    // 0   ... 171   = Display 1
    // 172 ... 343   = Display 2
    // 344 ... 515   = Display 3
    // 516 ... 687   = Display 4
    // ========================================================

    void drawPixel(
        int16_t x,
        int16_t y,
        uint16_t color
    );

    void drawFastHLine(
        int16_t x,
        int16_t y,
        int16_t width,
        uint16_t color
    );

    void drawFastVLine(
        int16_t x,
        int16_t y,
        int16_t height,
        uint16_t color
    );

    void fillRect(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        uint16_t color
    );

    void drawRect(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        uint16_t color
    );

    void fillCircle(
        int16_t x,
        int16_t y,
        int16_t radius,
        uint16_t color
    );

    void drawCircle(
        int16_t x,
        int16_t y,
        int16_t radius,
        uint16_t color
    );


    // ========================================================
    // Текст на виртуальном Canvas
    // ========================================================

    void drawText(
        const char* text,
        int16_t x,
        int16_t y,
        uint16_t color,
        const GFXfont* font
    );


    // ========================================================
    // Работа с координатами
    // ========================================================

    bool validPoint(
        int16_t x,
        int16_t y
    ) const;

    uint8_t displayIndex(
        int16_t x
    ) const;

    int16_t localX(
        int16_t x
    ) const;


    // ========================================================
    // Цвет
    // ========================================================

    uint16_t dimColor(
        uint16_t color,
        uint8_t amount
    );


private:

    Display& _display;

    bool _initialized;


    // ========================================================
    // Внутренние функции
    // ========================================================

    bool validRect(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height
    ) const;

    void drawTextSingleDisplay(
        ST7789_172x320& tft,
        const char* text,
        int16_t x,
        int16_t y,
        uint16_t color,
        const GFXfont* font
    );
};
