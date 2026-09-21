#include <Arduino.h>

#include "Settings.h"

#include "./managers/SPIManager.h"

#include "./managers/SDManager.h"
#include "./managers/MicrophoneManager.h"
#include "./managers/SoundManager.h"

#include "./hardware/audio/Microphone.h"
#include "./hardware/audio/AudioRecorder.h"


// ============================================================
// HARDWARE
// ============================================================

static constexpr uint8_t SD_CS_PIN = 35;


// ============================================================
// RECORDING TEST
// ============================================================

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

uint32_t recordingStartTime = 0;


// ============================================================
// FINISH RECORDING
// ============================================================

void finishRecording()
{
    if (recordingFinished)
        return;

    recordingFinished = true;

    Serial0.println();
    Serial0.println("==============================");
    Serial0.println(" FINISH RECORDING");
    Serial0.println("==============================");


    // --------------------------------------------------------
    // STOP RECORDING
    // --------------------------------------------------------

    microphoneManager.stopRecording();


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

    Serial0.println(" ms");


    // --------------------------------------------------------
    // STOP MICROPHONE
    // --------------------------------------------------------

    microphoneManager.stopListening();


    // --------------------------------------------------------
    // CHECK FILE
    // --------------------------------------------------------

    if (!sdManager.card().exists(RECORDING_PATH))
    {
        Serial0.println(
            "[TEST] ERROR: WAV file does not exist"
        );

        return;
    }


    File file =
        sdManager.card().fs().open(
            RECORDING_PATH,
            FILE_READ
        );


    if (!file)
    {
        Serial0.println(
            "[TEST] ERROR: cannot open WAV file"
        );

        return;
    }


    Serial0.print(
        "[TEST] WAV file size: "
    );

    Serial0.println(
        file.size()
    );

    file.close();


    // --------------------------------------------------------
    // START SOUND SYSTEM
    // --------------------------------------------------------

    Serial0.println();

    Serial0.println(
        "[TEST] Starting SoundManager..."
    );


    if (!soundManager.begin())
    {
        Serial0.println(
            "[TEST] ERROR: SoundManager.begin() failed"
        );

        return;
    }


    // --------------------------------------------------------
    // PLAY RECORDING
    // --------------------------------------------------------

    Serial0.println();

    Serial0.println(
        "[TEST] Playing recorded WAV..."
    );


    if (!soundManager.playWav(
        RECORDING_PATH
    ))
    {
        Serial0.println(
            "[TEST] ERROR: failed to play WAV"
        );

        return;
    }


    Serial0.println(
        "[TEST] PLAYBACK STARTED"
    );
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);

    delay(1000);

    Serial0.println();
    Serial0.println("==============================");
    Serial0.println(" SMART CLOCK AUDIO TEST");
    Serial0.println("==============================");


    // ========================================================
    // SPI
    // ========================================================

    Serial0.println(
        "[TEST] Starting SPI..."
    );

    spiManager.begin();


    // ========================================================
    // SD
    // ========================================================

    Serial0.println(
        "[TEST] Starting SD..."
    );

    if (!sdManager.begin(SD_CS_PIN))
    {
        Serial0.println(
            "[TEST] ERROR: SD initialization failed"
        );

        return;
    }


    Serial0.println(
        "[TEST] SD READY"
    );


    // ========================================================
    // MICROPHONE
    // ========================================================

    Serial0.println(
        "[TEST] Starting microphone..."
    );

    if (!microphoneManager.begin())
    {
        Serial0.println(
            "[TEST] ERROR: microphone initialization failed"
        );

        return;
    }


    Serial0.println(
        "[TEST] MICROPHONE READY"
    );


    // ========================================================
    // LISTENING
    // ========================================================

    if (!microphoneManager.startListening())
    {
        Serial0.println(
            "[TEST] ERROR: failed to start microphone"
        );

        return;
    }


    // ========================================================
    // START RECORDING
    // ========================================================

    if (!microphoneManager.startRecording(
        RECORDING_PATH
    ))
    {
        Serial0.println(
            "[TEST] ERROR: failed to start recording"
        );

        return;
    }


    recordingStartTime = millis();

    Serial0.println();
    Serial0.println(
        "[TEST] RECORDING STARTED"
    );
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // MICROPHONE + RECORDER
    // --------------------------------------------------------

    if (!recordingFinished)
    {
        microphoneManager.update();


        // ----------------------------------------------------
        // DEBUG LEVEL
        // ----------------------------------------------------

        static uint32_t lastPrint = 0;

        if (millis() - lastPrint >= 500)
        {
            lastPrint = millis();

            Serial0.print(
                "[MIC] RMS="
            );

            Serial0.print(
                microphoneManager.getRMS()
            );

            Serial0.print(
                " LEVEL="
            );

            Serial0.print(
                microphoneManager.getLevel()
            );

            Serial0.print(
                " PEAK="
            );

            Serial0.println(
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


    // --------------------------------------------------------
    // PLAYBACK
    // --------------------------------------------------------

    soundManager.update();
}
