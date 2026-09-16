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
#include "./managers/EncoderManager.h"

EncoderManager encoder(
    PIN_ENCODER_CLK,
    PIN_ENCODER_DT,
    PIN_ENCODER_SW
);
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
Serial0.println("[MAIN] Initializing EncoderManager...");

if (!encoder.begin())
{
    Serial0.println("[MAIN] Encoder ERROR");
}
else
{
    Serial0.println("[MAIN] Encoder OK");
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
encoder.update();

EncoderManager::Event event;

while (
    (event = encoder.getEvent())
    != EncoderManager::Event::NONE
)
{
    switch (event)
    {
        case EncoderManager::Event::ROTATE_CW:
            Serial0.println("[ENCODER] CW");
            break;

        case EncoderManager::Event::ROTATE_CCW:
            Serial0.println("[ENCODER] CCW");
            break;

        case EncoderManager::Event::PRESS:
            Serial0.println("[ENCODER] PRESS");
            break;

        case EncoderManager::Event::RELEASE:
            Serial0.println("[ENCODER] RELEASE");
            break;

        case EncoderManager::Event::LONG_PRESS:
            Serial0.println("[ENCODER] LONG PRESS");
            break;

        case EncoderManager::Event::DOUBLE_PRESS:
            Serial0.println("[ENCODER] DOUBLE PRESS");
            break;

        default:
            break;
    }
}
    delay(1);
}
