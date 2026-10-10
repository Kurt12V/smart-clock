#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace SunriseConfig
{
    // ========================================================
    // DURATION
    // ========================================================

    constexpr uint32_t DEFAULT_DURATION_MS = 25UL * 60UL * 1000UL;
    constexpr uint32_t PHASE_1_END_MS      = 20UL * 60UL * 1000UL;
    constexpr uint32_t PHASE_2_END_MS      = 22UL * 60UL * 1000UL;
    constexpr uint32_t MIN_DURATION_MS     = PHASE_2_END_MS + 1UL;

    // ========================================================
    // COLOR SPEED
    // Color changes faster than light/sound percentages.
    // 1.0 = same speed, 1.5 = 50% faster, 2.0 = twice as fast.
    // The color reaches its target early, then holds until the phase ends.
    // ========================================================

    constexpr float COLOR_SPEED_MULTIPLIER = 1.5f;

    // ========================================================
    // PHASE 1: RED SUNRISE
    // ========================================================

    constexpr uint8_t PHASE_1_START_RED   = 255;
    constexpr uint8_t PHASE_1_START_GREEN = 0;
    constexpr uint8_t PHASE_1_START_BLUE  = 0;

    constexpr uint8_t PHASE_1_END_RED   = 255;
    constexpr uint8_t PHASE_1_END_GREEN = 140;
    constexpr uint8_t PHASE_1_END_BLUE  = 0;

    constexpr uint8_t PHASE_1_LIGHT_START_PERCENT = 0;
    constexpr uint8_t PHASE_1_LIGHT_END_PERCENT   = 15;

    constexpr uint8_t PHASE_1_SOUND_START_PERCENT = 5;
    constexpr uint8_t PHASE_1_SOUND_END_PERCENT   = 50;

    // ========================================================
    // PHASE 2: WARM LIGHT
    // ========================================================

    constexpr uint8_t PHASE_2_START_RED   = 255;
    constexpr uint8_t PHASE_2_START_GREEN = 140;
    constexpr uint8_t PHASE_2_START_BLUE  = 0;

    constexpr uint8_t PHASE_2_END_RED   = 255;
    constexpr uint8_t PHASE_2_END_GREEN = 60;
    constexpr uint8_t PHASE_2_END_BLUE  = 40;

    constexpr uint8_t PHASE_2_LIGHT_START_PERCENT = 15;
    constexpr uint8_t PHASE_2_LIGHT_END_PERCENT   = 30;

    constexpr uint8_t PHASE_2_SOUND_START_PERCENT = 5;
    constexpr uint8_t PHASE_2_SOUND_END_PERCENT   = 15;

    // ========================================================
    // PHASE 3: FINAL BRIGHTENING
    // ========================================================

    constexpr uint8_t PHASE_3_START_RED   = 255;
    constexpr uint8_t PHASE_3_START_GREEN = 60;
    constexpr uint8_t PHASE_3_START_BLUE  = 40;

    constexpr uint8_t PEAK_RED   = 60;
    constexpr uint8_t PEAK_GREEN = 90;
    constexpr uint8_t PEAK_BLUE  = 255;

    constexpr uint8_t PHASE_3_LIGHT_START_PERCENT = 30;
    constexpr uint8_t PHASE_3_LIGHT_END_PERCENT   = 50;

    constexpr uint8_t PHASE_3_SOUND_START_PERCENT = 15;
    constexpr uint8_t PHASE_3_SOUND_END_PERCENT   = 35;

    // ========================================================
    // PEAK
    // ========================================================

    constexpr uint8_t PEAK_LIGHT_PERCENT = 100;
    constexpr uint8_t PEAK_SOUND_PERCENT = 40;

    // brightness() is a percentage for the matrix API: 0..100.
    constexpr uint8_t MAX_OUTPUT_BRIGHTNESS = 100;

    // Retained for compatibility with existing project configuration.
    constexpr float GAMMA = 2.2f;

    // ========================================================
    // AUXILIARY LIGHT
    // ========================================================

    constexpr uint32_t AUX_FLASH_FREQUENCY_HZ = 40;
    constexpr uint8_t AUX_FLASH_DUTY_PERCENT = 50;
    constexpr uint8_t DEFAULT_AUX_BRIGHTNESS_PERCENT = 15;
}

class SunriseLightEffect
{
public:
    enum class State : uint8_t
    {
        Stopped,
        Running,
        Peak
    };

    SunriseLightEffect();

    void begin();
    void start();
    void update(uint32_t elapsedMs);
    void stop();
    void reset();

    void setDuration(uint32_t durationMs);
    uint32_t duration() const;

    void setAuxiliaryBrightness(uint8_t percent);
    uint8_t auxiliaryBrightness() const;

    State state() const;
    bool isRunning() const;
    bool isPeak() const;
    bool isStopped() const;
    bool begun() const;

    uint32_t elapsed() const;

    uint8_t red() const;
    uint8_t green() const;
    uint8_t blue() const;

    // Perceived light level in percent: 0..100.
    uint8_t lightPercent() const;

    // Matrix output brightness in percent: 0..100 (100% -> 100).
    uint8_t brightness() const;

    // Desired audio volume in percent: 0..100.
    uint8_t soundPercent() const;

    bool auxiliaryEnabled() const;
    bool auxiliaryFlashState() const;

private:
    void updateSunrise();
    void updatePeak();
    void updateAuxiliaryFlash();

    void calculateColor();
    void calculateOutputs();

    float calculateProgress(uint32_t startMs, uint32_t endMs) const;
    static float clampProgress(float progress);
    static uint8_t interpolate(uint8_t start, uint8_t end, float progress);
    static uint8_t clampPercent(uint8_t value);
    static uint8_t lightPercentToOutput(float percent);

    State _state;
    bool _begun;

    uint32_t _durationMs;
    uint32_t _elapsedMs;

    uint8_t _red;
    uint8_t _green;
    uint8_t _blue;

    uint8_t _lightPercent;
    uint8_t _brightness;
    uint8_t _soundPercent;

    uint8_t _auxiliaryBrightnessPercent;

    bool _auxiliaryEnabled;
    bool _auxiliaryFlashState;
    uint32_t _lastAuxiliaryToggleUs;

    uint8_t _currentPhase;
};
