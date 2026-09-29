#pragma once

#include <Arduino.h>
#include "CobLedManager.h"

// ============================================================
// EFFECT TYPE
// ============================================================

enum class CobEffectType : uint8_t
{
    Static = 0,   // постоянная яркость
    Breath,       // плавное дыхание
    Strobe,       // мигание
    Wave,         // бегущая волна по 4 LED
    COUNT
};

// ============================================================
// COB EFFECTS
//
// Управляет эффектами через CobLedManager.
// Наружу работает в 0..100, внутри мапит в 0..255.
// ============================================================

class CobEffects
{
public:

    explicit CobEffects(CobLedManager& manager);

    void begin();
    void update();

    // ------------------------------------
    // EFFECT
    // ------------------------------------

    void setEffect(CobEffectType e);
    CobEffectType effect() const { return _effect; }

    // ------------------------------------
    // SPEED (0..100)
    // ------------------------------------

    void setSpeed(uint8_t speed);
    uint8_t speed() const { return _speed; }

    // ------------------------------------
    // BRIGHTNESS (0..100)
    // ------------------------------------

    void setBrightness(uint8_t index, uint8_t value);  // index 0..3
    uint8_t brightness(uint8_t index) const;
    void setAllBrightness(uint8_t value);

    // ------------------------------------
    // ENABLE
    // ------------------------------------

    void setEnabled(bool enabled);
    bool isEnabled() const { return _enabled; }

private:

    static constexpr uint8_t LED_COUNT = 4;

    CobLedManager& _manager;

    CobEffectType _effect;
    uint8_t       _speed;
    uint8_t       _base[LED_COUNT];
    bool          _enabled;

    uint32_t _lastUpdate;
    uint32_t _phase;

    // helpers
    void    apply(uint8_t index, uint8_t value);   // 0..100 → через manager
    uint8_t triangle(uint32_t p) const;

    void applyStatic();
    void applyBreath();
    void applyStrobe();
    void applyWave();
};