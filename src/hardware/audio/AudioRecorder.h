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

    bool begin();

    bool startRecording(const char* path);
    void stopRecording();

    bool isRecording() const;

    void update();

    uint32_t getRecordedBytes() const;
    uint32_t getRecordedSamples() const;
    uint32_t getRecordingDurationMs() const;

private:
    Microphone& microphone;
    fs::FS& filesystem;

    File file;

    bool recording;

    uint32_t recordedBytes;
    uint32_t recordedSamples;
    uint32_t recordingStartTime;
    uint32_t lastChunkId;

    static constexpr uint32_t SAMPLE_RATE = 16000;
    static constexpr uint16_t CHANNELS = 1;
    static constexpr uint16_t BITS_PER_SAMPLE = 16;

    void writeWavHeader();
    void updateWavHeader();

    void writeSamples(
        const int16_t* samples,
        size_t count
    );

    void writeLE16(uint16_t value);
    void writeLE32(uint32_t value);
};
