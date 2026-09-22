#include "CobLed.h"

#include <math.h>


// ============================================================
// Static
// ============================================================

uint8_t CobLed::_gammaTable[256];

bool CobLed::_gammaReady = false;


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
// Gamma
// ============================================================

void CobLed::buildGamma()
{
    if (_gammaReady)
        return;

    for (int i = 0; i < 256; i++)
    {
        float x =
            i / 255.0f;

        float value =
            powf(
                x,
                2.2f
            ) * 255.0f;

        _gammaTable[i] =
            (uint8_t)(
                value + 0.5f
            );
    }

    _gammaReady = true;
}


// ============================================================
// Begin
// ============================================================

bool CobLed::begin()
{
    buildGamma();

    uint8_t result =
        ledcSetup(
            _channel,
            _frequency,
            _resolution
        );

    if (result == 0)
    {
        Serial.printf(
            "CobLed: LEDC setup FAILED "
            "GPIO=%u CH=%u\n",
            _pin,
            _channel
        );

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

    Serial.printf(
        "CobLed: GPIO=%u CH=%u OK\n",
        _pin,
        _channel
    );

    return true;
}


// ============================================================
// Gamma
// ============================================================

uint8_t CobLed::gamma(
    uint8_t value
) const
{
    return _gammaTable[value];
}


// ============================================================
// Apply
// ============================================================

void CobLed::apply()
{
    if (!_initialized)
        return;

    uint8_t pwm = 0;

    if (_isOn)
    {
        pwm =
            gamma(
                _brightness
            );
    }

    ledcWrite(
        _channel,
        pwm
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
        brightness;

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
        (uint16_t)_brightness +
        step;

    if (value > 255)
        value = 255;

    setBrightness(
        (uint8_t)value
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
        (int)_brightness -
        step;

    if (value < 0)
        value = 0;

    setBrightness(
        (uint8_t)value
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
// Update fade
// ============================================================

void CobLed::update()
{
    if (!_fading)
        return;

    uint32_t elapsed =
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

    float progress =
        (float)elapsed /
        (float)_fadeDuration;

    int value =
        (int)_fadeStart +
        (
            (int)_fadeTarget -
            (int)_fadeStart
        ) * progress;

    if (value < 0)
        value = 0;

    if (value > 255)
        value = 255;

    _brightness =
        (uint8_t)value;

    apply();
}


// ============================================================
// Is fading
// ============================================================

bool CobLed::isFading() const
{
    return _fading;
}
