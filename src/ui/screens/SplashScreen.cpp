#include "SplashScreen.h"

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

namespace
{
    constexpr uint16_t COLOR_BLACK = ST77XX_BLACK;
    constexpr uint16_t COLOR_TEXT = ST77XX_WHITE;
    constexpr uint16_t COLOR_ACCENT = 0x05FF;
    constexpr uint16_t COLOR_ACCENT_DIM = 0x0330;
    constexpr uint16_t COLOR_MUTED = 0x7BEF;

    constexpr int16_t PROGRESS_X = 24;
    constexpr int16_t PROGRESS_Y = 270;
    constexpr int16_t PROGRESS_WIDTH = 640;
    constexpr int16_t PROGRESS_HEIGHT = 8;
}

SplashScreen::SplashScreen(DisplayManager& display)
    : _display(display),
      _state(State::Idle),
      _startTime(0),
      _lastFrameTime(0)
{
}

void SplashScreen::start()
{
    if (!_display.isInitialized())
    {
        return;
    }

    _startTime = millis();
    _lastFrameTime = _startTime - FRAME_TIME_MS;
    _state = State::Booting;

    drawLayout();
    drawProgress(0);
}

bool SplashScreen::update()
{
    if (_state == State::Idle)
    {
        return false;
    }

    const uint32_t now = millis();
    const uint32_t elapsed = now - _startTime;

    if (elapsed >= DURATION_MS)
    {
        _display.clear(COLOR_BLACK);
        _state = State::Idle;
        return false;
    }

    if (now - _lastFrameTime >= FRAME_TIME_MS)
    {
        _lastFrameTime = now;
        drawProgress(elapsed);
    }

    return true;
}

bool SplashScreen::isActive() const
{
    return _state != State::Idle;
}

void SplashScreen::drawLayout()
{
    _display.clear(COLOR_BLACK);

    drawPanel(0, "SMART", "CLOCK");
    drawPanel(1, "PRECISION", "TIME");
    drawPanel(2, "SENSORS", "READY");
    drawPanel(3, "SYSTEM", "BOOT");

    _display.drawFastHLine(0, 54, WIDTH, COLOR_ACCENT_DIM);
    _display.drawFastHLine(0, 246, WIDTH, COLOR_ACCENT_DIM);

    ST7789_172x320& statusDisplay = _display.get(1);
    statusDisplay.setFont(&FreeMono9pt7b);
    statusDisplay.setTextSize(1);
    statusDisplay.setTextColor(COLOR_MUTED);
    statusDisplay.setCursor(32, 262);
    statusDisplay.print("INITIALIZING");
}

void SplashScreen::drawPanel(
    uint8_t index,
    const char* title,
    const char* value
)
{
    ST7789_172x320& tft = _display.get(index);

    tft.fillRect(0, 0, 172, 4, COLOR_ACCENT);
    tft.drawFastVLine(8, 74, 140, COLOR_ACCENT_DIM);

    tft.setFont(&FreeMono9pt7b);
    tft.setTextSize(1);
    tft.setTextColor(COLOR_MUTED);
    tft.setCursor(20, 34);
    tft.print(title);

    tft.setFont(&FreeMonoBold18pt7b);
    tft.setTextSize(2);
    tft.setTextColor(COLOR_TEXT);
    tft.setCursor(20, 158);
    tft.print(value);

    tft.setTextSize(1);
    tft.drawFastHLine(20, 222, 132, COLOR_ACCENT_DIM);
}

void SplashScreen::drawProgress(uint32_t elapsed)
{
    uint32_t completed = (elapsed * PROGRESS_WIDTH) / DURATION_MS;

    if (completed > PROGRESS_WIDTH)
    {
        completed = PROGRESS_WIDTH;
    }

    _display.fillRect(
        PROGRESS_X,
        PROGRESS_Y,
        PROGRESS_WIDTH,
        PROGRESS_HEIGHT,
        COLOR_ACCENT_DIM
    );

    if (completed > 0)
    {
        _display.fillRect(
            PROGRESS_X,
            PROGRESS_Y,
            static_cast<int16_t>(completed),
            PROGRESS_HEIGHT,
            COLOR_ACCENT
        );
    }
}
