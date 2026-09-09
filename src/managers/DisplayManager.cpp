#include "DisplayManager.h"


// ============================================================
// Constructor
// ============================================================

DisplayManager::DisplayManager(
    Display& display
)
    :
      _display(display),
      _initialized(false)
{
}


// ============================================================
// begin()
// ============================================================

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


// ============================================================
// isInitialized()
// ============================================================

bool DisplayManager::isInitialized() const
{
    return _initialized;
}


// ============================================================
// Получить физический дисплей
// ============================================================

ST7789_172x320& DisplayManager::get(
    uint8_t index
)
{
    return _display.get(index);
}


// ============================================================
// Получить указатель
// ============================================================

ST7789_172x320* DisplayManager::getPtr(
    uint8_t index
)
{
    return _display.getPtr(index);
}


// ============================================================
// Очистить все дисплеи
// ============================================================

void DisplayManager::clear(
    uint16_t color
)
{
    _display.clear(color);
}


// ============================================================
// Очистить один дисплей
// ============================================================

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


// ============================================================
// Проверка координаты
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
        y < TOTAL_HEIGHT;
}


// ============================================================
// Проверка прямоугольника
// ============================================================

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


    if (x >= TOTAL_WIDTH)
    {
        return false;
    }


    if (y >= TOTAL_HEIGHT)
    {
        return false;
    }


    if (x + width <= 0)
    {
        return false;
    }


    if (y + height <= 0)
    {
        return false;
    }


    return true;
}


// ============================================================
// Определить дисплей по глобальному X
// ============================================================

uint8_t DisplayManager::displayIndex(
    int16_t x
) const
{
    if (x < 0)
    {
        return 0;
    }


    uint8_t index =
        x / DISPLAY_WIDTH;


    if (index >= DISPLAY_COUNT)
    {
        index = DISPLAY_COUNT - 1;
    }


    return index;
}


// ============================================================
// Глобальный X -> локальный X
// ============================================================

int16_t DisplayManager::localX(
    int16_t x
) const
{
    return x % DISPLAY_WIDTH;
}


// ============================================================
// drawPixel()
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


    const uint8_t display =
        displayIndex(x);


    const int16_t xLocal =
        localX(x);


    _display
        .get(display)
        .drawPixel(
            xLocal,
            y,
            color
        );
}


// ============================================================
// drawFastHLine()
// ============================================================

void DisplayManager::drawFastHLine(
    int16_t x,
    int16_t y,
    int16_t width,
    uint16_t color
)
{
    if (!validRect(
            x,
            y,
            width,
            1))
    {
        return;
    }


    /*
     * Обрезаем левую часть.
     */

    if (x < 0)
    {
        width += x;
        x = 0;
    }


    /*
     * Обрезаем правую часть.
     */

    if (x + width > TOTAL_WIDTH)
    {
        width =
            TOTAL_WIDTH - x;
    }


    if (width <= 0)
    {
        return;
    }


    /*
     * Горизонтальная линия может
     * пересекать несколько дисплеев.
     */

    while (width > 0)
    {
        const uint8_t display =
            displayIndex(x);

        const int16_t local =
            localX(x);

        const int16_t available =
            DISPLAY_WIDTH - local;

        const int16_t part =
            min(
                width,
                available
            );


        _display
            .get(display)
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


// ============================================================
// drawFastVLine()
// ============================================================

void DisplayManager::drawFastVLine(
    int16_t x,
    int16_t y,
    int16_t height,
    uint16_t color
)
{
    if (!validRect(
            x,
            y,
            1,
            height))
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
        height =
            TOTAL_HEIGHT - y;
    }


    if (height <= 0)
    {
        return;
    }


    const uint8_t display =
        displayIndex(x);


    const int16_t local =
        localX(x);


    _display
        .get(display)
        .drawFastVLine(
            local,
            y,
            height,
            color
        );
}


// ============================================================
// fillRect()
// ============================================================

void DisplayManager::fillRect(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height,
    uint16_t color
)
{
    if (!validRect(
            x,
            y,
            width,
            height))
    {
        return;
    }


    /*
     * Обрезка.
     */

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


    if (y + height > TOTAL_HEIGHT)
    {
        height =
            TOTAL_HEIGHT - y;
    }


    if (width <= 0 || height <= 0)
    {
        return;
    }


    /*
     * Разбиваем прямоугольник
     * между физическими дисплеями.
     */

    while (width > 0)
    {
        const uint8_t display =
            displayIndex(x);


        const int16_t local =
            localX(x);


        const int16_t available =
            DISPLAY_WIDTH - local;


        const int16_t part =
            min(
                width,
                available
            );


        _display
            .get(display)
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


// ============================================================
// drawRect()
// ============================================================

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


// ============================================================
// fillCircle()
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
        return;
    }


    /*
     * Если круг полностью находится
     * внутри одного дисплея — рисуем
     * напрямую, быстрее.
     */

    if (
        x - radius >= 0 &&
        x + radius < TOTAL_WIDTH
    )
    {
        const uint8_t first =
            displayIndex(
                x - radius
            );

        const uint8_t last =
            displayIndex(
                x + radius
            );


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


    /*
     * Если круг пересекает границу
     * дисплеев — используем пиксельный
     * алгоритм.
     *
     * Это редкий случай и нужен только
     * для графики, которая пересекает
     * физическую границу.
     */

    const int16_t r2 =
        radius * radius;


    for (
        int16_t py = -radius;
        py <= radius;
        py++
    )
    {
        const int16_t xx =
            sqrt(
                max(
                    0,
                    r2 - py * py
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


// ============================================================
// drawCircle()
// ============================================================

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


    /*
     * Обычный алгоритм круга через
     * Adafruit_GFX, если круг целиком
     * находится на одном дисплее.
     */

    if (
        x - radius >= 0 &&
        x + radius < TOTAL_WIDTH
    )
    {
        const uint8_t first =
            displayIndex(
                x - radius
            );

        const uint8_t last =
            displayIndex(
                x + radius
            );


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


    /*
     * Если пересекает дисплеи —
     * рисуем через точки.
     */

    const int16_t r2 =
        radius * radius;


    for (
        int16_t py = -radius;
        py <= radius;
        py++
    )
    {
        const int16_t value =
            r2 - py * py;


        if (value < 0)
        {
            continue;
        }


        const int16_t xx =
            sqrt(value);


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


// ============================================================
// drawText()
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


    if (!validPoint(x, y))
    {
        return;
    }


    /*
     * Текст в Adafruit_GFX может
     * использовать фиксированный
     * шрифт и ширину.
     *
     * Для обычных надписей ClockScreen
     * достаточно определить дисплей
     * по стартовой координате.
     */


    const uint8_t display =
        displayIndex(x);


    const int16_t local =
        localX(x);


    drawTextSingleDisplay(
        _display.get(display),
        text,
        local,
        y,
        color,
        font
    );
}


// ============================================================
// drawTextSingleDisplay()
// ============================================================

void DisplayManager::drawTextSingleDisplay(
    ST7789_172x320& tft,
    const char* text,
    int16_t x,
    int16_t y,
    uint16_t color,
    const GFXfont* font
)
{
    /*
     * Устанавливаем шрифт.
     */

    tft.setFont(font);


    /*
     * Цвет текста.
     */

    tft.setTextColor(color);


    /*
     * Положение.
     */

    tft.setCursor(
        x,
        y
    );


    /*
     * Вывод.
     */

    tft.print(text);
}


// ============================================================
// dimColor()
// ============================================================

uint16_t DisplayManager::dimColor(
    uint16_t color,
    uint8_t amount
)
{
    /*
     * amount:
     *
     * 0   = чёрный
     * 255 = исходный цвет
     */

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
        (r2 << 11) |
        (g2 << 5) |
        b2;
}
