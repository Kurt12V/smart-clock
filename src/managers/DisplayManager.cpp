#include "DisplayManager.h"


DisplayManager::DisplayManager(
    Display& display
)
    : _display(display)
{
}


// ============================================================
// BEGIN
// ============================================================

bool DisplayManager::begin()
{
    return _display.begin();
}


// ============================================================
// Физический дисплей
// ============================================================

ST7789_172x320& DisplayManager::get(
    uint8_t index
)
{
    return _display.get(index);
}


// ============================================================
// Размеры
// ============================================================

int16_t DisplayManager::width() const
{
    return TOTAL_WIDTH;
}


int16_t DisplayManager::height() const
{
    return DISPLAY_HEIGHT;
}


// ============================================================
// Очистка
// ============================================================

void DisplayManager::clear(
    uint16_t color
)
{
    for (uint8_t i = 0; i < DISPLAY_COUNT; i++)
    {
        _display.get(i).fillScreen(color);
    }
}


// ============================================================
// Проверка координат
// ============================================================

bool DisplayManager::validPoint(
    int16_t x,
    int16_t y
) const
{
    return
        x >= 0 &&
        x < TOTAL_WIDTH &&
        y >= 0 &&
        y < DISPLAY_HEIGHT;
}


// ============================================================
// Определить физический дисплей
// ============================================================

uint8_t DisplayManager::displayIndex(
    int16_t x
) const
{
    return x / DISPLAY_WIDTH;
}


// ============================================================
// Локальная X координата
// ============================================================

int16_t DisplayManager::localX(
    int16_t x
) const
{
    return x % DISPLAY_WIDTH;
}


// ============================================================
// Pixel
// ============================================================

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

    uint8_t index = displayIndex(x);

    int16_t lx = localX(x);

    _display.get(index).drawPixel(
        lx,
        y,
        color
    );
}


// ============================================================
// Horizontal line
// ============================================================

void DisplayManager::drawFastHLine(
    int16_t x,
    int16_t y,
    int16_t width,
    uint16_t color
)
{
    if (width <= 0)
    {
        return;
    }

    int16_t xEnd = x + width;

    if (y < 0 || y >= DISPLAY_HEIGHT)
    {
        return;
    }

    if (xEnd <= 0 || x >= TOTAL_WIDTH)
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
        uint8_t index = displayIndex(x);

        int16_t lx = localX(x);

        int16_t available =
            DISPLAY_WIDTH - lx;

        int16_t part =
            min(width, available);

        _display.get(index).drawFastHLine(
            lx,
            y,
            part,
            color
        );

        x += part;
        width -= part;
    }
}


// ============================================================
// Vertical line
// ============================================================

void DisplayManager::drawFastVLine(
    int16_t x,
    int16_t y,
    int16_t height,
    uint16_t color
)
{
    if (!validPoint(x, y) || height <= 0)
    {
        return;
    }

    if (y + height > DISPLAY_HEIGHT)
    {
        height =
            DISPLAY_HEIGHT - y;
    }

    uint8_t index =
        displayIndex(x);

    int16_t lx =
        localX(x);

    _display.get(index).drawFastVLine(
        lx,
        y,
        height,
        color
    );
}


// ============================================================
// Fill rectangle
// ============================================================

void DisplayManager::fillRect(
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

    if (
        x + width <= 0 ||
        x >= TOTAL_WIDTH ||
        y + height <= 0 ||
        y >= DISPLAY_HEIGHT
    )
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
        width =
            TOTAL_WIDTH - x;
    }

    if (y + height > DISPLAY_HEIGHT)
    {
        height =
            DISPLAY_HEIGHT - y;
    }

    while (width > 0)
    {
        uint8_t index =
            displayIndex(x);

        int16_t lx =
            localX(x);

        int16_t available =
            DISPLAY_WIDTH - lx;

        int16_t part =
            min(width, available);

        _display.get(index).fillRect(
            lx,
            y,
            part,
            height,
            color
        );

        x += part;
        width -= part;
    }
}


// ============================================================
// Rectangle
// ============================================================

void DisplayManager::drawRect(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height,
    uint16_t color
)
{
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


// ============================================================
// Circle
// ============================================================

void DisplayManager::fillCircle(
    int16_t x,
    int16_t y,
    int16_t radius,
    uint16_t color
)
{
    if (radius <= 0)
    {
        drawPixel(x, y, color);
        return;
    }

    int16_t left =
        x - radius;

    int16_t right =
        x + radius;

    uint8_t leftDisplay =
        displayIndex(
            max<int16_t>(left, 0)
        );

    uint8_t rightDisplay =
        displayIndex(
            min<int16_t>(
                right,
                TOTAL_WIDTH - 1
            )
        );

    // Если круг целиком находится
    // на одном физическом дисплее,
    // используем аппаратную функцию.
    if (leftDisplay == rightDisplay)
    {
        _display.get(leftDisplay).fillCircle(
            localX(x),
            y,
            radius,
            color
        );

        return;
    }

    // Иначе рисуем по строкам
    for (int16_t yy = -radius; yy <= radius; yy++)
    {
        int16_t dx =
            sqrt(
                radius * radius -
                yy * yy
            );

        drawFastHLine(
            x - dx,
            y + yy,
            dx * 2 + 1,
            color
        );
    }
}


// ============================================================
// Circle outline
// ============================================================

void DisplayManager::drawCircle(
    int16_t x,
    int16_t y,
    int16_t radius,
    uint16_t color
)
{
    // Простая универсальная версия
    // через точки.

    if (radius <= 0)
    {
        drawPixel(
            x,
            y,
            color
        );

        return;
    }

    for (int16_t angle = 0; angle < 360; angle++)
    {
        float rad =
            angle * 0.0174532925f;

        int16_t px =
            x + cos(rad) * radius;

        int16_t py =
            y + sin(rad) * radius;

        drawPixel(
            px,
            py,
            color
        );
    }
}


// ============================================================
// TEXT
// ============================================================

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

    if (font == nullptr)
    {
        return;
    }

    // Текст может пересекать границу
    // физических дисплеев.
    //
    // Рисуем его на каждом дисплее,
    // используя виртуальную координату.

    for (uint8_t i = 0; i < DISPLAY_COUNT; i++)
    {
        int16_t offset =
            i * DISPLAY_WIDTH;

        int16_t local =
            x - offset;

        _display.get(i).setFont(font);

        _display.get(i).setTextColor(
            color
        );

        _display.get(i).setCursor(
            local,
            y
        );

        _display.get(i).print(
            text
        );
    }
}


// ============================================================
// Затемнение RGB565
// ============================================================

uint16_t DisplayManager::dimColor(
    uint16_t color,
    uint8_t amount
)
{
    uint8_t r =
        (color >> 11) & 0x1F;

    uint8_t g =
        (color >> 5) & 0x3F;

    uint8_t b =
        color & 0x1F;

    r =
        (r * amount) / 255;

    g =
        (g * amount) / 255;

    b =
        (b * amount) / 255;

    return
        (r << 11) |
        (g << 5) |
        b;
}