#pragma once

#include <Arduino.h>

#include "LedMatrix.h"

class LedMatrixEffects
{
public:

    // ============================================================
    // EFFECTS
    // ============================================================

    enum class Type : uint8_t
    {
        None = 0,

        Rainbow,
        Pulse,
        Wave,
        Scanner,
        Fire,
        Twinkle,
        Aurora,
        MatrixRain,

        Comet,
        Sparkle,
        ColorWipe,
        Meteor,
        Plasma,
        Fireworks,
        Ocean,
        Lava,
        Police,
        Confetti,
        Breathe,
        Galaxy,

        COUNT
    };

    // ============================================================
    // FIRE DIRECTION
    // ============================================================

    enum class FireDirection : uint8_t
    {
        BottomToTop = 0,
        LeftToRight,
        RightToLeft,
        TopToBottom
    };

    // ============================================================
    // CONSTRUCTOR
    // ============================================================

    explicit LedMatrixEffects(
        LedMatrix& matrix
    );

    // ============================================================
    // LIFECYCLE
    // ============================================================

    void begin();

    void reset();

    void update();

    // ============================================================
    // EFFECT
    // ============================================================

    void setType(
        Type type
    );

    Type type() const;

    // ============================================================
    // SPEED
    // ============================================================

    void setSpeed(
        uint8_t speed
    );

    uint8_t speed() const;

    // ============================================================
    // FIRE
    // ============================================================

    void setFireDirection(
        FireDirection direction
    );

    FireDirection fireDirection() const;

private:

    // ============================================================
    // MATRIX
    // ============================================================

    LedMatrix& _matrix;

    // ============================================================
    // STATE
    // ============================================================

    Type _type;

    uint8_t _speed;

    uint32_t _step;

    uint32_t _lastUpdate;

    // ============================================================
    // FIRE
    // ============================================================

    FireDirection _fireDirection;

    uint8_t _fireHeat[
        LedMatrix::WIDTH
    ][
        LedMatrix::HEIGHT
    ];

    static constexpr uint8_t FIRE_COOLING = 55;
    static constexpr uint8_t FIRE_SPARKING = 120;

    // ============================================================
    // TWINKLE
    // ============================================================

    uint8_t _twinkleLevel[
        LedMatrix::LED_COUNT
    ];

    uint8_t _twinkleHue[
        LedMatrix::LED_COUNT
    ];

    // ============================================================
    // RAIN
    // ============================================================

    int16_t _rainPosition[
        LedMatrix::WIDTH
    ];

    // ============================================================
    // SPARKLE
    // ============================================================

    uint8_t _sparkleLevel[
        LedMatrix::LED_COUNT
    ];

    uint8_t _sparkleHue[
        LedMatrix::LED_COUNT
    ];

    // ============================================================
    // RENDER
    // ============================================================

    void renderRainbow();
    void renderPulse();
    void renderWave();
    void renderScanner();
    void renderFire();
    void renderTwinkle();
    void renderAurora();
    void renderMatrixRain();

    void renderComet();
    void renderSparkle();
    void renderColorWipe();
    void renderMeteor();
    void renderPlasma();
    void renderFireworks();
    void renderOcean();
    void renderLava();
    void renderPolice();
    void renderConfetti();
    void renderBreathe();
    void renderGalaxy();

    // ============================================================
    // HELPERS
    // ============================================================

    uint8_t sin8(
        uint16_t value
    );

    uint8_t scale8(
        uint8_t value,
        uint8_t scale
    );

    uint32_t wheel(
        uint8_t position
    );

    uint32_t scaleColor(
        uint32_t color,
        uint8_t scale
    );

    uint32_t fireColor(
        uint8_t temperature
    );

    // ============================================================
    // RESET HELPERS
    // ============================================================

    void resetFire();
    void resetTwinkle();
    void resetRain();
    void resetSparkle();

    // ============================================================
    // TIMING
    // ============================================================

    uint16_t updateInterval() const;
};