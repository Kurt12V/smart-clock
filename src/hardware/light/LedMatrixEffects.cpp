#include "LedMatrixEffects.h"

#include <math.h>
#include <string.h>

// ============================================================
// CONSTRUCTOR
// ============================================================

LedMatrixEffects::LedMatrixEffects(
    LedMatrix& matrix
)
    : _matrix(matrix),
      _type(Type::None),
      _speed(50),
      _step(0),
      _lastUpdate(0),
      _fireDirection(FireDirection::BottomToTop)
{
    reset();
}

// ============================================================
// BEGIN
// ============================================================

void LedMatrixEffects::begin()
{
    reset();

    _lastUpdate = millis();
}

// ============================================================
// RESET
// ============================================================

void LedMatrixEffects::reset()
{
    _step = 0;

    _lastUpdate = millis();

    resetFire();
    resetTwinkle();
    resetRain();
    resetSparkle();
}

// ============================================================
// SET TYPE
// ============================================================

void LedMatrixEffects::setType(
    Type type
)
{
    if (type >= Type::COUNT)
    {
        type = Type::None;
    }

    if (_type == type)
    {
        return;
    }

    _type = type;

    reset();

    _matrix.clear();
}

// ============================================================
// GET TYPE
// ============================================================

LedMatrixEffects::Type
LedMatrixEffects::type() const
{
    return _type;
}

// ============================================================
// SET SPEED
// ============================================================

void LedMatrixEffects::setSpeed(
    uint8_t speed
)
{
    _speed =
        constrain(
            speed,
            (uint8_t)1,
            (uint8_t)100
        );
}

// ============================================================
// GET SPEED
// ============================================================

uint8_t LedMatrixEffects::speed() const
{
    return _speed;
}

// ============================================================
// FIRE DIRECTION
// ============================================================

void LedMatrixEffects::setFireDirection(
    FireDirection direction
)
{
    _fireDirection = direction;

    resetFire();
}

// ============================================================
// GET FIRE DIRECTION
// ============================================================

LedMatrixEffects::FireDirection
LedMatrixEffects::fireDirection() const
{
    return _fireDirection;
}

// ============================================================
// UPDATE
// ============================================================

void LedMatrixEffects::update()
{
    const uint32_t now = millis();

    if (
        now - _lastUpdate <
        updateInterval()
    )
    {
        return;
    }

    _lastUpdate = now;

    ++_step;

    switch (_type)
    {
        case Type::None:
            _matrix.clear();
            break;

        case Type::Rainbow:
            renderRainbow();
            break;

        case Type::Pulse:
            renderPulse();
            break;

        case Type::Wave:
            renderWave();
            break;

        case Type::Scanner:
            renderScanner();
            break;

        case Type::Fire:
            renderFire();
            break;

        case Type::Twinkle:
            renderTwinkle();
            break;

        case Type::Aurora:
            renderAurora();
            break;

        case Type::MatrixRain:
            renderMatrixRain();
            break;

        case Type::Comet:
            renderComet();
            break;

        case Type::Sparkle:
            renderSparkle();
            break;

        case Type::ColorWipe:
            renderColorWipe();
            break;

        case Type::Meteor:
            renderMeteor();
            break;

        case Type::Plasma:
            renderPlasma();
            break;

        case Type::Fireworks:
            renderFireworks();
            break;

        case Type::Ocean:
            renderOcean();
            break;

        case Type::Lava:
            renderLava();
            break;

        case Type::Police:
            renderPolice();
            break;

        case Type::Confetti:
            renderConfetti();
            break;

        case Type::Breathe:
            renderBreathe();
            break;

        case Type::Galaxy:
            renderGalaxy();
            break;

        default:
            _matrix.clear();
            break;
    }
}

// ============================================================
// UPDATE INTERVAL
// ============================================================

uint16_t LedMatrixEffects::updateInterval() const
{
    /*
     * 1   = slow
     * 50  = medium
     * 100 = fast
     */

    return (uint16_t)map(
        _speed,
        1,
        100,
        100,
        10
    );
}

// ============================================================
// SIN8
// ============================================================

uint8_t LedMatrixEffects::sin8(
    uint16_t value
)
{
    const float angle =
        (
            (float)(value & 0xFF) /
            255.0f
        ) *
        2.0f *
        PI;

    const float result =
        (
            sinf(angle) +
            1.0f
        ) *
        127.5f;

    return (uint8_t)constrain(
        (int)result,
        0,
        255
    );
}

// ============================================================
// SCALE8
// ============================================================

uint8_t LedMatrixEffects::scale8(
    uint8_t value,
    uint8_t scale
)
{
    return (uint8_t)(
        (
            (uint16_t)value *
            (uint16_t)scale
        ) / 255
    );
}

// ============================================================
// WHEEL
// ============================================================

uint32_t LedMatrixEffects::wheel(
    uint8_t position
)
{
    position = 255 - position;

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
// SCALE COLOR
// ============================================================

uint32_t LedMatrixEffects::scaleColor(
    uint32_t color,
    uint8_t scale
)
{
    uint8_t r =
        (color >> 16) & 0xFF;

    uint8_t g =
        (color >> 8) & 0xFF;

    uint8_t b =
        color & 0xFF;

    r = scale8(r, scale);
    g = scale8(g, scale);
    b = scale8(b, scale);

    return _matrix.color(
        r,
        g,
        b
    );
}

// ============================================================
// FIRE COLOR
// ============================================================

uint32_t LedMatrixEffects::fireColor(
    uint8_t temperature
)
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    if (temperature < 85)
    {
        r = temperature * 3;
        g = 0;
        b = 0;
    }
    else if (temperature < 170)
    {
        r = 255;
        g = (temperature - 85) * 3;
        b = 0;
    }
    else
    {
        r = 255;
        g = 255;
        b = (temperature - 170) * 3;
    }

    return _matrix.color(
        r,
        g,
        b
    );
}

// ============================================================
// RAINBOW
// ============================================================

void LedMatrixEffects::renderRainbow()
{
    for (
        uint8_t y = 0;
        y < LedMatrix::HEIGHT;
        ++y
    )
    {
        for (
            uint8_t x = 0;
            x < LedMatrix::WIDTH;
            ++x
        )
        {
            const uint8_t hue =
                (
                    _step * 2 +
                    x * 8 +
                    y * 8
                ) & 0xFF;

            _matrix.setPixel(
                x,
                y,
                wheel(hue)
            );
        }
    }
}

// ============================================================
// PULSE
// ============================================================

void LedMatrixEffects::renderPulse()
{
    const uint8_t level =
        sin8(
            _step * 3
        );

    const uint32_t color =
        _matrix.color(
            level,
            level,
            level
        );

    _matrix.fill(
        color
    );
}

// ============================================================
// WAVE
// ============================================================

void LedMatrixEffects::renderWave()
{
    for (
        uint8_t y = 0;
        y < LedMatrix::HEIGHT;
        ++y
    )
    {
        for (
            uint8_t x = 0;
            x < LedMatrix::WIDTH;
            ++x
        )
        {
            const uint8_t wave =
                sin8(
                    _step * 4 +
                    x * 16 +
                    y * 8
                );

            _matrix.setPixel(
                x,
                y,
                _matrix.color(
                    0,
                    wave / 2,
                    wave
                )
            );
        }
    }
}

// ============================================================
// SCANNER
// ============================================================

void LedMatrixEffects::renderScanner()
{
    _matrix.clear();

    const uint16_t period =
        (
            LedMatrix::WIDTH * 2 -
            2
        );

    uint16_t position =
        _step % period;

    if (
        position >=
        LedMatrix::WIDTH
    )
    {
        position =
            LedMatrix::WIDTH * 2 -
            2 -
            position;
    }

    for (
        uint8_t y = 0;
        y < LedMatrix::HEIGHT;
        ++y
    )
    {
        const uint8_t distance =
            (uint8_t)abs(
                (int)y -
                (int)position
            );

        uint8_t level = 0;

        if (distance < 8)
        {
            const int calculated =
                255 -
                (int)distance * 30;

            level =
                calculated > 0
                    ? (uint8_t)calculated
                    : 0;
        }

        _matrix.setPixel(
            position,
            y,
            _matrix.color(
                level,
                0,
                0
            )
        );
    }
}

// ============================================================
// FIRE
// ============================================================

void LedMatrixEffects::renderFire()
{
    // --------------------------------------------------------
    // COOLING
    // --------------------------------------------------------

    const int coolingRange =
        (
            FIRE_COOLING *
            10 /
            LedMatrix::HEIGHT
        ) + 2;

    for (
        uint8_t x = 0;
        x < LedMatrix::WIDTH;
        ++x
    )
    {
        for (
            uint8_t y = 0;
            y < LedMatrix::HEIGHT;
            ++y
        )
        {
            const uint8_t cooldown =
                (uint8_t)random(
                    0,
                    coolingRange
                );

            if (
                _fireHeat[x][y] >
                cooldown
            )
            {
                _fireHeat[x][y] -=
                    cooldown;
            }
            else
            {
                _fireHeat[x][y] = 0;
            }
        }
    }

    // --------------------------------------------------------
    // HEAT PROPAGATION
    // --------------------------------------------------------

    for (
        uint8_t x = 0;
        x < LedMatrix::WIDTH;
        ++x
    )
    {
        for (
            int y = LedMatrix::HEIGHT - 1;
            y > 1;
            --y
        )
        {
            const uint16_t heat =
                (uint16_t)_fireHeat[x][y - 1] +
                (uint16_t)_fireHeat[x][y - 2] +
                (uint16_t)_fireHeat[x][y - 2];

            _fireHeat[x][y] =
                (uint8_t)(heat / 3);
        }
    }

    // --------------------------------------------------------
    // SPARKS
    // --------------------------------------------------------

    for (
        uint8_t x = 0;
        x < LedMatrix::WIDTH;
        ++x
    )
    {
        if (
            random(0, 255) <
            FIRE_SPARKING
        )
        {
            const uint16_t heat =
                (uint16_t)_fireHeat[x][0] +
                (uint16_t)random(100, 255);

            _fireHeat[x][0] =
                heat > 255
                    ? 255
                    : (uint8_t)heat;
        }
    }

    // --------------------------------------------------------
    // RENDER
    // --------------------------------------------------------

    for (
        uint8_t x = 0;
        x < LedMatrix::WIDTH;
        ++x
    )
    {
        for (
            uint8_t y = 0;
            y < LedMatrix::HEIGHT;
            ++y
        )
        {
            uint8_t sourceY = y;

            switch (_fireDirection)
            {
                case FireDirection::BottomToTop:

                    sourceY = y;

                    break;

                case FireDirection::TopToBottom:

                    sourceY =
                        LedMatrix::HEIGHT -
                        1 -
                        y;

                    break;

                default:

                    sourceY = y;

                    break;
            }

            _matrix.setPixel(
                x,
                y,
                fireColor(
                    _fireHeat[x][sourceY]
                )
            );
        }
    }
}

// ============================================================
// TWINKLE
// ============================================================

void LedMatrixEffects::renderTwinkle()
{
    for (
        uint16_t i = 0;
        i < LedMatrix::LED_COUNT;
        ++i
    )
    {
        if (_twinkleLevel[i] > 0)
        {
            _twinkleLevel[i] =
                _twinkleLevel[i] > 8
                    ? _twinkleLevel[i] - 8
                    : 0;
        }
    }

    if (
        random(0, 100) <
        20
    )
    {
        const uint16_t i =
            (uint16_t)random(
                0,
                LedMatrix::LED_COUNT
            );

        _twinkleLevel[i] =
            255;

        _twinkleHue[i] =
            (uint8_t)random(
                0,
                255
            );
    }

    for (
        uint8_t y = 0;
        y < LedMatrix::HEIGHT;
        ++y
    )
    {
        for (
            uint8_t x = 0;
            x < LedMatrix::WIDTH;
            ++x
        )
        {
            const uint16_t i =
                _matrix.index(x, y);

            const uint8_t level =
                _twinkleLevel[i];

            _matrix.setPixel(
                x,
                y,
                scaleColor(
                    wheel(
                        _twinkleHue[i]
                    ),
                    level
                )
            );
        }
    }
}

// ============================================================
// AURORA
// ============================================================

void LedMatrixEffects::renderAurora()
{
    for (
        uint8_t y = 0;
        y < LedMatrix::HEIGHT;
        ++y
    )
    {
        for (
            uint8_t x = 0;
            x < LedMatrix::WIDTH;
            ++x
        )
        {
            const uint8_t a =
                sin8(
                    _step * 2 +
                    x * 12
                );

            const uint8_t b =
                sin8(
                    _step +
                    y * 18
                );

            _matrix.setPixel(
                x,
                y,
                _matrix.color(
                    a / 5,
                    b,
                    a
                )
            );
        }
    }
}

// ============================================================
// MATRIX RAIN
// ============================================================

void LedMatrixEffects::renderMatrixRain()
{
    _matrix.clear();

    for (
        uint8_t x = 0;
        x < LedMatrix::WIDTH;
        ++x
    )
    {
        const int16_t head =
            _rainPosition[x];

        for (
            uint8_t tail = 0;
            tail < 6;
            ++tail
        )
        {
            const int16_t y =
                head -
                tail;

            if (
                y >= 0 &&
                y < LedMatrix::HEIGHT
            )
            {
                const int level =
                    255 -
                    (int)tail * 40;

                _matrix.setPixel(
                    x,
                    y,
                    _matrix.color(
                        0,
                        level > 0
                            ? (uint8_t)level
                            : 0,
                        0
                    )
                );
            }
        }

        ++_rainPosition[x];

        if (
            _rainPosition[x] >
            LedMatrix::HEIGHT + 8
        )
        {
            _rainPosition[x] =
                (int16_t)-random(
                    1,
                    12
                );
        }
    }
}

// ============================================================
// COMET
// ============================================================

void LedMatrixEffects::renderComet()
{
    _matrix.clear();

    const uint16_t position =
        _step %
        (
            LedMatrix::LED_COUNT +
            20
        );

    for (
        uint8_t tail = 0;
        tail < 15;
        ++tail
    )
    {
        const int16_t p =
            (int16_t)position -
            (int16_t)tail;

        if (
            p < 0 ||
            p >= LedMatrix::LED_COUNT
        )
        {
            continue;
        }

        const int calculated =
            255 -
            (int)tail * 16;

        const uint8_t level =
            calculated > 0
                ? (uint8_t)calculated
                : 0;

        const uint8_t x =
            p % LedMatrix::WIDTH;

        const uint8_t y =
            p / LedMatrix::WIDTH;

        _matrix.setPixel(
            x,
            y,
            scaleColor(
                _matrix.color(
                    255,
                    255,
                    255
                ),
                level
            )
        );
    }
}

// ============================================================
// SPARKLE
// ============================================================

void LedMatrixEffects::renderSparkle()
{
    for (
        uint16_t i = 0;
        i < LedMatrix::LED_COUNT;
        ++i
    )
    {
        if (_sparkleLevel[i] > 5)
        {
            _sparkleLevel[i] -= 5;
        }
        else
        {
            _sparkleLevel[i] = 0;
        }
    }

    for (
        uint8_t i = 0;
        i < 3;
        ++i
    )
    {
        const uint16_t pixel =
            (uint16_t)random(
                0,
                LedMatrix::LED_COUNT
            );

        _sparkleLevel[pixel] =
            255;

        _sparkleHue[pixel] =
            (uint8_t)random(
                0,
                255
            );
    }

    for (
        uint8_t y = 0;
        y < LedMatrix::HEIGHT;
        ++y
    )
    {
        for (
            uint8_t x = 0;
            x < LedMatrix::WIDTH;
            ++x
        )
        {
            const uint16_t i =
                _matrix.index(x, y);

            _matrix.setPixel(
                x,
                y,
                scaleColor(
                    wheel(
                        _sparkleHue[i]
                    ),
                    _sparkleLevel[i]
                )
            );
        }
    }
}

// ============================================================
// COLOR WIPE
// ============================================================

void LedMatrixEffects::renderColorWipe()
{
    const uint16_t position =
        _step %
        (
            LedMatrix::LED_COUNT +
            20
        );

    _matrix.clear();

    const uint16_t count =
        position < LedMatrix::LED_COUNT
            ? position
            : LedMatrix::LED_COUNT;

    for (
        uint16_t i = 0;
        i < count;
        ++i
    )
    {
        const uint8_t x =
            i % LedMatrix::WIDTH;

        const uint8_t y =
            i / LedMatrix::WIDTH;

        _matrix.setPixel(
            x,
            y,
            wheel(
                (_step * 2) & 0xFF
            )
        );
    }
}

// ============================================================
// METEOR
// ============================================================

void LedMatrixEffects::renderMeteor()
{
    _matrix.clear();

    const uint16_t position =
        _step %
        (
            LedMatrix::LED_COUNT +
            25
        );

    for (
        uint8_t tail = 0;
        tail < 25;
        ++tail
    )
    {
        const int16_t p =
            (int16_t)position -
            (int16_t)tail;

        if (
            p < 0 ||
            p >= LedMatrix::LED_COUNT
        )
        {
            continue;
        }

        const int calculated =
            255 -
            (int)tail * 10;

        const uint8_t level =
            calculated > 0
                ? (uint8_t)calculated
                : 0;

        const uint8_t x =
            p % LedMatrix::WIDTH;

        const uint8_t y =
            p / LedMatrix::WIDTH;

        _matrix.setPixel(
            x,
            y,
            scaleColor(
                _matrix.color(
                    255,
                    100,
                    20
                ),
                level
            )
        );
    }
}

// ============================================================
// PLASMA
// ============================================================

void LedMatrixEffects::renderPlasma()
{
    for (
        uint8_t y = 0;
        y < LedMatrix::HEIGHT;
        ++y
    )
    {
        for (
            uint8_t x = 0;
            x < LedMatrix::WIDTH;
            ++x
        )
        {
            const uint8_t a =
                sin8(
                    x * 16 +
                    _step * 3
                );

            const uint8_t b =
                sin8(
                    y * 16 +
                    _step * 2
                );

            const uint8_t c =
                sin8(
                    x * 10 +
                    y * 10 +
                    _step * 4
                );

            _matrix.setPixel(
                x,
                y,
                _matrix.color(
                    a,
                    b,
                    c
                )
            );
        }
    }
}

// ============================================================
// FIREWORKS
// ============================================================

void LedMatrixEffects::renderFireworks()
{
    _matrix.clear();

    const uint16_t centerX =
        LedMatrix::WIDTH / 2;

    const uint16_t centerY =
        LedMatrix::HEIGHT / 2;

    const uint8_t radius =
        _step % 16;

    for (
        uint16_t angle = 0;
        angle < 360;
        angle += 30
    )
    {
        const float rad =
            (float)angle *
            PI /
            180.0f;

        const int x =
            (int)centerX +
            (int)(
                cosf(rad) *
                radius
            );

        const int y =
            (int)centerY +
            (int)(
                sinf(rad) *
                radius
            );

        if (
            x >= 0 &&
            x < LedMatrix::WIDTH &&
            y >= 0 &&
            y < LedMatrix::HEIGHT
        )
        {
            _matrix.setPixel(
                (uint8_t)x,
                (uint8_t)y,
                wheel(
                    (uint8_t)angle
                )
            );
        }
    }
}

// ============================================================
// OCEAN
// ============================================================

void LedMatrixEffects::renderOcean()
{
    for (
        uint8_t y = 0;
        y < LedMatrix::HEIGHT;
        ++y
    )
    {
        for (
            uint8_t x = 0;
            x < LedMatrix::WIDTH;
            ++x
        )
        {
            const uint8_t wave =
                sin8(
                    _step * 2 +
                    x * 12 +
                    y * 7
                );

            _matrix.setPixel(
                x,
                y,
                _matrix.color(
                    0,
                    wave / 2,
                    wave
                )
            );
        }
    }
}

// ============================================================
// LAVA
// ============================================================

void LedMatrixEffects::renderLava()
{
    for (
        uint8_t y = 0;
        y < LedMatrix::HEIGHT;
        ++y
    )
    {
        for (
            uint8_t x = 0;
            x < LedMatrix::WIDTH;
            ++x
        )
        {
            const uint8_t value =
                sin8(
                    x * 10 +
                    y * 15 +
                    _step * 2
                );

            _matrix.setPixel(
                x,
                y,
                _matrix.color(
                    value,
                    value / 3,
                    0
                )
            );
        }
    }
}

// ============================================================
// POLICE
// ============================================================

void LedMatrixEffects::renderPolice()
{
    const bool state =
        ((_step / 5) & 1) != 0;

    if (state)
    {
        _matrix.fill(
            _matrix.color(
                255,
                0,
                0
            )
        );
    }
    else
    {
        _matrix.fill(
            _matrix.color(
                0,
                0,
                255
            )
        );
    }
}

// ============================================================
// CONFETTI
// ============================================================

void LedMatrixEffects::renderConfetti()
{
    for (
        uint8_t i = 0;
        i < 5;
        ++i
    )
    {
        const uint8_t x =
            (uint8_t)random(
                0,
                LedMatrix::WIDTH
            );

        const uint8_t y =
            (uint8_t)random(
                0,
                LedMatrix::HEIGHT
            );

        _matrix.setPixel(
            x,
            y,
            wheel(
                (uint8_t)random(
                    0,
                    255
                )
            )
        );
    }
}

// ============================================================
// BREATHE
// ============================================================

void LedMatrixEffects::renderBreathe()
{
    const uint8_t level =
        sin8(
            _step * 2
        );

    _matrix.fill(
        _matrix.color(
            level,
            level,
            level
        )
    );
}

// ============================================================
// GALAXY
// ============================================================

void LedMatrixEffects::renderGalaxy()
{
    _matrix.clear();

    for (
        uint8_t i = 0;
        i < 25;
        ++i
    )
    {
        const uint8_t x =
            (
                i * 7 +
                _step
            ) %
            LedMatrix::WIDTH;

        const uint8_t y =
            (
                i * 11 +
                _step / 2
            ) %
            LedMatrix::HEIGHT;

        _matrix.setPixel(
            x,
            y,
            wheel(
                i * 10
            )
        );
    }
}

// ============================================================
// RESET FIRE
// ============================================================

void LedMatrixEffects::resetFire()
{
    memset(
        _fireHeat,
        0,
        sizeof(_fireHeat)
    );
}

// ============================================================
// RESET TWINKLE
// ============================================================

void LedMatrixEffects::resetTwinkle()
{
    memset(
        _twinkleLevel,
        0,
        sizeof(_twinkleLevel)
    );

    memset(
        _twinkleHue,
        0,
        sizeof(_twinkleHue)
    );
}

// ============================================================
// RESET RAIN
// ============================================================

void LedMatrixEffects::resetRain()
{
    for (
        uint8_t x = 0;
        x < LedMatrix::WIDTH;
        ++x
    )
    {
        _rainPosition[x] =
            (int16_t)-random(
                0,
                LedMatrix::HEIGHT
            );
    }
}

// ============================================================
// RESET SPARKLE
// ============================================================

void LedMatrixEffects::resetSparkle()
{
    memset(
        _sparkleLevel,
        0,
        sizeof(_sparkleLevel)
    );

    memset(
        _sparkleHue,
        0,
        sizeof(_sparkleHue)
    );
}