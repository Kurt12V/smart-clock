#include <Arduino.h>

#include "Config.h"
#include "Pins.h"
#include "Constants.h"

#include "./managers/SettingsManager.h"
#include "./managers/SPIManager.h"
#include "./managers/I2SManager.h"
#include "./managers/SDManager.h"
#include "./managers/SoundManager.h"
#include "./managers/LightingManager.h"

#include "./hardware/MusicGenerator.h"
#include "./hardware/AlarmEffects.h"

// ============================================================
// MANAGERS
// ============================================================

SettingsManager g_settings;

SPIManager g_spiManager;

I2SManager g_i2sManager;

SDManager g_sd;

SoundManager g_sound(
    g_sd,
    g_settings,
    g_i2sManager
);

LightingManager g_lighting(
    g_settings
);

MusicGenerator g_musicGenerator;

AlarmEffects g_alarmEffects(
    g_lighting,
    g_sound,
    g_musicGenerator
);

// ============================================================
// TEST CONFIGURATION
// ============================================================

static constexpr uint32_t TEST_DURATION_MS =
    27UL * 60UL * 1000UL;

static constexpr uint32_t DEBUG_INTERVAL_MS =
    1000UL;

static constexpr uint32_t MUSIC_SAMPLE_RATE =
    44100UL;

// ============================================================
// TEST STATE
// ============================================================

static uint32_t g_testStartMs = 0;
static uint32_t g_lastDebugMs = 0;

static bool g_testStarted = false;

// ============================================================
// INITIALIZATION: SETTINGS
// ============================================================

static bool initSettings()
{
    Serial0.println("[INIT] Settings");

    if (!g_settings.begin())
    {
        Serial0.println("[ERROR] Settings initialization failed");
        return false;
    }

    Serial0.println("[INIT] Settings ready");

    return true;
}

// ============================================================
// INITIALIZATION: SPI
// ============================================================

static bool initSPI()
{
    Serial0.println("[INIT] SPI");

    if (!g_spiManager.begin())
    {
        Serial0.println("[ERROR] SPI initialization failed");
        return false;
    }

    Serial0.println("[INIT] SPI ready");

    return true;
}

// ============================================================
// INITIALIZATION: I2S
// ============================================================

static bool initI2S()
{
    Serial0.println("[INIT] I2S");

    if (!g_i2sManager.begin())
    {
        Serial0.println("[ERROR] I2S initialization failed");
        return false;
    }

    Serial0.println("[INIT] I2S ready");

    return true;
}

// ============================================================
// INITIALIZATION: SD
// ============================================================

static bool initSD()
{
    Serial0.println("[INIT] SD");

    // SPI должен быть инициализирован до SD.
    if (!g_sd.begin(PIN_SD_CS))
    {
        Serial0.println("[ERROR] SD initialization failed");
        return false;
    }

    Serial0.println("[INIT] SD ready");

    return true;
}

// ============================================================
// INITIALIZATION: SOUND
// ============================================================

static bool initSound()
{
    Serial0.println("[INIT] Sound");

    if (!g_sound.begin())
    {
        Serial0.println("[ERROR] Sound initialization failed");
        return false;
    }

    Serial0.println("[INIT] Sound ready");

    return true;
}

// ============================================================
// INITIALIZATION: LIGHTING
// ============================================================

static bool initLighting()
{
    Serial0.println("[INIT] Lighting");

    if (!g_lighting.begin())
    {
        Serial0.println("[ERROR] Lighting initialization failed");
        return false;
    }

    Serial0.println("[INIT] Lighting ready");

    return true;
}

// ============================================================
// INITIALIZATION: MUSIC GENERATOR / ALARM EFFECTS
// ============================================================

static bool initAlarmEffects()
{
    Serial0.println("[INIT] MusicGenerator");

    // MusicGenerator::begin() возвращает void.
    g_musicGenerator.begin(MUSIC_SAMPLE_RATE);

    Serial0.println("[INIT] MusicGenerator initialized");
    Serial0.println("[INIT] AlarmEffects ready");

    return true;
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);

    delay(500);

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println(" SmartClock - Sunrise Test");
    Serial0.println("========================================");

    // --------------------------------------------------------
    // 1. SETTINGS
    // --------------------------------------------------------

    if (!initSettings())
        return;

    // --------------------------------------------------------
    // 2. SPI
    // --------------------------------------------------------

    if (!initSPI())
        return;

    // --------------------------------------------------------
    // 3. I2S
    // --------------------------------------------------------

    if (!initI2S())
        return;

    // --------------------------------------------------------
    // 4. SD
    // --------------------------------------------------------

    if (!initSD())
        return;

    // --------------------------------------------------------
    // 5. SOUND
    // --------------------------------------------------------

    if (!initSound())
        return;

    // --------------------------------------------------------
    // 6. LIGHTING
    // --------------------------------------------------------

    if (!initLighting())
        return;

    // --------------------------------------------------------
    // 7. MUSIC GENERATOR / ALARM EFFECTS
    // --------------------------------------------------------

    if (!initAlarmEffects())
        return;

    // --------------------------------------------------------
    // TEST CONFIGURATION
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println("----------------------------------------");
    Serial0.println("[TEST] Configuration");
    Serial0.println("----------------------------------------");

    Serial0.println("[TEST] Duration: 27 minutes");
    Serial0.println("[TEST] Sunrise starts immediately");
    Serial0.println("[TEST] Music starts at 21:00");
    Serial0.println("[TEST] Volume phases are controlled by the effect");

    Serial0.println("----------------------------------------");
    Serial0.println();

    // --------------------------------------------------------
    // START SUNRISE
    // --------------------------------------------------------

    // AlarmEffects::startSunrise() возвращает void.
    g_alarmEffects.startSunrise();

    g_testStartMs = millis();
    g_lastDebugMs = g_testStartMs;

    g_testStarted = true;

    Serial0.println("[TEST] Sunrise started");
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
    // SOUND
    // Вызывается ровно один раз за итерацию loop().
    // --------------------------------------------------------

    g_sound.update();

    // --------------------------------------------------------
    // ALARM EFFECTS
    // --------------------------------------------------------

    g_alarmEffects.update(elapsedMs);

    // --------------------------------------------------------
    // LIGHTING
    // --------------------------------------------------------

    g_lighting.update();

    // --------------------------------------------------------
    // DEBUG OUTPUT
    // --------------------------------------------------------

    if (now - g_lastDebugMs >= DEBUG_INTERVAL_MS)
    {
        g_lastDebugMs = now;

        const uint32_t elapsedSeconds =
            elapsedMs / 1000UL;

        const uint32_t minutes =
            elapsedSeconds / 60UL;

        const uint32_t seconds =
            elapsedSeconds % 60UL;

        Serial0.printf(
            "[TEST] Time: %02lu:%02lu | Sound: %s | AlarmEffects: %s\n",
            static_cast<unsigned long>(minutes),
            static_cast<unsigned long>(seconds),
            g_sound.isActive() ? "ACTIVE" : "IDLE",
            g_alarmEffects.isActive() ? "ACTIVE" : "IDLE"
        );
    }

    // --------------------------------------------------------
    // FINISH TEST
    // --------------------------------------------------------

    if (elapsedMs >= TEST_DURATION_MS)
    {
        Serial0.println();
        Serial0.println("========================================");
        Serial0.println("[TEST] 27 minutes elapsed");
        Serial0.println("[TEST] Stopping sunrise effect");
        Serial0.println("========================================");

        g_alarmEffects.stop();

        Serial0.printf(
            "[TEST] Sound active after stop: %s\n",
            g_sound.isActive() ? "YES" : "NO"
        );

        Serial0.println("[TEST] Sunrise test finished");

        g_testStarted = false;
    }
}