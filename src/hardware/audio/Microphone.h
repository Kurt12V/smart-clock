#pragma once

#include <Arduino.h>

class I2SManager;


class Microphone
{
public:

    explicit Microphone(
        I2SManager& i2sManager
    );


    // =====================================================
    // LIFECYCLE
    // =====================================================

    bool begin();

    void end();

    bool isInitialized() const;


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


    // =====================================================
    // AUDIO
    // =====================================================

    bool updateAudio();


    bool hasNewAudio() const;


    size_t getAudioChunk(
        const int16_t*& data
    ) const;


    size_t getAudioChunkSize() const;


    uint32_t getAudioChunkId() const;


    // =====================================================
    // ANALYSIS
    // =====================================================

    float getRMS() const;

    float getLevel() const;

    int16_t getPeak() const;


    // =====================================================
    // INFO
    // =====================================================

    const char* getName() const;


private:

    I2SManager& i2sManager;


    bool initialized;

    bool enabled;

    bool listening;


    // =====================================================
    // AUDIO BUFFER
    // =====================================================

    static constexpr size_t BUFFER_SAMPLES =
        512;


    int16_t audioBuffer[
        BUFFER_SAMPLES
    ];


    size_t audioChunkSize;

    uint32_t audioChunkId;

    bool newAudio;


    // =====================================================
    // ANALYSIS
    // =====================================================

    float rms;

    float level;

    int16_t peak;


    // =====================================================
    // INTERNAL
    // =====================================================

    void analyzeAudio(
        const int16_t* samples,
        size_t count
    );
};