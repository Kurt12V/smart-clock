#pragma once

#include <Arduino.h>
#include "CobLed.h"

// ============================================================
// EFFECT TYPES
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
// ============================================================

class CobEffects
{
public:
    CobEffects(
        CobLed& cob1,
        CobLed& cob2,
        CobLed& cob3,
        CobLed& cob4
    );

    void begin();
    void update();

    void setEffect(CobEffectType effect);
    CobEffectType effect() const;

    void setSpeed(uint8_t speed);
    uint8_t speed() const;

    void setBrightness(
        uint8_t index,
        uint8_t value,
        bool applyNow = true
    );

    uint8_t brightness(uint8_t index) const;

    void setAllBrightness(uint8_t value);

    void setChannelEnabled(
        uint8_t index,
        bool enabled
    );

    bool isChannelEnabled(uint8_t index) const;

    void setEnabled(bool enabled);
    bool isEnabled() const;

private:
    static constexpr uint8_t LED_COUNT = 4;

    CobLed& _cob1;
    CobLed& _cob2;
    CobLed& _cob3;
    CobLed& _cob4;

    CobEffectType _effect;

    uint8_t _speed;

    uint8_t _base[LED_COUNT];

    bool _channelEnabled[LED_COUNT];

    bool _enabled;

    uint32_t _lastUpdate;
    uint32_t _phase;

    CobLed& led(uint8_t index);
    const CobLed& led(uint8_t index) const;

    void render();

    void apply(
        uint8_t index,
        uint8_t value
    );

    uint8_t triangle(uint32_t phase) const;

    void applyStatic();
    void applyBreath();
    void applyStrobe();
    void applyWave();
};