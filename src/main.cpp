#include <Arduino.h>

// ============================================================
// CORE
// ============================================================

#include "Config.h"
#include "Pins.h"
#include "Constants.h"
#include "Version.h"

// ============================================================
// SYSTEMS
// ============================================================

#include "Settings.h"

#include "./core/ClockSystem.h"
#include "./managers/SensorsManager.h"
#include "./core/DisplaySystem.h"
#include "./managers/InputManager.h"

InputManager inputManager;


Settings::Clock clockSettings;

ClockSystem clockSystem(
    clockSettings
);

// ------------------------------------------------------------
// Sensors
// ------------------------------------------------------------

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
    // ========================================================
    // SERIAL
    // ========================================================

    Serial0.begin(115200);

    delay(1000);

    Serial0.println();
    Serial0.println(
        "========================================"
    );
    Serial0.println(
        "        ESP32-S3 SMART CLOCK"
    );
    Serial0.println(
        "========================================"
    );

    // ========================================================
    // CLOCK
    // ========================================================

    Serial0.println(
        "[MAIN] Initializing ClockSystem..."
    );

    if (!clockSystem.begin())
    {
        Serial0.println(
            "[MAIN] ClockSystem ERROR"
        );
    }
    else
    {
        Serial0.println(
            "[MAIN] ClockSystem OK"
        );
    }

    // ========================================================
    // SENSORS
    // ========================================================

    Serial0.println(
        "[MAIN] Initializing SensorManager..."
    );

    if (!sensorManager.begin())
    {
        Serial0.println(
            "[MAIN] SensorManager ERROR"
        );
    }
    else
    {
        Serial0.println(
            "[MAIN] SensorManager OK"
        );
    }

    // ========================================================
    // DISPLAY
    // ========================================================

    Serial0.println(
        "[MAIN] Initializing DisplaySystem..."
    );

 

    if (!displaySystem.begin())
    {
        Serial0.println(
            "[MAIN] DisplaySystem ERROR"
        );



        while (true)
        {
            delay(1000);

            Serial0.println(
                "[MAIN] DisplaySystem is not available"
            );
        }
    }

    Serial0.println(
        "[MAIN] DisplaySystem OK"
    );
Serial0.println("[MAIN] Initializing InputManager...");

if (!inputManager.begin())
{
    Serial0.println("[MAIN] InputManager ERROR");
}
else
{
    Serial0.println("[MAIN] InputManager OK");
}
    // ========================================================
    // SYSTEM READY
    // ========================================================

    Serial0.println();
    Serial0.println(
        "========================================"
    );
    Serial0.println(
        "          SYSTEM READY"
    );
    Serial0.println(
        "========================================"
    );
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // ========================================================
    // CLOCK
    // ========================================================

    clockSystem.update();

    // ========================================================
    // SENSORS
    // ========================================================

    sensorManager.update();


    displaySystem.update();

    // ========================================================
    // MINIMAL DELAY
    // ========================================================
inputManager.update();

Constants::Event event;

while (
    (event = inputManager.getEvent())
    != Constants::Event::NONE
)
{
    switch (event)
    {
        case Constants::Event::ROTATE_CW:
            Serial0.println("[INPUT] ROTATE CW");
            break;

        case Constants::Event::ROTATE_CCW:
            Serial0.println("[INPUT] ROTATE CCW");
            break;

        case Constants::Event::PRESS:
            Serial0.println("[INPUT] PRESS");
            break;

        case Constants::Event::RELEASE:
            Serial0.println("[INPUT] RELEASE");
            break;

        case Constants::Event::LONG_PRESS:
            Serial0.println("[INPUT] LONG PRESS");
            break;

        case Constants::Event::DOUBLE_PRESS:
            Serial0.println("[INPUT] DOUBLE PRESS");
            break;

        case Constants::Event::NONE:
        default:
            break;
    }
}


    delay(1);
}
