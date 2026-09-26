#include "CobLed.h"

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

void CobLed::apply()
{
    if (_isOn)
        ledcWrite(_channel, _brightness);
    else
        ledcWrite(_channel, 0);
}

void CobLed::setBrightness(uint8_t brightness)
{
    _brightness = brightness;
    apply();
}

void CobLed::increase(uint8_t step)
{
    int value = _brightness + step;

    if (value > 255)
        value = 255;

    _brightness = value;

    apply();
}

void CobLed::decrease(uint8_t step)
{
    int value = _brightness - step;

    if (value < 0)
        value = 0;

    _brightness = value;

    apply();
}

void CobLed::on()
{
    _isOn = true;
    apply();
}

void CobLed::off()
{
    _isOn = false;
    apply();
}

void CobLed::toggle()
{
    _isOn = !_isOn;
    apply();
}

bool CobLed::isOn() const
{
    return _isOn;
}

uint8_t CobLed::getBrightness() const
{
    return _brightness;
}