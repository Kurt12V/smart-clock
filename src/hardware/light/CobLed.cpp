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
      _frequency(frequency),
      _resolution(resolution)
{
}


// ============================================================
// BEGIN
// ============================================================

void CobLed::begin()
{
    ledcSetup(
        _channel,
        _frequency,
        _resolution
    );

    ledcAttachPin(
        _pin,
        _channel
    );

    apply();
}


// ============================================================
// APPLY
// ============================================================

void CobLed::apply()
{
    if (_isOn)
    {
        ledcWrite(
            _channel,
            _brightness
        );
    }
    else
    {
        ledcWrite(
            _channel,
            0
        );
    }
}


// ============================================================
// BRIGHTNESS
// ============================================================

void CobLed::setBrightness(uint8_t brightness)
{
    _brightness = brightness;

    apply();
}


// ============================================================
// INCREASE
// ============================================================

void CobLed::increase(uint8_t step)
{
    uint16_t value =
        static_cast<uint16_t>(_brightness) + step;

    if (value > 255)
        value = 255;

    _brightness =
        static_cast<uint8_t>(value);

    apply();
}


// ============================================================
// DECREASE
// ============================================================

void CobLed::decrease(uint8_t step)
{
    int value =
        static_cast<int>(_brightness) -
        static_cast<int>(step);

    if (value < 0)
        value = 0;

    _brightness =
        static_cast<uint8_t>(value);

    apply();
}


// ============================================================
// ON
// ============================================================

void CobLed::on()
{
    _isOn = true;

    apply();
}


// ============================================================
// OFF
// ============================================================

void CobLed::off()
{
    _isOn = false;

    apply();
}


// ============================================================
// TOGGLE
// ============================================================

void CobLed::toggle()
{
    _isOn = !_isOn;

    apply();
}


// ============================================================
// STATE
// ============================================================

bool CobLed::isOn() const
{
    return _isOn;
}


// ============================================================
// BRIGHTNESS GETTER
// ============================================================

uint8_t CobLed::getBrightness() const
{
    return _brightness;
}
