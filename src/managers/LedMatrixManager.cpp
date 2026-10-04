#include "LedMatrixManager.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

LedMatrixManager::LedMatrixManager(
    uint8_t dataPin
)
    : _matrix(
        dataPin,
        30
    ),
      _effects(_matrix),
      _isOn(true),
      _brightness(30)
{
}

// ============================================================
// BEGIN
// ============================================================

void LedMatrixManager::begin()
{
    _matrix.begin();

    _matrix.setBrightness(
        _brightness
    );

    _effects.begin();

    _matrix.clear();
    _matrix.show();

    _isOn = true;
}

// ============================================================
// UPDATE
// ============================================================

void LedMatrixManager::update()
{
    if (!_isOn)
        return;

    _effects.update();
}

// ============================================================
// POWER
// ============================================================

void LedMatrixManager::on()
{
    if (_isOn)
        return;

    _isOn = true;

    _matrix.setBrightness(
        _brightness
    );

    _matrix.show();
}

void LedMatrixManager::off()
{
    if (!_isOn)
        return;

    _isOn = false;

    _matrix.clear();
    _matrix.show();
}

void LedMatrixManager::toggle()
{
    if (_isOn)
        off();
    else
        on();
}

bool LedMatrixManager::isOn() const
{
    return _isOn;
}

// ============================================================
// BRIGHTNESS
// ============================================================

void LedMatrixManager::setBrightness(
    uint8_t brightness
)
{
    if (brightness >
        LedMatrix::MAX_BRIGHTNESS)
    {
        brightness =
            LedMatrix::MAX_BRIGHTNESS;
    }

    _brightness = brightness;

    _matrix.setBrightness(
        _brightness
    );

    if (_isOn)
        _matrix.show();
}

uint8_t LedMatrixManager::brightness() const
{
    return _brightness;
}

// ============================================================
// BASIC DRAWING
// ============================================================

void LedMatrixManager::clear()
{
    _matrix.clear();

    if (_isOn)
        _matrix.show();
}

void LedMatrixManager::show()
{
    if (!_isOn)
        return;

    _matrix.show();
}

void LedMatrixManager::fill(
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    if (!_isOn)
        return;

    _matrix.fill(
        r,
        g,
        b
    );
}

void LedMatrixManager::setPixel(
    uint8_t x,
    uint8_t y,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    if (!_isOn)
        return;

    _matrix.setPixel(
        x,
        y,
        r,
        g,
        b
    );
}

// ============================================================
// EFFECT
// ============================================================

void LedMatrixManager::setEffect(
    Effect effect
)
{
    _effects.setEffect(
        effect
    );
}

LedMatrixManager::Effect
LedMatrixManager::effect() const
{
    return _effects.effect();
}

void LedMatrixManager::stopEffect()
{
    _effects.setEffect(
        Effect::None
    );
}

// ============================================================
// EFFECT SPEED
// ============================================================

void LedMatrixManager::setEffectSpeed(
    uint8_t speed
)
{
    _effects.setSpeed(
        speed
    );
}

uint8_t LedMatrixManager::effectSpeed() const
{
    return _effects.speed();
}

// ============================================================
// TRANSITION
// ============================================================

void LedMatrixManager::setTransitionTime(
    uint16_t milliseconds
)
{
    _effects.setTransitionTime(
        milliseconds
    );
}

uint16_t LedMatrixManager::transitionTime() const
{
    return _effects.transitionTime();
}

bool LedMatrixManager::isTransitioning() const
{
    return _effects.isTransitioning();
}