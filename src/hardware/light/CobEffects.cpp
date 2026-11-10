#include "CobEffects.h"

// ============================================================
// CONSTRUCTOR
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
      _base{100, 100, 100, 100},
      _channelEnabled{true, true, true, true},
      _enabled(false),
      _lastUpdate(0),
      _phase(0)
{
}

// ============================================================
// BEGIN
// ============================================================

void CobEffects::begin()
{
    _phase = 0;
    _lastUpdate = millis();

    render();
}

// ============================================================
// GET LED
// ============================================================

CobLed& CobEffects::led(uint8_t index)
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

const CobLed& CobEffects::led(uint8_t index) const
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
// EFFECT
// ============================================================

void CobEffects::setEffect(CobEffectType effect)
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

    render();
}

CobEffectType CobEffects::effect() const
{
    return _effect;
}

// ============================================================
// SPEED
// ============================================================

void CobEffects::setSpeed(uint8_t speed)
{
    _speed = constrain(
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
// BRIGHTNESS
// ============================================================

void CobEffects::setBrightness(
    uint8_t index,
    uint8_t value,
    bool applyNow
)
{
    if (index >= LED_COUNT)
        return;

    value = constrain(
        value,
        0,
        100
    );

    _base[index] = value;

    _channelEnabled[index] = (value > 0);

    if (applyNow)
        render();
}

uint8_t CobEffects::brightness(uint8_t index) const
{
    if (index >= LED_COUNT)
        return 0;

    return _base[index];
}

void CobEffects::setAllBrightness(uint8_t value)
{
    value = constrain(
        value,
        0,
        100
    );

    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        _base[i] = value;
        _channelEnabled[i] = (value > 0);
    }

    render();
}

// ============================================================
// CHANNEL ENABLE
// ============================================================

void CobEffects::setChannelEnabled(
    uint8_t index,
    bool enabled
)
{
    if (index >= LED_COUNT)
        return;

    _channelEnabled[index] = enabled;

    render();
}

bool CobEffects::isChannelEnabled(uint8_t index) const
{
    if (index >= LED_COUNT)
        return false;

    return _channelEnabled[index];
}

// ============================================================
// MASTER ENABLE
// ============================================================

void CobEffects::setEnabled(bool enabled)
{
    _enabled = enabled;

    _phase = 0;
    _lastUpdate = millis();

    render();
}

bool CobEffects::isEnabled() const
{
    return _enabled;
}

// ============================================================
// UPDATE
// ============================================================

void CobEffects::update()
{
    if (!_enabled)
        return;

    uint32_t now = millis();

    uint32_t elapsed =
        now - _lastUpdate;

    if (elapsed < 20)
        return;

    _lastUpdate = now;

    // Скорость эффекта:
    // 0   = медленно
    // 100 = быстро

    uint32_t period =
        5000UL -
        (
            static_cast<uint32_t>(_speed) *
            4500UL /
            100UL
        );

    if (_effect != CobEffectType::Static)
    {
        _phase =
            (
                _phase +
                (elapsed * 1000UL / period)
            ) % 1000UL;

        render();
    }
}

// ============================================================
// RENDER
// ============================================================

void CobEffects::render()
{
    if (!_enabled)
    {
        for (uint8_t i = 0; i < LED_COUNT; i++)
            led(i).off();

        return;
    }

    switch (_effect)
    {
        case CobEffectType::Static:
            applyStatic();
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
            applyStatic();
            break;
    }
}

// ============================================================
// APPLY CHANNEL
// ============================================================

void CobEffects::apply(
    uint8_t index,
    uint8_t value
)
{
    if (index >= LED_COUNT)
        return;

    if (
        !_enabled ||
        !_channelEnabled[index] ||
        value == 0
    )
    {
        led(index).off();
        return;
    }

    led(index).setBrightness(value);
}

// ============================================================
// TRIANGLE WAVE
// ============================================================

uint8_t CobEffects::triangle(uint32_t phase) const
{
    phase %= 1000UL;

    if (phase < 500UL)
    {
        return static_cast<uint8_t>(
            phase * 100UL / 500UL
        );
    }

    return static_cast<uint8_t>(
        (1000UL - phase) * 100UL / 500UL
    );
}

// ============================================================
// STATIC
// ============================================================

void CobEffects::applyStatic()
{
    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        apply(
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
    uint8_t factor = triangle(_phase);

    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        uint8_t value =
            static_cast<uint8_t>(
                static_cast<uint16_t>(_base[i]) *
                factor / 100U
            );

        apply(i, value);
    }
}

// ============================================================
// STROBE
// ============================================================

void CobEffects::applyStrobe()
{
    uint8_t factor =
        (_phase < 500UL) ? 100 : 0;

    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        uint8_t value =
            static_cast<uint8_t>(
                static_cast<uint16_t>(_base[i]) *
                factor / 100U
            );

        apply(i, value);
    }
}

// ============================================================
// WAVE
// ============================================================

void CobEffects::applyWave()
{
    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        uint32_t phase =
            (
                _phase +
                static_cast<uint32_t>(i) * 250UL
            ) % 1000UL;

        uint8_t factor = triangle(phase);

        uint8_t value =
            static_cast<uint8_t>(
                static_cast<uint16_t>(_base[i]) *
                factor / 100U
            );

        apply(i, value);
    }
}