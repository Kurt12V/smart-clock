#pragma once

#include "Display.h"
#include "DisplayManager.h"
#include "SplashScreen.h"
#include "ClockScreen.h"

class DisplaySystem
{
public:
    DisplaySystem();

    void begin();

    void showSplash();
    void showClock();

    ClockScreen& clock();

private:
    Display _display;
    DisplayManager _displayManager;
    SplashScreen _splashScreen;
    ClockScreen _clockScreen;

    bool _initialized;
};
