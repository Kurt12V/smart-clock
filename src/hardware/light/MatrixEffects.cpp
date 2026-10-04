#include "MatrixEffects.h"

#include <math.h>

// ============================================================
// CONSTRUCTOR
// ============================================================

MatrixEffects::MatrixEffects(
    LedMatrix& matrix
)
    : _matrix(matrix),
      _currentEffect(Effect::None),
      _nextEffect(Effect::None),
      _transitioning(false),
      _transitionTime(1000),
      _transitionStart(0),
      _speed(50),
      _lastUpdate(0),
      _step(0)
{
    clearBuffer(_oldFrame);
    clearBuffer(_newFrame);
}

// ============================================================
// BEGIN
// ============================================================

void MatrixEffects::begin()
{
    clearBuffer(_oldFrame);
    clearBuffer(_newFrame);

    _currentEffect = Effect::None;
    _nextEffect = Effect::None;

    _transitioning = false;

    _step = 0;
    _lastUpdate = millis();
}

// ============================================================
// UPDATE
// ============================================================

void MatrixEffects::update()
{
    const uint32_t now = millis();

    // Speed controls animation FPS.
    const uint32_t interval =
        map(
            _speed,
            0,
            100,
            100,
            15
        );

    if (now - _lastUpdate < interval)
        return;

    _lastUpdate = now;

    // --------------------------------------------------------
    // Transition
    // --------------------------------------------------------

    if (_transitioning)
    {
        render(
            _nextEffect,
            _newFrame
        );

        const uint32_t elapsed =
            now - _transitionStart;

        if (elapsed >= _transitionTime)
        {
            _currentEffect =
                _nextEffect;

            _transitioning = false;

            render(
                _currentEffect,
                _oldFrame
            );

            for (uint16_t i = 0;
                 i < LedMatrix::LED_COUNT;
                 i++)
            {
                _oldFrame[i] =
                    _newFrame[i];
            }

            for (uint8_t y = 0;
                 y < LedMatrix::HEIGHT;
                 y++)
            {
                for (uint8_t x = 0;
                     x < LedMatrix::WIDTH;
                     x++)
                {
                    const uint16_t i =
                        index(x, y);

                    _matrix.setPixel(
                        x,
                        y,
                        getR(_newFrame[i]),
                        getG(_newFrame[i]),
                        getB(_newFrame[i])
                    );
                }
            }

            _matrix.show();

            _step++;

            return;
        }

        const uint8_t amount =
            static_cast<uint8_t>(
                constrain(
                    (
                        elapsed * 255UL
                    ) /
                    _transitionTime,
                    0UL,
                    255UL
                )
            );

        mixFrames(amount);

        _step++;

        return;
    }

    // --------------------------------------------------------
    // Normal effect
    // --------------------------------------------------------

    render(
        _currentEffect,
        _oldFrame
    );

    for (uint8_t y = 0;
         y < LedMatrix::HEIGHT;
         y++)
    {
        for (uint8_t x = 0;
             x < LedMatrix::WIDTH;
             x++)
        {
            const uint16_t i =
                index(x, y);

            _matrix.setPixel(
                x,
                y,
                getR(_oldFrame[i]),
                getG(_oldFrame[i]),
                getB(_oldFrame[i])
            );
        }
    }

    _matrix.show();

    _step++;
}

// ============================================================
// EFFECT
// ============================================================

void MatrixEffects::setEffect(
    Effect effect
)
{
    if (effect >= Effect::COUNT)
        effect = Effect::None;

    if (effect == _currentEffect &&
        !_transitioning)
    {
        return;
    }

    if (_transitioning &&
        effect == _nextEffect)
    {
        return;
    }

    // Capture the currently visible frame.
    if (!_transitioning)
    {
        render(
            _currentEffect,
            _oldFrame
        );
    }

    _nextEffect = effect;

    _transitionStart = millis();

    _transitioning = true;

    _step = 0;
}

MatrixEffects::Effect
MatrixEffects::effect() const
{
    if (_transitioning)
        return _nextEffect;

    return _currentEffect;
}

// ============================================================
// TRANSITION TIME
// ============================================================

void MatrixEffects::setTransitionTime(
    uint16_t milliseconds
)
{
    _transitionTime =
        constrain(
            milliseconds,
            0,
            10000
        );
}

uint16_t MatrixEffects::transitionTime() const
{
    return _transitionTime;
}

bool MatrixEffects::isTransitioning() const
{
    return _transitioning;
}

// ============================================================
// SPEED
// ============================================================

void MatrixEffects::setSpeed(
    uint8_t speed
)
{
    _speed =
        constrain(
            speed,
            0,
            100
        );
}

uint8_t MatrixEffects::speed() const
{
    return _speed;
}

// ============================================================
// RENDER
// ============================================================

void MatrixEffects::render(
    Effect effect,
    uint32_t* buffer
)
{
    clearBuffer(buffer);

    switch (effect)
    {
        case Effect::None:
            break;

        case Effect::Rainbow:
            renderRainbow(buffer);
            break;

        case Effect::RainbowWave:
            renderRainbowWave(buffer);
            break;

        case Effect::Fire:
            renderFire(buffer);
            break;

        case Effect::Fireplace:
            renderFireplace(buffer);
            break;

        case Effect::Plasma:
            renderPlasma(buffer);
            break;

        case Effect::Aurora:
            renderAurora(buffer);
            break;

        case Effect::Ocean:
            renderOcean(buffer);
            break;

        case Effect::Twinkle:
            renderTwinkle(buffer);
            break;

        case Effect::Meteor:
            renderMeteor(buffer);
            break;

        case Effect::Matrix:
            renderMatrix(buffer);
            break;

        default:
            break;
    }
}

// ============================================================
// RAINBOW
// ============================================================

void MatrixEffects::renderRainbow(
    uint32_t* buffer
)
{
    for (uint8_t y = 0;
         y < LedMatrix::HEIGHT;
         y++)
    {
        for (uint8_t x = 0;
             x < LedMatrix::WIDTH;
             x++)
        {
            const uint8_t hue =
                _step * 2
                + x * 8
                + y * 8;

            setPixel(
                buffer,
                x,
                y,
                getR(hsv(hue, 255, 255)),
                getG(hsv(hue, 255, 255)),
                getB(hsv(hue, 255, 255))
            );
        }
    }
}

// ============================================================
// RAINBOW WAVE
// ============================================================

void MatrixEffects::renderRainbowWave(
    uint32_t* buffer
)
{
    for (uint8_t y = 0;
         y < LedMatrix::HEIGHT;
         y++)
    {
        for (uint8_t x = 0;
             x < LedMatrix::WIDTH;
             x++)
        {
            const uint8_t hue =
                _step * 3
                + x * 12
                + y * 6;

            const uint8_t wave =
                sin8(
                    _step * 3
                    + x * 10
                    + y * 10
                );

            const uint8_t value =
                map(
                    wave,
                    0,
                    255,
                    50,
                    255
                );

            const uint32_t c =
                hsv(
                    hue,
                    255,
                    value
                );

            setPixel(
                buffer,
                x,
                y,
                getR(c),
                getG(c),
                getB(c)
            );
        }
    }
}

// ============================================================
// FIRE
// ============================================================

void MatrixEffects::renderFire(
    uint32_t* buffer
)
{
    for (uint8_t x = 0;
         x < LedMatrix::WIDTH;
         x++)
    {
        uint8_t heat =
            random8(
                150,
                255
            );

        for (uint8_t y = 0;
             y < LedMatrix::HEIGHT;
             y++)
        {
            const uint8_t flicker =
                random8(0, 45);

            const uint8_t value =
                heat > flicker
                ? heat - flicker
                : 0;

            setPixel(
                buffer,
                x,
                LedMatrix::HEIGHT - 1 - y,
                value,
                value / 3,
                0
            );

            if (heat > 8)
                heat -= random8(4, 12);
            else
                heat = 0;
        }
    }
}

// ============================================================
// FIREPLACE
// ============================================================

void MatrixEffects::renderFireplace(
    uint32_t* buffer
)
{
    for (uint8_t y = 0;
         y < LedMatrix::HEIGHT;
         y++)
    {
        for (uint8_t x = 0;
             x < LedMatrix::WIDTH;
             x++)
        {
            const uint8_t wave =
                sin8(
                    _step * 3
                    + x * 13
                    + y * 7
                );

            const uint8_t flicker =
                random8(0, 35);

            uint8_t value =
                wave > flicker
                ? wave - flicker
                : 0;

            value =
                map(
                    value,
                    0,
                    255,
                    15,
                    180
                );

            setPixel(
                buffer,
                x,
                y,
                value,
                value / 3,
                value / 20
            );
        }
    }
}

// ============================================================
// PLASMA
// ============================================================

void MatrixEffects::renderPlasma(
    uint32_t* buffer
)
{
    for (uint8_t y = 0;
         y < LedMatrix::HEIGHT;
         y++)
    {
        for (uint8_t x = 0;
             x < LedMatrix::WIDTH;
             x++)
        {
            const uint8_t a =
                sin8(
                    x * 16
                    + _step * 3
                );

            const uint8_t b =
                sin8(
                    y * 16
                    + _step * 2
                );

            const uint8_t c =
                sin8(
                    x * 8
                    + y * 8
                    + _step * 4
                );

            const uint8_t value =
                (
                    static_cast<uint16_t>(a)
                    + b
                    + c
                ) / 3;

            const uint32_t c1 =
                hsv(
                    value + _step,
                    220,
                    180
                );

            setPixel(
                buffer,
                x,
                y,
                getR(c1),
                getG(c1),
                getB(c1)
            );
        }
    }
}

// ============================================================
// AURORA
// ============================================================

void MatrixEffects::renderAurora(
    uint32_t* buffer
)
{
    for (uint8_t x = 0;
         x < LedMatrix::WIDTH;
         x++)
    {
        const uint8_t wave =
            sin8(
                _step * 2
                + x * 16
            );

        const uint8_t center =
            map(
                wave,
                0,
                255,
                2,
                LedMatrix::HEIGHT - 3
            );

        for (uint8_t y = 0;
             y < LedMatrix::HEIGHT;
             y++)
        {
            const uint8_t distance =
                abs(
                    static_cast<int>(y)
                    - static_cast<int>(center)
                );

            if (distance > 7)
                continue;

            const uint8_t value =
                255 - distance * 32;

            const uint32_t c =
                hsv(
                    90
                    + _step
                    + x * 3,
                    220,
                    value
                );

            setPixel(
                buffer,
                x,
                y,
                getR(c),
                getG(c),
                getB(c)
            );
        }
    }
}

// ============================================================
// OCEAN
// ============================================================

void MatrixEffects::renderOcean(
    uint32_t* buffer
)
{
    for (uint8_t y = 0;
         y < LedMatrix::HEIGHT;
         y++)
    {
        for (uint8_t x = 0;
             x < LedMatrix::WIDTH;
             x++)
        {
            const uint8_t wave1 =
                sin8(
                    x * 14
                    + _step * 2
                );

            const uint8_t wave2 =
                sin8(
                    y * 12
                    - _step * 3
                );

            const uint8_t value =
                (
                    static_cast<uint16_t>(wave1)
                    + wave2
                ) / 2;

            const uint8_t blue =
                map(
                    value,
                    0,
                    255,
                    70,
                    220
                );

            const uint8_t green =
                map(
                    value,
                    0,
                    255,
                    15,
                    120
                );

            setPixel(
                buffer,
                x,
                y,
                0,
                green,
                blue
            );
        }
    }
}

// ============================================================
// TWINKLE
// ============================================================

void MatrixEffects::renderTwinkle(
    uint32_t* buffer
)
{
    for (uint16_t i = 0;
         i < LedMatrix::LED_COUNT;
         i++)
    {
        buffer[i] =
            color(
                1,
                1,
                3
            );
    }

    for (uint8_t i = 0;
         i < 5;
         i++)
    {
        const uint8_t x =
            random8(
                0,
                LedMatrix::WIDTH
            );

        const uint8_t y =
            random8(
                0,
                LedMatrix::HEIGHT
            );

        const uint8_t brightness =
            random8(
                100,
                255
            );

        setPixel(
            buffer,
            x,
            y,
            brightness,
            brightness,
            brightness
        );
    }
}

// ============================================================
// METEOR
// ============================================================

void MatrixEffects::renderMeteor(
    uint32_t* buffer
)
{
    const uint16_t head =
        _step % 40;

    for (uint8_t y = 0;
         y < LedMatrix::HEIGHT;
         y++)
    {
        for (uint8_t x = 0;
             x < LedMatrix::WIDTH;
             x++)
        {
            const int16_t diagonal =
                x + y;

            const int16_t distance =
                abs(
                    diagonal -
                    static_cast<int16_t>(head)
                );

            if (distance > 6)
                continue;

            const uint8_t value =
                255 - distance * 38;

            setPixel(
                buffer,
                x,
                y,
                value,
                value / 4,
                0
            );
        }
    }
}

// ============================================================
// MATRIX
// ============================================================

void MatrixEffects::renderMatrix(
    uint32_t* buffer
)
{
    for (uint8_t x = 0;
         x < LedMatrix::WIDTH;
         x++)
    {
        const uint8_t head =
            (
                x * 7
                + _step * 2
            ) % LedMatrix::HEIGHT;

        for (uint8_t trail = 0;
             trail < 7;
             trail++)
        {
            if (head < trail)
                continue;

            const uint8_t y =
                head - trail;

            uint8_t brightness;

            if (trail == 0)
                brightness = 255;
            else
                brightness =
                    255 - trail * 38;

            setPixel(
                buffer,
                x,
                y,
                0,
                brightness,
                brightness / 3
            );
        }
    }
}

// ============================================================
// BUFFER
// ============================================================

void MatrixEffects::clearBuffer(
    uint32_t* buffer
)
{
    for (uint16_t i = 0;
         i < LedMatrix::LED_COUNT;
         i++)
    {
        buffer[i] = 0;
    }
}

// ============================================================
// SET PIXEL
// ============================================================

void MatrixEffects::setPixel(
    uint32_t* buffer,
    uint8_t x,
    uint8_t y,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    if (x >= LedMatrix::WIDTH ||
        y >= LedMatrix::HEIGHT)
    {
        return;
    }

    buffer[index(x, y)] =
        color(r, g, b);
}

// ============================================================
// INDEX
// ============================================================

uint16_t MatrixEffects::index(
    uint8_t x,
    uint8_t y
) const
{
    if ((y & 1) == 0)
        return y * LedMatrix::WIDTH + x;

    return y * LedMatrix::WIDTH
        + (LedMatrix::WIDTH - 1 - x);
}

// ============================================================
// COLOR
// ============================================================

uint32_t MatrixEffects::color( uint8_t r, uint8_t g, uint8_t b ) const { return _matrix.color(r, g, b); }

// ============================================================
// RGB COMPONENTS
// ============================================================

uint8_t MatrixEffects::getR(
    uint32_t color
) const
{
    return
        (color >> 16) & 0xFF;
}

uint8_t MatrixEffects::getG(
    uint32_t color
) const
{
    return
        (color >> 8) & 0xFF;
}

uint8_t MatrixEffects::getB(
    uint32_t color
) const
{
    return
        color & 0xFF;
}

// ============================================================
// MIX FRAMES
// ============================================================

void MatrixEffects::mixFrames(
    uint8_t amount
)
{
    for (uint8_t y = 0;
         y < LedMatrix::HEIGHT;
         y++)
    {
        for (uint8_t x = 0;
             x < LedMatrix::WIDTH;
             x++)
        {
            const uint16_t i =
                index(x, y);

            const uint32_t c =
                mixColor(
                    _oldFrame[i],
                    _newFrame[i],
                    amount
                );

            _matrix.setPixel(
                x,
                y,
                getR(c),
                getG(c),
                getB(c)
            );
        }
    }

    _matrix.show();
}

// ============================================================
// MIX COLOR
// ============================================================

uint32_t MatrixEffects::mixColor(
    uint32_t a,
    uint32_t b,
    uint8_t amount
)
{
    const uint8_t ar = getR(a);
    const uint8_t ag = getG(a);
    const uint8_t ab = getB(a);

    const uint8_t br = getR(b);
    const uint8_t bg = getG(b);
    const uint8_t bb = getB(b);

    const uint8_t r =
        ar +
        (
            (
                static_cast<int16_t>(br)
                - ar
            )
            * amount
        ) / 255;

    const uint8_t g =
        ag +
        (
            (
                static_cast<int16_t>(bg)
                - ag
            )
            * amount
        ) / 255;

    const uint8_t bl =
        ab +
        (
            (
                static_cast<int16_t>(bb)
                - ab
            )
            * amount
        ) / 255;

    return color(r, g, bl);
}

// ============================================================
// HSV
// ============================================================

uint32_t MatrixEffects::hsv(
    uint8_t h,
    uint8_t s,
    uint8_t v
) const
{
    if (s == 0)
        return color(v, v, v);

    const uint8_t region =
        h / 43;

    const uint8_t remainder =
        (h - region * 43) * 6;

    const uint8_t p =
        (v * (255 - s)) >> 8;

    const uint8_t q =
        (
            v *
            (
                255 -
                (
                    s * remainder >> 8
                )
            )
        ) >> 8;

    const uint8_t t =
        (
            v *
            (
                255 -
                (
                    s *
                    (255 - remainder) >> 8
                )
            )
        ) >> 8;

    switch (region)
    {
        case 0:
            return color(v, t, p);

        case 1:
            return color(q, v, p);

        case 2:
            return color(p, v, t);

        case 3:
            return color(p, q, v);

        case 4:
            return color(t, p, v);

        default:
            return color(v, p, q);
    }
}

// ============================================================
// SIN8
// ============================================================

uint8_t MatrixEffects::sin8(
    uint8_t value
) const
{
    const float radians =
        value *
        2.0f *
        PI /
        255.0f;

    const float result =
        (
            sinf(radians)
            + 1.0f
        )
        * 127.5f;

    return static_cast<uint8_t>(
        result
    );
}

// ============================================================
// RANDOM
// ============================================================

uint8_t MatrixEffects::random8(
    uint8_t min,
    uint8_t max
) const
{
    if (max <= min)
        return min;

    return static_cast<uint8_t>(
        random(min, max)
    );
}

// ============================================================
// COLOR WHEEL
// ============================================================

uint32_t MatrixEffects::wheel(
    uint8_t position
) const
{
    position = 255 - position;

    if (position < 85)
    {
        return color(
            255 - position * 3,
            0,
            position * 3
        );
    }

    if (position < 170)
    {
        position -= 85;

        return color(
            0,
            position * 3,
            255 - position * 3
        );
    }

    position -= 170;

    return color(
        position * 3,
        255 - position * 3,
        0
    );
}