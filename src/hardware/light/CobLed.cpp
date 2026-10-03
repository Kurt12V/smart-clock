#include "CobLed.h"

#include <math.h>

// ============================================================
// STATIC
// ============================================================

uint8_t CobLed::_gammaTable[256];
bool CobLed::_gammaReady = false;

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

      _initialized(false),

      _brightness(0),
      _isOn(false),

      _outputBrightness(0),

      _fading(false),

      _fadeStart(0),
      _fadeTarget(0),
      _fadeStartTime(0),
      _fadeDuration(0)
{
}

// ============================================================
// GAMMA
// ============================================================

void CobLed::buildGamma()
{
    if (_gammaReady)
        return;

    for (uint16_t i = 0; i < 256; ++i)
    {
        float x =
            (float)i / 255.0f;

        float value =
            powf(x, 2.2f) * 255.0f;

        _gammaTable[i] =
            (uint8_t)(value + 0.5f);
    }

    _gammaReady = true;
}

// ============================================================
// BEGIN
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
        Serial0.printf(
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
    _outputBrightness = 0;

    _isOn = false;
    _fading = false;

    ledcWrite(
        _channel,
        0
    );

    Serial0.printf(
        "CobLed: GPIO=%u CH=%u OK\n",
        _pin,
        _channel
    );

    return true;
}

// ============================================================
// GAMMA
// ============================================================

uint8_t CobLed::gamma(
    uint8_t value
) const
{
    return _gammaTable[value];
}

// ============================================================
// APPLY NORMAL OUTPUT
// ============================================================

void CobLed::apply()
{
    if (!_initialized)
        return;

    uint8_t pwm = 0;

    if (_isOn)
    {
        pwm =
            gamma(_brightness);
    }

    _outputBrightness =
        _isOn
            ? _brightness
            : 0;

    ledcWrite(
        _channel,
        pwm
    );
}

// ============================================================
// SET BRIGHTNESS
// ============================================================

void CobLed::setBrightness(
    uint8_t brightness
)
{
    _fading = false;

    _brightness =
        brightness;

    _isOn =
        (_brightness > 0);

    apply();
}

// ============================================================
// GET BRIGHTNESS
// ============================================================

uint8_t CobLed::getBrightness() const
{
    return _brightness;
}

// ============================================================
// DIRECT EFFECT OUTPUT
//
// IMPORTANT:
//
// Does not modify _brightness.
//
// This allows:
//
// user brightness = 200
//
// effect output:
// 200 → 100 → 200
//
// Base brightness remains 200.
// ============================================================

void CobLed::output(
    uint8_t brightness
)
{
    if (!_initialized)
        return;

    brightness =
        constrain(
            brightness,
            0,
            255
        );

    _outputBrightness =
        brightness;

    ledcWrite(
        _channel,
        gamma(brightness)
    );
}

// ============================================================
// GET OUTPUT BRIGHTNESS
// ============================================================

uint8_t CobLed::getOutputBrightness() const
{
    return _outputBrightness;
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

    if (!_initialized)
        return;

    _outputBrightness = 0;

    ledcWrite(
        _channel,
        0
    );
}

// ============================================================
// TOGGLE
// ============================================================

void CobLed::toggle()
{
    if (_isOn)
        off();
    else
        on();
}

// ============================================================
// IS ON
// ============================================================

bool CobLed::isOn() const
{
    return _isOn;
}

// ============================================================
// INCREASE
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
// DECREASE
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
// FADE TO
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
// UPDATE
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

        _isOn =
            (_brightness > 0);

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
        ) *
        progress;

    value =
        constrain(
            value,
            0,
            255
        );

    _brightness =
        (uint8_t)value;

    _isOn =
        (_brightness > 0);

    apply();
}

// ============================================================
// IS FADING
// ============================================================

bool CobLed::isFading() const
{
    return _fading;
}