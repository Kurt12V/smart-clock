#include <Arduino.h>

#include "Config.h"
#include "Constants.h"
#include "Pins.h"
#include "Settings.h"

#include "core/ClockSystem.h"
#include "managers/SensorsManager.h"
#include "core/DisplaySystem.h"


// ============================================================
// SETTINGS
// ============================================================

Settings::Clock clockSettings;


// ============================================================
// SYSTEMS
// ============================================================

ClockSystem clockSystem(
    clockSettings
);

SensorManager sensorManager;

DisplaySystem displaySystem(
    clockSystem,
    sensorManager
);


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println("          SMART CLOCK");
    Serial.println("================================");


    // ========================================================
    // CLOCK
    // ========================================================

    if (!clockSystem.begin())
    {
        Serial.println("ClockSystem: ERROR");
    }
    else
    {
        Serial.println("ClockSystem: OK");
    }


    // ========================================================
    // SENSORS
    // ========================================================

    if (!sensorManager.begin())
    {
        Serial.println("SensorManager: ERROR");
    }
    else
    {
        Serial.println("SensorManager: OK");
    }


    // ========================================================
    // DISPLAY
    // ========================================================

    if (!displaySystem.begin())
    {
        Serial.println("DisplaySystem: ERROR");

        while (true)
        {
            delay(1000);
        }
    }

    Serial.println("DisplaySystem: OK");

    Serial.println("================================");
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    // ========================================================
    // SYSTEMS
    // ========================================================

    clockSystem.update();
    sensorManager.update();

    // DisplaySystem получает уже актуальные данные
    // и самостоятельно распределяет их по TFT.
    displaySystem.update();


    delay(5);
}
