#pragma once

#include <Arduino.h>
#include <driver/i2s.h>
#include <FS.h>

#include "./managers/SDManager.h"
#include "Settings.h"

class SoundManager
{
public:

    // ============================================================
    // I2S
    // ============================================================

    static constexpr i2s_port_t I2S_PORT = I2S_NUM_0;

    static constexpr int I2S_BCLK = 26;
    static constexpr int I2S_LRC  = 25;
    static constexpr int I2S_DOUT = 22;

    static constexpr uint32_t DEFAULT_SAMPLE_RATE = 44100;

    static constexpr uint16_t BITS_PER_SAMPLE = 16;

    static constexpr uint32_t MIN_SAMPLE_RATE = 8000;
    static constexpr uint32_t MAX_SAMPLE_RATE = 48000;

    // ============================================================
    // Buffers
    // ============================================================

    static constexpr size_t INPUT_BUFFER_SIZE  = 4096;
    static constexpr size_t OUTPUT_BUFFER_SIZE = 8192;

    // ============================================================
    // Volume
    // ============================================================

    static constexpr uint8_t MIN_VOLUME = 0;
    static constexpr uint8_t MAX_VOLUME = 100;

    // ============================================================
    // Sound type
    // ============================================================

    enum class SoundType : uint8_t
    {
        NONE,
        WAV,
        TONE,
        SIREN
    };

    // ============================================================
    // Volume source
    // ============================================================

    enum class VolumeMode : uint8_t
    {
        GLOBAL,
        LOCAL
    };

    // ============================================================
    // Fade curve
    // ============================================================

    enum class FadeCurve : uint8_t
    {
        Linear,
        Exponential,
        Logarithmic
    };

    // ============================================================
    // Constructor
    // ============================================================

    SoundManager(
        SDManager& sd,
        const Settings::Audio& settings
    );

    // ============================================================
    // Lifecycle
    // ============================================================

    bool begin();

    void update();

    void end();

    // ============================================================
    // State
    // ============================================================

    bool isReady() const;

    bool isPlaying() const;

    bool isPaused() const;

    SoundType getSoundType() const;

    VolumeMode getVolumeMode() const;

    // ============================================================
    // Volume
    // ============================================================

    uint8_t getGlobalVolume() const;

    void setLocalVolume(uint8_t volume);

    uint8_t getLocalVolume() const;

    void clearLocalVolume();

    uint8_t getCurrentTargetVolume() const;

    // ============================================================
    // Normal sounds
    // ============================================================

    // Uses Settings::Audio.volume
    bool playWav(
        const char* path,
        uint32_t fadeInMs = 0,
        uint32_t fadeOutMs = 0,
        FadeCurve curve = FadeCurve::Linear
    );

    bool playWav(
        const String& path,
        uint32_t fadeInMs = 0,
        uint32_t fadeOutMs = 0,
        FadeCurve curve = FadeCurve::Linear
    );

    // Uses Settings::Audio.volume
    bool playTone(
        uint16_t frequency,
        uint32_t durationMs,
        uint32_t fadeInMs = 0,
        uint32_t fadeOutMs = 0,
        FadeCurve curve = FadeCurve::Linear
    );

    // Uses Settings::Audio.volume
    bool playBeep(
        uint32_t fadeInMs = 0,
        uint32_t fadeOutMs = 0,
        FadeCurve curve = FadeCurve::Linear
    );

    // Uses Settings::Audio.volume
    bool playSiren(
        uint32_t durationMs = 3000,
        uint32_t fadeInMs = 0,
        uint32_t fadeOutMs = 0,
        FadeCurve curve = FadeCurve::Linear
    );

    // ============================================================
    // Local sounds
    // ============================================================

    // Does NOT use Settings::Audio.volume
    bool playWavLocal(
        const char* path,
        uint8_t localVolume,
        uint32_t fadeInMs = 0,
        uint32_t fadeOutMs = 0,
        FadeCurve curve = FadeCurve::Linear
    );

    bool playWavLocal(
        const String& path,
        uint8_t localVolume,
        uint32_t fadeInMs = 0,
        uint32_t fadeOutMs = 0,
        FadeCurve curve = FadeCurve::Linear
    );

    // Does NOT use Settings::Audio.volume
    bool playToneLocal(
        uint16_t frequency,
        uint32_t durationMs,
        uint8_t localVolume,
        uint32_t fadeInMs = 0,
        uint32_t fadeOutMs = 0,
        FadeCurve curve = FadeCurve::Linear
    );

    // ============================================================
    // Alarm
    // ============================================================

    // Alarm ALWAYS uses local volume.
    //
    // Example:
    //
    // playAlarm(
    //     "/system/sounds/alarm.wav",
    //     100,
    //     30000
    // );
    //
    // 30 seconds exponential fade-in to 100%.
    //
    bool playAlarm(
        const char* path,
        uint8_t localVolume = 100,
        uint32_t fadeInMs = 30000,
        uint32_t fadeOutMs = 0,
        FadeCurve curve = FadeCurve::Exponential
    );

    bool playAlarm(
        const String& path,
        uint8_t localVolume = 100,
        uint32_t fadeInMs = 30000,
        uint32_t fadeOutMs = 0,
        FadeCurve curve = FadeCurve::Exponential
    );

    // ============================================================
    // Control
    // ============================================================

    void stop();

    // Immediately stops audible output,
    // but keeps current playback position.
    void pause();

    // Continues from current position.
    void resume();

    // Smoothly fades current sound to zero and stops.
    void fadeOut(
        uint32_t durationMs,
        FadeCurve curve = FadeCurve::Exponential
    );

    // ============================================================
    // Fade configuration
    // ============================================================

    void setFadeIn(uint32_t durationMs);

    void setFadeOut(uint32_t durationMs);

    void setFadeCurve(FadeCurve curve);

    uint32_t getFadeIn() const;

    uint32_t getFadeOut() const;

    FadeCurve getFadeCurve() const;

    // ============================================================
    // WAV information
    // ============================================================

    uint32_t getSampleRate() const;

    uint16_t getChannels() const;

    uint16_t getBitsPerSample() const;

    uint32_t getDataSize() const;

    uint32_t getDataPosition() const;

    uint32_t getDurationMs() const;

private:

    // ============================================================
    // WAV
    // ============================================================

    bool parseWav();

    bool findFmtChunk();

    bool findDataChunk();

    bool readChunkHeader(
        uint32_t& id,
        uint32_t& size
    );

    bool skipBytes(uint32_t bytes);

    // ============================================================
    // I2S
    // ============================================================

    bool initI2S();

    bool configureI2S(
        uint32_t sampleRate,
        uint16_t channels
    );

    void clearI2S();

    // ============================================================
    // Processing
    // ============================================================

    void processAudio();

    bool processWav();

    bool processTone();

    bool processSiren();

    bool writeOutput();

    // ============================================================
    // WAV conversion
    // ============================================================

    size_t processWavMono(
        const int16_t* input,
        size_t samples
    );

    size_t processWavStereo(
        const int16_t* input,
        size_t samples
    );

    // ============================================================
    // Generators
    // ============================================================

    size_t generateTone(
        int16_t* output,
        size_t frames
    );

    size_t generateSiren(
        int16_t* output,
        size_t frames
    );

    // ============================================================
    // Volume / Fade
    // ============================================================

    uint8_t getTargetVolume() const;

    uint8_t getFadeVolume() const;

    float getFadeFactor() const;

    float applyFadeCurve(float progress) const;

    int16_t applyVolume(int16_t sample) const;

    // ============================================================
    // State reset
    // ============================================================

    void resetPlaybackState();

    // ============================================================
    // Dependencies
    // ============================================================

    SDManager& _sd;

    const Settings::Audio& _settings;

    // ============================================================
    // General state
    // ============================================================

    bool _ready;
    bool _playing;
    bool _paused;

    SoundType _soundType;

    VolumeMode _volumeMode;

    // ============================================================
    // Volume
    // ============================================================

    uint8_t _localVolume;

    // ============================================================
    // Fade
    // ============================================================

    uint32_t _fadeInMs;

    uint32_t _fadeOutMs;

    FadeCurve _fadeCurve;

    bool _manualFadeOut;

    uint32_t _manualFadeOutStartMs;

    uint32_t _manualFadeOutDurationMs;

    FadeCurve _manualFadeOutCurve;

    // ============================================================
    // WAV
    // ============================================================

    File _file;

    uint16_t _audioFormat;

    uint16_t _channels;

    uint32_t _sampleRate;

    uint32_t _byteRate;

    uint16_t _blockAlign;

    uint16_t _bitsPerSample;

    uint32_t _dataSize;

    uint32_t _dataPosition;

    uint32_t _soundDurationMs;

    // ============================================================
    // Buffers
    // ============================================================

    uint8_t _inputBuffer[INPUT_BUFFER_SIZE];

    uint8_t _outputBuffer[OUTPUT_BUFFER_SIZE];

    size_t _inputSize;

    size_t _outputSize;

    size_t _outputPosition;

    // ============================================================
    // Tone
    // ============================================================

    uint16_t _toneFrequency;

    uint32_t _toneDurationMs;

    uint32_t _toneElapsedMs;

    // ============================================================
    // Siren
    // ============================================================

    uint32_t _sirenDurationMs;

    uint32_t _sirenElapsedMs;

    // ============================================================
    // Oscillator
    // ============================================================

    float _phase;

    float _sirenPhase;
};
