
#include <Arduino.h>

#include "Config.h"

#include "./managers/SettingsManager.h"
#include "./managers/SPIManager.h"
#include "./managers/I2SManager.h"
#include "./managers/SDManager.h"
#include "./managers/SoundManager.h"
#include "./managers/LedMatrixManager.h"

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

// Замените PIN_MATRIX_DATA на имя константы
// из вашего Config.h, если оно отличается.
LedMatrixManager g_matrix(PIN_MATRIX_DATA);

// ============================================================
// TEST CONFIGURATION
// ============================================================

static constexpr char TEST_AUDIO_PATH[] =
    "/audio/test.wav";

// Громкость аудио в процентах
static constexpr int TEST_MEDIA_VOLUME  = 30;
static constexpr int TEST_ALARM_VOLUME  = 50;
static constexpr int TEST_SYSTEM_VOLUME = 20;

// Яркость матрицы: 0–100%
static constexpr uint8_t TEST_MATRIX_BRIGHTNESS = 30;

// Скорость эффекта: 0–100
static constexpr uint8_t TEST_MATRIX_SPEED = 50;

// Время перехода между эффектами
static constexpr uint16_t TEST_TRANSITION_MS = 1000;

// ============================================================
// EFFECT LIST
// ============================================================

// Нумерация соответствует порядку эффектов в MatrixEffects.h.
// Если enum Effect имеет другой порядок, скорректируйте список.

struct EffectEntry
{
    const char* name;
    LedMatrixManager::Effect effect;
};

static constexpr EffectEntry EFFECTS[] =
{
    { "None",        LedMatrixManager::Effect::None        },
    { "Rainbow",     LedMatrixManager::Effect::Rainbow     },
    { "RainbowWave", LedMatrixManager::Effect::RainbowWave },
    { "Fire",        LedMatrixManager::Effect::Fire        },
    { "Fireplace",   LedMatrixManager::Effect::Fireplace   },
    { "Plasma",      LedMatrixManager::Effect::Plasma      },
    { "Aurora",      LedMatrixManager::Effect::Aurora      },
    { "Ocean",       LedMatrixManager::Effect::Ocean       },
    { "Twinkle",     LedMatrixManager::Effect::Twinkle     },
    { "Meteor",      LedMatrixManager::Effect::Meteor      },
    { "Matrix",      LedMatrixManager::Effect::Matrix      }
};

static constexpr size_t EFFECT_COUNT =
    sizeof(EFFECTS) / sizeof(EFFECTS[0]);

// ============================================================
// STATE
// ============================================================

static uint8_t g_brightness =
    TEST_MATRIX_BRIGHTNESS;

static uint8_t g_speed =
    TEST_MATRIX_SPEED;

static size_t g_effectIndex = 0;

static bool g_matrixInitialized = false;
static bool g_audioInitialized = false;

// ============================================================
// SERIAL HELPERS
// ============================================================

static void printHelp()
{
    Serial0.println();
    Serial0.println("========== SMARTCLOCK TEST ==========");
    Serial0.println("1..9 : select matrix effect");
    Serial0.println("0    : select None effect");
    Serial0.println("+    : brightness +5%");
    Serial0.println("-    : brightness -5%");
    Serial0.println("b N  : set brightness, 0..100");
    Serial0.println("s N  : set effect speed, 0..100");
    Serial0.println("t N  : transition time, 0..10000 ms");
    Serial0.println("p    : play WAV again");
    Serial0.println("x    : stop audio");
    Serial0.println("?    : show help");
    Serial0.println("=====================================");
    Serial0.println();
}

static void printMatrixState()
{
    Serial0.println();
    Serial0.println("--- MATRIX STATE ---");

    Serial0.printf(
        "Effect: %s\n",
        EFFECTS[g_effectIndex].name
    );

    Serial0.printf(
        "Brightness: %u%%\n",
        static_cast<unsigned>(g_brightness)
    );

    Serial0.printf(
        "Speed: %u\n",
        static_cast<unsigned>(g_speed)
    );

    Serial0.printf(
        "Transition: %u ms\n",
        static_cast<unsigned>(
            g_matrix.transitionTime()
        )
    );

    Serial0.println();
}

// ============================================================
// AUDIO VOLUMES
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

    Serial0.println("Audio settings saved.");
}

// ============================================================
// MATRIX CONTROL
// ============================================================

static void selectEffect(size_t index)
{
    if (!g_matrixInitialized)
    {
        Serial0.println("[ERROR] Matrix is not initialized.");
        return;
    }

    if (index >= EFFECT_COUNT)
    {
        Serial0.println("[ERROR] Invalid effect index.");
        return;
    }

    g_effectIndex = index;

    g_matrix.setEffect(
        EFFECTS[index].effect
    );

    Serial0.printf(
        "[MATRIX] Effect: %s\n",
        EFFECTS[index].name
    );
}

static void setMatrixBrightness(int value)
{
    value = constrain(value, 0, 100);

    g_brightness = static_cast<uint8_t>(value);

    // LedMatrixManager может ограничивать физическую
    // яркость собственным MAX_BRIGHTNESS.
    // Здесь значение задаётся в диапазоне 0–100.
    g_matrix.setBrightness(g_brightness);

    Serial0.printf(
        "[MATRIX] Brightness: %u%%\n",
        static_cast<unsigned>(g_brightness)
    );
}

static void changeBrightness(int delta)
{
    setMatrixBrightness(
        static_cast<int>(g_brightness) + delta
    );
}

static void setMatrixSpeed(int value)
{
    value = constrain(value, 0, 100);

    g_speed = static_cast<uint8_t>(value);

    g_matrix.setEffectSpeed(g_speed);

    Serial0.printf(
        "[MATRIX] Speed: %u\n",
        static_cast<unsigned>(g_speed)
    );
}

static void setMatrixTransition(int value)
{
    value = constrain(value, 0, 10000);

    g_matrix.setTransitionTime(
        static_cast<uint16_t>(value)
    );

    Serial0.printf(
        "[MATRIX] Transition: %u ms\n",
        static_cast<unsigned>(value)
    );
}

// ============================================================
// AUDIO CONTROL
// ============================================================

static void playTestAudio()
{
    if (!g_audioInitialized)
    {
        Serial0.println("[ERROR] SoundManager is not initialized.");
        return;
    }

    Serial0.printf(
        "[AUDIO] Playing: %s\n",
        TEST_AUDIO_PATH
    );

    SoundManager::PlayOptions options{};

    g_sound.play(
        TEST_AUDIO_PATH,
        options
    );
}

static void stopTestAudio()
{
    // Предполагается, что SoundManager предоставляет stop().
    // Если в вашем SoundManager метод называется иначе,
    // замените вызов на фактическое имя метода.
    g_sound.stop();

    Serial0.println("[AUDIO] Stop command sent.");
}

// ============================================================
// SERIAL COMMAND PROCESSING
// ============================================================

static String g_commandBuffer;

static void processCommand(String command)
{
    command.trim();

    if (command.isEmpty())
        return;

    // Команды без аргументов
    if (command == "?")
    {
        printHelp();
        return;
    }

    if (command == "+")
    {
        changeBrightness(5);
        return;
    }

    if (command == "-")
    {
        changeBrightness(-5);
        return;
    }

    if (command == "p")
    {
        playTestAudio();
        return;
    }

    if (command == "x")
    {
        stopTestAudio();
        return;
    }

    // Выбор эффекта по номеру.
    if (command.length() == 1 &&
        command[0] >= '0' &&
        command[0] <= '9')
    {
        const size_t index =
            command[0] == '0'
                ? 0
                : static_cast<size_t>(
                    command[0] - '0'
                );

        if (index < EFFECT_COUNT)
        {
            selectEffect(index);
        }
        else
        {
            Serial0.println("[ERROR] No effect with this number.");
        }

        return;
    }

    // Команды с аргументами: b 50, s 70, t 1500.
    const int separator = command.indexOf(' ');

    if (separator > 0)
    {
        String key = command.substring(0, separator);
        String valueText = command.substring(separator + 1);

        key.trim();
        valueText.trim();

        if (valueText.isEmpty())
        {
            Serial0.println("[ERROR] Missing command value.");
            return;
        }

        // Не допускаем случайного преобразования текста в 0.
        for (size_t i = 0; i < valueText.length(); ++i)
        {
            if (!isDigit(valueText[i]))
            {
                Serial0.println("[ERROR] Value must be an integer.");
                return;
            }
        }

        const long value = valueText.toInt();

        if (key == "b")
        {
            setMatrixBrightness(
                static_cast<int>(
                    constrain(value, 0L, 100L)
                )
            );
            return;
        }

        if (key == "s")
        {
            setMatrixSpeed(
                static_cast<int>(
                    constrain(value, 0L, 100L)
                )
            );
            return;
        }

        if (key == "t")
        {
            setMatrixTransition(
                static_cast<int>(
                    constrain(value, 0L, 10000L)
                )
            );
            return;
        }
    }

    Serial0.printf(
        "[ERROR] Unknown command: %s\n",
        command.c_str()
    );

    printHelp();
}

static void updateSerialCommands()
{
    while (Serial0.available() > 0)
    {
        const char c =
            static_cast<char>(Serial0.read());

        if (c == '\r' || c == '\n')
        {
            if (!g_commandBuffer.isEmpty())
            {
                processCommand(g_commandBuffer);
                g_commandBuffer = "";
            }

            continue;
        }

        if (g_commandBuffer.length() < 40)
        {
            g_commandBuffer += c;
        }
        else
        {
            g_commandBuffer = "";
            Serial0.println("[ERROR] Command is too long.");
        }
    }
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
    Serial0.println(" SmartClock Matrix + Audio Test");
    Serial0.println("================================");

    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    Serial0.println("[1] Initializing SettingsManager...");

    if (!g_settings.begin())
    {
        Serial0.println("[ERROR] SettingsManager initialization failed.");
        return;
    }

    configureAudioVolumes();

    // --------------------------------------------------------
    // SPI
    // --------------------------------------------------------

    Serial0.println("[2] Initializing SPIManager...");

    if (!g_spiManager.begin())
    {
        Serial0.println("[ERROR] SPIManager initialization failed.");
        return;
    }

    // --------------------------------------------------------
    // I2S
    // --------------------------------------------------------

    Serial0.println("[3] Initializing I2SManager...");

    if (!g_i2sManager.begin())
    {
        Serial0.println("[ERROR] I2SManager initialization failed.");
        return;
    }

    // --------------------------------------------------------
    // SD CARD
    // --------------------------------------------------------

    Serial0.println("[4] Initializing SD card...");

    if (!g_sd.begin(PIN_SD_CS))
    {
        Serial0.println("[ERROR] SD card initialization failed.");
        return;
    }

    // --------------------------------------------------------
    // MATRIX
    // --------------------------------------------------------

    Serial0.println("[5] Initializing LedMatrixManager...");

    g_matrix.begin();
    g_matrixInitialized = true;

    g_matrix.setTransitionTime(TEST_TRANSITION_MS);
    g_matrix.setEffectSpeed(g_speed);
    g_matrix.setBrightness(g_brightness);

    // Запускаем первый визуальный эффект.
    selectEffect(1);

    // --------------------------------------------------------
    // SOUND
    // --------------------------------------------------------

    Serial0.println("[6] Initializing SoundManager...");

    if (!g_sound.begin())
    {
        Serial0.println("[ERROR] SoundManager initialization failed.");
        return;
    }

    g_audioInitialized = true;

    // --------------------------------------------------------
    // PLAY WAV WHILE MATRIX IS RUNNING
    // --------------------------------------------------------

    Serial0.println("[7] Starting WAV playback...");

    playTestAudio();

    printMatrixState();
    printHelp();

    Serial0.println("Matrix animation and audio test started.");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // Обслуживание отложенного сохранения настроек.
    g_settings.update();

    // Матрица продолжает анимацию независимо от звука.
    if (g_matrixInitialized)
    {
        g_matrix.update();
    }

    // Обслуживание воспроизведения WAV.
    if (g_audioInitialized)
    {
        g_sound.update();
    }

    // Приём команд с Serial Monitor.
    updateSerialCommands();

    // Без длительных delay(), чтобы звук и анимация
    // могли обслуживаться непрерывно.
    delay(1);
}
