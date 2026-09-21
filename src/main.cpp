#include <Arduino.h>
#include <driver/i2s.h>
#include <cstring>
#include <cmath>

// ============================================================
// Core
// ============================================================

#include "Pins.h"
#include "Settings.h"

// ============================================================
// Hardware
// ============================================================

#include "./managers/SPIManager.h"
#include "./managers/I2SManager.h"
#include "./managers/SDManager.h"

// ============================================================
// Audio
// ============================================================

#include "./hardware/audio/Microphone.h"
#include "./managers/MicrophoneManager.h"
#include "./hardware/audio/AudioRecorder.h"
#include "./managers/SoundManager.h"

// ============================================================
// TEST FILE
// ============================================================

static constexpr const char* RECORDING_PATH =
    "/recordings/test.wav";

// ============================================================
// GLOBALS
// ============================================================

Settings::Data settings;

SPIManager spiManager;
I2SManager i2sManager;
SDManager sdManager;

Microphone microphone(
    i2sManager
);

AudioRecorder audioRecorder(
    microphone,
    sdManager.card().fs()
);

MicrophoneManager microphoneManager(
    microphone,
    audioRecorder
);

SoundManager soundManager(
    sdManager,
    settings.audio,
    i2sManager
);

// ============================================================
// TEST STATE
// ============================================================

enum class TestState
{
    IDLE,
    RECORDING,
    PLAYING,
    DONE,
    ERROR
};

TestState testState =
    TestState::IDLE;

uint32_t recordingStartMs = 0;

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println();
    Serial.println(
        "############################################"
    );
    Serial.println(
        "# ESP32-S3 AUDIO TEST"
    );
    Serial.println(
        "# INMP441 -> I2S_NUM_0"
    );
    Serial.println(
        "# MAX98357 -> I2S_NUM_1"
    );
    Serial.println(
        "# WAV -> /recordings/test.wav"
    );
    Serial.println(
        "############################################"
    );

    // --------------------------------------------------------
    // SPI
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[TEST] Initializing SPI..."
    );

    if (!spiManager.begin())
    {
        Serial.println(
            "[TEST] ERROR: SPI failed"
        );

        testState =
            TestState::ERROR;

        return;
    }

    Serial.println(
        "[TEST] SPI OK"
    );

    // --------------------------------------------------------
    // I2S Manager
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[TEST] Initializing I2SManager..."
    );

    if (!i2sManager.begin())
    {
        Serial.println(
            "[TEST] ERROR: I2SManager failed"
        );

        testState =
            TestState::ERROR;

        return;
    }

    Serial.println(
        "[TEST] I2SManager OK"
    );

    Serial.printf(
        "[TEST] Mic port: I2S_NUM_%d\n",
        static_cast<int>(
            i2sManager.microphonePort()
        )
    );

    Serial.printf(
        "[TEST] Speaker port: I2S_NUM_%d\n",
        static_cast<int>(
            i2sManager.speakerPort()
        )
    );

    // --------------------------------------------------------
    // SD
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[TEST] Initializing SD..."
    );

    if (
        !sdManager.begin(
            PIN_SD_CS
        )
    )
    {
        Serial.println(
            "[TEST] ERROR: SD failed"
        );

        testState =
            TestState::ERROR;

        return;
    }

    Serial.println(
        "[TEST] SD OK"
    );

    // --------------------------------------------------------
    // Directory
    // --------------------------------------------------------

    fs::FS& fs =
        sdManager.card().fs();

    if (!fs.exists("/recordings"))
    {
        Serial.println(
            "[TEST] Creating /recordings"
        );

        if (!fs.mkdir("/recordings"))
        {
            Serial.println(
                "[TEST] ERROR: mkdir /recordings failed"
            );

            testState =
                TestState::ERROR;

            return;
        }
    }

    Serial.println(
        "[TEST] /recordings OK"
    );

    // --------------------------------------------------------
    // AudioRecorder
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[TEST] Initializing AudioRecorder..."
    );

    if (!audioRecorder.begin())
    {
        Serial.println(
            "[TEST] ERROR: AudioRecorder failed"
        );

        testState =
            TestState::ERROR;

        return;
    }

    Serial.println(
        "[TEST] AudioRecorder OK"
    );

    // --------------------------------------------------------
    // Microphone
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[TEST] Starting microphone..."
    );

    if (!microphoneManager.begin())
    {
        Serial.println(
            "[TEST] ERROR: microphone begin failed"
        );

        testState =
            TestState::ERROR;

        return;
    }

    microphoneManager.setEnabled(
        true
    );

    if (!microphoneManager.startListening())
    {
        Serial.println(
            "[TEST] ERROR: startListening failed"
        );

        testState =
            TestState::ERROR;

        return;
    }

    Serial.println(
        "[TEST] Microphone listening"
    );

    // --------------------------------------------------------
    // Recording
    // --------------------------------------------------------

    Serial.printf(
        "[TEST] Recording to: %s\n",
        RECORDING_PATH
    );

    if (
        !microphoneManager.startRecording(
            RECORDING_PATH
        )
    )
    {
        Serial.println(
            "[TEST] ERROR: startRecording failed"
        );

        testState =
            TestState::ERROR;

        return;
    }

    recordingStartMs =
        millis();

    testState =
        TestState::RECORDING;

    Serial.println(
        "[TEST] RECORDING STARTED"
    );
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // Microphone
    // --------------------------------------------------------

    if (
        testState ==
        TestState::RECORDING
    )
    {
        microphoneManager.update();

        if (
            millis() -
            recordingStartMs >=
            5000
        )
        {
            Serial.println();
            Serial.println(
                "[TEST] Stopping recording..."
            );

            microphoneManager.stopRecording();

            Serial.printf(
                "[REC] Bytes: %lu\n",
                static_cast<unsigned long>(
                    microphoneManager
                        .getRecordedBytes()
                )
            );

            Serial.printf(
                "[REC] Samples: %lu\n",
                static_cast<unsigned long>(
                    microphoneManager
                        .getRecordedSamples()
                )
            );

            Serial.printf(
                "[REC] Duration: %lu ms\n",
                static_cast<unsigned long>(
                    microphoneManager
                        .getRecordingDurationMs()
                )
            );

            microphoneManager.end();

            Serial.println(
                "[MIC] Microphone released"
            );

            // ------------------------------------------------
            // Check file
            // ------------------------------------------------

            fs::FS& fs =
                sdManager.card().fs();

            if (
                !fs.exists(
                    RECORDING_PATH
                )
            )
            {
                Serial.println(
                    "[TEST] ERROR: WAV does not exist"
                );

                testState =
                    TestState::ERROR;

                return;
            }

            File file =
                fs.open(
                    RECORDING_PATH,
                    FILE_READ
                );

            if (!file)
            {
                Serial.println(
                    "[TEST] ERROR: cannot open WAV"
                );

                testState =
                    TestState::ERROR;

                return;
            }

            Serial.printf(
                "[WAV] File: %s\n",
                RECORDING_PATH
            );

            Serial.printf(
                "[WAV] Size: %lu bytes\n",
                static_cast<unsigned long>(
                    file.size()
                )
            );

            file.close();

            // ------------------------------------------------
            // SoundManager
            // ------------------------------------------------

            Serial.println();
            Serial.println(
                "[TEST] Starting SoundManager..."
            );

            if (!soundManager.begin())
            {
                Serial.println(
                    "[TEST] ERROR: SoundManager.begin failed"
                );

                testState =
                    TestState::ERROR;

                return;
            }

            Serial.println(
                "[TEST] SoundManager ready"
            );

            Serial.println();
            Serial.println(
                "[TEST] Playing WAV..."
            );

            if (
                !soundManager.playWav(
                    RECORDING_PATH
                )
            )
            {
                Serial.println(
                    "[TEST] ERROR: playWav failed"
                );

                soundManager.printStatus();

                testState =
                    TestState::ERROR;

                return;
            }

            Serial.println(
                "[TEST] playWav SUCCESS"
            );

            soundManager.printStatus();

            testState =
                TestState::PLAYING;
        }
    }

    // --------------------------------------------------------
    // Sound
    // --------------------------------------------------------

    if (
        testState ==
        TestState::PLAYING
    )
    {
        soundManager.update();

        static uint32_t lastPrint = 0;

        if (
            millis() -
            lastPrint >=
            1000
        )
        {
            lastPrint =
                millis();

            Serial.printf(
                "[PLAY] %lu / %lu ms\n",
                static_cast<unsigned long>(
                    soundManager.getPositionMs()
                ),
                static_cast<unsigned long>(
                    soundManager.getDurationMs()
                )
            );
        }

        if (
            !soundManager.isPlaying() &&
            !soundManager.isPaused()
        )
        {
            Serial.println();
            Serial.println(
                "[TEST] PLAYBACK FINISHED"
            );

            testState =
                TestState::DONE;
        }
    }

    // --------------------------------------------------------
    // Done
    // --------------------------------------------------------

    if (
        testState ==
        TestState::DONE
    )
    {
        static bool printed = false;

        if (!printed)
        {
            printed = true;

            Serial.println();
            Serial.println(
                "############################################"
            );
            Serial.println(
                "# AUDIO TEST PASSED"
            );
            Serial.println(
                "############################################"
            );
        }
    }

    // --------------------------------------------------------
    // Error
    // --------------------------------------------------------

    if (
        testState ==
        TestState::ERROR
    )
    {
        static uint32_t lastError = 0;

        if (
            millis() -
            lastError >=
            2000
        )
        {
            lastError =
                millis();

            Serial.println(
                "[TEST] ERROR STATE"
            );
        }
    }

    delay(1);
}

