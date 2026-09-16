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

#include "./core/BluetoothSystem.h"


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
BluetoothSystem bluetoothSystem( sensorManager, clockSystem );
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

    // ========================================================
    // BLUETOOTH
    // ========================================================

    if (!bluetoothSystem.begin())
    {
        Serial.println(
            "ERROR: Bluetooth initialization failed"
        );

        return;
    }

    Serial.println(
        "Bluetooth OK"
    );
    
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

    bluetoothSystem.update();
    // --------------------------------------------------------
    // Bluetooth
    // --------------------------------------------------------


    // --------------------------------------------------------
    // Commands from phone
    // --------------------------------------------------------

    
    delay(1);
}


