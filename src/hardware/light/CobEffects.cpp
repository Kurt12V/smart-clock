#include "CobEffects.h"


// ============================================================
// Constructor
// ============================================================

CobEffects::CobEffects(
    CobLed& cob1,
    CobLed& cob2,
    CobLed& cob3,
    CobLed& cob4
)
    : _cob1(cob1),
      _cob2(cob2),
      _cob3(cob3),
      _cob4(cob4),
      _effect(CobEffectType::Static),
      _speed(50),
      _enabled(true),
      _lastUpdate(0),
      _phase(0)
{
    for (uint8_t i = 0; i < LED_COUNT; ++i)
        _base[i] = 100;
}


// ============================================================
// Begin
// ============================================================

void CobEffects::begin()
{
    _phase = 0;
    _lastUpdate = millis();

    applyStatic();
}


// ============================================================
// Get LED
// ============================================================

CobLed& CobEffects::led(
    uint8_t index
)
{
    switch (index)
    {
        case 0: return _cob1;
        case 1: return _cob2;
        case 2: return _cob3;
        case 3: return _cob4;

        default:
            return _cob1;
    }
}


const CobLed& CobEffects::led(
    uint8_t index
) const
{
    switch (index)
    {
        case 0: return _cob1;
        case 1: return _cob2;
        case 2: return _cob3;
        case 3: return _cob4;

        default:
            return _cob1;
    }
}


// ============================================================
// Set effect
// ============================================================

void CobEffects::setEffect(
    CobEffectType effect
)
{
    if (
        static_cast<uint8_t>(effect) >=
        static_cast<uint8_t>(CobEffectType::COUNT)
    )
    {
        effect = CobEffectType::Static;
    }

    if (_effect == effect)
        return;

    _effect = effect;

    _phase = 0;
    _lastUpdate = millis();

    if (_enabled && _effect == CobEffectType::Static)
        applyStatic();
}


// ============================================================
// Set speed
// ============================================================

void CobEffects::setSpeed(
    uint8_t speed
)
{
    _speed =
        constrain(
            speed,
            0,
            100
        );
}


// ============================================================
// Set brightness
// ============================================================

void CobEffects::setBrightness(
    uint8_t index,
    uint8_t value
)
{
    if (index >= LED_COUNT)
        return;

    _base[index] =
        constrain(
            value,
            0,
            100
        );

    if (_effect == CobEffectType::Static)
    {
        apply(
            index,
            _enabled
                ? _base[index]
                : 0
        );
    }
}


// ============================================================
// Get brightness
// ============================================================

uint8_t CobEffects::brightness(
    uint8_t index
) const
{
    if (index >= LED_COUNT)
        return 0;

    return _base[index];
}


// ============================================================
// Set all brightness
// ============================================================

void CobEffects::setAllBrightness(
    uint8_t value
)
{
    value =
        constrain(
            value,
            0,
            100
        );

    for (uint8_t i = 0; i < LED_COUNT; ++i)
        _base[i] = value;

    if (_effect == CobEffectType::Static)
        applyStatic();
}


// ============================================================
// Enable
// ============================================================

void CobEffects::setEnabled(
    bool enabled
)
{
    if (_enabled == enabled)
        return;

    _enabled = enabled;

    if (!_enabled)
    {
        _cob1.off();
        _cob2.off();
        _cob3.off();
        _cob4.off();

        return;
    }

    _phase = 0;
    _lastUpdate = millis();

    if (_effect == CobEffectType::Static)
        applyStatic();
}


// ============================================================
// Update
// ============================================================

void CobEffects::update()
{
    if (!_enabled)
        return;

    const uint32_t now = millis();

    if (now - _lastUpdate < 20)
        return;

    _lastUpdate = now;

    const uint32_t period =
        3000UL -
        static_cast<uint32_t>(_speed) * 25UL;

    _phase =
        (now % period) *
        1000UL /
        period;

    switch (_effect)
    {
        case CobEffectType::Static:
            break;

        case CobEffectType::Breath:
            applyBreath();
            break;

        case CobEffectType::Strobe:
            applyStrobe();
            break;

        case CobEffectType::Wave:
            applyWave();
            break;

        default:
            break;
    }
}


// ============================================================
// Apply
// ============================================================

void CobEffects::apply(
    uint8_t index,
    uint8_t value
)
{
    if (index >= LED_COUNT)
        return;

    if (!_enabled)
    {
        led(index).off();
        return;
    }

    led(index).setBrightness(value);
}


// ============================================================
// Triangle
// ============================================================

uint8_t CobEffects::triangle(
    uint32_t phase
) const
{
    phase %= 1000;

    if (phase < 500)
    {
        return static_cast<uint8_t>(
            phase * 100UL / 500UL
        );
    }

    return static_cast<uint8_t>(
        (1000UL - phase) *
        100UL /
        500UL
    );
}


// ============================================================
// Static
// ============================================================

void CobEffects::applyStatic()
{
    if (!_enabled)
        return;

    for (uint8_t i = 0; i < LED_COUNT; ++i)
        apply(i, _base[i]);
}


// ============================================================
// Breath
// ============================================================

void CobEffects::applyBreath()
{
    const uint8_t factor =
        triangle(_phase);

    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        const uint8_t value =
            static_cast<uint8_t>(
                static_cast<uint16_t>(_base[i]) *
                factor /
                100
            );

        apply(i, value);
    }
}


// ============================================================
// Strobe
// ============================================================

void CobEffects::applyStrobe()
{
    const uint8_t factor =
        (_phase < 500)
            ? 100
            : 0;

    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        const uint8_t value =
            static_cast<uint8_t>(
                static_cast<uint16_t>(_base[i]) *
                factor /
                100
            );

        apply(i, value);
    }
}


// ============================================================
// Wave
// ============================================================

void CobEffects::applyWave()
{
    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        const uint32_t phase =
            (
                _phase +
                static_cast<uint32_t>(i) * 250UL
            ) % 1000UL;

        const uint8_t factor =
            triangle(phase);

        const uint8_t value =
            static_cast<uint8_t>(
                static_cast<uint16_t>(_base[i]) *
                factor /
                100
            );

        apply(i, value);
    }
}
