
#include <Arduino.h>

#include "Pins.h"
#include "Config.h"
#include "./managers/SettingsManager.h"

#include "./managers/LightingManager.h"
#include "./managers/I2SManager.h"
#include "./hardware/AlarmEffects.h"

// ============================================================
// SETTINGS
// ============================================================

SettingsManager g_settings;

// ============================================================
// LIGHTING
// ============================================================

LightingManager g_lighting(
    g_settings
);

// ============================================================
// AUDIO
// ============================================================

I2SManager g_i2s;

// ============================================================
// ALARM EFFECTS
// ============================================================

AlarmEffects g_alarmEffects(
    g_lighting,
    g_i2s
);

// ============================================================
// TEST STATE
// ============================================================

static constexpr uint32_t TEST_DURATION_MS =
    25UL * 60UL * 1000UL;

uint32_t g_testStartMs = 0;
uint32_t g_lastDebugMs = 0;

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
    // LIGHTING MANAGER
    // --------------------------------------------------------

    Serial0.println(
        "[TEST] Starting LightingManager..."
    );

    if (!g_lighting.begin())
    {
        Serial0.println(
            "[TEST][ERROR] LightingManager failed"
        );

        return;
    }

    Serial0.println(
        "[TEST] LightingManager READY"
    );

    // --------------------------------------------------------
    // I2S MANAGER
    // --------------------------------------------------------

    Serial0.println(
        "[TEST] Starting I2SManager..."
    );

    if (!g_i2s.begin())
    {
        Serial0.println(
            "[TEST][ERROR] I2SManager failed"
        );

        return;
    }

    Serial0.println(
        "[TEST] I2SManager READY"
    );

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
        "[TEST] Volume reaches 30% at 25 minutes"
    );

    Serial0.println(
        "[TEST] Volume reaches 100% at 26 minutes"
    );

    Serial0.println();

    g_testStartMs = millis();
    g_lastDebugMs = g_testStartMs;
    g_testStarted = true;

    g_alarmEffects.startSunrise();
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    if (!g_testStarted)
    {
        delay(10);
        return;
    }

    const uint32_t now = millis();

    const uint32_t elapsedMs =
        now - g_testStartMs;

    // --------------------------------------------------------
    // UPDATE ALARM
    // --------------------------------------------------------

    g_alarmEffects.update(
        elapsedMs
    );

    // --------------------------------------------------------
    // UPDATE LIGHTING
    //
    // LightingManager пропускает обычные эффекты и применение
    // пользовательских настроек, пока активен alarm override.
    // --------------------------------------------------------

    g_lighting.update();

    // --------------------------------------------------------
    // AUTOMATIC TEST STOP
    // --------------------------------------------------------

    if (elapsedMs >= TEST_DURATION_MS)
    {
        Serial0.println();
        Serial0.println(
            "[TEST] Duration complete. Stopping sunrise..."
        );

        g_alarmEffects.stop();

        g_testStarted = false;

        Serial0.println(
            "[TEST] Sunrise stopped. Lighting restored."
        );

        return;
    }

    // --------------------------------------------------------
    // DEBUG EVERY SECOND
    // --------------------------------------------------------

    if (now - g_lastDebugMs >= 1000)
    {
        g_lastDebugMs = now;

        const uint32_t seconds =
            elapsedMs / 1000;

        const uint32_t minutes =
            seconds / 60;

        const uint32_t remainingSeconds =
            seconds % 60;

        Serial0.printf(
            "[TEST] Sunrise %02lu:%02lu | "
            "Active=%d | "
            "Audio=%d | "
            "LightingOverride=%d\n",

            static_cast<unsigned long>(minutes),

            static_cast<unsigned long>(remainingSeconds),

            g_alarmEffects.isActive() ? 1 : 0,

            g_i2s.isSpeakerInitialized() ? 1 : 0,

            g_lighting.isAlarmOverrideActive() ? 1 : 0
        );
    }

    delay(1);
}