
#pragma once

#include <Arduino.h>

class Microphone;
class AudioRecorder;

class MicrophoneManager
{
public:

    MicrophoneManager(
        Microphone& microphone,
        AudioRecorder& recorder
    );

    bool begin();
    void end();
    void update();

    bool startListening();
    void stopListening();
    bool isListening() const;

    void setEnabled(bool enabled);
    bool isEnabled() const;
    bool isInitialized() const;

    float getRMS() const;
    float getLevel() const;
    int16_t getPeak() const;

    bool hasNewAudio() const;

    size_t getAudioChunk(
        const int16_t*& data
    ) const;

    size_t copyAudioChunk(
        int16_t* destination,
        size_t capacity,
        uint32_t& chunkId
    ) const;

    size_t getAudioChunkSize() const;

    uint32_t getAudioChunkId() const;

    bool startRecording(const char* path);
    void stopRecording();
    bool isRecording() const;

    uint32_t getRecordedBytes() const;
    uint32_t getRecordedSamples() const;
    uint32_t getRecordingDurationMs() const;

    const char* getName() const;

private:

    Microphone& microphone;
    AudioRecorder& recorder;

    bool initialized;
    bool enabled;

    uint32_t recorderChunkId;

    void updateMicrophone();
    void updateRecorder();
};
