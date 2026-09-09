#include "DisplayManager.h"

#include <math.h>

DisplayManager::DisplayManager(
    Display& display
)
    : _display(display),
      _initialized(false)
{
}

bool DisplayManager::begin()
{
    if (_initialized)
    {
        return true;
    }

    if (!_display.begin())
    {
        return false;
    }

    _initialized = true;

    return true;
}

bool DisplayManager::isInitialized() const
{
    return _initialized;
}

ST7789_172x320& DisplayManager::get(
    uint8_t index
)
{
    return _display.get(index);
}

ST7789_172x320* DisplayManager::getPtr(
    uint8_t index
)
{
    return _display.getPtr(index);
}

void DisplayManager::clear(
    uint16_t color
)
{
    _display.clear(color);
}

void DisplayManager::clearDisplay(
    uint8_t index,
    uint16_t color
)
{
    if (index >= DISPLAY_COUNT)
    {
        return;
    }

    _display.clear(
        index,
        color
    );
}

bool DisplayManager::validPoint(
    int16_t x,
    int16_t y
) const
{
    return
        x >= 0 &&
        x < TOTAL_WIDTH &&
        y >= 0 &&
        y < TOTAL_HEIGHT;
}

bool DisplayManager::validRect(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height
) const
{
    if (width <= 0 || height <= 0)
    {
        return false;
    }

    if (x >= TOTAL_WIDTH || y >= TOTAL_HEIGHT)
    {
        return false;
    }

    if (x + width <= 0 || y + height <= 0)
    {
        return false;
    }

    return true;
}

uint8_t DisplayManager::displayIndex(
    int16_t x
) const
{
    if (x < 0)
    {
        return 0;
    }

    uint8_t index =
        static_cast<uint8_t>(
            x / DISPLAY_WIDTH
        );

    if (index >= DISPLAY_COUNT)
    {
        index = DISPLAY_COUNT - 1;
    }

    return index;
}

int16_t DisplayManager::localX(
    int16_t x
) const
{
    return x % DISPLAY_WIDTH;
}

void DisplayManager::drawPixel(
    int16_t x,
    int16_t y,
    uint16_t color
)
{
    if (!validPoint(x, y))
    {
        return;
    }

    const uint8_t index =
        displayIndex(x);

    _display
        .get(index)
        .drawPixel(
            localX(x),
            y,
            color
        );
}

void DisplayManager::drawFastHLine(
    int16_t x,
    int16_t y,
    int16_t width,
    uint16_t color
)
{
    if (!validRect(x, y, width, 1))
    {
        return;
    }

    if (x < 0)
    {
        width += x;
        x = 0;
    }

    if (x + width > TOTAL_WIDTH)
    {
        width = TOTAL_WIDTH - x;
    }

    while (width > 0)
    {
        const uint8_t index =
            displayIndex(x);

        const int16_t local =
            localX(x);

        const int16_t available =
            DISPLAY_WIDTH - local;

        const int16_t part =
            min(width, available);

        _display
            .get(index)
            .drawFastHLine(
                local,
                y,
                part,
                color
            );

        x += part;
        width -= part;
    }
}

void DisplayManager::drawFastVLine(
    int16_t x,
    int16_t y,
    int16_t height,
    uint16_t color
)
{
    if (!validRect(x, y, 1, height))
    {
        return;
    }

    if (x < 0 || x >= TOTAL_WIDTH)
    {
        return;
    }

    if (y < 0)
    {
        height += y;
        y = 0;
    }

    if (y + height > TOTAL_HEIGHT)
    {
        height = TOTAL_HEIGHT - y;
    }

    if (height <= 0)
    {
        return;
    }

    const uint8_t index =
        displayIndex(x);

    _display
        .get(index)
        .drawFastVLine(
            localX(x),
            y,
            height,
            color
        );
}

void DisplayManager::fillRect(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height,
    uint16_t color
)
{
    if (!validRect(x, y, width, height))
    {
        return;
    }

    if (x < 0)
    {
        width += x;
        x = 0;
    }

    if (y < 0)
    {
        height += y;
        y = 0;
    }

    if (x + width > TOTAL_WIDTH)
    {
        width = TOTAL_WIDTH - x;
    }

    if (y + height > TOTAL_HEIGHT)
    {
        height = TOTAL_HEIGHT - y;
    }

    if (width <= 0 || height <= 0)
    {
        return;
    }

    while (width > 0)
    {
        const uint8_t index =
            displayIndex(x);

        const int16_t local =
            localX(x);

        const int16_t available =
            DISPLAY_WIDTH - local;

        const int16_t part =
            min(width, available);

        _display
            .get(index)
            .fillRect(
                local,
                y,
                part,
                height,
                color
            );

        x += part;
        width -= part;
    }
}

void DisplayManager::drawRect(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height,
    uint16_t color
)
{
    if (width <= 0 || height <= 0)
    {
        return;
    }

    drawFastHLine(
        x,
        y,
        width,
        color
    );

    drawFastHLine(
        x,
        y + height - 1,
        width,
        color
    );

    drawFastVLine(
        x,
        y,
        height,
        color
    );

    drawFastVLine(
        x + width - 1,
        y,
        height,
        color
    );
}

void DisplayManager::fillCircle(
    int16_t x,
    int16_t y,
    int16_t radius,
    uint16_t color
)
{
    if (radius <= 0)
    {
        return;
    }

    if (
        x - radius >= 0 &&
        x + radius < TOTAL_WIDTH &&
        y - radius >= 0 &&
        y + radius < TOTAL_HEIGHT
    )
    {
        const uint8_t first =
            displayIndex(x - radius);

        const uint8_t last =
            displayIndex(x + radius);

        if (first == last)
        {
            _display
                .get(first)
                .fillCircle(
                    localX(x),
                    y,
                    radius,
                    color
                );

            return;
        }
    }

    const int32_t r2 =
        static_cast<int32_t>(radius) * radius;

    for (
        int16_t py = -radius;
        py <= radius;
        ++py
    )
    {
        const int32_t value =
            r2 -
            static_cast<int32_t>(py) * py;

        if (value < 0)
        {
            continue;
        }

        const int16_t xx =
            static_cast<int16_t>(
                sqrt(
                    static_cast<double>(value)
                )
            );

        drawFastHLine(
            x - xx,
            y + py,
            xx * 2 + 1,
            color
        );
    }
}

void DisplayManager::drawCircle(
    int16_t x,
    int16_t y,
    int16_t radius,
    uint16_t color
)
{
    if (radius <= 0)
    {
        return;
    }

    if (
        x - radius >= 0 &&
        x + radius < TOTAL_WIDTH &&
        y - radius >= 0 &&
        y + radius < TOTAL_HEIGHT
    )
    {
        const uint8_t first =
            displayIndex(x - radius);

        const uint8_t last =
            displayIndex(x + radius);

        if (first == last)
        {
            _display
                .get(first)
                .drawCircle(
                    localX(x),
                    y,
                    radius,
                    color
                );

            return;
        }
    }

    const int32_t r2 =
        static_cast<int32_t>(radius) * radius;

    for (
        int16_t py = -radius;
        py <= radius;
        ++py
    )
    {
        const int32_t value =
            r2 -
            static_cast<int32_t>(py) * py;

        if (value < 0)
        {
            continue;
        }

        const int16_t xx =
            static_cast<int16_t>(
                sqrt(
                    static_cast<double>(value)
                )
            );

        drawPixel(
            x - xx,
            y + py,
            color
        );

        drawPixel(
            x + xx,
            y + py,
            color
        );
    }
}

void DisplayManager::drawText(
    const char* text,
    int16_t x,
    int16_t y,
    uint16_t color,
    const GFXfont* font
)
{
    if (text == nullptr)
    {
        return;
    }

    if (!validPoint(x, y))
    {
        return;
    }

    const uint8_t index =
        displayIndex(x);

    drawTextSingleDisplay(
        _display.get(index),
        text,
        localX(x),
        y,
        color,
        font
    );
}

void DisplayManager::drawTextSingleDisplay(
    ST7789_172x320& tft,
    const char* text,
    int16_t x,
    int16_t y,
    uint16_t color,
    const GFXfont* font
)
{
    tft.setFont(font);
    tft.setTextColor(color);
    tft.setCursor(x, y);
    tft.print(text);
}

uint16_t DisplayManager::dimColor(
    uint16_t color,
    uint8_t amount
)
{
    const uint8_t r =
        (color >> 11) & 0x1F;

    const uint8_t g =
        (color >> 5) & 0x3F;

    const uint8_t b =
        color & 0x1F;

    const uint8_t r2 =
        (r * amount) / 255;

    const uint8_t g2 =
        (g * amount) / 255;

    const uint8_t b2 =
        (b * amount) / 255;

    return
        static_cast<uint16_t>(
            (r2 << 11) |
            (g2 << 5) |
            b2
        );
}