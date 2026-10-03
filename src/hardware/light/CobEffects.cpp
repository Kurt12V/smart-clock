#include "CobEffects.h"

#include <math.h>

// ============================================================
// CONSTRUCTOR
// ============================================================

CobEffects::CobEffects(
    CobLed& cob1,
    CobLed& cob2,
    CobLed& cob3,
    CobLed& cob4
)
    : _effect(CobEffectType::Static),
      _speed(50),
      _enabled(true),
      _lastUpdate(0),
      _phase(0)
{
    _leds[0] = &cob1;
    _leds[1] = &cob2;
    _leds[2] = &cob3;
    _leds[3] = &cob4;

    for (uint8_t i = 0; i < LED_COUNT; ++i)
        _base[i] = 255;
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
    if (effect >= CobEffectType::COUNT)
        effect = CobEffectType::Static;

    _effect = effect;

    _phase = 0;
    _lastUpdate = millis();

    if (_effect == CobEffectType::Static)
        applyStatic();
}

// ============================================================
// GET EFFECT
// ============================================================

CobEffectType CobEffects::effect() const
{
    return _effect;
}

// ============================================================
// SPEED
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

uint8_t CobEffects::speed() const
{
    return _speed;
}

// ============================================================
// ENABLE
// ============================================================

void CobEffects::setEnabled(
    bool enabled
)
{
    _enabled = enabled;

    _phase = 0;
    _lastUpdate = millis();

    if (!_enabled)
    {
        outputAll(0);
        return;
    }

    if (_effect == CobEffectType::Static)
        applyStatic();
}

// ============================================================
// IS ENABLED
// ============================================================

bool CobEffects::isEnabled() const
{
    return _enabled;
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

    _base[index] = value;

    if (_effect == CobEffectType::Static)
    {
        output(
            index,
            _enabled
                ? value
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
    for (uint8_t i = 0; i < LED_COUNT; ++i)
        _base[i] = value;

    if (_effect == CobEffectType::Static)
        applyStatic();
}

// ============================================================
// RESET
// ============================================================

void CobEffects::reset()
{
    _effect = CobEffectType::Static;

    _speed = 50;

    _phase = 0;

    _enabled = true;

    applyStatic();
}

// ============================================================
// UPDATE
// ============================================================

void CobEffects::update()
{
    if (!_enabled)
        return;

    uint32_t now = millis();

    if (now - _lastUpdate < 20)
        return;

    _lastUpdate = now;

    uint32_t period =
        calculatePeriod();

    if (period == 0)
        period = 1;

    _phase =
        (
            (uint64_t)(now % period) *
            1000ULL
        ) /
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

        case CobEffectType::Blink:
            applyBlink();
            break;

        case CobEffectType::Pulse:
            applyPulse();
            break;

        default:
            applyStatic();
            break;
    }
}

// ============================================================
// PERIOD
//
// Speed 0   = 3000 ms
// Speed 100 = 500 ms
// ============================================================

uint32_t CobEffects::calculatePeriod() const
{
    return
        3000UL -
        (
            (uint32_t)_speed *
            25UL
        );
}

// ============================================================
// OUTPUT
// ============================================================

void CobEffects::output(
    uint8_t index,
    uint8_t value
)
{
    if (index >= LED_COUNT)
        return;

    if (!_enabled)
        value = 0;

    _leds[index]->output(value);
}

// ============================================================
// OUTPUT ALL
// ============================================================

void CobEffects::outputAll(
    uint8_t value
)
{
    for (uint8_t i = 0; i < LED_COUNT; ++i)
        output(
            i,
            value
        );
}

// ============================================================
// STATIC
// ============================================================

void CobEffects::applyStatic()
{
    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        output(
            i,
            _base[i]
        );
    }
}

// ============================================================
// BREATH
// ============================================================

void CobEffects::applyBreath()
{
    uint8_t factor =
        triangle(_phase);

    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        uint8_t value =
            (
                (uint16_t)_base[i] *
                factor
            ) /
            100;

        output(
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
    uint8_t factor =
        (_phase < 500)
            ? 100
            : 0;

    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        uint8_t value =
            (
                (uint16_t)_base[i] *
                factor
            ) /
            100;

        output(
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
        uint32_t phase =
            (
                _phase +
                (i * 250UL)
            ) %
            1000UL;

        uint8_t factor =
            triangle(phase);

        uint8_t value =
            (
                (uint16_t)_base[i] *
                factor
            ) /
            100;

        output(
            i,
            value
        );
    }
}

// ============================================================
// BLINK
// ============================================================

void CobEffects::applyBlink()
{
    bool state =
        _phase < 500;

    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        output(
            i,
            state
                ? _base[i]
                : 0
        );
    }
}

// ============================================================
// PULSE
// ============================================================

void CobEffects::applyPulse()
{
    uint8_t factor =
        sineWave(_phase);

    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        uint8_t value =
            (
                (uint16_t)_base[i] *
                factor
            ) /
            100;

        output(
            i,
            value
        );
    }
}

// ============================================================
// TRIANGLE
//
// 0 -> 100 -> 0
// ============================================================

uint8_t CobEffects::triangle(
    uint32_t phase
) const
{
    phase %= 1000;

    if (phase < 500)
    {
        return
            (
                (uint32_t)phase *
                100
            ) /
            500;
    }

    return
        (
            (uint32_t)(1000 - phase) *
            100
        ) /
        500;
}

// ============================================================
// SINE
//
// 0 -> 100 -> 0 -> 100
// ============================================================

uint8_t CobEffects::sineWave(
    uint32_t phase
) const
{
    float angle =
        (
            (float)(phase % 1000) /
            1000.0f
        ) *
        2.0f *
        PI;

    float value =
        (
            sinf(angle) +
            1.0f
        ) *
        50.0f;

    return constrain(
        (int)value,
        0,
        100
    );
}