#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "./managers/LightingManager.h"
#include "./managers/SoundManager.h"
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
    static constexpr char ALARM_WAV_PATH[] =
        "/audio/alarms/alarm.wav";

    static constexpr uint32_t MUSIC_START_MS =
        21UL * 60UL * 1000UL;

    static constexpr uint32_t PEAK_TIME_MS =
        25UL * 60UL * 1000UL;

    static constexpr uint32_t FULL_VOLUME_RAMP_MS =
        60UL * 1000UL;

    static constexpr uint32_t WAV_DURATION_MS =
        40UL * 1000UL;

    void updateSunrise(uint32_t elapsedMs);
    void updateSunriseLight();
    void applyAuxiliaryLeds();
    void updateSunriseAudio(uint32_t elapsedMs);

    uint8_t calculateVolumePercent(uint32_t elapsedMs) const;
    bool startAlarmWav(uint8_t volumePercent);

    LightingManager& _lighting;
    SoundManager& _sound;

    SunriseLightEffect _sunrise;

    EffectType _effect;
    bool _begun;
    bool _active;
    bool _audioStarted;

    uint32_t _nextWavStartMs;
};