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
// Test configuration
// ============================================================

static constexpr char RECORDING_PATH[] =
    "/audio/recordings/test.wav";

static constexpr char RECORDING_PATH_ALT[] =
    "/recordings/test.wav";

static constexpr uint32_t RECORDING_TIME_MS =
    5000;

static constexpr uint32_t SPEAKER_TEST_TIME_MS =
    3000;

static constexpr uint32_t SPEAKER_TEST_SAMPLE_RATE =
    16000;

// ============================================================
// Test state
// ============================================================

enum class TestStage : uint8_t
{
    START,
    RECORDING,
    AFTER_RECORDING,
    WAV_CHECK,
    SPEAKER_I2S_CHECK,
    SPEAKER_DIRECT_TEST,
    SOUND_MANAGER_TEST,
    COMPLETE,
    ERROR
};

TestStage testStage = TestStage::START;

uint32_t recordingStartTime = 0;
uint32_t speakerTestStartTime = 0;

bool recordingStarted = false;
bool speakerDirectStarted = false;
bool soundManagerStarted = false;

// ============================================================
// Utility
// ============================================================

void printLine()
{
    Serial0.println(
        "========================================"
    );
}

void printTestHeader(
    const char* title
)
{
    Serial0.println();
    printLine();

    Serial0.print("[TEST] ");
    Serial0.println(title);

    printLine();
}

// ============================================================
// SD directories
// ============================================================

bool prepareDirectories()
{
    printTestHeader(
        "Preparing SD directories"
    );

    fs::FS& fs =
        sdManager.card().fs();

    if (!fs.exists("/audio"))
    {
        Serial0.println(
            "[SD] Creating /audio"
        );

        if (!fs.mkdir("/audio"))
        {
            Serial0.println(
                "[SD] ERROR: cannot create /audio"
            );

            return false;
        }
    }

    if (!fs.exists("/audio/recordings"))
    {
        Serial0.println(
            "[SD] Creating /audio/recordings"
        );

        if (!fs.mkdir("/audio/recordings"))
        {
            Serial0.println(
                "[SD] ERROR: cannot create /audio/recordings"
            );

            return false;
        }
    }

    Serial0.println(
        "[SD] Directory check OK"
    );

    return true;
}

// ============================================================
// SD info
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
        "Total bytes: "
    );

    Serial0.println(
        info.totalBytes
    );

    Serial0.print(
        "Used bytes: "
    );

    Serial0.println(
        info.usedBytes
    );

    Serial0.println(
        "============================="
    );
}

// ============================================================
// File test
// ============================================================

bool checkRecordingFile()
{
    printTestHeader(
        "Checking WAV file"
    );

    fs::FS& fs =
        sdManager.card().fs();

    // --------------------------------------------------------
    // Check expected path
    // --------------------------------------------------------

    Serial0.print(
        "[WAV] Expected path: "
    );

    Serial0.println(
        RECORDING_PATH
    );

    bool existsMain =
        fs.exists(RECORDING_PATH);

    Serial0.print(
        "[WAV] Exists: "
    );

    Serial0.println(
        existsMain
            ? "YES"
            : "NO"
    );

    // --------------------------------------------------------
    // Check alternative path
    // --------------------------------------------------------

    Serial0.print(
        "[WAV] Alternative path: "
    );

    Serial0.println(
        RECORDING_PATH_ALT
    );

    bool existsAlt =
        fs.exists(RECORDING_PATH_ALT);

    Serial0.print(
        "[WAV] Exists alternative: "
    );

    Serial0.println(
        existsAlt
            ? "YES"
            : "NO"
    );

    // --------------------------------------------------------
    // Select path
    // --------------------------------------------------------

    const char* path = nullptr;

    if (existsMain)
    {
        path = RECORDING_PATH;
    }
    else if (existsAlt)
    {
        path = RECORDING_PATH_ALT;
    }
    else
    {
        Serial0.println(
            "[WAV] ERROR: no recording file found"
        );

        return false;
    }

    Serial0.print(
        "[WAV] Using file: "
    );

    Serial0.println(
        path
    );

    // --------------------------------------------------------
    // Open file
    // --------------------------------------------------------

    File file =
        fs.open(
            path,
            FILE_READ
        );

    if (!file)
    {
        Serial0.println(
            "[WAV] ERROR: cannot open file"
        );

        return false;
    }

    size_t fileSize =
        file.size();

    Serial0.print(
        "[WAV] File size: "
    );

    Serial0.println(
        fileSize
    );

    if (fileSize < 44)
    {
        Serial0.println(
            "[WAV] ERROR: file smaller than WAV header"
        );

        file.close();

        return false;
    }

    // --------------------------------------------------------
    // Read header
    // --------------------------------------------------------

    uint8_t header[44];

    size_t bytesRead =
        file.read(
            header,
            sizeof(header)
        );

    file.close();

    Serial0.print(
        "[WAV] Header bytes read: "
    );

    Serial0.println(
        bytesRead
    );

    if (bytesRead != 44)
    {
        Serial0.println(
            "[WAV] ERROR: cannot read complete header"
        );

        return false;
    }

    // --------------------------------------------------------
    // Print raw header
    // --------------------------------------------------------

    Serial0.println(
        "[WAV] Header:"
    );

    for (int i = 0; i < 44; i++)
    {
        if (header[i] < 16)
        {
            Serial0.print('0');
        }

        Serial0.print(
            header[i],
            HEX
        );

        Serial0.print(' ');

        if ((i + 1) % 16 == 0)
        {
            Serial0.println();
        }
    }

    // --------------------------------------------------------
    // RIFF
    // --------------------------------------------------------

    if (memcmp(
            header,
            "RIFF",
            4
        ) != 0)
    {
        Serial0.println(
            "[WAV] ERROR: missing RIFF"
        );

        return false;
    }

    // --------------------------------------------------------
    // WAVE
    // --------------------------------------------------------

    if (memcmp(
            header + 8,
            "WAVE",
            4
        ) != 0)
    {
        Serial0.println(
            "[WAV] ERROR: missing WAVE"
        );

        return false;
    }

    // --------------------------------------------------------
    // fmt
    // --------------------------------------------------------

    if (memcmp(
            header + 12,
            "fmt ",
            4
        ) != 0)
    {
        Serial0.println(
            "[WAV] ERROR: missing fmt chunk"
        );

        return false;
    }

    // --------------------------------------------------------
    // Parse standard PCM WAV header
    // --------------------------------------------------------

    uint16_t audioFormat =
        static_cast<uint16_t>(
            header[20] |
            (header[21] << 8)
        );

    uint16_t channels =
        static_cast<uint16_t>(
            header[22] |
            (header[23] << 8)
        );

    uint32_t sampleRate =
        static_cast<uint32_t>(
            header[24] |
            (header[25] << 8) |
            (header[26] << 16) |
            (header[27] << 24)
        );

    uint32_t byteRate =
        static_cast<uint32_t>(
            header[28] |
            (header[29] << 8) |
            (header[30] << 16) |
            (header[31] << 24)
        );

    uint16_t blockAlign =
        static_cast<uint16_t>(
            header[32] |
            (header[33] << 8)
        );

    uint16_t bitsPerSample =
        static_cast<uint16_t>(
            header[34] |
            (header[35] << 8)
        );

    Serial0.println();
    Serial0.println(
        "[WAV] Parsed format:"
    );

    Serial0.print(
        "  Audio format: "
    );

    Serial0.println(
        audioFormat
    );

    Serial0.print(
        "  Channels: "
    );

    Serial0.println(
        channels
    );

    Serial0.print(
        "  Sample rate: "
    );

    Serial0.println(
        sampleRate
    );

    Serial0.print(
        "  Byte rate: "
    );

    Serial0.println(
        byteRate
    );

    Serial0.print(
        "  Block align: "
    );

    Serial0.println(
        blockAlign
    );

    Serial0.print(
        "  Bits/sample: "
    );

    Serial0.println(
        bitsPerSample
    );

    // --------------------------------------------------------
    // Expected PCM
    // --------------------------------------------------------

    if (audioFormat != 1)
    {
        Serial0.println(
            "[WAV] ERROR: not PCM"
        );

        return false;
    }

    if (channels != 1)
    {
        Serial0.println(
            "[WAV] WARNING: WAV is not mono"
        );
    }

    if (bitsPerSample != 16)
    {
        Serial0.println(
            "[WAV] ERROR: not 16-bit"
        );

        return false;
    }

    if (sampleRate != 16000)
    {
        Serial0.println(
            "[WAV] WARNING: sample rate is not 16000"
        );
    }

    Serial0.println(
        "[WAV] Basic WAV validation OK"
    );

    return true;
}

// ============================================================
// Direct speaker I2S test
// ============================================================

bool startDirectSpeakerTest()
{
    printTestHeader(
        "Direct MAX98357 I2S test"
    );

    Serial0.println(
        "[SPEAKER] Checking I2S_NUM_1"
    );

    if (!i2sManager.isSpeakerInitialized())
    {
        Serial0.println(
            "[SPEAKER] Speaker I2S is not initialized"
        );

        return false;
    }

    i2s_port_t port =
        i2sManager.speakerPort();

    Serial0.print(
        "[SPEAKER] Port: I2S_NUM_"
    );

    Serial0.println(
        static_cast<int>(port)
    );

    Serial0.print(
        "[SPEAKER] Sample rate: "
    );

    Serial0.println(
        SPEAKER_TEST_SAMPLE_RATE
    );

    // --------------------------------------------------------
    // Generate a small 440 Hz sine wave
    // --------------------------------------------------------

    static constexpr size_t SAMPLES = 256;

    static int16_t buffer[SAMPLES];

    static constexpr float FREQUENCY = 440.0f;

    for (size_t i = 0; i < SAMPLES; i++)
    {
        float phase =
            2.0f *
            PI *
            FREQUENCY *
            static_cast<float>(i) /
            static_cast<float>(SPEAKER_TEST_SAMPLE_RATE);

        buffer[i] =
            static_cast<int16_t>(
                sinf(phase) * 5000.0f
            );
    }

    size_t bytesWritten = 0;

    esp_err_t result =
        i2s_write(
            port,
            buffer,
            sizeof(buffer),
            &bytesWritten,
            pdMS_TO_TICKS(100)
        );

    Serial0.print(
        "[SPEAKER] i2s_write result: "
    );

    Serial0.println(
        static_cast<int>(result)
    );

    Serial0.print(
        "[SPEAKER] Bytes written: "
    );

    Serial0.println(
        bytesWritten
    );

    if (result != ESP_OK)
    {
        Serial0.println(
            "[SPEAKER] ERROR: i2s_write failed"
        );

        return false;
    }

    if (bytesWritten == 0)
    {
        Serial0.println(
            "[SPEAKER] ERROR: zero bytes written"
        );

        return false;
    }

    Serial0.println(
        "[SPEAKER] Direct I2S write OK"
    );

    speakerTestStartTime =
        millis();

    speakerDirectStarted = true;

    return true;
}

// ============================================================
// Continue direct speaker test
// ============================================================

void updateDirectSpeakerTest()
{
    if (!speakerDirectStarted)
    {
        return;
    }

    i2s_port_t port =
        i2sManager.speakerPort();

    static constexpr size_t SAMPLES = 256;

    static int16_t buffer[SAMPLES];

    static uint32_t phaseIndex = 0;

    static constexpr float FREQUENCY = 440.0f;

    for (size_t i = 0; i < SAMPLES; i++)
    {
        float phase =
            2.0f *
            PI *
            FREQUENCY *
            static_cast<float>(phaseIndex++) /
            static_cast<float>(SPEAKER_TEST_SAMPLE_RATE);

        buffer[i] =
            static_cast<int16_t>(
                sinf(phase) * 5000.0f
            );
    }

    size_t bytesWritten = 0;

    esp_err_t result =
        i2s_write(
            port,
            buffer,
            sizeof(buffer),
            &bytesWritten,
            pdMS_TO_TICKS(100)
        );

    if (result != ESP_OK)
    {
        Serial0.print(
            "[SPEAKER] ERROR during playback: "
        );

        Serial0.println(
            static_cast<int>(result)
        );

        speakerDirectStarted = false;
        testStage = TestStage::ERROR;

        return;
    }

    if (
        millis() - speakerTestStartTime
        >= SPEAKER_TEST_TIME_MS
    )
    {
        Serial0.println(
            "[SPEAKER] Direct test finished"
        );

        speakerDirectStarted = false;

        i2sManager.endSpeaker();

        Serial0.println(
            "[SPEAKER] I2S_NUM_1 released"
        );

        testStage =
            TestStage::SOUND_MANAGER_TEST;
    }
}

// ============================================================
// SoundManager test
// ============================================================

bool startSoundManagerTest()
{
    printTestHeader(
        "SoundManager test"
    );

    // --------------------------------------------------------
    // Start speaker again
    // --------------------------------------------------------

    Serial0.println(
        "[SOUND] Starting SoundManager..."
    );

    if (!soundManager.begin())
    {
        Serial0.println(
            "[SOUND] ERROR: SoundManager.begin() failed"
        );

        return false;
    }

    Serial0.println(
        "[SOUND] SoundManager.begin() OK"
    );

    Serial0.print(
        "[SOUND] Speaker initialized: "
    );

    Serial0.println(
        i2sManager.isSpeakerInitialized()
            ? "YES"
            : "NO"
    );

    // --------------------------------------------------------
    // Check WAV again
    // --------------------------------------------------------

    Serial0.println(
        "[SOUND] Calling playWav()..."
    );

    Serial0.print(
        "[SOUND] Path: "
    );

    Serial0.println(
        RECORDING_PATH
    );

    bool result =
        soundManager.playWav(
            RECORDING_PATH
        );

    Serial0.print(
        "[SOUND] playWav() returned: "
    );

    Serial0.println(
        result
            ? "TRUE"
            : "FALSE"
    );

    if (!result)
    {
        Serial0.println(
            "[SOUND] ERROR: playWav() failed"
        );

        Serial0.println(
            "[SOUND] The failure is inside SoundManager"
        );

        return false;
    }

    Serial0.println(
        "[SOUND] PLAYBACK STARTED"
    );

    soundManagerStarted = true;

    return true;
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
        "############################################"
    );

    Serial0.println(
        "# ESP32-S3 AUDIO DIAGNOSTIC TEST"
    );

    Serial0.println(
        "# INMP441  -> I2S_NUM_0"
    );

    Serial0.println(
        "# MAX98357 -> I2S_NUM_1"
    );

    Serial0.println(
        "############################################"
    );

    // ========================================================
    // SPI
    // ========================================================

    printTestHeader(
        "Initializing SPIManager"
    );

    if (!spiManager.begin())
    {
        Serial0.println(
            "[SPI] ERROR: SPIManager.begin() failed"
        );

        testStage =
            TestStage::ERROR;

        return;
    }

    Serial0.println(
        "[SPI] OK"
    );

    // ========================================================
    // I2S Manager
    // ========================================================

    printTestHeader(
        "Initializing I2SManager"
    );

    if (!i2sManager.begin())
    {
        Serial0.println(
            "[I2S] ERROR: I2SManager.begin() failed"
        );

        testStage =
            TestStage::ERROR;

        return;
    }

    Serial0.println(
        "[I2S] Manager OK"
    );

    Serial0.print(
        "[I2S] Microphone port: I2S_NUM_"
    );

    Serial0.println(
        static_cast<int>(
            i2sManager.microphonePort()
        )
    );

    Serial0.print(
        "[I2S] Speaker port: I2S_NUM_"
    );

    Serial0.println(
        static_cast<int>(
            i2sManager.speakerPort()
        )
    );

    Serial0.print(
        "[I2S] Mic initialized: "
    );

    Serial0.println(
        i2sManager.isMicrophoneInitialized()
            ? "YES"
            : "NO"
    );

    Serial0.print(
        "[I2S] Speaker initialized: "
    );

    Serial0.println(
        i2sManager.isSpeakerInitialized()
            ? "YES"
            : "NO"
    );

    // ========================================================
    // SD
    // ========================================================

    printTestHeader(
        "Initializing SDManager"
    );

    if (!sdManager.begin(PIN_SD_CS))
    {
        Serial0.println(
            "[SD] ERROR: SDManager.begin() failed"
        );

        testStage =
            TestStage::ERROR;

        return;
    }

    Serial0.println(
        "[SD] OK"
    );

    printSDInfo();

    // ========================================================
    // Directories
    // ========================================================

    if (!prepareDirectories())
    {
        testStage =
            TestStage::ERROR;

        return;
    }

    // ========================================================
    // AudioRecorder
    // ========================================================

    printTestHeader(
        "Initializing AudioRecorder"
    );

    if (!audioRecorder.begin())
    {
        Serial0.println(
            "[REC] ERROR: AudioRecorder.begin() failed"
        );

        testStage =
            TestStage::ERROR;

        return;
    }

    Serial0.println(
        "[REC] AudioRecorder OK"
    );

    // ========================================================
    // Start recording
    // ========================================================

    printTestHeader(
        "Starting microphone recording"
    );

    if (!microphoneManager.begin())
    {
        Serial0.println(
            "[MIC] ERROR: MicrophoneManager.begin() failed"
        );

        testStage =
            TestStage::ERROR;

        return;
    }

    Serial0.println(
        "[MIC] MicrophoneManager OK"
    );

    Serial0.print(
        "[MIC] I2S initialized: "
    );

    Serial0.println(
        i2sManager.isMicrophoneInitialized()
            ? "YES"
            : "NO"
    );

    microphoneManager.setEnabled(true);

    if (!microphoneManager.startListening())
    {
        Serial0.println(
            "[MIC] ERROR: startListening() failed"
        );

        microphoneManager.end();

        testStage =
            TestStage::ERROR;

        return;
    }

    Serial0.println(
        "[MIC] Listening started"
    );

    Serial0.print(
        "[MIC] Recording path: "
    );

    Serial0.println(
        RECORDING_PATH
    );

    if (!microphoneManager.startRecording(
            RECORDING_PATH
        ))
    {
        Serial0.println(
            "[MIC] ERROR: startRecording() failed"
        );

        microphoneManager.stopListening();
        microphoneManager.end();

        testStage =
            TestStage::ERROR;

        return;
    }

    recordingStarted = true;

    recordingStartTime =
        millis();

    Serial0.println(
        "[MIC] RECORDING STARTED"
    );

    testStage =
        TestStage::RECORDING;
}

// ============================================================
// Finish recording
// ============================================================

void finishRecording()
{
    if (!recordingStarted)
    {
        return;
    }

    recordingStarted = false;

    printTestHeader(
        "Finishing microphone recording"
    );

    microphoneManager.stopRecording();

    Serial0.print(
        "[REC] Recorded bytes: "
    );

    Serial0.println(
        microphoneManager.getRecordedBytes()
    );

    Serial0.print(
        "[REC] Recorded samples: "
    );

    Serial0.println(
        microphoneManager.getRecordedSamples()
    );

    Serial0.print(
        "[REC] Duration: "
    );

    Serial0.print(
        microphoneManager.getRecordingDurationMs()
    );

    Serial0.println(
        " ms"
    );

    // --------------------------------------------------------
    // Stop microphone
    // --------------------------------------------------------

    microphoneManager.stopListening();

    microphoneManager.end();

    Serial0.println(
        "[MIC] Microphone stopped"
    );

    Serial0.print(
        "[I2S] I2S_NUM_0 initialized: "
    );

    Serial0.println(
        i2sManager.isMicrophoneInitialized()
            ? "YES"
            : "NO"
    );

    // --------------------------------------------------------
    // WAV check
    // --------------------------------------------------------

    if (!checkRecordingFile())
    {
        testStage =
            TestStage::ERROR;

        return;
    }

    testStage =
        TestStage::SPEAKER_I2S_CHECK;
}

// ============================================================
// Speaker I2S initialization
// ============================================================

bool startSpeakerI2SCheck()
{
    printTestHeader(
        "Initializing speaker I2S"
    );

    Serial0.println(
        "[SPEAKER] Calling beginSpeaker(16000)"
    );

    if (!i2sManager.beginSpeaker(
            SPEAKER_TEST_SAMPLE_RATE
        ))
    {
        Serial0.println(
            "[SPEAKER] ERROR: beginSpeaker() failed"
        );

        return false;
    }

    Serial0.println(
        "[SPEAKER] beginSpeaker() OK"
    );

    Serial0.print(
        "[SPEAKER] Initialized: "
    );

    Serial0.println(
        i2sManager.isSpeakerInitialized()
            ? "YES"
            : "NO"
    );

    return true;
}

// ============================================================
// Update
// ============================================================

void loop()
{
    // ========================================================
    // Recording
    // ========================================================

    if (
        testStage == TestStage::RECORDING
    )
    {
        microphoneManager.update();

        if (
            millis() - recordingStartTime
            >= RECORDING_TIME_MS
        )
        {
            finishRecording();
        }

        delay(1);

        return;
    }

    // ========================================================
    // Speaker I2S check
    // ========================================================

    if (
        testStage == TestStage::SPEAKER_I2S_CHECK
    )
    {
        if (!startSpeakerI2SCheck())
        {
            testStage =
                TestStage::ERROR;

            return;
        }

        if (!startDirectSpeakerTest())
        {
            testStage =
                TestStage::ERROR;

            return;
        }

        testStage =
            TestStage::SPEAKER_DIRECT_TEST;

        return;
    }

    // ========================================================
    // Direct speaker
    // ========================================================

    if (
        testStage == TestStage::SPEAKER_DIRECT_TEST
    )
    {
        updateDirectSpeakerTest();

        delay(1);

        return;
    }

    // ========================================================
    // SoundManager
    // ========================================================

    if (
        testStage == TestStage::SOUND_MANAGER_TEST
    )
    {
        if (!startSoundManagerTest())
        {
            testStage =
                TestStage::ERROR;

            return;
        }

        testStage =
            TestStage::COMPLETE;

        return;
    }

    // ========================================================
    // SoundManager playback
    // ========================================================

    if (
        soundManagerStarted
    )
    {
        soundManager.update();

        if (!soundManager.isPlaying())
        {
            Serial0.println();
            printLine();

            Serial0.println(
                "[SOUND] PLAYBACK FINISHED"
            );

            Serial0.print(
                "[SOUND] Position: "
            );

            Serial0.print(
                soundManager.getPositionMs()
            );

            Serial0.println(
                " ms"
            );

            printLine();

            soundManager.end();

            soundManagerStarted = false;

            Serial0.println(
                "[SOUND] SoundManager stopped"
            );

            Serial0.println(
                "[SOUND] I2S_NUM_1 released"
            );

            Serial0.println();
            Serial0.println(
                "[TEST] AUDIO DIAGNOSTIC COMPLETE"
            );

            testStage =
                TestStage::COMPLETE;
        }

        delay(1);

        return;
    }

    // ========================================================
    // Complete
    // ========================================================

    if (
        testStage == TestStage::COMPLETE
    )
    {
        delay(100);

        return;
    }

    // ========================================================
    // Error
    // ========================================================

    if (
        testStage == TestStage::ERROR
    )
    {
        Serial0.println(
            "[TEST] ERROR STATE"
        );

        delay(1000);

        return;
    }

    delay(1);
}
