#include <Arduino.h>

#include "Pins.h"
#include "Config.h"

#include "./core/ClockSystem.h"
#include "./managers/SensorsManager.h"

#include "./core/DisplaySystem.h"


// ============================================================
// OBJECTS
// ============================================================

// ВАЖНО:
// Здесь используется твой существующий Settings::Clock.
// Если у него есть конструктор по умолчанию,
// это устранит старую ошибку {Plus3}.


Settings::Clock clockSettings{};

ClockSystem clockSystem(
    clockSettings
);

// ------------------------------------------------------------
// Sensors
// ------------------------------------------------------------

SensorManager sensorManager;

// ------------------------------------------------------------
// Display
//
// DisplaySystem внутри содержит:
//
// SPIManager
// LVGLManager
// ScreenManager
//
// Поэтому main.cpp не управляет SPI/TFT напрямую.
// ------------------------------------------------------------

DisplaySystem displaySystem(
    clockSystem,
    sensorManager
);

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);

    delay(1000);

    Serial0.println();
    Serial0.println("==============================");
    Serial0.println("       SMART CLOCK");
    Serial0.println("==============================");


    // ========================================================
    // CLOCK
    // --------------------------------------------------------

    if (!clockSystem.begin())
    {
        Serial0.println(
            "[ERROR] ClockSystem begin failed"
        );
    }
    else
    {
        Serial0.println(
            "[OK] ClockSystem"
        );
    }


    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    if (!sensorManager.begin())
    {
        Serial0.println(
            "[ERROR] SensorManager begin failed"
        );
    }
    else
    {
        Serial0.println(
            "[OK] SensorManager"
        );
    }


    // --------------------------------------------------------
    // DISPLAY
    // --------------------------------------------------------

    if (!displaySystem.begin())
    {
        Serial0.println(
            "[ERROR] DisplaySystem begin failed"
        );

        return;
    }

    Serial0.println(
        "[OK] DisplaySystem"
    );

    Serial0.println(
        "System started."
    );
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // CLOCK
    // --------------------------------------------------------

    clockSystem.update();


    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    sensorManager.update();


    // --------------------------------------------------------
    // DISPLAY
    // --------------------------------------------------------

    displaySystem.update();


    // --------------------------------------------------------
    // CPU
    // --------------------------------------------------------

    delay(2);
}
