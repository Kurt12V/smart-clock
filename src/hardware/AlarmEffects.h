
#pragma once

#include <Arduino.h>

#include "managers/LightingManager.h"
#include "managers/SoundManager.h"
#include "SunriseLightEffect.h"

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

    explicit AlarmEffects(
        LightingManager& lighting,
        SoundManager& sound
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
    // CONFIGURATION
    // --------------------------------------------------------

    static constexpr uint32_t MUSIC_START_MS =
        21UL * 60UL * 1000UL;

    static constexpr uint32_t AUDIO_FADE_IN_MS =
        2UL * 60UL * 1000UL;

    static constexpr const char* ALARM_WAV_PATH =
        "/audio/alarms/alarm.wav";

    // --------------------------------------------------------
    // DEPENDENCIES
    // --------------------------------------------------------

    LightingManager& _lighting;
    SoundManager& _sound;

    // --------------------------------------------------------
    // EFFECT
    // --------------------------------------------------------

    SunriseLightEffect _sunrise;

    EffectType _effect;

    bool _begun;
    bool _active;

    // --------------------------------------------------------
    // AUDIO STATE
    // --------------------------------------------------------

    bool _audioStarted;
    bool _audioFadeCompleted;

    uint32_t _lastAudioCheckMs;

    // --------------------------------------------------------
    // INTERNAL METHODS
    // --------------------------------------------------------

    void updateSunrise(uint32_t elapsedMs);

    void updateSunriseLight();

    void applyAuxiliaryLeds();

    void updateSunriseAudio(uint32_t elapsedMs);

    bool startAlarmWav(bool fadeIn);
};
