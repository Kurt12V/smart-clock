#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>

#include "./ui/Display.h"


class DisplayManager
{
public:

    static constexpr int16_t DISPLAY_WIDTH  = 172;
    static constexpr int16_t DISPLAY_HEIGHT = 320;

    static constexpr int16_t DISPLAY_COUNT = 4;

    static constexpr int16_t TOTAL_WIDTH =
        DISPLAY_WIDTH * DISPLAY_COUNT;


    explicit DisplayManager(
        Display& display
    );


    // --------------------------------------------------------
    // Инициализация
    // --------------------------------------------------------

    bool begin();


    // --------------------------------------------------------
    // Физические дисплеи
    // --------------------------------------------------------

    ST7789_172x320& get(
        uint8_t index
    );


    // --------------------------------------------------------
    // Размер виртуального экрана
    // --------------------------------------------------------

    int16_t width() const;

    int16_t height() const;


    // --------------------------------------------------------
    // Очистка
    // --------------------------------------------------------

    void clear(
        uint16_t color = ST77XX_BLACK
    );


    // --------------------------------------------------------
    // Графика
    // --------------------------------------------------------

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


    // --------------------------------------------------------
    // Текст
    // --------------------------------------------------------

    void drawText(
        const char* text,
        int16_t x,
        int16_t y,
        uint16_t color,
        const GFXfont* font
    );


    // --------------------------------------------------------
    // Цвет
    // --------------------------------------------------------

    uint16_t dimColor(
        uint16_t color,
        uint8_t amount
    );


private:

    Display& _display;


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
};