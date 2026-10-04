
// ============================================================
// CobLed.h
// ============================================================

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

    // ========================================================
    // BRIGHTNESS
    // ========================================================

    void setBrightness(uint8_t brightness);

    uint8_t getBrightness() const;

    void increase(uint8_t step = 5);

    void decrease(uint8_t step = 5);

    // ========================================================
    // ON / OFF
    // ========================================================

    void on();

    void off();

    void toggle();

    bool isOn() const;

    // ========================================================
    // FADE
    // ========================================================

    void fadeTo(
        uint8_t target,
        uint32_t durationMs
    );

    void update();

    bool isFading() const;

private:

    uint8_t _pin;
    uint8_t _channel;

    uint32_t _frequency;
    uint8_t _resolution;

    // 0..100
    uint8_t _brightness = 0;

    bool _isOn = false;
    bool _initialized = false;

    // ========================================================
    // FADE
    // ========================================================

    bool _fading = false;

    uint8_t _fadeStart = 0;
    uint8_t _fadeTarget = 0;

    uint32_t _fadeStartTime = 0;
    uint32_t _fadeDuration = 0;

private:

    uint8_t toPwm(
        uint8_t brightness
    ) const;

    void apply();
};
