#pragma once

#include <Arduino.h>
#include <FS.h>

#include "./managers/I2SManager.h"
#include "./managers/SDManager.h"
#include "Settings.h"

// #include <driver/i2s.h>

// ============================================================
// SoundManager
//
// MAX98357A -> I2S_NUM_1
//
// Поддерживает:
//   - WAV PCM 16 bit
//   - mono / stereo
//   - любой sample rate, который поддерживает I2S
//   - global volume
//   - local volume
//   - alarm playback
//   - fade in / fade out
//   - pause / resume
//   - подробную диагностику
// ============================================================

class SoundManager
{
public:

    enum class State : uint8_t
    {
        STOPPED,
        PLAYING,
        PAUSED,
        FADING_IN,
        FADING_OUT
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

    ~SoundManager();

    // --------------------------------------------------------
    // Initialization
    // --------------------------------------------------------

    bool begin();
    void end();

    bool isInitialized() const;

    // --------------------------------------------------------
    // Playback
    // --------------------------------------------------------

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

    // --------------------------------------------------------
    // Control
    // --------------------------------------------------------

    void update();

    void stop();

    bool pause();

    bool resume();

    // --------------------------------------------------------
    // State
    // --------------------------------------------------------

    bool isPlaying() const;
    bool isPaused() const;

    State getState() const;

    // --------------------------------------------------------
    // Volume
    // --------------------------------------------------------

    void setLocalVolume(uint8_t volume);

    uint8_t getLocalVolume() const;

    uint8_t getEffectiveVolume() const;

    // --------------------------------------------------------
    // Position
    // --------------------------------------------------------

    uint32_t getPositionMs() const;

    uint32_t getDurationMs() const;

    uint32_t getPositionBytes() const;

    uint32_t getDataBytes() const;

    // --------------------------------------------------------
    // WAV information
    // --------------------------------------------------------

    uint32_t getSampleRate() const;

    uint16_t getChannels() const;

    uint16_t getBitsPerSample() const;

    uint16_t getAudioFormat() const;

    // --------------------------------------------------------
    // Diagnostics
    // --------------------------------------------------------

    void printStatus() const;

private:

    // ========================================================
    // WAV
    // ========================================================

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

        bool valid = false;
    };

    // ========================================================
    // References
    // ========================================================

    SDManager& _sdManager;
    Settings::Audio& _settings;
    I2SManager& _i2sManager;

    // ========================================================
    // State
    // ========================================================

    bool _initialized;
    State _state;

    // ========================================================
    // File
    // ========================================================

    File _file;

    char _currentPath[128];

    // ========================================================
    // WAV
    // ========================================================

    WavInfo _wav;

    uint32_t _positionBytes;

    // ========================================================
    // Volume
    // ========================================================

    uint8_t _localVolume;

    // ========================================================
    // Fade
    // ========================================================

    bool _fadeEnabled;

    uint32_t _fadeStartMs;
    uint32_t _fadeDurationMs;

    FadeCurve _fadeCurve;

    uint8_t _fadeStartVolume;
    uint8_t _fadeTargetVolume;

    // ========================================================
    // Buffer
    // ========================================================

    static constexpr size_t BUFFER_SIZE = 2048;

    uint8_t _buffer[BUFFER_SIZE];

    // ========================================================
    // Internal
    // ========================================================

    bool openWav(const char* path);

    bool parseWavHeader(File& file);

    bool parseFmtChunk(
        File& file,
        uint32_t chunkSize
    );

    bool configureSpeaker();

    bool configureSpeakerForSampleRate(
        uint32_t sampleRate
    );

    bool readAndPlayChunk();

    bool writeAudio(
        const uint8_t* data,
        size_t bytes
    );

    void closeFile();

    void finishPlayback();

    void updateFade();

    float calculateFade(float progress) const;

    uint8_t calculateCurrentVolume() const;

    void applyVolume(
        int16_t* samples,
        size_t sampleCount,
        uint8_t volume
    );

    void downmixStereoToMono(
        int16_t* samples,
        size_t sampleCount
    );

    bool validateWav() const;

    bool isSupportedSampleRate(
        uint32_t sampleRate
    ) const;

    void printWavInfo() const;

    void printError(
        const char* message
    ) const;
};
