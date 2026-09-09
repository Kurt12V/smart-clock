#pragma once

#include <Arduino.h>

#include "./managers/DisplayManager.h"

class SplashScreen
{
public:
    explicit SplashScreen(
        DisplayManager& display
    );

    void start();
    bool update();
    bool isActive() const;

private:
    static constexpr uint8_t DISPLAY_COUNT = 4;
    static constexpr uint8_t COLUMNS = 28;

    enum class State : uint8_t
    {
        Idle,
        Running,
        Finished
    };

    DisplayManager& _display;

    State _state;

    uint32_t _startTime;
    uint32_t _lastFrameTime;

    int16_t _head[DISPLAY_COUNT][COLUMNS];
    uint8_t _length[DISPLAY_COUNT][COLUMNS];
    uint8_t _speed[DISPLAY_COUNT][COLUMNS];

    void initializeRain();
    void drawFrame();
    void clear();
};