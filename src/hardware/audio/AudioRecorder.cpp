
#include "AudioRecorder.h"

#include <Arduino.h>
#include <cstring>

#include "Microphone.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

AudioRecorder::AudioRecorder(
    Microphone& microphone,
    fs::FS& filesystem
)
    : microphone(microphone),
      filesystem(filesystem),
      file(),
      initialized(false),
      recording(false),
      error(false),
      recordedBytes(0),
      recordedSamples(0),
      recordingStartTime(0),
      lastChunkId(0),
      audioBuffer{}
{
}

// ============================================================
// BEGIN
// ============================================================

bool AudioRecorder::begin()
{
    if (initialized)
        return true;

    initialized = true;

    Serial0.println(
        "[AudioRecorder] Initialized"
    );

    return true;
}

// ============================================================
// END
// ============================================================

void AudioRecorder::end()
{
    if (!initialized)
        return;

    stopRecording();

    initialized = false;

    Serial0.println(
        "[AudioRecorder] Stopped"
    );
}

// ============================================================
// START RECORDING
// ============================================================

bool AudioRecorder::startRecording(
    const char* path
)
{
    if (!initialized)
    {
        Serial0.println(
            "[AudioRecorder][ERROR] Not initialized"
        );

        return false;
    }

    if (path == nullptr || path[0] == '\0')
    {
        Serial0.println(
            "[AudioRecorder][ERROR] Invalid path"
        );

        return false;
    }

    if (recording)
    {
        Serial0.println(
            "[AudioRecorder][ERROR] Already recording"
        );

        return false;
    }

    if (!microphone.isInitialized() ||
        !microphone.isEnabled() ||
        !microphone.isListening())
    {
        Serial0.println(
            "[AudioRecorder][ERROR] Microphone unavailable"
        );

        return false;
    }

    // --------------------------------------------------------
    // OPEN FILE
    // --------------------------------------------------------

    file = filesystem.open(
        path,
        FILE_WRITE
    );

    if (!file)
    {
        Serial0.printf(
            "[AudioRecorder][ERROR] Cannot open: %s\n",
            path
        );

        return false;
    }

    // --------------------------------------------------------
    // RESET STATE
    // --------------------------------------------------------

    recordedBytes = 0;
    recordedSamples = 0;

    recordingStartTime = millis();

    lastChunkId = microphone.getAudioChunkId();

    error = false;

    // --------------------------------------------------------
    // WRITE WAV HEADER
    // --------------------------------------------------------

    if (!writeWavHeader())
    {
        file.close();

        Serial0.println(
            "[AudioRecorder][ERROR] WAV header failed"
        );

        return false;
    }

    recording = true;

    Serial0.printf(
        "[AudioRecorder] Recording started: %s\n",
        path
    );

    return true;
}

// ============================================================
// STOP RECORDING
// ============================================================

void AudioRecorder::stopRecording()
{
    if (!recording)
        return;

    if (file)
    {
        if (!updateWavHeader())
        {
            error = true;
        }

        file.flush();
        file.close();
    }

    recording = false;

    Serial0.println(
        "[AudioRecorder] Recording stopped"
    );

    Serial0.printf(
        "[AudioRecorder] Bytes: %lu\n",
        static_cast<unsigned long>(recordedBytes)
    );

    Serial0.printf(
        "[AudioRecorder] Samples: %lu\n",
        static_cast<unsigned long>(recordedSamples)
    );

    Serial0.printf(
        "[AudioRecorder] Duration: %lu ms\n",
        static_cast<unsigned long>(
            getRecordingDurationMs()
        )
    );

    if (error)
    {
        Serial0.println(
            "[AudioRecorder][ERROR] Recording finished with errors"
        );
    }
}

// ============================================================
// UPDATE
// ============================================================

void AudioRecorder::update()
{
    if (!recording || !file)
        return;

    uint32_t currentChunkId = 0;

    const size_t count =
        microphone.copyAudioChunk(
            audioBuffer,
            AUDIO_BUFFER_SAMPLES,
            currentChunkId
        );

    if (count == 0)
        return;

    if (currentChunkId == lastChunkId)
        return;

    // Update only after copying.
    lastChunkId = currentChunkId;

    if (!writeSamples(audioBuffer, count))
    {
        setError(
            "PCM write failed"
        );

        stopRecording();
    }
}

// ============================================================
// WRITE PCM SAMPLES
// ============================================================

bool AudioRecorder::writeSamples(
    const int16_t* samples,
    size_t count
)
{
    if (!file || samples == nullptr || count == 0)
        return false;

    const uint64_t bytes64 =
        static_cast<uint64_t>(count) *
        sizeof(int16_t);

    if (bytes64 > MAX_WAV_DATA_SIZE - recordedBytes)
    {
        setError(
            "WAV size limit reached"
        );

        return false;
    }

    const size_t bytes =
        static_cast<size_t>(bytes64);

    const size_t written = file.write(
        reinterpret_cast<const uint8_t*>(samples),
        bytes
    );

    if (written != bytes)
    {
        Serial0.printf(
            "[AudioRecorder][ERROR] Partial write: %u/%u\n",
            static_cast<unsigned>(written),
            static_cast<unsigned>(bytes)
        );

        return false;
    }

    recordedBytes += static_cast<uint32_t>(written);

    recordedSamples += static_cast<uint32_t>(count);

    return true;
}

// ============================================================
// WRITE WAV HEADER
// ============================================================

bool AudioRecorder::writeWavHeader()
{
    if (!file)
        return false;

    if (!file.seek(0))
        return false;

    // RIFF
    if (!writeBytes(
            reinterpret_cast<const uint8_t*>("RIFF"),
            4
        ))
        return false;

    if (!writeLE32(36))
        return false;

    // WAVE
    if (!writeBytes(
            reinterpret_cast<const uint8_t*>("WAVE"),
            4
        ))
        return false;

    // fmt
    if (!writeBytes(
            reinterpret_cast<const uint8_t*>("fmt "),
            4
        ))
        return false;

    if (!writeLE32(16))
        return false;

    // PCM format
    if (!writeLE16(1))
        return false;

    if (!writeLE16(CHANNELS))
        return false;

    if (!writeLE32(SAMPLE_RATE))
        return false;

    const uint32_t byteRate =
        SAMPLE_RATE *
        CHANNELS *
        (BITS_PER_SAMPLE / 8);

    if (!writeLE32(byteRate))
        return false;

    const uint16_t blockAlign =
        CHANNELS *
        (BITS_PER_SAMPLE / 8);

    if (!writeLE16(blockAlign))
        return false;

    if (!writeLE16(BITS_PER_SAMPLE))
        return false;

    // data
    if (!writeBytes(
            reinterpret_cast<const uint8_t*>("data"),
            4
        ))
        return false;

    if (!writeLE32(0))
        return false;

    return true;
}

// ============================================================
// UPDATE WAV HEADER
// ============================================================

bool AudioRecorder::updateWavHeader()
{
    if (!file)
        return false;

    const uint32_t riffSize =
        36 + recordedBytes;

    if (!file.seek(4))
        return false;

    if (!writeLE32(riffSize))
        return false;

    if (!file.seek(40))
        return false;

    if (!writeLE32(recordedBytes))
        return false;

    if (!file.seek(
            WAV_HEADER_SIZE + recordedBytes
        ))
    {
        return false;
    }

    return true;
}

// ============================================================
// WRITE BYTES
// ============================================================

bool AudioRecorder::writeBytes(
    const uint8_t* data,
    size_t length
)
{
    if (!file || data == nullptr || length == 0)
        return false;

    return file.write(data, length) == length;
}

// ============================================================
// WRITE LE16
// ============================================================

bool AudioRecorder::writeLE16(
    uint16_t value
)
{
    const uint8_t data[2] =
    {
        static_cast<uint8_t>(value & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF)
    };

    return writeBytes(data, sizeof(data));
}

// ============================================================
// WRITE LE32
// ============================================================

bool AudioRecorder::writeLE32(
    uint32_t value
)
{
    const uint8_t data[4] =
    {
        static_cast<uint8_t>(value & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>((value >> 16) & 0xFF),
        static_cast<uint8_t>((value >> 24) & 0xFF)
    };

    return writeBytes(data, sizeof(data));
}

// ============================================================
// ERROR
// ============================================================

void AudioRecorder::setError(
    const char* message
)
{
    error = true;

    Serial0.printf(
        "[AudioRecorder][ERROR] %s\n",
        message != nullptr ? message : "Unknown error"
    );
}

// ============================================================
// STATUS
// ============================================================

bool AudioRecorder::isRecording() const
{
    return recording;
}

bool AudioRecorder::hasError() const
{
    return error;
}

// ============================================================
// STATISTICS
// ============================================================

uint32_t AudioRecorder::getRecordedBytes() const
{
    return recordedBytes;
}

uint32_t AudioRecorder::getRecordedSamples() const
{
    return recordedSamples;
}

uint32_t AudioRecorder::getRecordingDurationMs() const
{
    return static_cast<uint32_t>(
        (
            static_cast<uint64_t>(recordedSamples) * 1000ULL
        ) / SAMPLE_RATE
    );
}
