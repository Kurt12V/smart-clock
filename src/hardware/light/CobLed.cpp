#include "CobLed.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

CobLed::CobLed(
    uint8_t pin,
    uint8_t channel,
    uint32_t frequency,
    uint8_t resolution
)
    : _pin(pin),
      _channel(channel),
      _resolution(resolution),
      _frequency(frequency),
      _brightness(0),
      _isOn(false),
      _initialized(false),
      _fading(false),
      _fadeStart(0),
      _fadeTarget(0),
      _fadeStartTime(0),
      _fadeDuration(0)
{
}

// ============================================================
// BEGIN
// ============================================================

bool CobLed::begin()
{
    if (_initialized)
        return true;

    if (_resolution == 0 || _resolution > 16)
        return false;

    if (_frequency == 0)
        return false;

    if (!ledcSetup(
            _channel,
            _frequency,
            _resolution
        ))
    {
        return false;
    }

    ledcAttachPin(
        _pin,
        _channel
    );

    _initialized = true;

    _brightness = 0;
    _isOn = false;
    _fading = false;

    apply();

    return true;
}

// ============================================================
// BRIGHTNESS
// ============================================================

void CobLed::setBrightness(uint8_t brightness)
{
    _brightness = constrain(
        brightness,
        0,
        100
    );

    _isOn = (_brightness > 0);

    _fading = false;

    apply();
}

uint8_t CobLed::getBrightness() const
{
    return _brightness;
}

// ============================================================
// ON / OFF
// ============================================================

void CobLed::on()
{
    if (_brightness == 0)
        _brightness = 100;

    _isOn = true;
    _fading = false;

    apply();
}

void CobLed::off()
{
    _isOn = false;
    _fading = false;

    apply();
}

void CobLed::toggle()
{
    if (_isOn)
        off();
    else
        on();
}

bool CobLed::isOn() const
{
    return _isOn;
}

// ============================================================
// INCREASE / DECREASE
// ============================================================

void CobLed::increase(uint8_t step)
{
    uint16_t value =
        static_cast<uint16_t>(_brightness) + step;

    if (value > 100)
        value = 100;

    setBrightness(
        static_cast<uint8_t>(value)
    );
}

void CobLed::decrease(uint8_t step)
{
    if (step >= _brightness)
    {
        setBrightness(0);
        return;
    }

    setBrightness(
        _brightness - step
    );
}

// ============================================================
// FADE
// ============================================================

void CobLed::fadeTo(
    uint8_t target,
    uint32_t durationMs
)
{
    target = constrain(
        target,
        0,
        100
    );

    if (!_initialized || durationMs == 0)
    {
        setBrightness(target);
        return;
    }

    _fadeStart = _brightness;
    _fadeTarget = target;

    _fadeStartTime = millis();
    _fadeDuration = durationMs;

    _fading = (_fadeStart != _fadeTarget);

    if (target > 0)
        _isOn = true;

    if (!_fading)
    {
        _brightness = target;
        _isOn = (target > 0);
        apply();
    }
}

// ============================================================
// UPDATE
// ============================================================

void CobLed::update()
{
    if (!_initialized || !_fading)
        return;

    uint32_t elapsed =
        millis() - _fadeStartTime;

    if (elapsed >= _fadeDuration)
    {
        _brightness = _fadeTarget;

        _fading = false;
        _isOn = (_brightness > 0);

        apply();

        return;
    }

    int32_t difference =
        static_cast<int32_t>(_fadeTarget) -
        static_cast<int32_t>(_fadeStart);

    int32_t value =
        static_cast<int32_t>(_fadeStart) +
        (
            difference *
            static_cast<int32_t>(elapsed)
        ) /
        static_cast<int32_t>(_fadeDuration);

    _brightness = static_cast<uint8_t>(
        constrain(value, 0, 100)
    );

    _isOn = (_brightness > 0);

    apply();
}

bool CobLed::isFading() const
{
    return _fading;
}

// ============================================================
// PWM CONVERSION
// ============================================================

uint32_t CobLed::brightnessToPwm(
    uint8_t brightness
) const
{
    const uint32_t maxDuty =
        (1UL << _resolution) - 1UL;

    if (brightness == 0)
        return 0;

    if (brightness >= 100)
        return maxDuty;

    // Gamma correction.
    // Улучшает визуальную плавность регулировки яркости.

    double value =
        static_cast<double>(brightness) / 100.0;

    value = value * value;

    return static_cast<uint32_t>(
        value * maxDuty + 0.5
    );
}

// ============================================================
// APPLY PWM
// ============================================================

void CobLed::apply()
{
    if (!_initialized)
        return;

    uint32_t duty = 0;

    if (_isOn && _brightness > 0)
    {
        duty = brightnessToPwm(
            _brightness
        );
    }

    ledcWrite(
        _channel,
        duty
    );
}