#include <Arduino.h>

#include "Settings.h"

#include "./managers/SPIManager.h"

#include "./managers/SDManager.h"
#include "./managers/MicrophoneManager.h"
#include "./managers/SoundManager.h"

#include "./hardware/audio/Microphone.h"
#include "./hardware/audio/AudioRecorder.h"


// ============================================================
// CONFIG
// ============================================================

static constexpr uint8_t SD_CS_PIN = 35;

static constexpr char RECORDING_PATH[] =
    "/audio/recordings/test.wav";

static constexpr uint32_t RECORDING_DURATION_MS =
    5000;


// ============================================================
// GLOBAL OBJECTS
// ============================================================

Settings::Data settings;

SPIManager spiManager;

SDManager sdManager;

Microphone microphone;

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
    settings.audio
);


// ============================================================
// TEST STATE
// ============================================================

bool recordingFinished = false;

bool recordingStarted = false;

bool playbackStarted = false;

uint32_t recordingStartTime = 0;


// ============================================================
// CREATE DIRECTORIES
// ============================================================

bool prepareRecordingDirectory()
{
    fs::FS& fs = sdManager.card().fs();

    Serial.println();
    Serial.println(
        "[TEST] Preparing recording directory..."
    );


    // --------------------------------------------------------
    // /audio
    // --------------------------------------------------------

    if (!fs.exists("/audio"))
    {
        Serial.println(
            "[TEST] Creating /audio"
        );

        if (!fs.mkdir("/audio"))
        {
            Serial.println(
                "[TEST] ERROR: failed to create /audio"
            );

            return false;
        }
    }


    // --------------------------------------------------------
    // /audio/recordings
    // --------------------------------------------------------

    if (!fs.exists("/audio/recordings"))
    {
        Serial.println(
            "[TEST] Creating /audio/recordings"
        );

        if (!fs.mkdir("/audio/recordings"))
        {
            Serial.println(
                "[TEST] ERROR: failed to create "
                "/audio/recordings"
            );

            return false;
        }
    }


    Serial.println(
        "[TEST] Recording directory READY"
    );

    return true;
}


// ============================================================
// FINISH RECORDING
// ============================================================

void finishRecording()
{
    if (recordingFinished)
        return;

    recordingFinished = true;

    Serial.println();
    Serial.println("==============================");
    Serial.println(" FINISH RECORDING");
    Serial.println("==============================");


    // --------------------------------------------------------
    // STOP RECORDING
    // --------------------------------------------------------

    if (microphoneManager.isRecording())
    {
        microphoneManager.stopRecording();
    }


    // --------------------------------------------------------
    // PRINT RECORDING INFO
    // --------------------------------------------------------

    Serial.print(
        "[TEST] Recorded bytes: "
    );

    Serial.println(
        microphoneManager.getRecordedBytes()
    );


    Serial.print(
        "[TEST] Recorded samples: "
    );

    Serial.println(
        microphoneManager.getRecordedSamples()
    );


    Serial.print(
        "[TEST] Duration: "
    );

    Serial.print(
        microphoneManager.getRecordingDurationMs()
    );

    Serial.println(" ms");


    // --------------------------------------------------------
    // STOP MICROPHONE
    // --------------------------------------------------------

    microphoneManager.stopListening();


    // --------------------------------------------------------
    // CHECK FILE
    // --------------------------------------------------------

    fs::FS& fs =
        sdManager.card().fs();


    if (!fs.exists(RECORDING_PATH))
    {
        Serial.println();
        Serial.println(
            "[TEST] ERROR: WAV file does not exist"
        );

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
            "[TEST] ERROR: cannot open WAV file"
        );

        return;
    }


    Serial.print(
        "[TEST] WAV file size: "
    );

    Serial.println(
        file.size()
    );


    file.close();


    // --------------------------------------------------------
    // START SOUND MANAGER
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[TEST] Starting SoundManager..."
    );


    if (!soundManager.begin())
    {
        Serial.println(
            "[TEST] ERROR: SoundManager.begin() failed"
        );

        return;
    }


    Serial.println(
        "[TEST] SoundManager READY"
    );


    // --------------------------------------------------------
    // PLAY WAV
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[TEST] Playing recorded WAV..."
    );


    if (!soundManager.playWav(
        RECORDING_PATH
    ))
    {
        Serial.println(
            "[TEST] ERROR: failed to play WAV"
        );

        return;
    }


    playbackStarted = true;


    Serial.println(
        "[TEST] PLAYBACK STARTED"
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
    Serial.println("==============================");
    Serial.println(" SMART CLOCK AUDIO TEST");
    Serial.println("==============================");


    // ========================================================
    // SPI
    // ========================================================

    Serial.println(
        "[TEST] Starting SPI..."
    );

    spiManager.begin();


    // ========================================================
    // SD
    // ========================================================

    Serial.println(
        "[TEST] Starting SD..."
    );


    if (!sdManager.begin(SD_CS_PIN))
    {
        Serial.println(
            "[TEST] ERROR: SD initialization failed"
        );

        return;
    }


    Serial.println(
        "[TEST] SD READY"
    );


    // ========================================================
    // DIRECTORY
    // ========================================================

    if (!prepareRecordingDirectory())
    {
        Serial.println(
            "[TEST] ERROR: failed to prepare "
            "recording directory"
        );

        return;
    }


    // ========================================================
    // MICROPHONE
    // ========================================================

    Serial.println(
        "[TEST] Starting microphone..."
    );


    if (!microphoneManager.begin())
    {
        Serial.println(
            "[TEST] ERROR: microphone initialization failed"
        );

        return;
    }


    Serial.println(
        "[TEST] MICROPHONE READY"
    );


    // ========================================================
    // LISTENING
    // ========================================================

    if (!microphoneManager.startListening())
    {
        Serial.println(
            "[TEST] ERROR: failed to start microphone"
        );

        return;
    }


    // ========================================================
    // RECORDING
    // ========================================================

    if (!microphoneManager.startRecording(
        RECORDING_PATH
    ))
    {
        Serial.println(
            "[TEST] ERROR: failed to start recording"
        );

        return;
    }


    recordingStarted = true;

    recordingStartTime = millis();


    Serial.println();
    Serial.println(
        "[TEST] =========================="
    );

    Serial.println(
        "[TEST] RECORDING STARTED"
    );

    Serial.println(
        "[TEST] =========================="
    );
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    // ========================================================
    // RECORDING
    // ========================================================

    if (
        recordingStarted &&
        !recordingFinished
    )
    {
        microphoneManager.update();


        // ----------------------------------------------------
        // MICROPHONE DEBUG
        // ----------------------------------------------------

        static uint32_t lastPrint = 0;


        if (
            millis() - lastPrint >= 500
        )
        {
            lastPrint = millis();


            Serial.print(
                "[MIC] RMS="
            );

            Serial.print(
                microphoneManager.getRMS()
            );


            Serial.print(
                " LEVEL="
            );

            Serial.print(
                microphoneManager.getLevel()
            );


            Serial.print(
                " PEAK="
            );

            Serial.println(
                microphoneManager.getPeak()
            );
        }


        // ----------------------------------------------------
        // RECORDING TIME
        // ----------------------------------------------------

        if (
            millis() - recordingStartTime
            >= RECORDING_DURATION_MS
        )
        {
            finishRecording();
        }
    }


    // ========================================================
    // PLAYBACK
    // ========================================================

    if (playbackStarted)
    {
        soundManager.update();
    }


    // ========================================================
    // SMALL DELAY
    // ========================================================

    delay(1);
}
