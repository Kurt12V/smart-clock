#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class LedMatrix
{
public:

    // ============================================================
    // SIZE
    // ============================================================

    static constexpr uint8_t WIDTH  = 16;
    static constexpr uint8_t HEIGHT = 16;

    static constexpr uint16_t LED_COUNT =
        WIDTH * HEIGHT;

    static constexpr uint8_t MAX_BRIGHTNESS = 255;

    // ============================================================
    // CONSTRUCTOR
    // ============================================================

    explicit LedMatrix(
        uint8_t dataPin,
        uint8_t brightness = 20
    );

    // ============================================================
    // LIFECYCLE
    // ============================================================

    void begin();

    // ============================================================
    // OUTPUT
    // ============================================================

    void show();

    void clear();

    // ============================================================
    // BRIGHTNESS
    // ============================================================

    void setBrightness(
        uint8_t brightness
    );

    uint8_t brightness() const;

    // ============================================================
    // PIXEL
    // ============================================================

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

    uint32_t getPixel(
        uint8_t x,
        uint8_t y
    ) const;

    // ============================================================
    // FILL
    // ============================================================

    void fill(
        uint8_t r,
        uint8_t g,
        uint8_t b
    );

    void fill(
        uint32_t color
    );

    // ============================================================
    // COLOR
    // ============================================================

    uint32_t color(
        uint8_t r,
        uint8_t g,
        uint8_t b
    ) const;

    // ============================================================
    // RAW ACCESS
    // ============================================================

    uint32_t pixel(
        uint16_t index
    ) const;

    void setPixelRaw(
        uint16_t index,
        uint32_t color
    );

    // ============================================================
    // INDEX
    // ============================================================

    uint16_t index(
        uint8_t x,
        uint8_t y
    ) const;

private:

    Adafruit_NeoPixel _matrix;

    uint8_t _brightness;
};