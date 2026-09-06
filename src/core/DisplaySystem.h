#pragma once

#include "./ui/Display.h"
#include "./managers/DisplayManager.h"
#include "./ui/screens/SplashScreen.h"
#include "./ui/screens/ClockScreen.h"

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
