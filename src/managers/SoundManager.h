#pragma once

#include <Arduino.h>
#include <FS.h>

#include "./managers/I2SManager.h"
#include "./managers/SDManager.h"
#include "Settings.h"

class SoundManager
{
public:

    // ========================================================
    // Fade curve
    // ========================================================

    enum class FadeCurve : uint8_t
    {
        Linear,
        Exponential,
        Logarithmic
    };

    // ========================================================
    // Constructor
    // ========================================================

    SoundManager(
        SDManager& sdManager,
        Settings::Audio& settings,
        I2SManager& i2sManager
    );

    // ========================================================
    // Lifecycle
    // ========================================================

    bool begin();

    void end();

    void update();

    bool isInitialized() const;
    bool isPlaying() const;
    bool isPaused() const;

    // ========================================================
    // Volume
    // ========================================================

    void setVolume(uint8_t volume);
    uint8_t getVolume() const;

    // ========================================================
    // Normal WAV
    // ========================================================

    bool playWav(const char* path);

    bool playWavLocal(
        const char* path,
        uint8_t localVolume
    );

    // ========================================================
    // Alarm
    // ========================================================

    bool playAlarm(
        const char* path,
        uint8_t localVolume = 100,
        uint32_t fadeInMs = 30000,
        uint32_t fadeOutMs = 0,
        FadeCurve curve = FadeCurve::Exponential
    );

    // ========================================================
    // Control
    // ========================================================

    void stop();

    void pause();

    void resume();

    // ========================================================
    // Fade
    // ========================================================

    void setFadeIn(
        uint32_t durationMs,
        FadeCurve curve = FadeCurve::Linear
    );

    void setFadeOut(
        uint32_t durationMs,
        FadeCurve curve = FadeCurve::Linear
    );

    // ========================================================
    // Status
    // ========================================================

    uint32_t getPositionMs() const;

    uint32_t getDurationMs() const;

    uint8_t getCurrentVolume() const;

    const char* getCurrentPath() const;

private:

    // ========================================================
    // WAV information
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

    // ========================================================
    // Dependencies
    // ========================================================

    SDManager& sdManager;
    Settings::Audio& settings;
    I2SManager& i2sManager;

    // ========================================================
    // State
    // ========================================================

    bool initialized;
    bool playing;
    bool paused;

    bool alarmMode;

    // ========================================================
    // File
    // ========================================================

    File file;

    WavInfo wav;

    char currentPath[128];

    // ========================================================
    // Playback
    // ========================================================

    uint32_t dataRead;
    uint32_t positionSamples;

    uint32_t playbackStartMs;
    uint32_t pausedAtMs;

    // ========================================================
    // Volume
    // ========================================================

    uint8_t localVolume;

    // ========================================================
    // Fade
    // ========================================================

    uint32_t fadeInDuration;
    uint32_t fadeOutDuration;

    uint32_t fadeInStart;
    uint32_t fadeOutStart;

    FadeCurve fadeInCurve;
    FadeCurve fadeOutCurve;

    bool fadingOut;

    // ========================================================
    // Buffer
    // ========================================================

    static constexpr size_t BUFFER_SAMPLES = 512;

    int16_t sampleBuffer[BUFFER_SAMPLES];

    // ========================================================
    // WAV
    // ========================================================

    bool readWavHeader(
        File& wavFile,
        WavInfo& info
    );

    bool findWavDataChunk(
        File& wavFile,
        WavInfo& info
    );

    // ========================================================
    // Playback
    // ========================================================

    bool openWav(
        const char* path
    );

    bool startPlayback();

    void processPlayback();

    bool writeAudio(
        const int16_t* samples,
        size_t count
    );

    // ========================================================
    // Volume
    // ========================================================

    uint8_t calculatePlaybackVolume();

    float calculateFade(
        uint32_t elapsed,
        uint32_t duration,
        FadeCurve curve
    ) const;

    void applyVolume(
        int16_t* samples,
        size_t count,
        uint8_t volume
    );

    // ========================================================
    // I2S
    // ========================================================

    bool ensureSpeaker();

    void clearSpeaker();

    // ========================================================
    // Utility
    // ========================================================

    uint16_t readLE16(File& file);

    uint32_t readLE32(File& file);
};
