#include <Arduino.h>

// ============================================================
// PROJECT
// ============================================================

#include "Pins.h"

// ============================================================
// MANAGERS
// ============================================================

#include "./managers/SPIManager.h"
#include "./managers/I2SManager.h"
#include "./managers/SDManager.h"

// ============================================================
// AUDIO
// ============================================================

#include "./hardware/audio/Microphone.h"
#include "./managers/MicrophoneManager.h"
#include "./hardware/audio/AudioRecorder.h"
#include "./managers/SoundManager.h"

// ============================================================
// TEST CONFIG
// ============================================================

namespace
{
    constexpr uint32_t RECORD_DURATION_MS = 5000;

    constexpr uint32_t START_DELAY_MS = 1000;
    constexpr uint32_t PLAYBACK_DELAY_MS = 500;
    constexpr uint32_t FINISH_DELAY_MS = 2000;

    constexpr const char* RECORDING_PATH =
        "/recordings/test.wav";
}

// ============================================================
// MANAGERS
// ============================================================

SPIManager spiManager;
I2SManager i2sManager;
SDManager sdManager;

// ============================================================
// AUDIO
// ============================================================

Microphone microphone(i2sManager);

// AudioRecorder создаём после инициализации SD.
// Поэтому указатель, а не глобальный объект.
AudioRecorder* recorder = nullptr;

MicrophoneManager* microphoneManager = nullptr;

Settings::Audio audioSettings;

SoundManager soundManager(
    sdManager,
    audioSettings,
    i2sManager
);

// ============================================================
// STATE MACHINE
// ============================================================

enum class TestState
{
    INITIALIZING,

    WAIT_BEFORE_RECORD,

    RECORDING,

    PREPARE_PLAYBACK,

    PLAYING,

    FINISHED
};

TestState state = TestState::INITIALIZING;

uint32_t stateStartTime = 0;

// ============================================================
// HELPERS
// ============================================================

void setState(TestState newState)
{
    state = newState;
    stateStartTime = millis();

    Serial.print("[TEST] State -> ");

    switch (state)
    {
        case TestState::INITIALIZING:
            Serial.println("INITIALIZING");
            break;

        case TestState::WAIT_BEFORE_RECORD:
            Serial.println("WAIT_BEFORE_RECORD");
            break;

        case TestState::RECORDING:
            Serial.println("RECORDING");
            break;

        case TestState::PREPARE_PLAYBACK:
            Serial.println("PREPARE_PLAYBACK");
            break;

        case TestState::PLAYING:
            Serial.println("PLAYING");
            break;

        case TestState::FINISHED:
            Serial.println("FINISHED");
            break;
    }
}

// ============================================================
// SYSTEM INITIALIZATION
// ============================================================

bool initializeSystem()
{
    Serial.println();
    Serial.println("========================================");
    Serial.println("ESP32-S3 AUDIO TEST");
    Serial.println("========================================");

    // --------------------------------------------------------
    // SPI
    // --------------------------------------------------------

    Serial.println("[TEST] Initializing SPI...");

    if (!spiManager.begin())
    {
        Serial.println("[TEST] ERROR: SPIManager.begin() failed");
        return false;
    }

    Serial.println("[TEST] SPI OK");

    // --------------------------------------------------------
    // I2S
    // --------------------------------------------------------

    Serial.println("[TEST] Initializing I2S...");

    if (!i2sManager.begin())
    {
        Serial.println("[TEST] ERROR: I2SManager.begin() failed");
        return false;
    }

    Serial.println("[TEST] I2S OK");

    // --------------------------------------------------------
    // SD
    // --------------------------------------------------------

    Serial.println("[TEST] Initializing SD...");

    if (!sdManager.begin(PIN_SD_CS))
    {
        Serial.println("[TEST] ERROR: SDManager.begin() failed");
        return false;
    }

    Serial.println("[TEST] SD OK");

    // --------------------------------------------------------
    // Recordings directory
    // --------------------------------------------------------

    Serial.println("[TEST] Checking /recordings...");

    if (!sdManager.card().fs().exists("/recordings"))
    {
        Serial.println("[TEST] Creating /recordings...");

        if (!sdManager.card().fs().mkdir("/recordings"))
        {
            Serial.println(
                "[TEST] ERROR: failed to create /recordings"
            );

            return false;
        }
    }

    Serial.println("[TEST] /recordings OK");

    // --------------------------------------------------------
    // AudioRecorder
    // --------------------------------------------------------
    //
    // ВАЖНО:
    // AudioRecorder получает FS после инициализации SD.
    //

    recorder = new AudioRecorder(
        microphone,
        sdManager.card().fs()
    );

    if (recorder == nullptr)
    {
        Serial.println(
            "[TEST] ERROR: AudioRecorder allocation failed"
        );

        return false;
    }

    Serial.println("[TEST] AudioRecorder created");

    // --------------------------------------------------------
    // MicrophoneManager
    // --------------------------------------------------------
    //
    // Реальный конструктор:
    //
    // MicrophoneManager(
    //     Microphone&,
    //     AudioRecorder&
    // )
    //

    microphoneManager = new MicrophoneManager(
        microphone,
        *recorder
    );

    if (microphoneManager == nullptr)
    {
        Serial.println(
            "[TEST] ERROR: MicrophoneManager allocation failed"
        );

        return false;
    }

    // --------------------------------------------------------
    // Microphone
    // --------------------------------------------------------

    Serial.println("[TEST] Initializing microphone...");

    if (!microphoneManager->begin())
    {
        Serial.println(
            "[TEST] ERROR: MicrophoneManager.begin() failed"
        );

        return false;
    }

    Serial.println("[TEST] Microphone OK");

    // --------------------------------------------------------
    // SoundManager
    // --------------------------------------------------------

    Serial.println("[TEST] Initializing speaker...");

    if (!soundManager.begin())
    {
        Serial.println(
            "[TEST] ERROR: SoundManager.begin() failed"
        );

        return false;
    }

    Serial.println("[TEST] Speaker OK");

    // --------------------------------------------------------
    // Volume
    // --------------------------------------------------------

    audioSettings.volume = 70;

    soundManager.setGlobalVolume(
        audioSettings.volume
    );

    Serial.print("[TEST] Global volume: ");
    Serial.println(soundManager.getGlobalVolume());

    // --------------------------------------------------------
    // READY
    // --------------------------------------------------------

    Serial.println();
    Serial.println("========================================");
    Serial.println("AUDIO SYSTEM READY");
    Serial.println("========================================");
    Serial.println();

    return true;
}

// ============================================================
// RECORDING
// ============================================================

bool startRecording()
{
    if (microphoneManager == nullptr)
        return false;

    Serial.println();
    Serial.println("----------------------------------------");
    Serial.println("[TEST] START RECORDING");
    Serial.println("----------------------------------------");

    Serial.print("[TEST] File: ");
    Serial.println(RECORDING_PATH);

    if (!microphoneManager->startListening())
    {
        Serial.println(
            "[TEST] ERROR: startListening() failed"
        );

        return false;
    }

    if (!recorder->startRecording(RECORDING_PATH))
    {
        Serial.println(
            "[TEST] ERROR: startRecording() failed"
        );

        microphoneManager->stopListening();

        return false;
    }

    Serial.println("[TEST] Recording started");

    return true;
}

// ============================================================
// STOP RECORDING
// ============================================================

void stopRecording()
{
    Serial.println();
    Serial.println("----------------------------------------");
    Serial.println("[TEST] STOP RECORDING");
    Serial.println("----------------------------------------");

    if (recorder != nullptr)
    {
        recorder->stopRecording();

        Serial.print("[TEST] Recorded bytes: ");
        Serial.println(recorder->getRecordedBytes());

        Serial.print("[TEST] Recorded samples: ");
        Serial.println(recorder->getRecordedSamples());

        Serial.print("[TEST] Duration: ");
        Serial.print(recorder->getRecordingDurationMs());
        Serial.println(" ms");
    }

    if (microphoneManager != nullptr)
    {
        microphoneManager->stopListening();
    }

    Serial.println("[TEST] Recording stopped");
}

// ============================================================
// PLAYBACK
// ============================================================

bool startPlayback()
{
    Serial.println();
    Serial.println("----------------------------------------");
    Serial.println("[TEST] START PLAYBACK");
    Serial.println("----------------------------------------");

    Serial.print("[TEST] File: ");
    Serial.println(RECORDING_PATH);

    if (!soundManager.playWav(RECORDING_PATH))
    {
        Serial.println(
            "[TEST] ERROR: playWav() failed"
        );

        return false;
    }

    Serial.println("[TEST] Playback started");

    Serial.print("[TEST] Duration: ");
    Serial.print(soundManager.getDurationMs());
    Serial.println(" ms");

    return true;
}

// ============================================================
// DEBUG MICROPHONE
// ============================================================

void printMicrophoneDebug()
{
    if (microphoneManager == nullptr)
        return;

    static uint32_t lastPrint = 0;

    const uint32_t now = millis();

    if (now - lastPrint < 500)
        return;

    lastPrint = now;

    Serial.print("[MIC] ");

    Serial.print("RMS=");
    Serial.print(microphoneManager->getRMS(), 2);

    Serial.print(" | LEVEL=");
    Serial.print(microphoneManager->getLevel(), 2);

    Serial.print(" | PEAK=");
    Serial.print(microphoneManager->getPeak());

    Serial.print(" | CHUNK=");
    Serial.print(microphoneManager->getAudioChunkId());

    Serial.print(" | SAMPLES=");
    Serial.println(
        microphoneManager->getAudioChunkSize()
    );
}

// ============================================================
// DEBUG PLAYBACK
// ============================================================

void printPlaybackDebug()
{
    static uint32_t lastPrint = 0;

    const uint32_t now = millis();

    if (now - lastPrint < 500)
        return;

    lastPrint = now;

    Serial.print("[PLAY] ");

    Serial.print(
        soundManager.getPositionMs()
    );

    Serial.print(" / ");

    Serial.print(
        soundManager.getDurationMs()
    );

    Serial.print(" ms");

    Serial.print(" | volume=");

    Serial.println(
        soundManager.getEffectiveVolume()
    );
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println();
    Serial.println("########################################");
    Serial.println("# ESP32-S3 SMART CLOCK AUDIO TEST");
    Serial.println("########################################");

    setState(TestState::INITIALIZING);
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // INITIALIZING
    // --------------------------------------------------------

    if (state == TestState::INITIALIZING)
    {
        if (initializeSystem())
        {
            setState(
                TestState::WAIT_BEFORE_RECORD
            );
        }
        else
        {
            Serial.println();
            Serial.println(
                "[TEST] SYSTEM INITIALIZATION FAILED"
            );

            Serial.println(
                "[TEST] Restarting in 5 seconds..."
            );

            delay(5000);

            ESP.restart();
        }

        return;
    }

    // --------------------------------------------------------
    // MICROPHONE UPDATE
    // --------------------------------------------------------

    if (microphoneManager != nullptr)
    {
        microphoneManager->update();
    }

    // --------------------------------------------------------
    // SOUND UPDATE
    // --------------------------------------------------------

    soundManager.update();

    // --------------------------------------------------------
    // RECORD WAIT
    // --------------------------------------------------------

    if (state == TestState::WAIT_BEFORE_RECORD)
    {
        if (millis() - stateStartTime >= START_DELAY_MS)
        {
            if (startRecording())
            {
                setState(TestState::RECORDING);
            }
            else
            {
                Serial.println(
                    "[TEST] Recording start failed"
                );

                setState(TestState::FINISHED);
            }
        }

        return;
    }

    // --------------------------------------------------------
    // RECORDING
    // --------------------------------------------------------

    if (state == TestState::RECORDING)
    {
        printMicrophoneDebug();

        if (millis() - stateStartTime >=
            RECORD_DURATION_MS)
        {
            stopRecording();

            setState(
                TestState::PREPARE_PLAYBACK
            );
        }

        return;
    }

    // --------------------------------------------------------
    // PREPARE PLAYBACK
    // --------------------------------------------------------

    if (state == TestState::PREPARE_PLAYBACK)
    {
        if (millis() - stateStartTime >=
            PLAYBACK_DELAY_MS)
        {
            if (startPlayback())
            {
                setState(TestState::PLAYING);
            }
            else
            {
                Serial.println(
                    "[TEST] Playback start failed"
                );

                setState(TestState::FINISHED);
            }
        }

        return;
    }

    // --------------------------------------------------------
    // PLAYING
    // --------------------------------------------------------

    if (state == TestState::PLAYING)
    {
        printPlaybackDebug();

        if (!soundManager.isActive())
        {
            Serial.println(
                "[TEST] Playback finished"
            );

            setState(TestState::FINISHED);
        }

        return;
    }

    // --------------------------------------------------------
    // FINISHED
    // --------------------------------------------------------

    if (state == TestState::FINISHED)
    {
        if (millis() - stateStartTime >=
            FINISH_DELAY_MS)
        {
            Serial.println();
            Serial.println(
                "[TEST] Test cycle finished."
            );

            Serial.println(
                "[TEST] Starting new cycle..."
            );

            setState(
                TestState::WAIT_BEFORE_RECORD
            );
        }

        return;
    }
}