#pragma once

#include <Arduino.h>

#include "./managers/DisplayManager.h"


class SplashScreen
{
public:

    explicit SplashScreen(
        DisplayManager& display
    );


    void show();


private:

    DisplayManager& _display;


    // --------------------------------------------------------
    // Элементы заставки
    // --------------------------------------------------------

    void drawBackground();

    void drawGrid();

    void drawFrame();

    void drawTitle();

    void drawSubtitle();

    void drawLoadingDots(
        uint8_t activeDot
    );

    void drawProgress(
        uint8_t progress
    );
};