#pragma once

#include <Arduino.h>
#include "DisplayManager.h"

class SplashScreen
{
public:
    explicit SplashScreen(DisplayManager& display);

    void show();

private:
    DisplayManager& _display;

    static constexpr int16_t WIDTH  = 688;
    static constexpr int16_t HEIGHT = 320;

    static constexpr uint8_t CELL_WIDTH  = 6;
    static constexpr uint8_t CELL_HEIGHT = 8;

    static constexpr uint16_t COLUMNS = WIDTH / CELL_WIDTH;
    static constexpr uint16_t ROWS    = HEIGHT / CELL_HEIGHT;

    static constexpr uint16_t FRAME_TIME = 35;
    static constexpr uint32_t DURATION   = 3000;

    struct Column
    {
        int16_t head;
        uint8_t length;
        uint8_t speed;
        uint8_t counter;
        bool active;
    };

    Column _columns[COLUMNS];

    uint32_t _startTime;

    void initializeColumns();

    void drawFrame();

    void drawCharacter(
        uint16_t column,
        int16_t row,
        char character,
        uint16_t color
    );

    void clearCharacter(
        uint16_t column,
        int16_t row
    );

    char randomCharacter();

    uint16_t greenColor(uint8_t brightness);

    void finalFlash();
};