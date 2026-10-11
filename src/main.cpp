#include <Arduino.h>

#include "Config.h"
#include "Pins.h"
#include "Constants.h"

#include "managers/SettingsManager.h"
#include "managers/SPIManager.h"
#include "managers/I2SManager.h"
#include "managers/SDManager.h"
#include "managers/SoundManager.h"
#include "managers/LightingManager.h"

#include "hardware/AlarmEffects.h"

// ============================================================
// GLOBAL OBJECTS
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

AlarmEffects g_alarmEffects(
    g_lighting,
    g_sound
);

// ============================================================
// TEST CONFIGURATION
// ============================================================

namespace
{
    constexpr uint32_t TEST_DURATION_MS =
        27UL * 60UL * 1000UL;

    constexpr uint32_t DEBUG_INTERVAL_MS =
        1000UL;

    constexpr int TEST_ALARM_VOLUME = 100;

    uint32_t g_testStartMs = 0;
    uint32_t g_lastDebugMs = 0;

    bool g_systemReady = false;
    bool g_testStarted = false;
    bool g_testFinished = false;
}

// ============================================================
// SERIAL LOGGING
// ============================================================

static void printSeparator()
{
    Serial0.println(
        F("============================================")
    );
}

static void printHeader()
{
    Serial0.println();

    printSeparator();

    Serial0.println(
        F("       SmartClock - Sunrise Test")
    );

    printSeparator();
}

// ============================================================
// SETTINGS
// ============================================================

static void initSettings()
{
    Serial0.println(
        F("[SETTINGS] Initializing...")
    );

    g_settings.begin();

    // Установить громкость будильника для теста.
    g_settings.set(
        SettingsManager::Param::VOLUME_ALARM,
        TEST_ALARM_VOLUME
    );

    // Сохранить настройки.
    g_settings.flush();

    const int actualVolume =
        g_settings.get(
            SettingsManager::Param::VOLUME_ALARM
        );

    Serial0.printf(
        "[SETTINGS] Alarm volume: %d%%\n",
        actualVolume
    );
}

// ============================================================
// SYSTEM INITIALIZATION
// ============================================================

static bool initializeSystem()
{
    printHeader();

    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    initSettings();

    // --------------------------------------------------------
    // SPI
    // --------------------------------------------------------

    Serial0.println(
        F("[SPI] Initializing...")
    );

    g_spiManager.begin();

    Serial0.println(
        F("[SPI] Ready.")
    );

    // --------------------------------------------------------
    // I2S
    // --------------------------------------------------------

    Serial0.println(
        F("[I2S] Initializing...")
    );

    g_i2sManager.begin();

    Serial0.println(
        F("[I2S] Ready.")
    );

    // --------------------------------------------------------
    // SD CARD
    // --------------------------------------------------------

    Serial0.println(
        F("[SD] Initializing...")
    );

    if (!g_sd.begin(PIN_SD_CS))
    {
        Serial0.println(
            F("[ERROR] SD initialization failed.")
        );

        return false;
    }

    Serial0.println(
        F("[SD] Ready.")
    );

    // --------------------------------------------------------
    // SOUND
    // --------------------------------------------------------

    Serial0.println(
        F("[SOUND] Initializing...")
    );

    g_sound.begin();

    Serial0.println(
        F("[SOUND] Ready.")
    );

    // --------------------------------------------------------
    // LIGHTING
    // --------------------------------------------------------

    Serial0.println(
        F("[LIGHTING] Initializing...")
    );

    g_lighting.begin();

    Serial0.println(
        F("[LIGHTING] Ready.")
    );

    // --------------------------------------------------------
    // ALARM EFFECTS
    // --------------------------------------------------------

    Serial0.println(
        F("[ALARM] Initializing...")
    );

    g_alarmEffects.begin();

    Serial0.println(
        F("[ALARM] Ready.")
    );

    return true;
}

// ============================================================
// TEST CONFIGURATION LOG
// ============================================================

static void printTestConfiguration()
{
    printSeparator();

    Serial0.println(
        F("[TEST] Configuration")
    );

    Serial0.printf(
        "[TEST] Duration: %lu minutes\n",
        static_cast<unsigned long>(
            TEST_DURATION_MS / 60000UL
        )
    );

    Serial0.printf(
        "[TEST] Alarm volume setting: %d%%\n",
        g_settings.get(
            SettingsManager::Param::VOLUME_ALARM
        )
    );

    Serial0.println(
        F("[TEST] Audio file: /audio/alarms/alarm.wav")
    );

    printSeparator();
}

// ============================================================
// START TEST
// ============================================================

static void startTest()
{
    printTestConfiguration();

    Serial0.println(
        F("[TEST] Starting sunrise effect...")
    );

    g_testStartMs = millis();
    g_lastDebugMs = g_testStartMs;

    g_alarmEffects.startSunrise();

    g_testStarted = true;

    Serial0.println(
        F("[TEST] Sunrise started.")
    );
}

// ============================================================
// TEST STATUS
// ============================================================

static void printTestStatus(
    uint32_t elapsedMs
)
{
    Serial0.printf(
        "[TEST] Elapsed: %lu s | Sound: %s | Effects: %s\n",
        static_cast<unsigned long>(
            elapsedMs / 1000UL
        ),
        g_sound.isActive() ? "ACTIVE" : "IDLE",
        g_alarmEffects.isActive() ? "ACTIVE" : "IDLE"
    );
}

// ============================================================
// FINISH TEST
// ============================================================

static void finishTest()
{
    Serial0.println();

    Serial0.println(
        F("[TEST] Test duration reached.")
    );

    g_alarmEffects.stop();

    // Обновить свет после снятия override.
    g_lighting.update();

    g_testFinished = true;

    Serial0.println(
        F("[TEST] Sunrise stopped.")
    );

    Serial0.println(
        F("[TEST] Test finished.")
    );
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);

    delay(500);

    if (!initializeSystem())
    {
        Serial0.println(
            F("[FATAL] Initialization failed.")
        );

        return;
    }

    g_systemReady = true;

    startTest();
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    if (!g_systemReady ||
        !g_testStarted ||
        g_testFinished)
    {
        delay(10);
        return;
    }

    // Обновление отложенного сохранения настроек.
    g_settings.update();

    // Обслуживание воспроизведения WAV.
    g_sound.update();

    const uint32_t now = millis();

    const uint32_t elapsedMs =
        now - g_testStartMs;

    // Обновление рассвета и его световых эффектов.
    g_alarmEffects.update(elapsedMs);

    // Обновление системы освещения.
    g_lighting.update();

    // Диагностика раз в секунду.
    if (now - g_lastDebugMs >= DEBUG_INTERVAL_MS)
    {
        g_lastDebugMs = now;

        printTestStatus(elapsedMs);
    }

    // Остановка после завершения тестового интервала.
    if (elapsedMs >= TEST_DURATION_MS)
    {
        finishTest();
    }

    delay(1);
}