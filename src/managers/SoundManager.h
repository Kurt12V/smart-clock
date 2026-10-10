
#pragma once

#include <Arduino.h>
#include <FS.h>
#include <cstddef>
#include <cstdint>

#include "SDManager.h"
#include "I2SManager.h"
#include "SettingsManager.h"
#include "Config.h"

class SoundManager
{
public:
    enum class State : uint8_t
    {
        STOPPED,
        PLAYING,
        PAUSED,
        FADING_OUT
    };

    enum class AudioStream : uint8_t
    {
        Media,
        Alarm,
        System
    };

    enum class FadeCurve : uint8_t
    {
        Linear,
        Exponential,
        Logarithmic
    };

    struct PlayOptions
    {
        AudioStream stream = AudioStream::Media;
        uint8_t localPercent = 100;
        uint32_t fadeInMs = 0;
        uint32_t fadeOutMs = 0;
        FadeCurve curve = FadeCurve::Linear;
    };

    using PcmSourceCallback =
        size_t (*)(void* context, int16_t* output, size_t sampleCapacity);

    using FinishedCallback =
        void (*)(void* context);

    SoundManager(
        SDManager& sdManager,
        SettingsManager& settings,
        I2SManager& i2sManager
    );

    bool begin();
    void end();
    void update();

    bool play(const char* path);
    bool play(const char* path, const PlayOptions& options);

    bool playSource(
        PcmSourceCallback source,
        void* context,
        uint32_t sampleRate,
        AudioStream stream = AudioStream::Alarm,
        FinishedCallback finished = nullptr
    );

    bool beginPCM(
        uint32_t sampleRate,
        AudioStream stream = AudioStream::Alarm
    );

    bool submitPCM(const int16_t* samples, size_t sampleCount);
    size_t availablePCM() const;

    bool pause();
    bool resume();

    void stop();
    void stop(uint32_t fadeOutMs);

    bool isInitialized() const;
    bool isPlaying() const;
    bool isPaused() const;
    bool isActive() const;

    State getState() const;
    const char* getStateString() const;

    const char* getCurrentPath() const;
    AudioStream getCurrentStream() const;

    uint32_t getPositionMs() const;
    uint32_t getDurationMs() const;

    uint8_t streamVolume(AudioStream stream) const;
    void setStreamVolume(AudioStream stream, uint8_t volume);

    uint8_t getStreamVolume(AudioStream stream) const
    {
        return streamVolume(stream);
    }

    uint8_t getLocalPercent() const;
    void setLocalPercent(uint8_t percent);

    uint8_t getEffectiveVolume() const;

    void printStatus();

private:
    enum class SourceMode : uint8_t
    {
        None,
        Wav,
        CallbackPCM,
        BufferedPCM
    };

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

    bool readAndPlayChunk();
    bool readWavChunk();
    bool readPcmChunk();

    bool writeSamples(int16_t* samples, size_t sampleCount);
    void finishPlayback(bool notify = true);

    void applyVolume(
        int16_t* samples,
        size_t sampleCount,
        uint8_t volume
    );

    uint8_t calculateBaseVolume() const;
    float calculateFadeMultiplier() const;
    float applyFadeCurve(float value) const;

    bool startPcmOutput(uint32_t sampleRate);
    void resetPcmQueue();

    void resetPlaybackState();
    void applyStopFade(int16_t* samples, size_t sampleCount);
    bool isFadeOutComplete() const;
    uint32_t getWavPositionMs() const;

    static uint16_t readLE16(const uint8_t* data);
    static uint32_t readLE32(const uint8_t* data);

    SDManager& _sdManager;
    SettingsManager& _settings;
    I2SManager& _i2sManager;

    bool _initialized = false;
    State _state = State::STOPPED;
    SourceMode _sourceMode = SourceMode::None;

    File _file;
    WavInfo _wav;

    String _currentPath;
    AudioStream _currentStream = AudioStream::Media;

    uint32_t _positionBytes = 0;
    uint32_t _durationMs = 0;
    uint8_t _localPercent = 100;

    bool _fadeInEnabled = false;
    bool _fadeOutEnabled = false;

    uint32_t _fadeInMs = 0;
    uint32_t _fadeOutMs = 0;
    uint32_t _fadeStartMs = 0;
    FadeCurve _fadeCurve = FadeCurve::Linear;

    bool _stopFadeActive = false;
    uint32_t _stopFadeStartMs = 0;
    uint32_t _stopFadeDurationMs = 0;
    uint8_t _stopFadeInitialVolume = 0;

    uint32_t _lastStatusMs = 0;
    uint32_t _lastUpdateMs = 0;
    uint32_t _audioBlocksWritten = 0;

    PcmSourceCallback _pcmSource = nullptr;
    void* _pcmContext = nullptr;
    FinishedCallback _finishedCallback = nullptr;
    uint32_t _pcmSampleRate = 0;

    static constexpr size_t PCM_QUEUE_CAPACITY = 4096;

    int16_t _pcmQueue[PCM_QUEUE_CAPACITY] = {};
    size_t _pcmHead = 0;
    size_t _pcmTail = 0;
    size_t _pcmCount = 0;

    static constexpr size_t BUFFER_BYTES = 1024;
    static constexpr size_t BUFFER_SAMPLES =
        BUFFER_BYTES / sizeof(int16_t);

    uint8_t _inputBuffer[BUFFER_BYTES] = {};
    int16_t _outputBuffer[BUFFER_SAMPLES] = {};
};
