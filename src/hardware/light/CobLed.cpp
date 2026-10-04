#include "CobLed.h"

#include <math.h>

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
      _frequency(frequency),
      _resolution(resolution),
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

    ledcSetup(
        _channel,
        _frequency,
        _resolution
    );

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
    brightness = constrain(brightness, 0, 100);

    _brightness = brightness;

    if (brightness == 0)
        _isOn = false;
    else
        _isOn = true;

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
    uint16_t value = _brightness + step;

    if (value > 100)
        value = 100;

    setBrightness(static_cast<uint8_t>(value));
}

void CobLed::decrease(uint8_t step)
{
    if (step >= _brightness)
    {
        setBrightness(0);
        return;
    }

    setBrightness(_brightness - step);
}

// ============================================================
// FADE
// ============================================================

void CobLed::fadeTo(uint8_t target, uint32_t durationMs)
{
    target = constrain(target, 0, 100);

    if (durationMs == 0)
    {
        setBrightness(target);
        return;
    }

    _fadeStart = _brightness;
    _fadeTarget = target;

    _fadeStartTime = millis();
    _fadeDuration = durationMs;

    _fading = true;

    if (target > 0)
        _isOn = true;
}

// ============================================================
// UPDATE
// ============================================================

void CobLed::update()
{
    if (!_initialized)
        return;

    if (!_fading)
        return;

    const uint32_t elapsed = millis() - _fadeStartTime;

    if (elapsed >= _fadeDuration)
    {
        _brightness = _fadeTarget;

        _fading = false;

        if (_brightness == 0)
            _isOn = false;

        apply();

        return;
    }

    const float progress =
        static_cast<float>(elapsed) /
        static_cast<float>(_fadeDuration);

    const float value =
        static_cast<float>(_fadeStart) +
        (
            static_cast<float>(_fadeTarget - _fadeStart)
            * progress
        );

    _brightness = static_cast<uint8_t>(value + 0.5f);

    apply();
}

// ============================================================
// FADE STATUS
// ============================================================

bool CobLed::isFading() const
{
    return _fading;
}

// ============================================================
// BRIGHTNESS -> PWM
// ============================================================
//
// Input:
//     0..100 %
//
// Output:
//     0..255 PWM
//
// Quadratic curve:
//
//     PWM = brightness²
//
// This makes the lower brightness range much softer.
// ============================================================

uint8_t CobLed::brightnessToPwm(uint8_t brightness) const
{
    if (brightness == 0)
        return 0;

    if (brightness >= 100)
        return 255;

    const float x =
        static_cast<float>(brightness) / 100.0f;

    const float corrected =
        x * x;

    const int pwm =
        static_cast<int>(
            corrected * 255.0f + 0.5f
        );

    return static_cast<uint8_t>(
        constrain(pwm, 0, 255)
    );
}

// ============================================================
// APPLY
// ============================================================

void CobLed::apply()
{
    if (!_initialized)
        return;

    if (!_isOn || _brightness == 0)
    {
        ledcWrite(_channel, 0);
        return;
    }

    const uint8_t pwm =
        brightnessToPwm(_brightness);

    ledcWrite(
        _channel,
        pwm
    );
}