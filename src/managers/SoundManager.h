#pragma once

#include <Arduino.h>
#include <FS.h>

#include "SDManager.h"
#include "I2SManager.h"
#include "SettingsManager.h"
#include "Config.h"

class SoundManager
{
public:

    // ========================================================
    // STATE / CURVE
    // ========================================================

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

    // ========================================================
    // LIFECYCLE
    // ========================================================

    SoundManager(
        SDManager&       sdManager,
        SettingsManager& settings,
        I2SManager&      i2sManager
    );

    bool begin();
    void end();
    void update();

    // ========================================================
    // PLAYBACK
    // ========================================================

    bool play(const char* path);
    bool playLocal(const char* path, uint8_t localVolume);

    bool pause();
    bool resume();
    void stop();

    bool playAlarm(
        const char* path,
        uint8_t localVolume,
        uint32_t fadeInMs,
        uint32_t fadeOutMs,
        FadeCurve curve
    );

    // ========================================================
    // STATUS
    // ========================================================

    bool        isInitialized() const;
    bool        isPlaying() const;
    bool        isPaused() const;
    bool        isActive() const;
    State       getState() const;
    const char* getStateString() const;

    // ========================================================
    // POSITION / DURATION
    // ========================================================

    uint32_t getPositionMs() const;
    uint32_t getDurationMs() const;
    uint32_t getPositionBytes() const;
    uint32_t getDataSize() const;

    // ========================================================
    // CURRENT TRACK
    // ========================================================

    const char* getCurrentPath() const;

    // ========================================================
    // VOLUME
    // ========================================================

    void    setGlobalVolume(uint8_t volume);
    uint8_t getGlobalVolume() const;

    void    setLocalVolume(uint8_t volume);
    uint8_t getLocalVolume() const;

    uint8_t getEffectiveVolume() const;

    // ========================================================
    // DEBUG
    // ========================================================

    void printStatus();

private:

    // ========================================================
    // WAV
    // ========================================================

    struct WavInfo
    {
        uint16_t audioFormat;
        uint16_t channels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample;
        uint32_t dataOffset;
        uint32_t dataSize;
    };

    bool openWav(const char* path);
    bool parseWav(File& file);
    bool validateWav() const;

    bool readAndPlayChunk();
    void finishPlayback();

    // ========================================================
    // VOLUME / FADE
    // ========================================================

    void  applyVolume(
        int16_t* samples,
        size_t   sampleCount,
        uint8_t  volume
    );

    uint8_t calculateEffectiveVolume() const;
    float   calculateFadeMultiplier() const;
    float   applyFadeCurve(float value) const;

    // ========================================================
    // MEMBERS
    // ========================================================

    SDManager&       _sdManager;
    SettingsManager& _settings;
    I2SManager&      _i2sManager;

    bool  _initialized;
    State _state;

    File  _file;
    WavInfo _wav;

    String _currentPath;

    uint32_t _positionBytes;
    uint32_t _durationMs;

    uint8_t _localVolume;

    bool      _fadeInEnabled;
    bool      _fadeOutEnabled;
    uint32_t  _fadeInMs;
    uint32_t  _fadeOutMs;
    uint32_t  _fadeStartMs;
    uint32_t  _fadeStopMs;
    FadeCurve _fadeCurve;

    uint32_t _lastStatusMs;

    // ========================================================
    // BUFFERS
    // ========================================================

    static constexpr size_t BUFFER_BYTES = 1024;

    uint8_t _inputBuffer[BUFFER_BYTES];
    int16_t _outputBuffer[BUFFER_BYTES / sizeof(int16_t)];
};