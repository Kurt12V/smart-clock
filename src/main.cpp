#include <Arduino.h>

#include "Config.h"
#include "./managers/SettingsManager.h"
#include "./managers/SPIManager.h"
#include "./managers/I2SManager.h"
#include "./managers/SDManager.h"
#include "./managers/SoundManager.h"

// ============================================================
// OBJECTS
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

// ============================================================
// TEST CONFIGURATION
// ============================================================

// WAV-файл на SD-карте
static constexpr char TEST_AUDIO_PATH[] =
    "/audio/test.wav";

// Громкость в процентах: 0–100
static constexpr int TEST_MEDIA_VOLUME = 30;
static constexpr int TEST_ALARM_VOLUME = 50;
static constexpr int TEST_SYSTEM_VOLUME = 20;

// ============================================================
// SET AUDIO VOLUMES
// ============================================================

static void configureAudioVolumes()
{
    Serial0.println();
    Serial0.println("=== AUDIO VOLUME CONFIGURATION ===");

    g_settings.set(
        SettingsManager::Param::VOLUME_MEDIA,
        TEST_MEDIA_VOLUME
    );

    g_settings.set(
        SettingsManager::Param::VOLUME_ALARM,
        TEST_ALARM_VOLUME
    );

    g_settings.set(
        SettingsManager::Param::VOLUME_SYSTEM,
        TEST_SYSTEM_VOLUME
    );

    // Сохранить изменённые настройки в NVS
    g_settings.flush();

    Serial0.printf(
        "Media volume:  %d%%\n",
        g_settings.get(
            SettingsManager::Param::VOLUME_MEDIA
        )
    );

    Serial0.printf(
        "Alarm volume:  %d%%\n",
        g_settings.get(
            SettingsManager::Param::VOLUME_ALARM
        )
    );

    Serial0.printf(
        "System volume: %d%%\n",
        g_settings.get(
            SettingsManager::Param::VOLUME_SYSTEM
        )
    );

    Serial0.println("Settings saved.");
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);
    delay(500);

    Serial0.println();
    Serial0.println("================================");
    Serial0.println(" SmartClock Audio Test");
    Serial0.println("================================");

    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    Serial0.println("[1] Initializing settings...");

    if (!g_settings.begin())
    {
        Serial0.println(
            "[ERROR] SettingsManager initialization failed"
        );

        return;
    }

    // Устанавливаем громкость и сохраняем её
    configureAudioVolumes();

    // --------------------------------------------------------
    // SPI
    // --------------------------------------------------------

    Serial0.println("[2] Initializing SPI...");

    if (!g_spiManager.begin())
    {
        Serial0.println(
            "[ERROR] SPIManager initialization failed"
        );

        return;
    }

    // --------------------------------------------------------
    // I2S
    // --------------------------------------------------------

    Serial0.println("[3] Initializing I2S...");

    if (!g_i2sManager.begin())
    {
        Serial0.println(
            "[ERROR] I2SManager initialization failed"
        );

        return;
    }

    // --------------------------------------------------------
    // SD CARD
    // --------------------------------------------------------

    Serial0.println("[4] Initializing SD card...");

    if (!g_sd.begin(PIN_SD_CS))
    {
        Serial0.println(
            "[ERROR] SD card initialization failed"
        );

        return;
    }

    // --------------------------------------------------------
    // SOUND
    // --------------------------------------------------------

    Serial0.println("[5] Initializing SoundManager...");

    if (!g_sound.begin())
    {
        Serial0.println(
            "[ERROR] SoundManager initialization failed"
        );

        return;
    }

    // --------------------------------------------------------
    // PLAY WAV
    // --------------------------------------------------------

    Serial0.println("[6] Starting WAV playback...");
    Serial0.printf("File: %s\n", TEST_AUDIO_PATH);

    SoundManager::PlayOptions options{};

    g_sound.play(
        TEST_AUDIO_PATH,
        options
    );

    Serial0.println("Playback command sent.");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // Обновление настроек
    g_settings.update();

    // Обслуживание воспроизведения
    g_sound.update();

    delay(1);
}