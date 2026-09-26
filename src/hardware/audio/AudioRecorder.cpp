
#include "AudioRecorder.h"

#include <cstring>

#include "Microphone.h"

// ============================================================
// Constructor
// ============================================================

AudioRecorder::AudioRecorder(
    Microphone& microphone,
    fs::FS& filesystem
)
    : microphone(microphone),
      filesystem(filesystem),
      file(),
      recording(false),
      recordedBytes(0),
      recordedSamples(0),
      recordingStartTime(0),
      lastChunkId(0)
{
}

// ============================================================
// Begin
// ============================================================

bool AudioRecorder::begin()
{
    Serial.println("[AudioRecorder] READY");

    return true;
}

// ============================================================
// Start recording
// ============================================================

bool AudioRecorder::startRecording(const char* path)
{
    if (path == nullptr)
    {
        Serial.println(
            "[AudioRecorder] ERROR: path is null"
        );

        return false;
    }

    if (recording)
    {
        Serial.println(
            "[AudioRecorder] ERROR: already recording"
        );

        return false;
    }

    if (!microphone.isInitialized())
    {
        Serial.println(
            "[AudioRecorder] ERROR: microphone is not initialized"
        );

        return false;
    }

    if (!microphone.isEnabled())
    {
        Serial.println(
            "[AudioRecorder] ERROR: microphone is disabled"
        );

        return false;
    }

    if (!microphone.isListening())
    {
        Serial.println(
            "[AudioRecorder] ERROR: microphone is not listening"
        );

        return false;
    }

    // ========================================================
    // Open file
    // ========================================================

    file = filesystem.open(
        path,
        FILE_WRITE
    );

    if (!file)
    {
        Serial.print(
            "[AudioRecorder] ERROR: cannot open file: "
        );

        Serial.println(path);

        return false;
    }

    // ========================================================
    // Reset
    // ========================================================

    recordedBytes = 0;
    recordedSamples = 0;

    recordingStartTime = millis();

    lastChunkId =
        microphone.getAudioChunkId();

    recording = true;

    // ========================================================
    // WAV header
    // ========================================================

    writeWavHeader();

    Serial.print(
        "[AudioRecorder] RECORDING: "
    );

    Serial.println(path);

    return true;
}

// ============================================================
// Stop recording
// ============================================================

void AudioRecorder::stopRecording()
{
    if (!recording)
    {
        return;
    }

    updateWavHeader();

    file.flush();
    file.close();

    recording = false;

    Serial.println(
        "[AudioRecorder] STOPPED"
    );

    Serial.print(
        "[AudioRecorder] Bytes: "
    );

    Serial.println(
        recordedBytes
    );

    Serial.print(
        "[AudioRecorder] Samples: "
    );

    Serial.println(
        recordedSamples
    );

    Serial.print(
        "[AudioRecorder] Duration: "
    );

    Serial.print(
        getRecordingDurationMs()
    );

    Serial.println(
        " ms"
    );
}

// ============================================================
// Is recording
// ============================================================

bool AudioRecorder::isRecording() const
{
    return recording;
}

// ============================================================
// Update
// ============================================================

void AudioRecorder::update()
{
    if (!recording)
    {
        return;
    }

    if (!file)
    {
        recording = false;
        return;
    }

    if (!microphone.hasNewAudio())
    {
        return;
    }

    // ========================================================
    // Check chunk ID
    // ========================================================

    const uint32_t chunkId =
        microphone.getAudioChunkId();

    if (chunkId == lastChunkId)
    {
        return;
    }

    lastChunkId = chunkId;

    // ========================================================
    // Get chunk
    // ========================================================

    const int16_t* samples = nullptr;

    const size_t count =
        microphone.getAudioChunk(samples);

    if (
        samples == nullptr ||
        count == 0
    )
    {
        return;
    }

    // ========================================================
    // Write
    // ========================================================

    writeSamples(
        samples,
        count
    );
}

// ============================================================
// Write samples
// ============================================================

void AudioRecorder::writeSamples(
    const int16_t* samples,
    size_t count
)
{
    if (
        !file ||
        samples == nullptr ||
        count == 0
    )
    {
        return;
    }

    const size_t bytes =
        count * sizeof(int16_t);

    const size_t written =
        file.write(
            reinterpret_cast<const uint8_t*>(
                samples
            ),
            bytes
        );

    if (written != bytes)
    {
        Serial.println(
            "[AudioRecorder] ERROR: incomplete write"
        );

        return;
    }

    recordedBytes += written;

    recordedSamples +=
        static_cast<uint32_t>(count);
}

// ============================================================
// WAV header
// ============================================================

void AudioRecorder::writeWavHeader()
{
    if (!file)
    {
        return;
    }

    // RIFF
    file.write(
        reinterpret_cast<const uint8_t*>(
            "RIFF"
        ),
        4
    );

    // Temporary RIFF size.
    writeLE32(36);

    // WAVE
    file.write(
        reinterpret_cast<const uint8_t*>(
            "WAVE"
        ),
        4
    );

    // fmt
    file.write(
        reinterpret_cast<const uint8_t*>(
            "fmt "
        ),
        4
    );

    // PCM fmt chunk size.
    writeLE32(16);

    // Audio format = PCM.
    writeLE16(1);

    // Channels.
    writeLE16(CHANNELS);

    // Sample rate.
    writeLE32(SAMPLE_RATE);

    // Byte rate.
    const uint32_t byteRate =
        SAMPLE_RATE *
        CHANNELS *
        (BITS_PER_SAMPLE / 8);

    writeLE32(byteRate);

    // Block align.
    const uint16_t blockAlign =
        CHANNELS *
        (BITS_PER_SAMPLE / 8);

    writeLE16(blockAlign);

    // Bits per sample.
    writeLE16(BITS_PER_SAMPLE);

    // data
    file.write(
        reinterpret_cast<const uint8_t*>(
            "data"
        ),
        4
    );

    // Temporary data size.
    writeLE32(0);
}

// ============================================================
// Update WAV header
// ============================================================

void AudioRecorder::updateWavHeader()
{
    if (!file)
    {
        return;
    }

    const uint32_t dataSize =
        recordedBytes;

    const uint32_t riffSize =
        36 + dataSize;

    // RIFF chunk size.
    file.seek(4);

    writeLE32(
        riffSize
    );

    // data chunk size.
    file.seek(40);

    writeLE32(
        dataSize
    );

    // Return to end.
    file.seek(
        44 + dataSize
    );
}

// ============================================================
// Write LE16
// ============================================================

void AudioRecorder::writeLE16(
    uint16_t value
)
{
    uint8_t data[2];

    data[0] =
        static_cast<uint8_t>(
            value & 0xFF
        );

    data[1] =
        static_cast<uint8_t>(
            (value >> 8) & 0xFF
        );

    file.write(
        data,
        2
    );
}

// ============================================================
// Write LE32
// ============================================================

void AudioRecorder::writeLE32(
    uint32_t value
)
{
    uint8_t data[4];

    data[0] =
        static_cast<uint8_t>(
            value & 0xFF
        );

    data[1] =
        static_cast<uint8_t>(
            (value >> 8) & 0xFF
        );

    data[2] =
        static_cast<uint8_t>(
            (value >> 16) & 0xFF
        );

    data[3] =
        static_cast<uint8_t>(
            (value >> 24) & 0xFF
        );

    file.write(
        data,
        4
    );
}

// ============================================================
// Get recorded bytes
// ============================================================

uint32_t AudioRecorder::getRecordedBytes() const
{
    return recordedBytes;
}

// ============================================================
// Get recorded samples
// ============================================================

uint32_t AudioRecorder::getRecordedSamples() const
{
    return recordedSamples;
}

// ============================================================
// Get duration
// ============================================================

uint32_t AudioRecorder::getRecordingDurationMs() const
{
    if (recordedSamples == 0)
    {
        return 0;
    }

    return static_cast<uint32_t>(
        (
            static_cast<uint64_t>(
                recordedSamples
            ) * 1000ULL
        ) /
        SAMPLE_RATE
    );
}
