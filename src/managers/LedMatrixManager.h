#pragma once

#include <Arduino.h>
#include "./hardware/light/LedMatrix.h"

class LedMatrixManager
{
public:
    enum class Animation
    {
        None,
        Rainbow,
        Pulse,
        Wave,
        Scanner,
        Fire
    };

    explicit LedMatrixManager(
        uint8_t dataPin
    );

    void begin();
    void update();

    void on();
    void off();
    void toggle();

    bool isOn() const;

    void setBrightness(uint8_t brightness);
    uint8_t brightness() const;

    void clear();
    void show();

    void fill(
        uint8_t r,
        uint8_t g,
        uint8_t b
    );

    void setPixel(
        uint8_t x,
        uint8_t y,
        uint8_t r,
        uint8_t g,
        uint8_t b
    );

    void setAnimation(Animation animation);
    void stopAnimation();

    Animation animation() const;

private:
    // Hardware
    LedMatrix _matrix;

    // State
    bool _isOn = true;

    uint8_t _brightness = 30;

    Animation _animation = Animation::None;

    uint32_t _lastAnimationUpdate = 0;
    uint16_t _animationStep = 0;

    // Animations
    void updateRainbow();
    void updatePulse();
    void updateWave();
    void updateScanner();
    void updateFire();

    // Helpers
    uint32_t wheel(uint8_t position);
    uint8_t sin8(uint8_t value);
};