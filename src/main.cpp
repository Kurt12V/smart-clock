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
    Serial.println("[INIT] Settings");

    if (!g_settings.begin())
    {
        Serial.println("[ERROR] Settings initialization failed");
        return false;
    }

    Serial.println("[INIT] Settings ready");

    return true;
}

// ============================================================
// INITIALIZATION: SPI
// ============================================================

static bool initSPI()
{
    Serial.println("[INIT] SPI");

    if (!g_spiManager.begin())
    {
        Serial.println("[ERROR] SPI initialization failed");
        return false;
    }

    Serial.println("[INIT] SPI ready");

    return true;
}

// ============================================================
// INITIALIZATION: I2S
// ============================================================

static bool initI2S()
{
    Serial.println("[INIT] I2S");

    if (!g_i2sManager.begin())
    {
        Serial.println("[ERROR] I2S initialization failed");
        return false;
    }

    Serial.println("[INIT] I2S ready");

    return true;
}

// ============================================================
// INITIALIZATION: SD
// ============================================================

static bool initSD()
{
    Serial.println("[INIT] SD");

    // SPI должен быть инициализирован до SD.
    if (!g_sd.begin(PIN_SD_CS))
    {
        Serial.println("[ERROR] SD initialization failed");
        return false;
    }

    Serial.println("[INIT] SD ready");

    return true;
}

// ============================================================
// INITIALIZATION: SOUND
// ============================================================

static bool initSound()
{
    Serial.println("[INIT] Sound");

    if (!g_sound.begin())
    {
        Serial.println("[ERROR] Sound initialization failed");
        return false;
    }

    Serial.println("[INIT] Sound ready");

    return true;
}

// ============================================================
// INITIALIZATION: LIGHTING
// ============================================================

static bool initLighting()
{
    Serial.println("[INIT] Lighting");

    if (!g_lighting.begin())
    {
        Serial.println("[ERROR] Lighting initialization failed");
        return false;
    }

    Serial.println("[INIT] Lighting ready");

    return true;
}

// ============================================================
// INITIALIZATION: MUSIC GENERATOR / ALARM EFFECTS
// ============================================================

static bool initAlarmEffects()
{
    Serial.println("[INIT] MusicGenerator");

    // MusicGenerator::begin() возвращает void.
    g_musicGenerator.begin(MUSIC_SAMPLE_RATE);

    Serial.println("[INIT] MusicGenerator initialized");
    Serial.println("[INIT] AlarmEffects ready");

    return true;
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(500);

    Serial.println();
    Serial.println("========================================");
    Serial.println(" SmartClock - Sunrise Test");
    Serial.println("========================================");

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

    Serial.println();
    Serial.println("----------------------------------------");
    Serial.println("[TEST] Configuration");
    Serial.println("----------------------------------------");

    Serial.println("[TEST] Duration: 27 minutes");
    Serial.println("[TEST] Sunrise starts immediately");
    Serial.println("[TEST] Music starts at 21:00");
    Serial.println("[TEST] Volume phases are controlled by the effect");

    Serial.println("----------------------------------------");
    Serial.println();

    // --------------------------------------------------------
    // START SUNRISE
    // --------------------------------------------------------

    // AlarmEffects::startSunrise() возвращает void.
    g_alarmEffects.startSunrise();

    g_testStartMs = millis();
    g_lastDebugMs = g_testStartMs;

    g_testStarted = true;

    Serial.println("[TEST] Sunrise started");
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

        Serial.printf(
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
        Serial.println();
        Serial.println("========================================");
        Serial.println("[TEST] 27 minutes elapsed");
        Serial.println("[TEST] Stopping sunrise effect");
        Serial.println("========================================");

        g_alarmEffects.stop();

        Serial.printf(
            "[TEST] Sound active after stop: %s\n",
            g_sound.isActive() ? "YES" : "NO"
        );

        Serial.println("[TEST] Sunrise test finished");

        g_testStarted = false;
    }
}