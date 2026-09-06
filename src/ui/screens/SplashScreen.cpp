#include "./ui/Display.h"
#include "./ui/screens/SplashScreen.h"
#include <Fonts/FreeMono9pt7b.h>

#include <Arduino.h>
#include <Adafruit_GFX.h>

#include <Fonts/FreeSans.h>
#include <Fonts/FreeSansBold.h>


// ============================================================
// COLORS
// ============================================================

static constexpr uint16_t COLOR_BLACK     = 0x0000;
static constexpr uint16_t COLOR_WHITE     = 0xFFFF;
static constexpr uint16_t COLOR_GRAY      = 0x6B6D;
static constexpr uint16_t COLOR_DARK_GRAY = 0x18E3;
static constexpr uint16_t COLOR_GRID      = 0x0821;
static constexpr uint16_t COLOR_CYAN      = 0x07FF;
static constexpr uint16_t COLOR_DARK_CYAN = 0x0128;


// ============================================================
// CONSTRUCTOR
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
    // --------------------------------------------------------
    // Начальный экран
    // --------------------------------------------------------

    drawBackground();
    drawGrid();
    drawFrame();
    drawTitle();
    drawSubtitle();

    drawLoadingDots(0);
    drawProgress(0);

    delay(250);


    // --------------------------------------------------------
    // Progress 20%
    // --------------------------------------------------------

    drawLoadingDots(1);
    drawProgress(20);

    delay(250);


    // --------------------------------------------------------
    // Progress 40%
    // --------------------------------------------------------

    drawLoadingDots(2);
    drawProgress(40);

    delay(250);


    // --------------------------------------------------------
    // Progress 60%
    // --------------------------------------------------------

    drawLoadingDots(3);
    drawProgress(60);

    delay(250);


    // --------------------------------------------------------
    // Progress 80%
    // --------------------------------------------------------

    drawLoadingDots(4);
    drawProgress(80);

    delay(250);


    // --------------------------------------------------------
    // Progress 100%
    // --------------------------------------------------------

    drawLoadingDots(5);
    drawProgress(100);

    delay(400);
}


// ============================================================
// BACKGROUND
// ============================================================

void SplashScreen::drawBackground()
{
    _display.clear(COLOR_BLACK);
}


// ============================================================
// GRID
// ============================================================

void SplashScreen::drawGrid()
{
    constexpr int16_t GRID_SIZE = 32;

    // Вертикальные линии
    for (
        int16_t x = 0;
        x < DisplayManager::TOTAL_WIDTH;
        x += GRID_SIZE
    )
    {
        _display.drawFastVLine(
            x,
            0,
            DisplayManager::DISPLAY_HEIGHT,
            COLOR_GRID
        );
    }

    // Горизонтальные линии
    for (
        int16_t y = 0;
        y < DisplayManager::DISPLAY_HEIGHT;
        y += GRID_SIZE
    )
    {
        _display.drawFastHLine(
            0,
            y,
            DisplayManager::TOTAL_WIDTH,
            COLOR_GRID
        );
    }
}


// ============================================================
// FRAME
// ============================================================

void SplashScreen::drawFrame()
{
    constexpr int16_t X = 30;
    constexpr int16_t Y = 55;

    constexpr int16_t WIDTH  = 628;
    constexpr int16_t HEIGHT = 210;

    // Внешняя рамка
    _display.drawRect(
        X,
        Y,
        WIDTH,
        HEIGHT,
        COLOR_DARK_CYAN
    );

    // Внутренняя рамка
    _display.drawRect(
        X + 3,
        Y + 3,
        WIDTH - 6,
        HEIGHT - 6,
        COLOR_CYAN
    );
}


// ============================================================
// TITLE
// ============================================================

void SplashScreen::drawTitle()
{
    const char* title = "SMART CLOCK";

    _display.drawText(
        title,
        228,
        125,
        COLOR_WHITE,
        &FreeSansBold
    );
}


// ============================================================
// SUBTITLE
// ============================================================

void SplashScreen::drawSubtitle()
{
    const char* subtitle =
        "INTELLIGENT HOME DEVICE";

    _display.drawText(
        subtitle,
        218,
        155,
        COLOR_GRAY,
        &FreeSans
    );

    _display.drawText(
        "SYSTEM INITIALIZATION",
        254,
        190,
        COLOR_CYAN,
        &FreeSans
    );
}


// ============================================================
// LOADING DOTS
// ============================================================

void SplashScreen::drawLoadingDots(
    uint8_t activeDot
)
{
    constexpr int16_t START_X = 300;
    constexpr int16_t Y       = 220;

    constexpr int16_t SPACING = 28;
    constexpr int16_t RADIUS  = 5;

    for (uint8_t i = 0; i < 5; i++)
    {
        uint16_t color;

        if (i < activeDot)
        {
            color = COLOR_CYAN;
        }
        else
        {
            color = COLOR_DARK_GRAY;
        }

        _display.fillCircle(
            START_X + i * SPACING,
            Y,
            RADIUS,
            color
        );
    }
}


// ============================================================
// PROGRESS BAR
// ============================================================

void SplashScreen::drawProgress(
    uint8_t progress
)
{
    constexpr int16_t X = 160;
    constexpr int16_t Y = 245;

    constexpr int16_t WIDTH  = 368;
    constexpr int16_t HEIGHT = 8;

    // Рамка
    _display.drawRect(
        X,
        Y,
        WIDTH,
        HEIGHT,
        COLOR_DARK_GRAY
    );

    // Внутренняя область
    _display.fillRect(
        X + 2,
        Y + 2,
        WIDTH - 4,
        HEIGHT - 4,
        COLOR_BLACK
    );

    // Заполнение
    int16_t fillWidth =
        ((WIDTH - 4) * progress) / 100;

    if (fillWidth > 0)
    {
        _display.fillRect(
            X + 2,
            Y + 2,
            fillWidth,
            HEIGHT - 4,
            COLOR_CYAN
        );
    }
}