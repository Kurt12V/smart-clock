
#pragma once

#include <Arduino.h>
#include <FS.h>

class Microphone;

class AudioRecorder
{
public:

    AudioRecorder(
        Microphone& microphone,
        fs::FS& filesystem
    );

    // Lifecycle
    bool begin();
    void end();

    // Recording
    bool startRecording(const char* path);
    void stopRecording();

    bool isRecording() const;
    bool hasError() const;

    // Processing
    void update();

    // Statistics
    uint32_t getRecordedBytes() const;
    uint32_t getRecordedSamples() const;
    uint32_t getRecordingDurationMs() const;

private:

    Microphone& microphone;
    fs::FS& filesystem;

    File file;

    bool initialized;
    bool recording;
    bool error;

    uint32_t recordedBytes;
    uint32_t recordedSamples;

    uint32_t recordingStartTime;
    uint32_t lastChunkId;

    // --------------------------------------------------------
    // AUDIO CONFIGURATION
    // --------------------------------------------------------

    static constexpr uint32_t SAMPLE_RATE = 16000;
    static constexpr uint16_t CHANNELS = 1;
    static constexpr uint16_t BITS_PER_SAMPLE = 16;

    static constexpr uint16_t WAV_HEADER_SIZE = 44;

    // Classic WAV uses a 32-bit RIFF size.
    static constexpr uint32_t MAX_WAV_DATA_SIZE =
        0xFFFFFFFFUL - 36UL;

    // --------------------------------------------------------
    // BUFFER
    // --------------------------------------------------------

    static constexpr size_t AUDIO_BUFFER_SAMPLES = 512;

    int16_t audioBuffer[AUDIO_BUFFER_SAMPLES];

    // --------------------------------------------------------
    // WAV
    // --------------------------------------------------------

    bool writeWavHeader();
    bool updateWavHeader();

    bool writeSamples(
        const int16_t* samples,
        size_t count
    );

    bool writeLE16(uint16_t value);
    bool writeLE32(uint32_t value);

    bool writeBytes(
        const uint8_t* data,
        size_t length
    );

    void setError(const char* message);
};
