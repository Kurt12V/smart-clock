
#include <Arduino.h>

#include "Pins.h"
#include "Config.h"

#include "./managers/SettingsManager.h"
#include "./managers/LightingManager.h"
#include "./managers/I2SManager.h"
#include "./managers/SoundManager.h"

#include "./hardware/MusicGenerator.h"
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
// AUDIO HARDWARE
// ============================================================

I2SManager g_i2s;

// ============================================================
// SD CARD
// ============================================================

SDManager g_sd;

// ============================================================
// SOUND MANAGER
// ============================================================

SoundManager g_sound(
    g_sd,
    g_settings,
    g_i2s
);

// ============================================================
// MUSIC GENERATOR
// ============================================================

MusicGenerator g_musicGenerator;

// ============================================================
// ALARM EFFECTS
// ============================================================

AlarmEffects g_alarmEffects(
    g_lighting,
    g_sound,
    g_musicGenerator
);

// ============================================================
// TEST CONFIGURATION
// ============================================================

// Полная продолжительность теста: 27 минут.
// Музыка должна стартовать на 21-й минуте.
// Настройки громкости задаются в SunriseLightEffect.

static constexpr uint32_t TEST_DURATION_MS =
    27UL * 60UL * 1000UL;

// Интервал отладочного вывода.

static constexpr uint32_t DEBUG_INTERVAL_MS =
    1000UL;

// ============================================================
// TEST STATE
// ============================================================

uint32_t g_testStartMs = 0;
uint32_t g_lastDebugMs = 0;

bool g_testStarted = false;
bool g_systemReady = false;

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
            "[TEST][ERROR] LightingManager initialization failed"
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
            "[TEST][ERROR] I2SManager initialization failed"
        );

        return;
    }

    Serial0.println(
        "[TEST] I2SManager READY"
    );

    // --------------------------------------------------------
    // SD MANAGER
    // --------------------------------------------------------

    Serial0.println(
        "[TEST] Starting SDManager..."
    );

    if (!g_sd.begin(PIN_SD_CS))
    {
        Serial0.println(
            "[TEST][ERROR] SDManager initialization failed"
        );

        return;
    }

    Serial0.println(
        "[TEST] SDManager READY"
    );

    // --------------------------------------------------------
    // SOUND MANAGER
    // --------------------------------------------------------

    Serial0.println(
        "[TEST] Starting SoundManager..."
    );

    if (!g_sound.begin())
    {
        Serial0.println(
            "[TEST][ERROR] SoundManager initialization failed"
        );

        return;
    }

    Serial0.println(
        "[TEST] SoundManager READY"
    );

    // --------------------------------------------------------
    // MUSIC GENERATOR
    // --------------------------------------------------------

    Serial0.println(
        "[TEST] MusicGenerator READY"
    );

    // MusicGenerator is initialized when sunrise audio starts.
    // Do not start it here, otherwise music may begin early.

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
    // TEST INFORMATION
    // --------------------------------------------------------

    delay(1000);

    Serial0.println();
    Serial0.println(
        "============================================"
    );
    Serial0.println(
        "[TEST] SUNRISE TEST CONFIGURATION"
    );
    Serial0.println(
        "============================================"
    );

    Serial0.println(
        "[TEST] Audio source: MusicGenerator"
    );

    Serial0.println(
        "[TEST] Music starts: 21:00"
    );

    Serial0.println(
        "[TEST] Volume phases: SunriseLightEffect"
    );

    Serial0.println(
        "[TEST] Test duration: 27 minutes"
    );

    Serial0.println(
        "============================================"
    );

    // --------------------------------------------------------
    // START SUNRISE
    // --------------------------------------------------------

    g_testStartMs = millis();
    g_lastDebugMs = g_testStartMs;

    g_systemReady = true;
    g_testStarted = true;

    g_alarmEffects.startSunrise();

    Serial0.println();
    Serial0.println(
        "[TEST] Sunrise started"
    );
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // WAIT IF SYSTEM IS NOT READY
    // --------------------------------------------------------

    if (!g_systemReady || !g_testStarted)
    {
        delay(10);
        return;
    }

    const uint32_t now = millis();

    const uint32_t elapsedMs =
        now - g_testStartMs;

    // --------------------------------------------------------
    // UPDATE SOUND MANAGER
    // --------------------------------------------------------
    // IMPORTANT:
    // SoundManager::update() must be called exactly once
    // per loop iteration. It services both WAV playback
    // and generated PCM audio.

    g_sound.update();

    // --------------------------------------------------------
    // UPDATE ALARM EFFECTS
    // --------------------------------------------------------

    g_alarmEffects.update(
        elapsedMs
    );

    // --------------------------------------------------------
    // UPDATE LIGHTING
    // --------------------------------------------------------

    g_lighting.update();

    // --------------------------------------------------------
    // AUTOMATIC TEST STOP
    // --------------------------------------------------------

    if (elapsedMs >= TEST_DURATION_MS)
    {
        Serial0.println();
        Serial0.println(
            "[TEST] Test duration complete"
        );

        Serial0.println(
            "[TEST] Stopping sunrise..."
        );

        g_alarmEffects.stop();

        g_testStarted = false;

        Serial0.println(
            "[TEST] Sunrise stopped"
        );

        Serial0.printf(
            "[TEST] Sound active: %d\n",
            g_sound.isActive() ? 1 : 0
        );

        Serial0.printf(
            "[TEST] Lighting override active: %d\n",
            g_lighting.isAlarmOverrideActive() ? 1 : 0
        );

        Serial0.println(
            "[TEST] Test finished"
        );

        return;
    }

    // --------------------------------------------------------
    // DEBUG OUTPUT
    // --------------------------------------------------------

    if (now - g_lastDebugMs >= DEBUG_INTERVAL_MS)
    {
        g_lastDebugMs = now;

        const uint32_t totalSeconds =
            elapsedMs / 1000UL;

        const uint32_t minutes =
            totalSeconds / 60UL;

        const uint32_t seconds =
            totalSeconds % 60UL;

        Serial0.printf(
            "[TEST] Sunrise %02lu:%02lu | "
            "Alarm=%d | "
            "SoundActive=%d | "
            "LightingOverride=%d\n",

            static_cast<unsigned long>(minutes),

            static_cast<unsigned long>(seconds),

            g_alarmEffects.isActive() ? 1 : 0,

            g_sound.isActive() ? 1 : 0,

            g_lighting.isAlarmOverrideActive() ? 1 : 0
        );
    }

    // --------------------------------------------------------
    // KEEP LOOP RESPONSIVE
    // --------------------------------------------------------

    delay(1);
}
