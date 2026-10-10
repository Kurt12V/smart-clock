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
    // STATE / STREAM / CURVE
    // ========================================================

    enum class State : uint8_t
    {
        STOPPED,
        PLAYING,
        PAUSED,
        FADING_OUT,      // ещё играет, но громкость падает
    };

    enum class AudioStream : uint8_t
    {
        Media,           // музыка с SD
        Alarm,           // будильник
        System,          // стартовый звук, клики
    };

    enum class FadeCurve : uint8_t
    {
        Linear,
        Exponential,
        Logarithmic,
    };

    // ========================================================
    // PLAY OPTIONS
    // ========================================================

    struct PlayOptions
    {
        AudioStream stream       = AudioStream::Media;
        uint8_t     localPercent = 100;      // 0..100 поверх громкости стрима
        uint32_t    fadeInMs     = 0;        // 0 = без плавного старта
        uint32_t    fadeOutMs    = 0;        // 0 = без плавного завершения
        FadeCurve   curve        = FadeCurve::Linear;
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

    bool play(const char* path);                              // defaults
    bool play(const char* path, const PlayOptions& opts);     // full API

    bool pause();
    bool resume();

    void stop();                                 // мгновенный стоп
    void stop(uint32_t fadeOutMs);               // плавный стоп

    // ========================================================
    // STATUS
    // ========================================================

    bool        isInitialized() const;
    bool        isPlaying() const;
    bool        isPaused() const;
    bool        isActive() const;
    State       getState() const;
    const char* getStateString() const;

    const char* getCurrentPath() const;
    AudioStream getCurrentStream() const;

    // ========================================================
    // POSITION / DURATION
    // ========================================================

    uint32_t getPositionMs() const;
    uint32_t getDurationMs() const;

    // ========================================================
    // VOLUME
    // ========================================================

    uint8_t streamVolume(AudioStream s) const;

    void    setStreamVolume(AudioStream s, uint8_t v);
    uint8_t getStreamVolume(AudioStream s) const
    {
        return streamVolume(s);
    }

    uint8_t getLocalPercent() const;
    uint8_t getEffectiveVolume() const;   // с учётом фейда

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

    uint8_t calculateBaseVolume() const;       // stream * local
    float   calculateFadeMultiplier() const;   // 0..1

    float   applyFadeCurve(float value) const;

    // ========================================================
    // MEMBERS
    // ========================================================

    SDManager&       _sdManager;
    SettingsManager& _settings;
    I2SManager&      _i2sManager;

    bool  _initialized;
    State _state;

    File    _file;
    WavInfo _wav;

    String      _currentPath;
    AudioStream _currentStream;

    uint32_t _positionBytes;
    uint32_t _durationMs;

    uint8_t _localPercent;

    // --------------------------------------------------------
    // FADE IN / OUT (natural end of track)
    // --------------------------------------------------------

    bool      _fadeInEnabled;
    bool      _fadeOutEnabled;
    uint32_t  _fadeInMs;
    uint32_t  _fadeOutMs;
    uint32_t  _fadeStartMs;
    FadeCurve _fadeCurve;

    // --------------------------------------------------------
    // FADE OUT on explicit stop()
    // --------------------------------------------------------

    bool     _stopFadeActive;
    uint32_t _stopFadeStartMs;
    uint32_t _stopFadeDurationMs;

    uint32_t _lastStatusMs;

    // --------------------------------------------------------
    // BUFFERS
    // --------------------------------------------------------

    static constexpr size_t BUFFER_BYTES = 1024;

    uint8_t _inputBuffer[BUFFER_BYTES];
    int16_t _outputBuffer[BUFFER_BYTES / sizeof(int16_t)];
};