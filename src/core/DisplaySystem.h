#pragma once

#include "ui/Display.h"
#include "./managers/DisplayManager.h"
#include "ui/screens/SplashScreen.h"

class DisplaySystem
{
public:
    DisplaySystem();

    void begin();

    void startSplash();
    bool updateSplash();

private:
    Display _display;
    DisplayManager _displayManager;
    SplashScreen _splashScreen;

    bool _initialized;
};