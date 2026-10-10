
#pragma once

#include <Arduino.h>

#include "./managers/LightingManager.h"
#include "./managers/SoundManager.h"
#include "./hardware/MusicGenerator.h"
#include "./hardware/SunriseLightEffect.h"

// ============================================================
// ALARM EFFECTS
// ============================================================

class AlarmEffects
{
public:

    enum class EffectType : uint8_t
    {
        None,
        Sunrise
    };

    // --------------------------------------------------------
    // CONSTRUCTOR
    // --------------------------------------------------------

    AlarmEffects(
        LightingManager& lighting,
        SoundManager& sound,
        MusicGenerator& music
    );

    // --------------------------------------------------------
    // LIFECYCLE
    // --------------------------------------------------------

    void begin();

    void update(uint32_t elapsedMs);

    // --------------------------------------------------------
    // EFFECT CONTROL
    // --------------------------------------------------------

    void startSunrise();

    void stop();

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    bool isActive() const;

    bool isSunriseActive() const;

    EffectType effect() const;

private:

    // --------------------------------------------------------
    // SUNRISE
    // --------------------------------------------------------

    void updateSunrise(uint32_t elapsedMs);

    void updateSunriseLight();

    void applyAuxiliaryLeds();

    void updateSunriseAudio(uint32_t elapsedMs);

    // --------------------------------------------------------
    // AUDIO
    // --------------------------------------------------------

    bool startGeneratedMusic();

    static size_t readGeneratedMusic(
        void* context,
        int16_t* buffer,
        size_t sampleCount
    );

    // --------------------------------------------------------
    // DEPENDENCIES
    // --------------------------------------------------------

    LightingManager& _lighting;
    SoundManager& _sound;
    MusicGenerator& _music;

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    EffectType _effect;

    bool _begun;
    bool _active;
    bool _audioStarted;

    // --------------------------------------------------------
    // EFFECTS
    // --------------------------------------------------------

    SunriseLightEffect _sunrise;

    // --------------------------------------------------------
    // TIMING
    // --------------------------------------------------------

    static constexpr uint32_t AUDIO_SAMPLE_RATE = 44100;

    static constexpr uint32_t MUSIC_START_MS =
        1UL * 60UL * 1000UL;

    static constexpr uint32_t PEAK_TIME_MS =
        25UL * 60UL * 1000UL;

    static constexpr uint32_t FULL_VOLUME_RAMP_MS =
        60UL * 1000UL;
};
