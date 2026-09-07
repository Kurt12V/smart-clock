#pragma once

#include <Arduino.h>
#include "managers/DisplayManager.h"

class SplashScreen
{
public:
    explicit SplashScreen(DisplayManager& display);

    void start();
    bool update();
    bool isActive() const;

private:
    static constexpr int16_t WIDTH = 688;
    static constexpr uint16_t FRAME_TIME_MS = 35;
    static constexpr uint32_t DURATION_MS = 2400;

    enum class State : uint8_t
    {
        Idle,
        Booting
    };

    DisplayManager& _display;
    State _state;
    uint32_t _startTime;
    uint32_t _lastFrameTime;

    void drawLayout();
    void drawPanel(uint8_t index, const char* title, const char* value);
    void drawProgress(uint32_t elapsed);
};
