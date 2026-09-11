#include "LedMatrixManager.h"
#include <math.h>

// ============================================================
// Constructor
// ============================================================

LedMatrixManager::LedMatrixManager(
    uint8_t dataPin
)
    : _matrix(
        dataPin,
        _brightness
    )
{
}

// ============================================================
// Initialization
// ============================================================

void LedMatrixManager::begin()
{
    _matrix.begin();

    _matrix.setBrightness(
        _brightness
    );

    _matrix.clear();
    _matrix.show();

    _isOn = true;

    _animation = Animation::None;
    _animationStep = 0;
    _lastAnimationUpdate = millis();
}

// ============================================================
// Update
// ============================================================

void LedMatrixManager::update()
{
    if (!_isOn)
        return;

    const uint32_t now = millis();

    // ~30 FPS
    if (now - _lastAnimationUpdate < 33)
        return;

    _lastAnimationUpdate = now;

    switch (_animation)
    {
        case Animation::None:
            return;

        case Animation::Rainbow:
            updateRainbow();
            break;

        case Animation::Pulse:
            updatePulse();
            break;

        case Animation::Wave:
            updateWave();
            break;

        case Animation::Scanner:
            updateScanner();
            break;

        case Animation::Fire:
            updateFire();
            break;
    }

    _matrix.show();

    _animationStep++;
}

// ============================================================
// Power
// ============================================================

void LedMatrixManager::on()
{
    _isOn = true;

    _matrix.setBrightness(
        _brightness
    );

    _matrix.show();
}

void LedMatrixManager::off()
{
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
// Brightness
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
// Basic drawing
// ============================================================

void LedMatrixManager::clear()
{
    _matrix.clear();
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
// Animation control
// ============================================================

void LedMatrixManager::setAnimation(
    Animation animation
)
{
    _animation = animation;

    _animationStep = 0;

    _lastAnimationUpdate = millis();

    _matrix.clear();

    if (_isOn)
        _matrix.show();
}

void LedMatrixManager::stopAnimation()
{
    _animation = Animation::None;

    _animationStep = 0;

    _matrix.clear();

    if (_isOn)
        _matrix.show();
}

LedMatrixManager::Animation
LedMatrixManager::animation() const
{
    return _animation;
}

// ============================================================
// Rainbow
// ============================================================

void LedMatrixManager::updateRainbow()
{
    for (uint8_t y = 0;
         y < LedMatrix::HEIGHT;
         y++)
    {
        for (uint8_t x = 0;
             x < LedMatrix::WIDTH;
             x++)
        {
            uint8_t position =
                (_animationStep * 3)
                + (x * 8)
                + (y * 8);

            uint32_t color =
                wheel(position);

            _matrix.setPixel(
                x,
                y,
                color
            );
        }
    }
}

// ============================================================
// Pulse
// ============================================================

void LedMatrixManager::updatePulse()
{
    uint8_t level =
        sin8(
            _animationStep * 2
        );

    // Ограничиваем рабочую яркость
    level =
        map(
            level,
            0,
            255,
            5,
            120
        );

    _matrix.fill(
        level,
        level / 4,
        level
    );
}

// ============================================================
// Wave
// ============================================================

void LedMatrixManager::updateWave()
{
    _matrix.clear();

    for (uint8_t x = 0;
         x < LedMatrix::WIDTH;
         x++)
    {
        uint8_t wave =
            sin8(
                _animationStep * 4
                + x * 16
            );

        uint8_t height =
            map(
                wave,
                0,
                255,
                1,
                LedMatrix::HEIGHT
            );

        for (uint8_t y = 0;
             y < height;
             y++)
        {
            uint8_t brightness =
                map(
                    y,
                    0,
                    height,
                    10,
                    100
                );

            _matrix.setPixel(
                x,
                LedMatrix::HEIGHT - 1 - y,
                0,
                brightness,
                brightness
            );
        }
    }
}

// ============================================================
// Scanner
// ============================================================

void LedMatrixManager::updateScanner()
{
    _matrix.clear();

    const uint16_t period =
        (LedMatrix::WIDTH - 1) * 2;

    uint16_t position =
        _animationStep % period;

    uint8_t x;

    if (position <
        LedMatrix::WIDTH)
    {
        x = position;
    }
    else
    {
        x =
            period - position;
    }

    for (uint8_t y = 0;
         y < LedMatrix::HEIGHT;
         y++)
    {
        uint8_t brightness =
            map(
                y,
                0,
                LedMatrix::HEIGHT - 1,
                20,
                100
            );

        _matrix.setPixel(
            x,
            y,
            brightness,
            0,
            brightness
        );
    }
}

// ============================================================
// Fire
// ============================================================

void LedMatrixManager::updateFire()
{
    _matrix.clear();

    for (uint8_t x = 0;
         x < LedMatrix::WIDTH;
         x++)
    {
        uint8_t heat =
            random(
                80,
                180
            );

        for (uint8_t y = 0;
             y < LedMatrix::HEIGHT;
             y++)
        {
            uint8_t reduction =
                random(
                    0,
                    50
                );

            uint8_t value;

            if (heat > reduction)
                value = heat - reduction;
            else
                value = 0;

            uint8_t red =
                value;

            uint8_t green =
                value / 3;

            uint8_t blue = 0;

            _matrix.setPixel(
                x,
                LedMatrix::HEIGHT - 1 - y,
                red,
                green,
                blue
            );

            if (heat > 5)
                heat -= 5;
            else
                heat = 0;
        }
    }
}

// ============================================================
// Color wheel
// ============================================================

uint32_t LedMatrixManager::wheel(
    uint8_t position
)
{
    position =
        255 - position;

    if (position < 85)
    {
        return _matrix.color(
            255 - position * 3,
            0,
            position * 3
        );
    }

    if (position < 170)
    {
        position -= 85;

        return _matrix.color(
            0,
            position * 3,
            255 - position * 3
        );
    }

    position -= 170;

    return _matrix.color(
        position * 3,
        255 - position * 3,
        0
    );
}

// ============================================================
// 8-bit sine
// ============================================================

uint8_t LedMatrixManager::sin8(
    uint8_t value
)
{
    float radians =
        value *
        2.0f *
        PI /
        255.0f;

    float result =
        (sinf(radians) + 1.0f)
        * 127.5f;

    return static_cast<uint8_t>(
        result
    );
}