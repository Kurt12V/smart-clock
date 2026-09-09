#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

class ST7789_172x320 : public Adafruit_ST7789
{
public:
    ST7789_172x320(
        int8_t cs,
        int8_t dc,
        int8_t rst
    );

    void begin172x320();
    void select();
    void deselect();
};

class Display
{
public:
    static constexpr uint8_t DISPLAY_COUNT = 4;

    Display();

    bool begin();
    bool isInitialized() const;

    ST7789_172x320& get(uint8_t index);
    ST7789_172x320* getPtr(uint8_t index);

    void select(uint8_t index);
    void deselectAll();

    void clear(uint16_t color = ST77XX_BLACK);
    void clear(
        uint8_t index,
        uint16_t color = ST77XX_BLACK
    );

    void setBacklight(bool state);
    bool backlightState() const;

    void beginTransaction();
    void endTransaction();

private:
    ST7789_172x320 _display[DISPLAY_COUNT];

    bool _initialized;
    bool _backlight;

    void setupPins();
    bool initializeDisplay(uint8_t index);
};