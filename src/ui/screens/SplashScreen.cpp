#include "SplashScreen.h"

#include <Arduino.h>
#include <Adafruit_GFX.h>

static constexpr uint16_t BLACK = 0x0000;
static constexpr uint16_t DARK_GREEN = 0x0200;
static constexpr uint16_t GREEN = 0x05E0;
static constexpr uint16_t BRIGHT_GREEN = 0x07E0;

static const char MATRIX_CHARS[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "!@#$%^&*"
    "+-=/<>[]{}"
    ":;";

static constexpr uint8_t CHAR_COUNT =
    sizeof(MATRIX_CHARS) - 1;

static constexpr uint32_t ANIMATION_TIME = 2200;
static constexpr uint16_t FRAME_TIME = 30;

SplashScreen::SplashScreen(
    DisplayManager& display
)
    : _display(display),
      _state(State::Idle),
      _startTime(0),
      _lastFrameTime(0),
      _head{},
      _length{},
      _speed{}
{
}

void SplashScreen::start()
{
    randomSeed(micros());

    initializeRain();
    clear();

    _startTime = millis();
    _lastFrameTime = _startTime;
    _state = State::Running;
}

bool SplashScreen::update()
{
    if (_state != State::Running)
    {
        return false;
    }

    const uint32_t now = millis();

    if (
        now - _startTime >=
        ANIMATION_TIME
    )
    {
        clear();

        _state = State::Finished;

        return false;
    }

    if (
        now - _lastFrameTime >=
        FRAME_TIME
    )
    {
        _lastFrameTime = now;

        drawFrame();
    }

    return true;
}

bool SplashScreen::isActive() const
{
    return _state == State::Running;
}

void SplashScreen::initializeRain()
{
    for (
        uint8_t d = 0;
        d < DISPLAY_COUNT;
        ++d
    )
    {
        for (
            uint8_t c = 0;
            c < COLUMNS;
            ++c
        )
        {
            _head[d][c] =
                random(-320, 0);

            _length[d][c] =
                random(5, 18);

            _speed[d][c] =
                random(1, 4);
        }
    }
}

void SplashScreen::clear()
{
    for (
        uint8_t d = 0;
        d < DISPLAY_COUNT;
        ++d
    )
    {
        _display
            .get(d)
            .fillScreen(BLACK);
    }
}

void SplashScreen::drawFrame()
{
    for (
        uint8_t d = 0;
        d < DISPLAY_COUNT;
        ++d
    )
    {
        ST7789_172x320& tft =
            _display.get(d);

        for (
            uint8_t c = 0;
            c < COLUMNS;
            ++c
        )
        {
            const int16_t x =
                c * 6;

            const int16_t head =
                _head[d][c];

            const uint8_t length =
                _length[d][c];

            for (
                uint8_t i = 0;
                i < length;
                ++i
            )
            {
                const int16_t y =
                    head - i * 8;

                if (
                    y < 0 ||
                    y >= 320
                )
                {
                    continue;
                }

                const char character =
                    MATRIX_CHARS[
                        random(
                            0,
                            CHAR_COUNT
                        )
                    ];

                tft.setFont(nullptr);
                tft.setTextSize(1);

                if (i == 0)
                {
                    tft.setTextColor(
                        BRIGHT_GREEN
                    );
                }
                else if (i < 3)
                {
                    tft.setTextColor(
                        GREEN
                    );
                }
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

                tft.print(character);
            }

            _head[d][c] +=
                _speed[d][c];

            if (
                _head[d][c] >
                320 + _length[d][c] * 8
            )
            {
                _head[d][c] =
                    random(-160, -10);

                _length[d][c] =
                    random(5, 18);

                _speed[d][c] =
                    random(1, 4);
            }
        }
    }
}