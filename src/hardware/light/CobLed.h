#pragma once

#include <Arduino.h>

class CobLed
{
public:
    CobLed(
        uint8_t pin,
        uint8_t channel = 0,
        uint32_t frequency = 1000,
        uint8_t resolution = 8
    );

    void begin();

    void setBrightness(uint8_t brightness);

    void increase(uint8_t step = 5);
    void decrease(uint8_t step = 5);

    void on();
    void off();
    void toggle();

    bool isOn() const;
    uint8_t getBrightness() const;

private:
    uint8_t _pin;
    uint8_t _channel;
    uint32_t _frequency;
    uint8_t _resolution;

    uint8_t _brightness = 128;
    bool _isOn = true;

    void apply();
};