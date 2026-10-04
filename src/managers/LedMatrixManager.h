#pragma once

#include <Arduino.h>

#include "./hardware/light/LedMatrix.h"
#include "./hardware/light/MatrixEffects.h"

class LedMatrixManager
{
public:
    using Effect = MatrixEffects::Effect;

    explicit LedMatrixManager(
        uint8_t dataPin
    );

    void begin();
    void update();

    // Power
    void on();
    void off();
    void toggle();

    bool isOn() const;

    // Brightness
    void setBrightness(
        uint8_t brightness
    );

    uint8_t brightness() const;

    // Basic drawing
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

    // Effects
    void setEffect(
        Effect effect
    );

    Effect effect() const;

    void stopEffect();

    // Effect settings
    void setEffectSpeed(
        uint8_t speed
    );

    uint8_t effectSpeed() const;

    void setTransitionTime(
        uint16_t milliseconds
    );

    uint16_t transitionTime() const;

    bool isTransitioning() const;

private:
    LedMatrix _matrix;
    MatrixEffects _effects;

    bool _isOn;
    uint8_t _brightness;
};