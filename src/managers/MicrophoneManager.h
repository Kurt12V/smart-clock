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


    // =====================================================
    // LIFECYCLE
    // =====================================================

    bool begin();

    void end();

    void update();


    // =====================================================
    // LISTENING
    // =====================================================

    bool startListening();

    void stopListening();

    bool isListening() const;


    // =====================================================
    // ENABLE
    // =====================================================

    void setEnabled(bool enabled);

    bool isEnabled() const;

    bool isInitialized() const;


    // =====================================================
    // ANALYSIS
    // =====================================================

    float getRMS() const;

    float getLevel() const;

    int16_t getPeak() const;


    // =====================================================
    // AUDIO CHUNK
    // =====================================================

    bool hasNewAudio() const;

    size_t getAudioChunk(
        const int16_t*& data
    ) const;

    size_t getAudioChunkSize() const;

    uint32_t getAudioChunkId() const;


    // =====================================================
    // RECORDING
    // =====================================================

    bool startRecording(
        const char* path
    );

    void stopRecording();

    bool isRecording() const;


    uint32_t getRecordedBytes() const;

    uint32_t getRecordedSamples() const;

    uint32_t getRecordingDurationMs() const;


    // =====================================================
    // INFO
    // =====================================================

    const char* getName() const;


private:

    Microphone& microphone;

    AudioRecorder& recorder;

    bool initialized;

    bool enabled;


    void updateMicrophone();

    void updateRecorder();
};