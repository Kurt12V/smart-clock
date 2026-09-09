#include "SplashScreen.h"

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

static constexpr uint32_t ANIMATION_TIME = 2200;
static constexpr uint16_t FRAME_TIME = 30;


// ============================================================
// CONSTRUCTOR
// ============================================================

SplashScreen::SplashScreen(DisplayManager& display)
    : _display(display),
      _state(State::Idle),
      _startTime(0),
      _lastFrameTime(0)
{
}

void SplashScreen::start()
{
    randomSeed(micros());

    initializeRain();

    clear();

    uint32_t start =
        millis();

    while (
        millis() - start <
        ANIMATION_TIME
    )
    {
        drawFrame();

        delay(FRAME_TIME);
    }

    // --------------------------------------------------------
    // Финальное заполнение зелёным
    // --------------------------------------------------------

    for (uint8_t d = 0; d < DISPLAY_COUNT; d++)
    {
        _display
            .get(d)
            .fillScreen(
                BRIGHT_GREEN
            );
    }

    delay(35);

    for (uint8_t d = 0; d < DISPLAY_COUNT; d++)
    {
        _display
            .get(d)
            .fillScreen(
                BLACK
            );
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
            _head[d][c] =
                random(
                    -320,
                    0
                );

            _length[d][c] =
                random(
                    5,
                    18
                );

            _speed[d][c] =
                random(
                    1,
                    4
                );
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
            .fillScreen(
                BLACK
            );
    }
}


// ============================================================
// DRAW FRAME
// ============================================================

void SplashScreen::drawFrame()
{
    for (uint8_t d = 0; d < DISPLAY_COUNT; d++)
    {
        ST7789_172x320& tft =
            _display.get(d);

        // ----------------------------------------------------
        // Очень лёгкое стирание предыдущего кадра
        // ----------------------------------------------------

        // Чёрные полосы сверху/снизу оставляют ощущение
        // движения и уменьшают количество старых символов.
        tft.fillRect(
            0,
            0,
            172,
            2,
            BLACK
        );

        // ----------------------------------------------------
        // Колонки
        // ----------------------------------------------------

        for (
            uint8_t c = 0;
            c < COLUMNS;
            c++
        )
        {
            const int16_t x =
                c * 6;

            int16_t head =
                _head[d][c];

            uint8_t length =
                _length[d][c];

            // ------------------------------------------------
            // Рисуем хвост
            // ------------------------------------------------

            for (
                uint8_t i = 0;
                i < length;
                i++
            )
            {
                int16_t y =
                    head -
                    i * 8;

                if (
                    y < 0 ||
                    y >= 320
                )
                {
                    continue;
                }

                char character =
                    MATRIX_CHARS[
                        random(
                            0,
                            CHAR_COUNT
                        )
                    ];

                tft.setFont(nullptr);
                tft.setTextSize(1);

                // ------------------------------------------------
                // Голова
                // ------------------------------------------------

                if (i == 0)
                {
                    tft.setTextColor(
                        BRIGHT_GREEN
                    );
                }

                // ------------------------------------------------
                // Первые символы
                // ------------------------------------------------

                else if (i < 3)
                {
                    tft.setTextColor(
                        GREEN
                    );
                }

                // ------------------------------------------------
                // Дальний хвост
                // ------------------------------------------------

                else
                {
                    tft.setTextColor(
                        DARK_GREEN
                    );
                }

                tft.setCursor(
                    x,
                    y
                );

                tft.print(
                    character
                );
            }

            // ------------------------------------------------
            // Движение
            // ------------------------------------------------

            _head[d][c] +=
                _speed[d][c];

            // ------------------------------------------------
            // Перезапуск колонки
            // ------------------------------------------------

            if (
                _head[d][c] >
                320 +
                _length[d][c] * 8
            )
            {
                _head[d][c] =
                    random(
                        -160,
                        -10
                    );

                _length[d][c] =
                    random(
                        5,
                        18
                    );

                _speed[d][c] =
                    random(
                        1,
                        4
                    );
            }
        }
    }
}
