#pragma once

#include <Arduino.h>

#include "./managers/DisplayManager.h"

class SplashScreen
{
public:
    explicit SplashScreen(DisplayManager& display);

    void show();

private:
    static constexpr uint8_t DISPLAY_COUNT = 4;
    static constexpr uint8_t COLUMNS = 28;

    DisplayManager& _display;

    int16_t _head[DISPLAY_COUNT][COLUMNS];
    uint8_t _length[DISPLAY_COUNT][COLUMNS];
    uint8_t _speed[DISPLAY_COUNT][COLUMNS];

    void initializeRain();
    void drawFrame();
    void clear();
}
