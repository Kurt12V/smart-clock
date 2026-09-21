#pragma once

#include <Arduino.h>

// ============================================================
// I2SManager
//
// Центральное управление I2S.
//
// Port 0 -> INMP441 microphone
// Port 1 -> MAX98357A speaker
//
// driver/i2s.h здесь НЕ подключаем.
// ============================================================

class I2SManager
{
public:

    // ========================================================
    // PORTS
    // ========================================================

    static constexpr int MICROPHONE_PORT = 0;
    static constexpr int SPEAKER_PORT = 1;

    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    I2SManager();

    // ========================================================
    // GLOBAL
    // ========================================================

    bool begin();
    void end();

    bool isInitialized() const;

    // ========================================================
    // MICROPHONE
    // ========================================================

    bool beginMicrophone(
        uint32_t sampleRate = 16000
    );

    void endMicrophone();

    bool isMicrophoneInitialized() const;

    int microphonePort() const;

    // Очистить DMA буфер микрофона.
    bool clearMicrophone();

    // Прочитать данные с микрофона.
    bool readMicrophone(
        void* buffer,
        size_t size,
        size_t& bytesRead,
        uint32_t timeoutMs = 100
    );

    // ========================================================
    // SPEAKER
    // ========================================================

    bool beginSpeaker(
        uint32_t sampleRate = 44100
    );

    void endSpeaker();

    bool isSpeakerInitialized() const;

    int speakerPort() const;

    bool startSpeaker();

    bool stopSpeaker();

    bool clearSpeaker();

    bool writeSpeaker(
        const uint8_t* data,
        size_t bytes,
        size_t& bytesWritten,
        uint32_t timeoutMs = 100
    );

    // ========================================================
    // DEBUG
    // ========================================================

    bool isPortInstalled(
        int port
    ) const;

private:

    // ========================================================
    // STATE
    // ========================================================

    bool _initialized;

    bool _microphoneInitialized;

    bool _speakerInitialized;

    // ========================================================
    // SAMPLE RATES
    // ========================================================

    uint32_t _microphoneSampleRate;

    uint32_t _speakerSampleRate;

    // ========================================================
    // INTERNAL
    // ========================================================

    bool installMicrophoneDriver(
        uint32_t sampleRate
    );

    bool installSpeakerDriver(
        uint32_t sampleRate
    );
};