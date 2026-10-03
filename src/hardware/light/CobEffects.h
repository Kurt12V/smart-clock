#pragma once

#include <Arduino.h>

#include "./hardware/light/CobLed.h"

// ============================================================
// EFFECT TYPE
// ============================================================

enum class CobEffectType : uint8_t
{
    Static = 0,
    Breath,
    Strobe,
    Wave,
    Blink,
    Pulse,

    COUNT
};

// ============================================================
// COB EFFECT ENGINE
//
// Responsible ONLY for dynamic effects.
//
// It does not know about:
// - SettingsManager
// - CobLedManager
// - AlarmManager
// - WebServer
//
// It only controls CobLed outputs.
// ============================================================

class CobEffects
{
public:

    static constexpr uint8_t LED_COUNT = 4;

    CobEffects(
        CobLed& cob1,
        CobLed& cob2,
        CobLed& cob3,
        CobLed& cob4
    );

    // ============================================================
    // LIFECYCLE
    // ============================================================

    void begin();
    void update();

    // ============================================================
    // EFFECT
    // ============================================================

    void setEffect(
        CobEffectType effect
    );

    CobEffectType effect() const;

    // ============================================================
    // SPEED
    //
    // 0..100
    // ============================================================

    void setSpeed(
        uint8_t speed
    );

    uint8_t speed() const;

    // ============================================================
    // ENABLE
    // ============================================================

    void setEnabled(
        bool enabled
    );

    bool isEnabled() const;

    // ============================================================
    // BASE BRIGHTNESS
    //
    // 0..255
    // ============================================================

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

    // ============================================================
    // RESET
    // ============================================================

    void reset();

private:

    CobLed* _leds[LED_COUNT];

    CobEffectType _effect;

    uint8_t _speed;

    uint8_t _base[LED_COUNT];

    bool _enabled;

    uint32_t _lastUpdate;
    uint32_t _phase;

    // ============================================================
    // TIMING
    // ============================================================

    uint32_t calculatePeriod() const;

    // ============================================================
    // EFFECTS
    // ============================================================

    void applyStatic();
    void applyBreath();
    void applyStrobe();
    void applyWave();
    void applyBlink();
    void applyPulse();

    // ============================================================
    // MATH
    // ============================================================

    uint8_t triangle(
        uint32_t phase
    ) const;

    uint8_t sineWave(
        uint32_t phase
    ) const;

    // ============================================================
    // OUTPUT
    // ============================================================

    void output(
        uint8_t index,
        uint8_t value
    );

    void outputAll(
        uint8_t value
    );
};