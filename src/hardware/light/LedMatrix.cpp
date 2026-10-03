#include "LedMatrix.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

LedMatrix::LedMatrix(
    uint8_t dataPin,
    uint8_t brightness
)
    : _matrix(
        LED_COUNT,
        dataPin,
        NEO_GRB + NEO_KHZ800
    ),
      _brightness(
          brightness > MAX_BRIGHTNESS
              ? MAX_BRIGHTNESS
              : brightness
      )
{
}

// ============================================================
// BEGIN
// ============================================================

void LedMatrix::begin()
{
    _matrix.begin();

    _matrix.clear();

    _matrix.setBrightness(
        _brightness
    );

    _matrix.show();
}

// ============================================================
// SHOW
// ============================================================

void LedMatrix::show()
{
    _matrix.show();
}

// ============================================================
// CLEAR
// ============================================================

void LedMatrix::clear()
{
    _matrix.clear();
}

// ============================================================
// BRIGHTNESS
// ============================================================

void LedMatrix::setBrightness(
    uint8_t brightness
)
{
    if (brightness > MAX_BRIGHTNESS)
        brightness = MAX_BRIGHTNESS;

    _brightness =
        brightness;

    _matrix.setBrightness(
        _brightness
    );
}

// ============================================================
// GET BRIGHTNESS
// ============================================================

uint8_t LedMatrix::brightness() const
{
    return _brightness;
}

// ============================================================
// SET PIXEL RGB
// ============================================================

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
        _matrix.Color(
            r,
            g,
            b
        )
    );
}

// ============================================================
// SET PIXEL COLOR
// ============================================================

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

// ============================================================
// GET PIXEL
// ============================================================

uint32_t LedMatrix::getPixel(
    uint8_t x,
    uint8_t y
) const
{
    if (x >= WIDTH || y >= HEIGHT)
        return 0;

    return _matrix.getPixelColor(
        index(x, y)
    );
}

// ============================================================
// FILL RGB
// ============================================================

void LedMatrix::fill(
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    _matrix.fill(
        _matrix.Color(
            r,
            g,
            b
        )
    );
}

// ============================================================
// FILL COLOR
// ============================================================

void LedMatrix::fill(
    uint32_t color
)
{
    _matrix.fill(
        color
    );
}

// ============================================================
// COLOR
// ============================================================

uint32_t LedMatrix::color(
    uint8_t r,
    uint8_t g,
    uint8_t b
) const
{
    return _matrix.Color(
        r,
        g,
        b
    );
}

// ============================================================
// RAW GET
// ============================================================

uint32_t LedMatrix::pixel(
    uint16_t index
) const
{
    if (index >= LED_COUNT)
        return 0;

    return _matrix.getPixelColor(
        index
    );
}

// ============================================================
// RAW SET
// ============================================================

void LedMatrix::setPixelRaw(
    uint16_t index,
    uint32_t color
)
{
    if (index >= LED_COUNT)
        return;

    _matrix.setPixelColor(
        index,
        color
    );
}

// ============================================================
// XY -> INDEX
//
// Serpentine:
//
// 0  → 1  → ... → 15
// 31 ← 30 ← ... ← 16
// 32 → 33 → ... → 47
// ============================================================

uint16_t LedMatrix::index(
    uint8_t x,
    uint8_t y
) const
{
    if ((y & 1) == 0)
    {
        return
            y * WIDTH +
            x;
    }

    return
        y * WIDTH +
        (WIDTH - 1 - x);
}