#include "LedMatrix.h"

LedMatrix::LedMatrix(
    uint8_t dataPin,
    uint8_t brightness
)
    : _matrix(
        LED_COUNT,
        dataPin,
        NEO_GRB + NEO_KHZ800
    )
{
    if (brightness > MAX_BRIGHTNESS)
        brightness = MAX_BRIGHTNESS;

    _brightness = brightness;
}

void LedMatrix::begin()
{
    _matrix.begin();

    _matrix.clear();

    _matrix.setBrightness(_brightness);

    _matrix.show();
}

void LedMatrix::show()
{
    _matrix.show();
}

void LedMatrix::clear()
{
    _matrix.clear();
}

void LedMatrix::setBrightness(uint8_t brightness)
{
    if (brightness > MAX_BRIGHTNESS)
        brightness = MAX_BRIGHTNESS;

    _brightness = brightness;

    _matrix.setBrightness(_brightness);
}

uint8_t LedMatrix::brightness() const
{
    return _brightness;
}

void LedMatrix::setPixel(
    uint8_t x,
    uint8_t y,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    if (x >= WIDTH || y >= HEIGHT)
        return;

    _matrix.setPixelColor(
        index(x, y),
        _matrix.Color(r, g, b)
    );
}

void LedMatrix::setPixel(
    uint8_t x,
    uint8_t y,
    uint32_t color
)
{
    if (x >= WIDTH || y >= HEIGHT)
        return;

    _matrix.setPixelColor(
        index(x, y),
        color
    );
}

void LedMatrix::fill(
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    _matrix.fill(
        _matrix.Color(r, g, b)
    );
}

void LedMatrix::fill(uint32_t color)
{
    _matrix.fill(color);
}

uint32_t LedMatrix::color(
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    return _matrix.Color(r, g, b);
}

uint16_t LedMatrix::index(
    uint8_t x,
    uint8_t y
) const
{
    // Четная строка:
    // 0 → 1 → 2 → ... → 15

    if (y % 2 == 0)
    {
        return y * WIDTH + x;
    }

    // Нечетная строка:
    // 15 → 14 → ... → 0

    return y * WIDTH + (WIDTH - 1 - x);
}