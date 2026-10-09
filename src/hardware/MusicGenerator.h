#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

/**
 * ============================================================
 * MusicGenerator
 * ============================================================
 *
 * Генератор PCM-аудио для alarm effects.
 *
 * ВАЖНО:
 *
 * Этот класс НЕ знает:
 *   - I2S
 *   - MAX98357A
 *   - Pins.h
 *   - AlarmEffects
 *   - SoundManager
 *
 * Он только генерирует PCM samples.
 *
 * AlarmEffects получает PCM через:
 *
 *     generateBlock()
 *
 * и самостоятельно отправляет его в I2S.
 *
 * ============================================================
 */
class MusicGenerator
{
public:

    // --------------------------------------------------------
    // CONSTANTS
    // --------------------------------------------------------

    static constexpr uint32_t DEFAULT_SAMPLE_RATE = 44100;

    static constexpr float DEFAULT_BPM = 110.0f;

    static constexpr size_t BLOCK_SAMPLES = 128;

    static constexpr size_t CHANNELS = 2;

    // --------------------------------------------------------
    // PRESET
    // --------------------------------------------------------

    enum class Preset : uint8_t
    {
        SunriseSoft = 0,
        SunriseAmbient,
        SunrisePiano
    };

    // --------------------------------------------------------
    // NOTE
    // --------------------------------------------------------

    struct NoteEvent
    {
        int note;
        uint8_t beats;
        float velocity;
    };

    // --------------------------------------------------------
    // CONSTRUCTOR
    // --------------------------------------------------------

    MusicGenerator();

    // --------------------------------------------------------
    // INITIALIZATION
    // --------------------------------------------------------

    void begin(
        uint32_t sampleRate = DEFAULT_SAMPLE_RATE
    );

    void end();

    bool isInitialized() const;

    // --------------------------------------------------------
    // PLAYBACK
    // --------------------------------------------------------

    void start(
        Preset preset = Preset::SunriseSoft
    );

    void stop();

    bool isPlaying() const;

    // --------------------------------------------------------
    // UPDATE
    // --------------------------------------------------------

    /**
     * Генерирует один PCM block.
     *
     * buffer должен содержать:
     *
     * BLOCK_SAMPLES * CHANNELS
     *
     * int16_t элементов.
     *
     * Формат:
     *
     * L R L R L R ...
     */
    void generateBlock(
        int16_t* buffer,
        size_t sampleCount
    );

    // --------------------------------------------------------
    // VOLUME
    // --------------------------------------------------------

    void setVolume(
        float volume01
    );

    float volume() const;

    // --------------------------------------------------------
    // BPM
    // --------------------------------------------------------

    void setBpm(
        float bpm
    );

    float bpm() const;

    // --------------------------------------------------------
    // SAMPLE RATE
    // --------------------------------------------------------

    uint32_t sampleRate() const;

private:

    // --------------------------------------------------------
    // VOICE
    // --------------------------------------------------------

    struct Voice
    {
        float frequency = 0.0f;
        float phase = 0.0f;

        float level = 0.0f;

        uint32_t ageSamples = 0;
        uint32_t lifeSamples = 0;

        float attack = 0.0f;
        float release = 0.0f;

        bool active = false;
    };

    // --------------------------------------------------------
    // TIMING
    // --------------------------------------------------------

    void updateTiming();

    // --------------------------------------------------------
    // AUDIO
    // --------------------------------------------------------

    float renderSample();

    float renderVoice(
        Voice& voice
    );

    // --------------------------------------------------------
    // BEAT
    // --------------------------------------------------------

    void triggerBeat();

    void triggerMelody();

    void triggerHarmony();

    void triggerBass();

    // --------------------------------------------------------
    // PATTERN
    // --------------------------------------------------------

    const NoteEvent* melodyPattern(
        size_t& count
    ) const;

    // --------------------------------------------------------
    // CHORD
    // --------------------------------------------------------

    void selectChord(
        int& root,
        int& third,
        int& fifth
    ) const;

    // --------------------------------------------------------
    // HELPERS
    // --------------------------------------------------------

    static float midiToFrequency(
        int midi
    );

    static float sine(
        float phase
    );

    static float clamp01(
        float value
    );

    static float smoothStep(
        float value
    );

private:

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    bool _initialized;
    bool _playing;

    Preset _preset;

    // --------------------------------------------------------
    // AUDIO
    // --------------------------------------------------------

    uint32_t _sampleRate;

    float _bpm;

    float _volume;
    float _targetVolume;

    // --------------------------------------------------------
    // TIMING
    // --------------------------------------------------------

    uint32_t _beatSamples;

    uint32_t _samplePosition;

    uint32_t _nextBeatSample;

    uint32_t _beat;

    uint32_t _bar;

    int _melodyIndex;

    // --------------------------------------------------------
    // VOICES
    // --------------------------------------------------------

    Voice _melody;
    Voice _bass;
    Voice _pad1;
    Voice _pad2;

    // --------------------------------------------------------
    // CONSTANTS
    // --------------------------------------------------------

    // static constexpr float TWO_PI =
    //     6.28318530717958647692f;
};