#pragma once

#include <Arduino.h>
#include <driver/i2s.h>

class Microphone
{
public:

    Microphone();

    bool begin();

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
    // AUDIO
    // =====================================================

    /*
     * Получает ОДИН chunk из I2S.
     *
     * Вызывать постоянно из loop().
     */
    bool updateAudio();


    /*
     * Проверяет, появился ли новый PCM chunk.
     */
    bool hasNewAudio() const;


    /*
     * Получить последний PCM chunk.
     *
     * Возвращает количество samples.
     */
    size_t getAudioChunk(
        const int16_t*& data
    ) const;


    /*
     * Размер последнего chunk.
     */
    size_t getAudioChunkSize() const;


    /*
     * Счётчик chunk.
     *
     * Можно использовать для определения,
     * появился ли новый chunk.
     */
    uint32_t getAudioChunkId() const;


    // =====================================================
    // ANALYSIS
    // =====================================================

    float getRMS() const;

    float getLevel() const;

    int16_t getPeak() const;


    const char* getName() const;


private:

    static constexpr i2s_port_t I2S_PORT =
        I2S_NUM_0;


    static constexpr uint32_t SAMPLE_RATE =
        16000;


    static constexpr size_t BUFFER_SAMPLES =
        512;


    bool initialized;

    bool enabled;

    bool listening;


    // =====================================================
    // LAST AUDIO CHUNK
    // =====================================================

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


    void analyzeAudio(
        const int16_t* samples,
        size_t count
    );
};