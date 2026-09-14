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

// ============================================================
// GLOBAL SYSTEM OBJECTS
// ============================================================

// ------------------------------------------------------------
// Clock
// ------------------------------------------------------------

Settings::Clock clockSettings;

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
    // ========================================================
    // SERIAL
    // ========================================================

    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println(
        "========================================"
    );
    Serial.println(
        "        ESP32-S3 SMART CLOCK"
    );
    Serial.println(
        "========================================"
    );

    // ========================================================
    // CLOCK
    // ========================================================

    Serial.println(
        "[MAIN] Initializing ClockSystem..."
    );

    if (!clockSystem.begin())
    {
        Serial.println(
            "[MAIN] ClockSystem ERROR"
        );
    }
    else
    {
        Serial.println(
            "[MAIN] ClockSystem OK"
        );
    }

    // ========================================================
    // SENSORS
    // ========================================================

    Serial.println(
        "[MAIN] Initializing SensorManager..."
    );

    if (!sensorManager.begin())
    {
        Serial.println(
            "[MAIN] SensorManager ERROR"
        );
    }
    else
    {
        Serial.println(
            "[MAIN] SensorManager OK"
        );
    }

    // ========================================================
    // DISPLAY
    // ========================================================

    Serial.println(
        "[MAIN] Initializing DisplaySystem..."
    );

    /*
     * Здесь запускается вся цепочка:
     *
     * DisplaySystem
     *      |
     *      +-- SPIManager.begin()
     *      |
     *      +-- LVGLManager.begin()
     *      |      |
     *      |      +-- Backlight GPIO1
     *      |      +-- LVGL
     *      |      +-- TFT1
     *      |      +-- TFT2
     *      |      +-- TFT3
     *      |      +-- TFT4
     *      |
     *      +-- ScreenManager.begin()
     */

    if (!displaySystem.begin())
    {
        Serial.println(
            "[MAIN] DisplaySystem ERROR"
        );

        /*
         * Дисплей является основной частью интерфейса.
         * Если он не запустился, остаёмся здесь,
         * чтобы ошибка была явно видна в Serial.
         */

        while (true)
        {
            delay(1000);

            Serial.println(
                "[MAIN] DisplaySystem is not available"
            );
        }
    }

    Serial.println(
        "[MAIN] DisplaySystem OK"
    );

    // ========================================================
    // SYSTEM READY
    // ========================================================

    Serial.println();
    Serial.println(
        "========================================"
    );
    Serial.println(
        "          SYSTEM READY"
    );
    Serial.println(
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

    // ========================================================
    // DISPLAY
    // ========================================================

    /*
     * DisplaySystem.update() выполняет:
     *
     * ScreenManager.update()
     * LVGLManager.update()
     *
     * ScreenManager обновляет:
     * - время
     * - температуру
     * - влажность
     * - освещённость
     * - дату
     *
     * LVGLManager:
     * - обрабатывает LVGL
     * - отправляет изменённые области на TFT
     */

    displaySystem.update();

    // ========================================================
    // MINIMAL DELAY
    // ========================================================

    delay(1);
}
