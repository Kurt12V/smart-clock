
#pragma once

#include <Arduino.h>

class I2SManager;

class Microphone
{
public:

    static constexpr uint32_t DEFAULT_SAMPLE_RATE = 16000;
    static constexpr size_t AUDIO_SAMPLES = 512;

    explicit Microphone(I2SManager& i2sManager);

    bool begin();
    void end();

    bool startListening();
    void stopListening();

    bool updateAudio();

    // Получение текущего блока
    bool hasNewAudio() const;

    uint32_t getAudioChunkId() const;

    size_t getAudioChunk(
        const int16_t*& samples
    ) const;

    size_t getAudioChunkSize() const;

    // Независимое копирование блока
    size_t copyAudioChunk(
        int16_t* destination,
        size_t capacity,
        uint32_t& chunkId
    ) const;

    // Анализ
    float getRMS() const;
    float getLevel() const;
    int16_t getPeak() const;

    // Состояние
    bool isInitialized() const;
    bool isListening() const;

    bool isEnabled() const;
    void setEnabled(bool enabled);

    const char* getName() const;

private:

    I2SManager& i2sManager;

    bool initialized;
    bool listening;
    bool enabled;

    uint32_t audioChunkId;
    size_t audioChunkSize;

    float rms;
    float level;
    int16_t peak;

    int16_t audioBuffer[AUDIO_SAMPLES];

    mutable portMUX_TYPE bufferMux =
        portMUX_INITIALIZER_UNLOCKED;
};
