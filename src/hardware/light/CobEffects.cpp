#include "CobEffects.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

CobEffects::CobEffects(
    CobLedManager& manager
)
    : _manager(manager),
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
// BEGIN
// ============================================================

void CobEffects::begin()
{
    _phase = 0;

    _lastUpdate = millis();

    applyStatic();
}


// ============================================================
// SET EFFECT
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

    _effect = effect;

    _phase = 0;

    _lastUpdate = millis();

    if (_effect == CobEffectType::Static)
        applyStatic();
}


// ============================================================
// SET SPEED
// ============================================================

void CobEffects::setSpeed(
    uint8_t speed
)
{
    _speed = constrain(
        speed,
        0,
        100
    );
}


// ============================================================
// SET BRIGHTNESS
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
// GET BRIGHTNESS
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
// SET ALL BRIGHTNESS
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
    {
        _base[i] = value;
    }

    if (_effect == CobEffectType::Static)
        applyStatic();
}


// ============================================================
// ENABLE
// ============================================================

void CobEffects::setEnabled(
    bool enabled
)
{
    _enabled = enabled;

    if (!_enabled)
    {
        for (uint8_t i = 0; i < LED_COUNT; ++i)
            apply(i, 0);

        return;
    }

    _phase = 0;

    _lastUpdate = millis();

    if (_effect == CobEffectType::Static)
        applyStatic();
}


// ============================================================
// UPDATE
// ============================================================

void CobEffects::update()
{
    if (!_enabled)
        return;

    const uint32_t now = millis();

    if (now - _lastUpdate < 20)
        return;

    _lastUpdate = now;

    // Speed 0   -> 3000 ms
    // Speed 100 -> 500 ms
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
// APPLY
//
// 0..100 -> 0..255
//
// CobLedManager использует номера COB:
//     1
//     2
//     3
//     4
// ============================================================

void CobEffects::apply(
    uint8_t index,
    uint8_t value
)
{
    if (index >= LED_COUNT)
        return;

    const uint8_t brightness255 =
        static_cast<uint8_t>(
            static_cast<uint32_t>(value) *
            255UL /
            100UL
        );

    _manager.set(
        index + 1,
        brightness255
    );
}


// ============================================================
// TRIANGLE
//
// 0..999 -> 0..100..0
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
// STATIC
// ============================================================

void CobEffects::applyStatic()
{
    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        apply(
            i,
            _enabled
                ? _base[i]
                : 0
        );
    }
}


// ============================================================
// BREATH
// ============================================================

void CobEffects::applyBreath()
{
    const uint8_t factor =
        triangle(_phase);

    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        const uint8_t value =
            static_cast<uint8_t>(
                static_cast<uint32_t>(_base[i]) *
                factor /
                100UL
            );

        apply(
            i,
            value
        );
    }
}


// ============================================================
// STROBE
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
                static_cast<uint32_t>(_base[i]) *
                factor /
                100UL
            );

        apply(
            i,
            value
        );
    }
}


// ============================================================
// WAVE
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
                static_cast<uint32_t>(_base[i]) *
                factor /
                100UL
            );

        apply(
            i,
            value
        );
    }
}