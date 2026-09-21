#pragma once

#include <Arduino.h>
#include <FS.h>

#include "./managers/I2SManager.h"
#include "./managers/SDManager.h"
#include "Settings.h"

class SoundManager
{
public:
    enum class State : uint8_t
    {
        STOPPED,
        PLAYING,
        PAUSED
    };

    enum class FadeCurve : uint8_t
    {
        Linear,
        Exponential,
        Logarithmic
    };

    SoundManager(
        SDManager& sdManager,
        Settings::Audio& settings,
        I2SManager& i2sManager
    );

    // ============================================================
    // LIFECYCLE
    // ============================================================

    bool begin();
    void end();

    void update();

    // ============================================================
    // PLAYBACK
    // ============================================================

    bool playWav(const char* path);

    bool playWavLocal(
        const char* path,
        uint8_t localVolume
    );

    bool playAlarm(
        const char* path,
        uint8_t localVolume = 100,
        uint32_t fadeInMs = 30000,
        uint32_t fadeOutMs = 0,
        FadeCurve curve = FadeCurve::Exponential
    );

    void stop();

    // ============================================================
    // PAUSE / RESUME
    // ============================================================

    bool pause();
    bool resume();

    // ============================================================
    // STATUS
    // ============================================================

    bool isInitialized() const;
    bool isPlaying() const;
    bool isPaused() const;
    bool isActive() const;

    State getState() const;

    // ============================================================
    // VOLUME
    // ============================================================

    void setGlobalVolume(uint8_t volume);
    uint8_t getGlobalVolume() const;

    void setLocalVolume(uint8_t volume);
    uint8_t getLocalVolume() const;

    uint8_t getEffectiveVolume() const;

    // ============================================================
    // PLAYBACK INFO
    // ============================================================

    uint32_t getPositionMs() const;
    uint32_t getDurationMs() const;

    uint32_t getPositionBytes() const;
    uint32_t getDataSize() const;

    // ============================================================
    // DEBUG
    // ============================================================

    void printStatus();

private:

    // ============================================================
    // WAV
    // ============================================================

    struct WavInfo
    {
        uint16_t audioFormat = 0;
        uint16_t channels = 0;

        uint32_t sampleRate = 0;
        uint32_t byteRate = 0;

        uint16_t blockAlign = 0;
        uint16_t bitsPerSample = 0;

        uint32_t dataOffset = 0;
        uint32_t dataSize = 0;
    };

    bool openWav(const char* path);
    bool parseWav(File& file);
    bool validateWav() const;

    // ============================================================
    // PLAYBACK
    // ============================================================

    bool readAndPlayChunk();

    void finishPlayback();

    void applyVolume(
        int16_t* samples,
        size_t sampleCount,
        uint8_t volume
    );

    uint8_t calculateEffectiveVolume() const;

    // ============================================================
    // FADE
    // ============================================================

    float calculateFadeMultiplier() const;

    float applyFadeCurve(float value) const;

    // ============================================================
    // MEMBERS
    // ============================================================

    SDManager& _sdManager;
    Settings::Audio& _settings;
    I2SManager& _i2sManager;

    File _file;

    bool _initialized;

    State _state;

    WavInfo _wav;

    uint32_t _positionBytes;
    uint32_t _durationMs;

    uint8_t _localVolume;

    // ============================================================
    // FADE STATE
    // ============================================================

    bool _fadeInEnabled;
    bool _fadeOutEnabled;

    uint32_t _fadeInMs;
    uint32_t _fadeOutMs;

    uint32_t _fadeStartMs;
    uint32_t _fadeStopMs;

    FadeCurve _fadeCurve;

    // ============================================================
    // PLAYBACK BUFFER
    // ============================================================

    static constexpr size_t BUFFER_BYTES = 1024;

    uint8_t _inputBuffer[BUFFER_BYTES];

    // Mono output buffer.
    // 1024 bytes / 2 = 512 samples.
    int16_t _outputBuffer[BUFFER_BYTES / 2];

    // ============================================================
    // DEBUG
    // ============================================================

    uint32_t _lastStatusMs;
};
