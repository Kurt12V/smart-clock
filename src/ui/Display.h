#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

class ST7789_172x320 : public Adafruit_ST7789
{
public:

    ST7789_172x320(
        int8_t cs,
        int8_t dc,
        int8_t rst,
        int8_t mosi,
        int8_t sclk
    );

    void begin172x320();
};


class Display
{
public:

    Display();
    ~Display();

    bool begin();

    ST7789_172x320& get(uint8_t index);

    void setBacklight(bool state);

private:

    ST7789_172x320* _display[4];

    bool _initialized;
};