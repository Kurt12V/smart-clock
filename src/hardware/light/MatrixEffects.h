#pragma once

#include <Arduino.h>
#include "./hardware/light/LedMatrix.h"

class MatrixEffects
{
public:
    enum class Effect : uint8_t
    {
        None = 0,
        Rainbow,
        RainbowWave,
        Fire,
        Fireplace,
        Plasma,
        Aurora,
        Ocean,
        Twinkle,
        Meteor,
        Matrix,
        COUNT
    };

    explicit MatrixEffects(LedMatrix& matrix);

    void begin();
    void update();

    void setEffect(Effect effect);
    Effect effect() const;

    void setTransitionTime(uint16_t milliseconds);
    uint16_t transitionTime() const;

    void setSpeed(uint8_t speed);
    uint8_t speed() const;

    bool isTransitioning() const;

private:
    LedMatrix& _matrix;

    Effect _currentEffect;
    Effect _nextEffect;

    bool _transitioning;

    uint16_t _transitionTime;
    uint32_t _transitionStart;

    uint8_t _speed;

    uint32_t _lastUpdate;
    uint32_t _step;

    // Transition frame buffers.
    uint32_t _oldFrame[LedMatrix::LED_COUNT];
    uint32_t _newFrame[LedMatrix::LED_COUNT];

    // Effects.
    void render(
        Effect effect,
        uint32_t* buffer
    );

    void renderRainbow(uint32_t* buffer);
    void renderRainbowWave(uint32_t* buffer);
    void renderFire(uint32_t* buffer);
    void renderFireplace(uint32_t* buffer);
    void renderPlasma(uint32_t* buffer);
    void renderAurora(uint32_t* buffer);
    void renderOcean(uint32_t* buffer);
    void renderTwinkle(uint32_t* buffer);
    void renderMeteor(uint32_t* buffer);
    void renderMatrix(uint32_t* buffer);

    // Frame helpers.
    void clearBuffer(uint32_t* buffer);

    void setPixel(
        uint32_t* buffer,
        uint8_t x,
        uint8_t y,
        uint8_t r,
        uint8_t g,
        uint8_t b
    );

    uint32_t color( uint8_t r, uint8_t g, uint8_t b ) const;
    void mixFrames(
        uint8_t amount
    );

    uint32_t mixColor(
        uint32_t a,
        uint32_t b,
        uint8_t amount
    );

    uint8_t getR(uint32_t color) const;
    uint8_t getG(uint32_t color) const;
    uint8_t getB(uint32_t color) const;

    uint16_t index(
        uint8_t x,
        uint8_t y
    ) const;

    uint8_t sin8(uint8_t value) const;

    uint32_t wheel(
        uint8_t position
    ) const;

    uint32_t hsv(
        uint8_t h,
        uint8_t s,
        uint8_t v
    ) const;

    uint8_t random8(
        uint8_t min,
        uint8_t max
    ) const;
};
