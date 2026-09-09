#include "./ui/Display.h"
#include "./ui/screens/SplashScreen.h"
#include <Fonts/FreeMono9pt7b.h>

#include <Arduino.h>
#include <Adafruit_GFX.h>


// ============================================================
// COLORS
// ============================================================

static constexpr uint16_t BLACK        = 0x0000;
static constexpr uint16_t DARK_GREEN   = 0x0200;
static constexpr uint16_t GREEN        = 0x05E0;
static constexpr uint16_t BRIGHT_GREEN = 0x07E0;


// ============================================================
// MATRIX CHARACTERS
// ============================================================

static const char MATRIX_CHARS[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "!@#$%^&*"
    "+-=/<>[]{}"
    ":;";


static constexpr uint8_t CHAR_COUNT =
    sizeof(MATRIX_CHARS) - 1;


// ============================================================
// TIMING
// ============================================================

static constexpr uint32_t ANIMATION_TIME = 15200;
static constexpr uint16_t FRAME_TIME = 150;


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
    randomSeed(micros());

    initializeRain();

    clear();

    uint32_t start = millis();

    while (millis() - start < ANIMATION_TIME)
    {
        drawFrame();
        delay(FRAME_TIME);
    }

    // --------------------------------------------------------
    // Финальное заполнение зелёным
    // --------------------------------------------------------

    // for (uint8_t d = 0; d < DISPLAY_COUNT; d++)
    // {
    //     _display
    //         .get(d)
    //         .fillScreen(0xF800);
    // }

    // delay(100);

    for (uint8_t d = 0; d < DISPLAY_COUNT; d++)
    {
        _display
            .get(d)
            .fillScreen(BLACK);
    }
}


// ============================================================
// INITIALIZE RAIN
// ============================================================

void SplashScreen::initializeRain()
{
    for (uint8_t d = 0; d < DISPLAY_COUNT; d++)
    {
        for (uint8_t c = 0; c < COLUMNS; c++)
        {
            _head[d][c] = random(-320, 0);
            _length[d][c] = random(5, 9);
            _speed[d][c] = random(10, 20);
        }
    }
}


// ============================================================
// CLEAR
// ============================================================

void SplashScreen::clear()
{
    for (uint8_t d = 0; d < DISPLAY_COUNT; d++)
    {
        _display
            .get(d)
            .fillScreen(BLACK);
    }
}

    void SplashScreen::drawFrame()
{
    for (uint8_t d = 0; d < DISPLAY_COUNT; d++)
    {
        ST7789_172x320& tft = _display.get(d);

        tft.setFont(nullptr);
        tft.setTextSize(1);

        for (uint8_t c = 0; c < COLUMNS; c++)
        {
            const int16_t x = c * 6;
            const uint8_t length = _length[d][c];

            // Сначала сдвигаем поток вниз.
            _head[d][c] += _speed[d][c];
            const int16_t head = _head[d][c];

            // Убираем символ, который вышел за хвост потока.
            const int16_t eraseY = head - length * 8;
            if (eraseY >= 0 && eraseY < 320)
            {
                tft.fillRect(x, eraseY, 6, 8, BLACK);
            }

            // Рисуем видимую часть потока.
            for (uint8_t i = 0; i < length; i++)
            {
                const int16_t y = head - i * 8;

                if (y < 0 || y >= 320)
                {
                    continue;
                }

                if (i == 0)
                {
                    tft.setTextColor(BRIGHT_GREEN);
                }
                else if (i < 3)
                {
                    tft.setTextColor(GREEN);
                }
                else
                {
                    tft.setTextColor(DARK_GREEN);
                }

                tft.setCursor(x, y);
                tft.print(MATRIX_CHARS[random(CHAR_COUNT)]);
            }

            // Начинаем поток заново, когда весь хвост прошёл экран.
            if (head - length * 8 > 320)
            {
                _head[d][c] = random(-160, -10);
                _length[d][c] = random(5, 9);
                _speed[d][c] = random(10, 20);
            }
        }
    }
}
