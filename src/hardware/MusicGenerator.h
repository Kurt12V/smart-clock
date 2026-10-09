
#pragma once

#include <Arduino.h>
#include <cstddef>
#include <cstdint>

// ============================================================
// MUSIC GENERATOR — ESP32-S3
//
// Mono signed 16-bit PCM.
// Static memory allocation.
// No I2S driver dependency.
// ============================================================

class MusicGenerator
{
public:
    static constexpr uint32_t DEFAULT_SAMPLE_RATE = 44100;
    static constexpr float DEFAULT_BPM = 96.0f;

    static constexpr size_t CHANNELS = 1;
    static constexpr size_t BLOCK_SAMPLES = 512;

    static constexpr uint32_t LOOP_BEATS = 64;
    static constexpr uint32_t BARS_PER_LOOP = 16;
    static constexpr size_t MAX_VOICES = 24;

    enum class VoiceType : uint8_t
    {
        Piano,
        Bass,
        Pad,
        Harp,
        Bell
    };

    struct NoteEvent
    {
        uint8_t beat;
        uint8_t note;
        uint8_t duration;
        float velocity;
    };

    MusicGenerator();

    // Lifecycle
    void begin(uint32_t sampleRate = DEFAULT_SAMPLE_RATE);
    void end();

    bool isInitialized() const;

    // Playback
    void start();
    void stop();

    bool isPlaying() const;

    // Audio
    void generateBlock(int16_t* buffer, size_t sampleCount);

    // Parameters
    void setVolume(float volume);
    float volume() const;

    void setBpm(float bpm);
    float bpm() const;

    uint32_t sampleRate() const;

private:
    // --------------------------------------------------------
    // VOICE
    // --------------------------------------------------------

    struct Voice
    {
        VoiceType type = VoiceType::Piano;
        bool active = false;

        float frequency = 440.0f;
        float phase = 0.0f;

        float velocity = 0.0f;
        float sustainLevel = 0.0f;

        uint64_t ageSamples = 0;
        uint64_t durationSamples = 0;

        uint32_t attackSamples = 1;
        uint32_t decaySamples = 1;
        uint32_t releaseSamples = 1;
    };

    struct Chord
    {
        uint8_t root;
        uint8_t third;
        uint8_t fifth;
        uint8_t seventh;
    };

    // --------------------------------------------------------
    // INTERNAL METHODS
    // --------------------------------------------------------

    void updateTiming();

    void triggerBeat();
    void triggerMelody();
    void triggerHarmony();
    void triggerBass();
    void triggerHarp();
    void triggerBell();

    void triggerNote(
        uint8_t midiNote,
        float durationBeats,
        float velocity,
        VoiceType type
    );

    float renderSample();
    float renderVoice(Voice& voice);
    float envelope(const Voice& voice) const;

    void clearVoices();

    static float midiToFrequency(uint8_t midiNote);
    static float clamp01(float value);
    static Chord chordForBar(uint32_t barIndex);

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    bool _initialized = false;
    bool _playing = false;

    uint32_t _sampleRate = DEFAULT_SAMPLE_RATE;

    float _bpm = DEFAULT_BPM;
    float _samplesPerBeat = 0.0f;
    float _samplesUntilBeat = 0.0f;

    float _volume = 0.8f;
    float _targetVolume = 0.8f;

    uint32_t _beat = 0;
    size_t _melodyIndex = 0;

    Voice _voices[MAX_VOICES];

    // static constexpr float TWO_PI = 6.28318530717958647692f;
};