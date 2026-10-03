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

    // ============================================================
    // LIFECYCLE
    // ============================================================

    bool begin();
    void update();

    // ============================================================
    // BRIGHTNESS
    //
    // 0..255
    // ============================================================

    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() const;

    // ============================================================
    // POWER
    // ============================================================

    void on();
    void off();
    void toggle();

    bool isOn() const;

    // ============================================================
    // BRIGHTNESS CONTROL
    // ============================================================

    void increase(uint8_t step = 5);
    void decrease(uint8_t step = 5);

    // ============================================================
    // FADE
    // ============================================================

    void fadeTo(
        uint8_t target,
        uint32_t durationMs
    );

    bool isFading() const;

    // ============================================================
    // DIRECT OUTPUT
    //
    // Used by CobEffects.
    //
    // This changes only physical PWM output.
    // Base brightness remains unchanged.
    // ============================================================

    void output(uint8_t brightness);

    uint8_t getOutputBrightness() const;

private:

    // ============================================================
    // HARDWARE
    // ============================================================

    uint8_t _pin;
    uint8_t _channel;

    uint32_t _frequency;
    uint8_t _resolution;

    bool _initialized;

    // ============================================================
    // NORMAL STATE
    // ============================================================

    uint8_t _brightness;
    bool _isOn;

    // ============================================================
    // CURRENT PHYSICAL OUTPUT
    // ============================================================

    uint8_t _outputBrightness;

    // ============================================================
    // FADE
    // ============================================================

    bool _fading;

    uint8_t _fadeStart;
    uint8_t _fadeTarget;

    uint32_t _fadeStartTime;
    uint32_t _fadeDuration;

    // ============================================================
    // GAMMA
    // ============================================================

    static uint8_t _gammaTable[256];
    static bool _gammaReady;

    static void buildGamma();

    uint8_t gamma(
        uint8_t value
    ) const;

    void apply();
};