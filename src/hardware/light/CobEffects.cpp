#include "CobEffects.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

CobEffects::CobEffects(CobLedManager& manager)
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
    _phase      = 0;
    _lastUpdate = millis();
    applyStatic();
}

// ============================================================
// EFFECT
// ============================================================

void CobEffects::setEffect(CobEffectType e)
{
    if (e >= CobEffectType::COUNT)
        e = CobEffectType::Static;

    _effect     = e;
    _phase      = 0;
    _lastUpdate = millis();

    if (e == CobEffectType::Static)
        applyStatic();
}

// ============================================================
// SPEED
// ============================================================

void CobEffects::setSpeed(uint8_t speed)
{
    _speed = constrain(speed, 0, 100);
}

// ============================================================
// BRIGHTNESS
// ============================================================

void CobEffects::setBrightness(uint8_t index, uint8_t value)
{
    if (index >= LED_COUNT) return;

    _base[index] = constrain(value, 0, 100);

    if (_effect == CobEffectType::Static)
        apply(index, _enabled ? _base[index] : 0);
}

uint8_t CobEffects::brightness(uint8_t index) const
{
    if (index >= LED_COUNT) return 0;
    return _base[index];
}

void CobEffects::setAllBrightness(uint8_t value)
{
    for (uint8_t i = 0; i < LED_COUNT; ++i)
        setBrightness(i, value);
}

// ============================================================
// ENABLE
// ============================================================

void CobEffects::setEnabled(bool enabled)
{
    _enabled = enabled;

    if (!enabled)
    {
        for (uint8_t i = 0; i < LED_COUNT; ++i)
            apply(i, 0);
        return;
    }

    _lastUpdate = 0;
}

// ============================================================
// UPDATE
// ============================================================

void CobEffects::update()
{
    if (!_enabled) return;

    uint32_t now = millis();

    if (now - _lastUpdate < 20) return;
    _lastUpdate = now;

    uint32_t period = 3000 - (uint32_t)_speed * 25;   // 500..3000 мс

    _phase = (now % period) * 1000UL / period;

    switch (_effect)
    {
        case CobEffectType::Static:  break;
        case CobEffectType::Breath:  applyBreath(); break;
        case CobEffectType::Strobe:  applyStrobe(); break;
        case CobEffectType::Wave:    applyWave();   break;
        default: break;
    }
}

// ============================================================
// APPLY — единственное место, где мы зовём CobLedManager
// ============================================================

void CobEffects::apply(uint8_t index, uint8_t value)
{
    // 0..100 -> 0..255
    uint8_t v255 = (uint32_t)value * 255 / 100;

    // CobLedManager принимает 1..4
    _manager.set(index + 1, v255);
}

// ============================================================
// TRIANGLE — 0..999 -> 0..100..0
// ============================================================

uint8_t CobEffects::triangle(uint32_t p) const
{
    if (p < 500)
        return (uint32_t)p * 100 / 500;
    else
        return (uint32_t)(1000 - p) * 100 / 500;
}

// ============================================================
// EFFECTS
// ============================================================

void CobEffects::applyStatic()
{
    for (uint8_t i = 0; i < LED_COUNT; ++i)
        apply(i, _enabled ? _base[i] : 0);
}

void CobEffects::applyBreath()
{
    uint8_t factor = triangle(_phase);

    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        uint8_t v = (uint32_t)_base[i] * factor / 100;
        apply(i, v);
    }
}

void CobEffects::applyStrobe()
{
    uint8_t factor = (_phase < 500) ? 100 : 0;

    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        uint8_t v = (uint32_t)_base[i] * factor / 100;
        apply(i, v);
    }
}

void CobEffects::applyWave()
{
    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        uint32_t p = (_phase + i * 250UL) % 1000;
        uint8_t factor = triangle(p);
        uint8_t v = (uint32_t)_base[i] * factor / 100;
        apply(i, v);
    }
}