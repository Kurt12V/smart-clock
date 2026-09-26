#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class LedMatrix
{
public:
    static constexpr uint8_t WIDTH = 16;
    static constexpr uint8_t HEIGHT = 16;
    static constexpr uint16_t LED_COUNT = WIDTH * HEIGHT;

    // Максимальная яркость: ~30%
    static constexpr uint8_t MAX_BRIGHTNESS = 76;

    LedMatrix(
        uint8_t dataPin,
        uint8_t brightness = 30
    );

    void begin();

    void show();
    void clear();

    void setBrightness(uint8_t brightness);
    uint8_t brightness() const;

    void setPixel(
        uint8_t x,
        uint8_t y,
        uint8_t r,
        uint8_t g,
        uint8_t b
    );

    void setPixel(
        uint8_t x,
        uint8_t y,
        uint32_t color
    );

    void fill(
        uint8_t r,
        uint8_t g,
        uint8_t b
    );

    void fill(uint32_t color);

    uint32_t color(
        uint8_t r,
        uint8_t g,
        uint8_t b
    );

private:
    Adafruit_NeoPixel _matrix;

    uint8_t _brightness;

    uint16_t index(
        uint8_t x,
        uint8_t y
    ) const;
};