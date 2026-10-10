
#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

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
    // --------------------------------------------------------
    // DEPENDENCIES
    // --------------------------------------------------------

    LightingManager& _lighting;
    SoundManager& _sound;
    MusicGenerator& _music;

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    SunriseLightEffect _sunrise;

    EffectType _effect = EffectType::None;

    bool _begun = false;
    bool _active = false;
    bool _audioStarted = false;

    // --------------------------------------------------------
    // AUDIO CONFIGURATION
    // --------------------------------------------------------

    static constexpr uint32_t AUDIO_SAMPLE_RATE = 22050;

    // Музыка начинает играть через минуту после старта рассвета.
    static constexpr uint32_t MUSIC_START_MS = 60UL * 1000UL;

    // К 25-й минуте громкость достигает 30%.
    static constexpr uint32_t PEAK_TIME_MS =
        25UL * 60UL * 1000UL;

    // С 25-й до 26-й минуты громкость растёт с 30% до 100%.
    static constexpr uint32_t FULL_VOLUME_RAMP_MS =
        60UL * 1000UL;

    // --------------------------------------------------------
    // SUNRISE
    // --------------------------------------------------------

    void updateSunrise(uint32_t elapsedMs);
    void updateSunriseLight();
    void applyAuxiliaryLeds();

    // --------------------------------------------------------
    // AUDIO
    // --------------------------------------------------------

    void updateSunriseAudio(uint32_t elapsedMs);

    float calculateMusicVolume(uint32_t elapsedMs);

    static size_t musicPcmSource(
        void* context,
        int16_t* buffer,
        size_t sampleCount
    );
};
