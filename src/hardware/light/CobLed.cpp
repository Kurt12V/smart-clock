
// ============================================================
// CobLed.cpp
// ============================================================

#include "CobLed.h"

#include <math.h>

// ============================================================
// Constructor
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
      _resolution(resolution)
{
}


// ============================================================
// Begin
// ============================================================

bool CobLed::begin()
{
    const uint8_t result =
        ledcSetup(
            _channel,
            _frequency,
            _resolution
        );

    if (result == 0)
    {
        _initialized = false;
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

    ledcWrite(
        _channel,
        0
    );

    return true;
}


// ============================================================
// Brightness -> PWM
//
// Input:
//     0..100
//
// Output:
//     0..255
//
// Inverse gamma makes the control perceptually closer
// to linear brightness.
//
// 30% -> approximately 148 PWM
// 50% -> approximately 186 PWM
// 100% -> 255 PWM
// ============================================================

uint8_t CobLed::toPwm(
    uint8_t brightness
) const
{
    if (brightness == 0)
        return 0;

    if (brightness >= 100)
        return 255;

    const float normalized =
        static_cast<float>(brightness) / 100.0f;

    const float corrected =
        powf(
            normalized,
            1.0f / 2.2f
        );

    const int pwm =
        static_cast<int>(
            corrected * 255.0f + 0.5f
        );

    return static_cast<uint8_t>(
        constrain(pwm, 0, 255)
    );
}


// ============================================================
// Apply
// ============================================================

void CobLed::apply()
{
    if (!_initialized)
        return;

    if (!_isOn)
    {
        ledcWrite(
            _channel,
            0
        );

        return;
    }

    ledcWrite(
        _channel,
        toPwm(_brightness)
    );
}


// ============================================================
// Set brightness
// ============================================================

void CobLed::setBrightness(
    uint8_t brightness
)
{
    _fading = false;

    _brightness =
        constrain(
            brightness,
            0,
            100
        );

    if (_brightness == 0)
        _isOn = false;
    else
        _isOn = true;

    apply();
}


// ============================================================
// Get brightness
// ============================================================

uint8_t CobLed::getBrightness() const
{
    return _brightness;
}


// ============================================================
// ON
// ============================================================

void CobLed::on()
{
    _fading = false;

    if (_brightness == 0)
        _brightness = 100;

    _isOn = true;

    apply();
}


// ============================================================
// OFF
// ============================================================

void CobLed::off()
{
    _fading = false;
    _isOn = false;

    apply();
}


// ============================================================
// Toggle
// ============================================================

void CobLed::toggle()
{
    if (_isOn)
        off();
    else
        on();
}


// ============================================================
// Is ON
// ============================================================

bool CobLed::isOn() const
{
    return _isOn;
}


// ============================================================
// Increase
// ============================================================

void CobLed::increase(
    uint8_t step
)
{
    uint16_t value =
        static_cast<uint16_t>(_brightness) +
        step;

    if (value > 100)
        value = 100;

    setBrightness(
        static_cast<uint8_t>(value)
    );
}


// ============================================================
// Decrease
// ============================================================

void CobLed::decrease(
    uint8_t step
)
{
    int value =
        static_cast<int>(_brightness) -
        step;

    if (value < 0)
        value = 0;

    setBrightness(
        static_cast<uint8_t>(value)
    );
}


// ============================================================
// Fade
// ============================================================

void CobLed::fadeTo(
    uint8_t target,
    uint32_t durationMs
)
{
    if (!_initialized)
        return;

    target =
        constrain(
            target,
            0,
            100
        );

    if (_brightness == target)
        return;

    if (durationMs == 0)
    {
        setBrightness(target);
        return;
    }

    _fadeStart =
        _brightness;

    _fadeTarget =
        target;

    _fadeStartTime =
        millis();

    _fadeDuration =
        durationMs;

    _fading = true;

    if (target > 0)
        _isOn = true;
}


// ============================================================
// Update
// ============================================================

void CobLed::update()
{
    if (!_fading)
        return;

    const uint32_t elapsed =
        millis() -
        _fadeStartTime;

    if (elapsed >= _fadeDuration)
    {
        _brightness =
            _fadeTarget;

        _fading = false;

        if (_brightness == 0)
            _isOn = false;
        else
            _isOn = true;

        apply();

        return;
    }

    const float progress =
        static_cast<float>(elapsed) /
        static_cast<float>(_fadeDuration);

    const int value =
        static_cast<int>(
            _fadeStart +
            (
                static_cast<int>(_fadeTarget) -
                static_cast<int>(_fadeStart)
            ) * progress
        );

    _brightness =
        static_cast<uint8_t>(
            constrain(value, 0, 100)
        );

    apply();
}


// ============================================================
// Is fading
// ============================================================

bool CobLed::isFading() const
{
    return _fading;
}

