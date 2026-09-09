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
    static constexpr uint8_t DISPLAY_COUNT = 4;
    static constexpr uint8_t COLUMNS = 28;

    DisplayManager& _display;
    State _state;
    uint32_t _startTime;
    uint32_t _lastFrameTime;

    void initializeRain();
    void drawFrame();
    void clear();
}
