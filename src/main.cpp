#include <Arduino.h>

#include "Pins.h"

#include "./managers/LedMatrixManager.h"
#include "./managers/CobLedManager.h"
#include "./managers/I2SManager.h"

#include "./hardware/AlarmEffects.h"
#include "Config.h"

// ============================================================
// HARDWARE
// ============================================================

// ------------------------------------------------------------
// MATRIX
// ------------------------------------------------------------

LedMatrixManager g_matrix(
    PIN_MATRIX
);

// ------------------------------------------------------------
// COB LEDS
// ------------------------------------------------------------
//
// Здесь используются твои реальные CobLed.
//
// Имена пинов нужно заменить на те,
// которые используются в твоём Pins.h.
// ------------------------------------------------------------

CobLed g_cob1(
    PIN_COB1,
    1,
    Config::COB_PWM_FREQUENCY,
    Config::COB_PWM_RESOLUTION
);

CobLed g_cob2(
    PIN_COB2,
    2,
    Config::COB_PWM_FREQUENCY,
    Config::COB_PWM_RESOLUTION
);

CobLed g_cob3(
    PIN_COB3,
    3,
    Config::COB_PWM_FREQUENCY,
    Config::COB_PWM_RESOLUTION
);

CobLed g_cob4(
    PIN_COB4,
    4,
    Config::COB_PWM_FREQUENCY,
    Config::COB_PWM_RESOLUTION
);

// ============================================================
// COB MANAGER
// ============================================================

CobLedManager g_cobManager(
    g_cob1,
    g_cob2,
    g_cob3,
    g_cob4
);

// ------------------------------------------------------------
// I2S
// ------------------------------------------------------------

I2SManager g_i2s;

// ------------------------------------------------------------
// ALARM EFFECTS
// ------------------------------------------------------------

AlarmEffects g_alarmEffects(
    g_matrix,
    g_cobManager,
    g_i2s
);

// ============================================================
// TEST STATE
// ============================================================

uint32_t g_testStartMs = 0;

bool g_testStarted = false;

// ============================================================
// SETUP
// ============================================================

void setup()
{
    // --------------------------------------------------------
    // SERIAL
    // --------------------------------------------------------

    Serial0.begin(115200);

    delay(1000);

    Serial0.println();
    Serial0.println(
        "============================================"
    );

    Serial0.println(
        "SMART CLOCK - ALARM EFFECT TEST"
    );

    Serial0.println(
        "============================================"
    );

    // --------------------------------------------------------
    // MATRIX
    // --------------------------------------------------------

    Serial0.println(
        "[TEST] Starting LedMatrixManager..."
    );

    g_matrix.begin();

    // --------------------------------------------------------
    // COB
    // --------------------------------------------------------

    Serial0.println(
        "[TEST] Starting CobLedManager..."
    );

    g_cobManager.begin();

    // --------------------------------------------------------
    // I2S
    // --------------------------------------------------------

    Serial0.println(
        "[TEST] Starting I2SManager..."
    );

    if (!g_i2s.begin())
    {
        Serial0.println(
            "[TEST][ERROR] I2SManager failed"
        );
    }
    else
    {
        Serial0.println(
            "[TEST] I2SManager READY"
        );
    }

    // --------------------------------------------------------
    // ALARM EFFECTS
    // --------------------------------------------------------

    Serial0.println(
        "[TEST] Starting AlarmEffects..."
    );

    g_alarmEffects.begin();

    Serial0.println(
        "[TEST] AlarmEffects READY"
    );

    // --------------------------------------------------------
    // START TEST
    // --------------------------------------------------------

    delay(1000);

    Serial0.println();
    Serial0.println(
        "[TEST] Starting SUNRISE"
    );

    Serial0.println(
        "[TEST] Duration: 25 minutes"
    );

    Serial0.println(
        "[TEST] Music starts: 21 minutes"
    );

    Serial0.println(
        "[TEST] Peak: 25 minutes"
    );

    Serial0.println(
        "[TEST] Auxiliary LEDs: 40 Hz after peak"
    );

    Serial0.println();

    g_alarmEffects.startSunrise();

    g_testStartMs =
        millis();

    g_testStarted = true;
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    if (!g_testStarted)
        return;

    // --------------------------------------------------------
    // ELAPSED TIME
    // --------------------------------------------------------

    const uint32_t elapsedMs =
        millis() -
        g_testStartMs;

    // --------------------------------------------------------
    // ALARM EFFECTS
    // --------------------------------------------------------

    g_alarmEffects.update(
        elapsedMs
    );

    // --------------------------------------------------------
    // NORMAL MANAGERS
    // --------------------------------------------------------
    //
    // Важно:
    //
    // matrix.update() и cob.update()
    // продолжают работать.
    //
    // AlarmEffects временно отключает
    // их обычные effects.
    // --------------------------------------------------------

    g_matrix.update();

    g_cobManager.update();

    // --------------------------------------------------------
    // DEBUG EVERY SECOND
    // --------------------------------------------------------

    static uint32_t lastDebug = 0;

    if (
        elapsedMs -
        lastDebug >=
        1000
    )
    {
        lastDebug =
            elapsedMs;

        const uint32_t seconds =
            elapsedMs / 1000;

        const uint32_t minutes =
            seconds / 60;

        const uint32_t remainingSeconds =
            seconds % 60;

        Serial0.printf(
            "[TEST] Sunrise %02lu:%02lu | "
            "Active=%d | "
            "Audio=%d\n",

            static_cast<unsigned long>(
                minutes
            ),

            static_cast<unsigned long>(
                remainingSeconds
            ),

            g_alarmEffects.isActive()
                ? 1
                : 0,

            g_i2s.isSpeakerInitialized()
                ? 1
                : 0
        );
    }

    // --------------------------------------------------------
    // SMALL DELAY
    // --------------------------------------------------------

    delay(1);
}