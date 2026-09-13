
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
    Serial.println("==============================");
    Serial.println("       SMART CLOCK");
    Serial.println("==============================");


    // --------------------------------------------------------
    // CLOCK
    // --------------------------------------------------------

    if (!clockSystem.begin())
    {
        Serial.println(
            "[ERROR] ClockSystem begin failed"
        );
    }
    else
    {
        Serial.println(
            "[OK] ClockSystem"
        );
    }


    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    if (!sensorManager.begin())
    {
        Serial.println(
            "[ERROR] SensorManager begin failed"
        );
    }
    else
    {
        Serial.println(
            "[OK] SensorManager"
        );
    }


    // --------------------------------------------------------
    // DISPLAY
    // --------------------------------------------------------

    if (!displaySystem.begin())
    {
        Serial.println(
            "[ERROR] DisplaySystem begin failed"
        );

        return;
    }

    Serial.println(
        "[OK] DisplaySystem"
    );

    Serial.println(
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