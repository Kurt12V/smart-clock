#include <Arduino.h>

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
// Settings
// ============================================================

Settings::Data settings;

// ============================================================
// Hardware managers
// ============================================================

SPIManager spiManager;
I2SManager i2sManager;
SDManager sdManager;

// ============================================================
// Audio
// ============================================================

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
// Test state
// ============================================================

static constexpr char RECORDING_PATH[] =
    "/recordings/test.wav";

static constexpr uint32_t RECORDING_TIME_MS =
    5000;

bool recordingFinished = false;
bool playbackStarted = false;
bool testFinished = false;

uint32_t recordingStartTime = 0;

// ============================================================
// Prepare SD directories
// ============================================================

bool prepareRecordingDirectory()
{
    fs::FS& fs = sdManager.card().fs();

    if (!fs.exists("/audio"))
    {
        Serial0.println(
            "[TEST] Creating /audio"
        );

        if (!fs.mkdir("/audio"))
        {
            Serial0.println(
                "[TEST] ERROR: cannot create /audio"
            );

            return false;
        }
    }

    if (!fs.exists("/recordings"))
    {
        Serial0.println(
            "[TEST] Creating /recordings"
        );

        if (!fs.mkdir("/recordings"))
        {
            Serial0.println(
                "[TEST] ERROR: cannot create /recordings"
            );

            return false;
        }
    }

    return true;
}

// ============================================================
// Print SD information
// ============================================================

void printSDInfo()
{
    const SDCardInfo info =
        sdManager.getInfo();

    Serial0.println();
    Serial0.println(
        "========== SD INFO =========="
    );

    Serial0.print(
        "Ready: "
    );

    Serial0.println(
        sdManager.isReady()
            ? "YES"
            : "NO"
    );

    Serial0.print(
        "Total: "
    );

    Serial0.println(
        info.totalBytes
    );

    Serial0.print(
        "Used: "
    );

    Serial0.println(
        info.usedBytes
    );

    Serial0.println(
        "============================="
    );

    Serial0.println();
}

// ============================================================
// Start recording
// ============================================================

bool startTestRecording()
{
    Serial0.println();
    Serial0.println(
        "================================"
    );

    Serial0.println(
        "[TEST] Starting microphone..."
    );

    if (!microphoneManager.begin())
    {
        Serial0.println(
            "[TEST] ERROR: MicrophoneManager.begin() failed"
        );

        return false;
    }

    Serial0.println(
        "[TEST] Microphone initialized"
    );

    // --------------------------------------------------------
    // Enable microphone
    // --------------------------------------------------------

    microphoneManager.setEnabled(true);

    // --------------------------------------------------------
    // Start listening
    // --------------------------------------------------------

    if (!microphoneManager.startListening())
    {
        Serial0.println(
            "[TEST] ERROR: startListening() failed"
        );

        microphoneManager.end();

        return false;
    }

    Serial0.println(
        "[TEST] Microphone listening"
    );

    // --------------------------------------------------------
    // Start recording
    // --------------------------------------------------------

    if (!microphoneManager.startRecording(
            RECORDING_PATH
        ))
    {
        Serial0.println(
            "[TEST] ERROR: startRecording() failed"
        );

        microphoneManager.stopListening();
        microphoneManager.end();

        return false;
    }

    recordingStartTime = millis();

    Serial0.println(
        "[TEST] RECORDING STARTED"
    );

    Serial0.print(
        "[TEST] File: "
    );

    Serial0.println(
        RECORDING_PATH
    );

    Serial0.print(
        "[TEST] Duration: "
    );

    Serial0.print(
        RECORDING_TIME_MS
    );

    Serial0.println(
        " ms"
    );

    Serial0.println(
        "================================"
    );

    return true;
}

// ============================================================
// Finish recording
// ============================================================

void finishRecording()
{
    if (recordingFinished)
    {
        return;
    }

    recordingFinished = true;

    Serial0.println();
    Serial0.println(
        "================================"
    );

    Serial0.println(
        "[TEST] Stopping recording..."
    );

    // --------------------------------------------------------
    // Stop recorder.
    // --------------------------------------------------------

    microphoneManager.stopRecording();

    // --------------------------------------------------------
    // Print result.
    // --------------------------------------------------------

    Serial0.print(
        "[TEST] Recorded bytes: "
    );

    Serial0.println(
        microphoneManager.getRecordedBytes()
    );

    Serial0.print(
        "[TEST] Recorded samples: "
    );

    Serial0.println(
        microphoneManager.getRecordedSamples()
    );

    Serial0.print(
        "[TEST] Duration: "
    );

    Serial0.print(
        microphoneManager.getRecordingDurationMs()
    );

    Serial0.println(
        " ms"
    );

    // --------------------------------------------------------
    // Stop microphone.
    //
    // This releases I2S_NUM_0.
    // --------------------------------------------------------

    microphoneManager.end();

    Serial0.println(
        "[TEST] Microphone stopped"
    );

    Serial0.println(
        "[TEST] I2S_NUM_0 released"
    );

    // --------------------------------------------------------
    // Verify WAV file.
    // --------------------------------------------------------

    fs::FS& fs =
        sdManager.card().fs();

    if (!fs.exists(RECORDING_PATH))
    {
        Serial0.println(
            "[TEST] ERROR: WAV file does not exist"
        );

        testFinished = true;

        return;
    }

    File file =
        fs.open(
            RECORDING_PATH,
            FILE_READ
        );

    if (!file)
    {
        Serial0.println(
            "[TEST] ERROR: cannot open WAV"
        );

        testFinished = true;

        return;
    }

    Serial0.print(
        "[TEST] WAV file size: "
    );

    Serial0.println(
        file.size()
    );

    file.close();

    Serial0.println(
        "================================"
    );

    // --------------------------------------------------------
    // Start speaker.
    // --------------------------------------------------------

    Serial0.println(
        "[TEST] Starting SoundManager..."
    );

    if (!soundManager.begin())
    {
        Serial0.println(
            "[TEST] ERROR: SoundManager.begin() failed"
        );

        testFinished = true;

        return;
    }

    Serial0.println(
        "[TEST] SoundManager ready"
    );

    Serial0.println(
        "[TEST] I2S_NUM_1 speaker ready"
    );

    // --------------------------------------------------------
    // Play recorded WAV.
    // --------------------------------------------------------

    Serial0.println(
        "[TEST] Playing recorded WAV..."
    );

    if (!soundManager.playWav(
            RECORDING_PATH
        ))
    {
        Serial0.println(
            "[TEST] ERROR: playWav() failed"
        );

        soundManager.end();

        testFinished = true;

        return;
    }

    playbackStarted = true;

    Serial0.println(
        "[TEST] PLAYBACK STARTED"
    );

    Serial0.println(
        "================================"
    );
}

// ============================================================
// Finish playback
// ============================================================

void finishPlayback()
{
    if (!playbackStarted)
    {
        return;
    }

    if (soundManager.isPlaying())
    {
        return;
    }

    playbackStarted = false;
    testFinished = true;

    Serial0.println();
    Serial0.println(
        "================================"
    );

    Serial0.println(
        "[TEST] PLAYBACK FINISHED"
    );

    Serial0.print(
        "[TEST] Position: "
    );

    Serial0.print(
        soundManager.getPositionMs()
    );

    Serial0.println(
        " ms"
    );

    Serial0.println(
        "[TEST] SoundManager stopping..."
    );

    soundManager.end();

    Serial0.println(
        "[TEST] SoundManager stopped"
    );

    Serial0.println(
        "[TEST] I2S_NUM_1 released"
    );

    Serial0.println(
        "================================"
    );

    Serial0.println();
    Serial0.println(
        "[TEST] AUDIO TEST COMPLETE"
    );

    Serial0.println();
}

// ============================================================
// Setup
// ============================================================

void setup()
{
    Serial0.begin(115200);

    delay(1000);

    Serial0.println();
    Serial0.println();
    Serial0.println(
        "################################"
    );
    Serial0.println(
        "# ESP32-S3 AUDIO TEST"
    );
    Serial0.println(
        "# I2SManager"
    );
    Serial0.println(
        "# INMP441  -> I2S_NUM_0"
    );
    Serial0.println(
        "# MAX98357 -> I2S_NUM_1"
    );
    Serial0.println(
        "################################"
    );

    // ========================================================
    // SPI
    // ========================================================

    Serial0.println();
    Serial0.println(
        "[TEST] Initializing SPI..."
    );

    if (!spiManager.begin())
    {
        Serial0.println(
            "[TEST] ERROR: SPIManager.begin() failed"
        );

        return;
    }

    Serial0.println(
        "[TEST] SPI ready"
    );

    // ========================================================
    // I2S Manager
    // ========================================================

    Serial0.println();
    Serial0.println(
        "[TEST] Initializing I2SManager..."
    );

    if (!i2sManager.begin())
    {
        Serial0.println(
            "[TEST] ERROR: I2SManager.begin() failed"
        );

        return;
    }

    Serial0.println(
        "[TEST] I2SManager ready"
    );

    Serial0.print(
        "[TEST] Microphone port: I2S_NUM_"
    );

    Serial0.println(
        static_cast<int>(
            i2sManager.microphonePort()
        )
    );

    Serial0.print(
        "[TEST] Speaker port: I2S_NUM_"
    );

    Serial0.println(
        static_cast<int>(
            i2sManager.speakerPort()
        )
    );

    // ========================================================
    // SD
    // ========================================================

    Serial0.println();
    Serial0.println(
        "[TEST] Initializing SD..."
    );

    if (!sdManager.begin(PIN_SD_CS))
    {
        Serial0.println(
            "[TEST] ERROR: SDManager.begin() failed"
        );

        return;
    }

    Serial0.println(
        "[TEST] SD ready"
    );

    printSDInfo();

    // ========================================================
    // Directories
    // ========================================================

    Serial0.println(
        "[TEST] Preparing recording directory..."
    );

    if (!prepareRecordingDirectory())
    {
        return;
    }

    Serial0.println(
        "[TEST] Recording directory ready"
    );

    // ========================================================
    // AudioRecorder
    // ========================================================

    Serial0.println();
    Serial0.println(
        "[TEST] Initializing AudioRecorder..."
    );

    if (!audioRecorder.begin())
    {
        Serial0.println(
            "[TEST] ERROR: AudioRecorder.begin() failed"
        );

        return;
    }

    // ========================================================
    // Start recording
    // ========================================================

    if (!startTestRecording())
    {
        Serial0.println(
            "[TEST] ERROR: recording test could not start"
        );

        return;
    }
}

// ============================================================
// Loop
// ============================================================

void loop()
{
    // ========================================================
    // Microphone / recorder
    // ========================================================

    if (!recordingFinished)
    {
        microphoneManager.update();

        if (
            millis() - recordingStartTime
            >= RECORDING_TIME_MS
        )
        {
            finishRecording();
        }
    }

    // ========================================================
    // Speaker
    // ========================================================

    if (playbackStarted)
    {
        soundManager.update();

        finishPlayback();
    }

    // ========================================================
    // Finished
    // ========================================================

    if (testFinished)
    {
        // Nothing else to do.
        //
        // Keep loop alive.
        delay(100);
    }
    else
    {
        delay(1);
    }
}
