#include "DisplaySystem.h"

#include <Arduino.h>

DisplaySystem::DisplaySystem()
    : _display(),
      _displayManager(_display),
      _splashScreen(_displayManager),
      _initialized(false)
{
}

void DisplaySystem::begin()
{
    if (_initialized)
    {
        return;
    }

    if (!_displayManager.begin())
    {
        while (true)
        {
            delay(1000);
        }
    }

    _initialized = true;
}

void DisplaySystem::startSplash()
{
    if (!_initialized)
    {
        return;
    }

    _splashScreen.start();
}

bool DisplaySystem::updateSplash()
{
    if (!_initialized)
    {
        return false;
    }

    return _splashScreen.update();
}