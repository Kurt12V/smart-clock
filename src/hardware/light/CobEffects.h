#pragma once

#include <Arduino.h>

#include "./managers/CobLedManager.h"

// ============================================================
// EFFECT TYPE
// ============================================================

enum class CobEffectType : uint8_t
{
    Static = 0,
    Breath,
    Strobe,
    Wave,
    COUNT
};


// ============================================================
// COB EFFECTS
//
// Управляет четырьмя COB через CobLedManager.
//
// Внешний диапазон яркости:
//     0..100
//
// В CobLedManager:
//     0..255
// ============================================================

class CobEffects
{
public:

    explicit CobEffects(
        CobLedManager& manager
    );

    void begin();

    void update();

    // ========================================================
    // EFFECT
    // ========================================================

    void setEffect(
        CobEffectType effect
    );

    CobEffectType effect() const
    {
        return _effect;
    }

    // ========================================================
    // SPEED
    // ========================================================

    void setSpeed(
        uint8_t speed
    );

    uint8_t speed() const
    {
        return _speed;
    }

    // ========================================================
    // BRIGHTNESS
    // ========================================================

    void setBrightness(
        uint8_t index,
        uint8_t value
    );

    uint8_t brightness(
        uint8_t index
    ) const;

    void setAllBrightness(
        uint8_t value
    );

    // ========================================================
    // ENABLE
    // ========================================================

    void setEnabled(
        bool enabled
    );

    bool isEnabled() const
    {
        return _enabled;
    }

private:

    static const uint8_t LED_COUNT = 4;

    CobLedManager& _manager;

    CobEffectType _effect;

    uint8_t _speed;

    uint8_t _base[LED_COUNT];

    bool _enabled;

    uint32_t _lastUpdate;
    uint32_t _phase;

private:

    void apply(
        uint8_t index,
        uint8_t value
    );

    uint8_t triangle(
        uint32_t phase
    ) const;

    void applyStatic();

    void applyBreath();

    void applyStrobe();

    void applyWave();
};