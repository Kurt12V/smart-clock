#pragma once

#include <Arduino.h>

#include "./managers/LightingManager.h"
#include "./managers/SoundManager.h"
#include "./hardware/MusicGenerator.h"
#include "./hardware/SunriseLightEffect.h"

class AlarmEffects
{
public:
    enum class EffectType : uint8_t
    {
        None,
        Sunrise
    };

    AlarmEffects(
        LightingManager& lighting,
        SoundManager& sound,
        MusicGenerator& music
    );

    void begin();

    void startSunrise();
    void update(uint32_t elapsedMs);
    void stop();

    bool isActive() const;
    bool isSunriseActive() const;

    EffectType effect() const;

private:
    static constexpr uint32_t MUSIC_START_MS =
        21UL * 60UL * 1000UL;

    static constexpr uint32_t MUSIC_SAMPLE_RATE = 44100;

    LightingManager& _lighting;
    SoundManager& _sound;
    MusicGenerator& _music;

    EffectType _effect;

    bool _begun;
    bool _active;
    bool _audioStarted;
    bool _audioAttempted;

    uint32_t _elapsedMs;

    SunriseLightEffect _sunrise;

    void updateSunrise(uint32_t elapsedMs);
    void updateSunriseLight();
    void applyAuxiliaryLeds();
    void updateSunriseAudio();

    void startGeneratedMusic();
    void stopAudio();

    static size_t readGeneratedMusic(
        void* context,
        int16_t* buffer,
        size_t sampleCount
    );

    static bool isGeneratedMusicFinished(void* context);
};