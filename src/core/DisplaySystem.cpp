#include "DisplaySystem.h"
#include <Arduino.h>

DisplaySystem::DisplaySystem()
    : _display(),
      _displayManager(_display),
      _splashScreen(_displayManager),
    //   _clockScreen(_displayManager),
      _initialized(false)
{
}

void DisplaySystem::begin()
{
    if (_initialized)
        return;

    if (!_displayManager.begin())
    {
        while (true)
        {
            delay(1000);
        }
    }

    _initialized = true;
}

void DisplaySystem::showSplash()
{
    if (!_initialized)
        return;

    _splashScreen.show();
}

// void DisplaySystem::showClock()
// {
//     if (!_initialized)
//         return;

//     _clockScreen.draw();
// }

// ClockScreen& DisplaySystem::clock()
// {
//     return _clockScreen;
// }