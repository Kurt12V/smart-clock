
#pragma once

#include <Arduino.h>
#include <stdint.h>

// ============================================================
// SUNRISE CONFIGURATION
// ============================================================

namespace SunriseConfig
{
    // --------------------------------------------------------
    // TIMING
    // --------------------------------------------------------

    constexpr uint32_t DEFAULT_DURATION_MS = 25UL * 60UL * 1000UL;

    constexpr uint32_t PHASE_1_END_MS = 20UL * 60UL * 1000UL;
    constexpr uint32_t PHASE_2_END_MS = 22UL * 60UL * 1000UL;

    constexpr uint32_t MIN_DURATION_MS = PHASE_2_END_MS + 1UL;

    // --------------------------------------------------------
    // PHASE 1: DARK -> WARM ORANGE
    // --------------------------------------------------------

    constexpr uint8_t PHASE_1_START_RED   = 0;
    constexpr uint8_t PHASE_1_START_GREEN = 0;
    constexpr uint8_t PHASE_1_START_BLUE  = 0;

    constexpr uint8_t PHASE_1_END_RED   = 255;
    constexpr uint8_t PHASE_1_END_GREEN = 140;
    constexpr uint8_t PHASE_1_END_BLUE  = 0;

    constexpr uint8_t PHASE_1_START_BRIGHTNESS = 0;
    constexpr uint8_t PHASE_1_END_BRIGHTNESS   = 60;

    // --------------------------------------------------------
    // PHASE 2: WARM ORANGE -> WARM RED
    // --------------------------------------------------------

    constexpr uint8_t PHASE_2_START_RED   = 255;
    constexpr uint8_t PHASE_2_START_GREEN = 140;
    constexpr uint8_t PHASE_2_START_BLUE  = 0;

    constexpr uint8_t PHASE_2_END_RED   = 255;
    constexpr uint8_t PHASE_2_END_GREEN = 60;
    constexpr uint8_t PHASE_2_END_BLUE  = 40;

    constexpr uint8_t PHASE_2_START_BRIGHTNESS = 60;
    constexpr uint8_t PHASE_2_END_BRIGHTNESS   = 75;

    // --------------------------------------------------------
    // PHASE 3: WARM RED -> BLUE PEAK
    // --------------------------------------------------------

    constexpr uint8_t PHASE_3_START_RED   = 255;
    constexpr uint8_t PHASE_3_START_GREEN = 60;
    constexpr uint8_t PHASE_3_START_BLUE  = 40;

    constexpr uint8_t PEAK_RED   = 60;
    constexpr uint8_t PEAK_GREEN = 90;
    constexpr uint8_t PEAK_BLUE  = 255;

    constexpr uint8_t PHASE_3_START_BRIGHTNESS = 75;
    constexpr uint8_t PHASE_3_END_BRIGHTNESS   = 100;

    // --------------------------------------------------------
    // BRIGHTNESS
    // --------------------------------------------------------

    constexpr float GAMMA = 2.2f;

    // --------------------------------------------------------
    // AUXILIARY / COB LIGHTING
    // --------------------------------------------------------

    constexpr uint32_t AUX_FLASH_FREQUENCY_HZ = 40;

    constexpr uint8_t AUX_FLASH_DUTY_PERCENT = 50;

    constexpr uint8_t DEFAULT_AUX_BRIGHTNESS_PERCENT = 15;
}

// ============================================================
// SUNRISE LIGHT EFFECT
// ============================================================

class SunriseLightEffect
{
public:

    enum class State : uint8_t
    {
        Stopped,
        Running,
        Peak
    };

    // --------------------------------------------------------
    // LIFECYCLE
    // --------------------------------------------------------

    SunriseLightEffect();

    void begin();
    void start();

    // elapsedMs: elapsed time since the sunrise started.
    void update(uint32_t elapsedMs);

    void stop();
    void reset();

    // --------------------------------------------------------
    // CONFIGURATION
    // --------------------------------------------------------

    void setDuration(uint32_t durationMs);
    uint32_t duration() const;

    void setAuxiliaryBrightness(uint8_t percent);
    uint8_t auxiliaryBrightness() const;

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    State state() const;

    bool isRunning() const;
    bool isPeak() const;
    bool isStopped() const;

    bool begun() const;

    uint32_t elapsed() const;

    // --------------------------------------------------------
    // RGB OUTPUT
    // --------------------------------------------------------

    uint8_t red() const;
    uint8_t green() const;
    uint8_t blue() const;

    // Perceived brightness: 0-100%.
    uint8_t perceivedBrightness() const;

    // Gamma-corrected output brightness: 0-255.
    uint8_t brightness() const;

    // --------------------------------------------------------
    // AUXILIARY LIGHT OUTPUT
    // --------------------------------------------------------

    bool auxiliaryEnabled() const;
    bool auxiliaryFlashState() const;

private:

    // --------------------------------------------------------
    // CALCULATIONS
    // --------------------------------------------------------

    void updateSunrise();
    void updatePeak();
    void updateAuxiliaryFlash();

    void calculateColor();
    void calculateBrightness();

    float calculateProgress(
        uint32_t startMs,
        uint32_t endMs
    ) const;

    static uint8_t interpolate(
        uint8_t start,
        uint8_t end,
        float progress
    );

    static uint8_t clampPercent(uint8_t value);

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    State _state;

    bool _begun;

    uint32_t _durationMs;
    uint32_t _elapsedMs;

    // --------------------------------------------------------
    // RGB
    // --------------------------------------------------------

    uint8_t _red;
    uint8_t _green;
    uint8_t _blue;

    uint8_t _perceivedBrightness;
    uint8_t _brightness;

    // --------------------------------------------------------
    // AUXILIARY LIGHTING
    // --------------------------------------------------------

    uint8_t _auxiliaryBrightnessPercent;

    bool _auxiliaryEnabled;
    bool _auxiliaryFlashState;

    uint32_t _lastAuxiliaryToggleUs;

    // --------------------------------------------------------
    // PHASE TRACKING
    // --------------------------------------------------------

    uint8_t _currentPhase;
};