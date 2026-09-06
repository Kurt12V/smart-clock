#include "./ui/Display.h"
#include "./ui/screens/SplashScreen.h"
#include <Fonts/FreeMono9pt7b.h>


// ============================================================
// Цвета
// ============================================================

static constexpr uint16_t COLOR_BLACK =
    0x0000;

static constexpr uint16_t COLOR_WHITE =
    0xFFFF;

static constexpr uint16_t COLOR_GRAY =
    0x6B6D;

static constexpr uint16_t COLOR_DARK_GRAY =
    0x18E3;

static constexpr uint16_t COLOR_GRID =
    0x0821;

static constexpr uint16_t COLOR_CYAN =
    0x07FF;

static constexpr uint16_t COLOR_DARK_CYAN =
    0x0128;


// ============================================================
// Constructor
// ============================================================

SplashScreen::SplashScreen(
    DisplayManager& display
)
    : _display(display)
{
}


// ============================================================
// SHOW
// ============================================================

void SplashScreen::show()
{
    _display.clear(
        COLOR_BLACK
    );


    drawBackground();

    drawGrid();

    drawFrame();

    drawTitle();

    drawSubtitle();

    delay(300);


    // --------------------------------------------------------
    // Loading
    // --------------------------------------------------------

    for (uint8_t i = 0; i < 4; i++)
    {
        drawLoadingDots(i);

        drawProgress(
            (i + 1) * 25
        );

        delay(180);
    }


    delay(400);
}


// ============================================================
// Background
// ============================================================

void SplashScreen::drawBackground()
{
    _display.fillRect(
        0,
        0,
        _display.width(),
        _display.height(),
        COLOR_BLACK
    );


    // Центральная область
    // слегка подсвечена циановым цветом

    _display.fillRect(
        100,
        90,
        488,
        150,
        COLOR_DARK_CYAN
    );
}


// ============================================================
// Grid
// ============================================================

void SplashScreen::drawGrid()
{
    const int16_t step = 32;


    for (
        int16_t x = 0;
        x < _display.width();
        x += step
    )
    {
        _display.drawFastVLine(
            x,
            0,
            _display.height(),
            COLOR_GRID
        );
    }


    for (
        int16_t y = 0;
        y < _display.height();
        y += step
    )
    {
        _display.drawFastHLine(
            0,
            y,
            _display.width(),
            COLOR_GRID
        );
    }
}


// ============================================================
// Frame
// ============================================================

void SplashScreen::drawFrame()
{
    const int16_t margin = 12;


    _display.drawRect(
        margin,
        margin,
        _display.width() - margin * 2,
        _display.height() - margin * 2,
        COLOR_DARK_GRAY
    );


    // Верхняя линия

    _display.drawFastHLine(
        40,
        35,
        180,
        COLOR_CYAN
    );


    _display.drawFastHLine(
        _display.width() - 220,
        35,
        180,
        COLOR_CYAN
    );


    // Нижняя линия

    _display.drawFastHLine(
        40,
        _display.height() - 35,
        180,
        COLOR_DARK_GRAY
    );


    _display.drawFastHLine(
        _display.width() - 220,
        _display.height() - 35,
        180,
        COLOR_DARK_GRAY
    );
}


// ============================================================
// Title
// ============================================================

void SplashScreen::drawTitle()
{
    _display.drawText(
        "SMART CLOCK",
        250,
        140,
        COLOR_WHITE,
        &FreeSansBold18pt7b
    );
}


// ============================================================
// Subtitle
// ============================================================

void SplashScreen::drawSubtitle()
{
    _display.drawText(
        "INTELLIGENT HOME DEVICE",
        220,
        185,
        COLOR_GRAY,
        &FreeSans9pt7b
    );


    _display.drawText(
        "SYSTEM INITIALIZATION",
        250,
        260,
        COLOR_DARK_GRAY,
        &FreeSans9pt7b
    );
}


// ============================================================
// Loading dots
// ============================================================

void SplashScreen::drawLoadingDots(
    uint8_t activeDot
)
{
    const int16_t startX = 300;

    const int16_t y = 225;

    const int16_t spacing = 28;

    for (uint8_t i = 0; i < 4; i++)
    {
        uint16_t color =
            (i == activeDot)
                ? COLOR_CYAN
                : COLOR_DARK_GRAY;

        _display.fillCircle(
            startX + i * spacing,
            y,
            4,
            color
        );
    }
}


// ============================================================
// Progress bar
// ============================================================

void SplashScreen::drawProgress(
    uint8_t progress
)
{
    const int16_t x = 224;

    const int16_t y = 285;

    const int16_t width = 240;

    const int16_t height = 5;


    // Background

    _display.fillRect(
        x,
        y,
        width,
        height,
        COLOR_DARK_GRAY
    );


    // Progress

    int16_t progressWidth =
        (width * progress) / 100;


    _display.fillRect(
        x,
        y,
        progressWidth,
        height,
        COLOR_CYAN
    );
}