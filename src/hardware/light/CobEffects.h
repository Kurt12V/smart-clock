
#pragma once

#include <Arduino.h>

#include "./hardware/light/CobLed.h"


enum class CobEffectType : uint8_t
{
    Static = 0,
    Breath,
    Strobe,
    Wave,
    COUNT
};


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

    static constexpr uint8_t LED_COUNT = 4;

    CobLed& _cob1;
    CobLed& _cob2;
    CobLed& _cob3;
    CobLed& _cob4;

    CobEffectType _effect;

    uint8_t _speed;

    uint8_t _base[LED_COUNT];

    bool _enabled;

    uint32_t _lastUpdate;
    uint32_t _phase;

private:

    CobLed& led(
        uint8_t index
    );

    const CobLed& led(
        uint8_t index
    ) const;

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
