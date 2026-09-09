#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>


// Количество физических дисплеев.
static constexpr uint8_t DISPLAY_COUNT = 4;


// ============================================================
// ST7789 172x320
// ============================================================

class ST7789_172x320 : public Adafruit_ST7789
{
public:
    ST7789_172x320(
        int8_t cs,
        int8_t dc,
        int8_t rst
    );

    void begin172x320();
};


// ============================================================
// DISPLAY MANAGER
// ============================================================

class Display
{
public:
    Display();
    ~Display();

    // Запускает все четыре экрана. Возвращает true при успехе.
    bool begin();

    // Возвращает экран по номеру: 0, 1, 2 или 3.
    ST7789_172x320& get(uint8_t index);

    // Включает или выключает общую подсветку экранов.
    void setBacklight(bool state);

private:
    ST7789_172x320* _display[DISPLAY_COUNT];
    bool _initialized;
};


// Если в остальных файлах проекта используется старое имя:
using DisplayManager = Display;
