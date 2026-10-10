
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
        None = 0,
        Sunrise
    };

    AlarmEffects(
        LightingManager& lighting,
        SoundManager& sound
    );

    void begin();

    // elapsedMs — время с момента начала сценария рассвета.
    void update(uint32_t elapsedMs);

    void stop();

    void startSunrise();

    void startSunrise(const char* musicPath);

    bool isActive() const;
    bool isSunriseActive() const;

    EffectType effect() const;

private:
    void updateSunrise(uint32_t elapsedMs);

    void updateSunriseLight();
    void updateSunriseAudio(uint32_t elapsedMs);
    void applyAuxiliaryLeds();

    uint8_t calculateMusicVolume(uint32_t elapsedMs) const;

    void stopAudio();

private:
    // Managers
    LightingManager& _lighting;
    SoundManager& _sound;

    // State
    EffectType _effect;
    bool _begun;
    bool _active;

    // Sunrise effect
    SunriseLightEffect _sunrise;

    // Audio
    String _musicPath;
    bool _audioStarted;
    bool _audioAttempted;

    // Sunrise timing
    static constexpr uint32_t MUSIC_START_MS =
        1UL * 60UL * 1000UL;

    static constexpr uint32_t PEAK_TIME_MS =
        25UL * 60UL * 1000UL;

    static constexpr uint32_t FULL_VOLUME_TIME_MS =
        26UL * 60UL * 1000UL;

    static constexpr uint32_t FULL_VOLUME_RAMP_MS =
        FULL_VOLUME_TIME_MS - PEAK_TIME_MS;

    // Volume percentages relative to VOLUME_ALARM.
    static constexpr uint8_t PEAK_VOLUME_PERCENT = 30;
    static constexpr uint8_t MAX_VOLUME_PERCENT = 100;
};
