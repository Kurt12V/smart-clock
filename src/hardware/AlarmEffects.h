
#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

#include "./managers/LightingManager.h"
#include "./managers/I2SManager.h"
#include "./hardware/SunriseLightEffect.h"
#include "./hardware/MusicGenerator.h"

class AlarmEffects
{
public:
    enum class EffectType : uint8_t
    {
        None = 0,
        Sunrise
    };

    AlarmEffects(
        LightingManager& lighting,
        I2SManager& i2s
    );

    void begin();

    void update(
        uint32_t elapsedMs
    );

    void stop();

    void startSunrise();

    bool isActive() const;

    bool isSunriseActive() const;

    EffectType effect() const;

private:
    // ========================================================
    // SUNRISE
    // ========================================================

    void updateSunrise(
        uint32_t elapsedMs
    );

    void updateSunriseLight();

    void updateSunriseAudio(
        uint32_t elapsedMs
    );

    void applyAuxiliaryLeds();

private:
    // ========================================================
    // MANAGERS
    // ========================================================

    LightingManager& _lighting;
    I2SManager& _i2s;

    // ========================================================
    // STATE
    // ========================================================

    EffectType _effect;

    bool _begun;
    bool _active;

    // ========================================================
    // EFFECTS
    // ========================================================

    SunriseLightEffect _sunrise;
    MusicGenerator _music;

    // ========================================================
    // AUDIO
    // ========================================================

    static constexpr uint32_t AUDIO_SAMPLE_RATE = 44100;

    static constexpr size_t AUDIO_SAMPLES =
        MusicGenerator::BLOCK_SAMPLES;

    static constexpr size_t AUDIO_CHANNELS = 2;

    int16_t _monoAudioBuffer[AUDIO_SAMPLES]{};

    int16_t _stereoAudioBuffer[
        AUDIO_SAMPLES * AUDIO_CHANNELS
    ]{};

    bool _audioStarted;

    // ========================================================
    // SUNRISE TIMING
    // ========================================================

    static constexpr uint32_t MUSIC_START_MS =
        21UL * 60UL * 1000UL;

    static constexpr uint32_t PEAK_TIME_MS =
        25UL * 60UL * 1000UL;

    static constexpr uint32_t FULL_VOLUME_RAMP_MS =
        60UL * 1000UL;
};