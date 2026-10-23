#include <Arduino.h>

#include "SettingsManager.h"
#include "SPIManager.h"
#include "SensorManager.h"
#include "ClockSystem.h"
#include "DisplaySystem.h"


// ============================================================
// MODULES
// ============================================================

SettingsManager settings;

SPIManager spiManager;

SensorManager sensorManager;

ClockSystem clockSystem(
    settings
);

DisplaySystem displaySystem(
    settings,
    clockSystem,
    sensorManager,
    spiManager
);


// ============================================================
// STATE
// ============================================================

bool initialized = false;


// ============================================================
// SETUP
// ============================================================

void setup()
{
    // SETTINGS
    if (!settings.begin())
        return;

    // SPI
    if (!spiManager.begin())
        return;

    // CLOCK
    if (!clockSystem.begin())
        return;

    // SENSORS
    if (!sensorManager.begin())
        return;

    // DISPLAY
    if (!displaySystem.begin())
        return;

    initialized = true;
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    if (!initialized)
        return;

    settings.update();

    clockSystem.update();

    sensorManager.update();

    displaySystem.update();
}