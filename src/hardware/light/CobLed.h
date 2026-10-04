#pragma once

#include <Arduino.h>

class CobLed
{
public:
    CobLed(
        uint8_t pin,
        uint8_t channel,
        uint32_t frequency = 1000,
        uint8_t resolution = 8
    );

    bool begin();

    // Brightness: 0..100 %
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() const;

    void on();
    void off();
    void toggle();

    bool isOn() const;

    void increase(uint8_t step = 5);
    void decrease(uint8_t step = 5);

    void fadeTo(uint8_t target, uint32_t durationMs);

    void update();

    bool isFading() const;

private:
    uint8_t _pin;
    uint8_t _channel;

    uint32_t _frequency;
    uint8_t _resolution;

    uint8_t _brightness;
    bool _isOn;
    bool _initialized;

    // Fade
    bool _fading;
    uint8_t _fadeStart;
    uint8_t _fadeTarget;
    uint32_t _fadeStartTime;
    uint32_t _fadeDuration;

    uint8_t brightnessToPwm(uint8_t brightness) const;

    void apply();
};