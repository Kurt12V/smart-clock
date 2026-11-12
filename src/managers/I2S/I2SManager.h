
#pragma once

#include <Arduino.h>
#include <cstddef>
#include <cstdint>

class I2SManager
{
public:

    static constexpr int MICROPHONE_PORT = 0;
    static constexpr int SPEAKER_PORT = 1;

    I2SManager();

    bool begin();
    void end();

    bool isInitialized() const;

    // ========================================================
    // MICROPHONE
    // ========================================================

    bool beginMicrophone(uint32_t sampleRate = 16000);
    void endMicrophone();

    bool isMicrophoneInitialized() const;
    int microphonePort() const;

    bool clearMicrophone();

    bool readMicrophone(
        void* buffer,
        size_t size,
        size_t& bytesRead,
        uint32_t timeoutMs = 100
    );

    // ========================================================
    // SPEAKER
    // ========================================================

    bool beginSpeaker(uint32_t sampleRate = 44100);
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

    // Ожидание расчётного завершения передачи данных.
    bool waitSpeakerTxDone(uint32_t timeoutMs = 1000);

    bool isPortInstalled(int port) const;

private:

    bool _initialized;
    bool _microphoneInitialized;
    bool _speakerInitialized;

    uint32_t _microphoneSampleRate;
    uint32_t _speakerSampleRate;

    // Оценка объёма данных, ещё ожидающих воспроизведения.
    uint64_t _txPendingUs;
    uint32_t _txLastUpdateUs;

    bool installMicrophoneDriver(uint32_t sampleRate);
    bool installSpeakerDriver(uint32_t sampleRate);

    void updateTxPending();
    void resetTxPending();
};
