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
    // Яркость
    // ========================================================

    void setBrightness(uint8_t brightness);

    uint8_t getBrightness() const;

    // ========================================================
    // ON / OFF
    // ========================================================

    void on();

    void off();

    void toggle();

    bool isOn() const;

    // ========================================================
    // Изменение яркости
    // ========================================================

    void increase(uint8_t step = 5);

    void decrease(uint8_t step = 5);

    // ========================================================
    // Fade
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

    uint8_t _brightness = 0;

    bool _isOn = false;

    bool _initialized = false;

    // ========================================================
    // Fade
    // ========================================================

    bool _fading = false;

    uint8_t _fadeStart = 0;

    uint8_t _fadeTarget = 0;

    uint32_t _fadeStartTime = 0;

    uint32_t _fadeDuration = 0;

    // ========================================================
    // Gamma
    // ========================================================

    static uint8_t _gammaTable[256];

    static bool _gammaReady;

    static void buildGamma();

    uint8_t gamma(
        uint8_t value
    ) const;

    void apply();
};
