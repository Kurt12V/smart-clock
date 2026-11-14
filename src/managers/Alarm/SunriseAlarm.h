#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "./managers/Led/LightingManager.h"
#include "./managers/Sound/SoundManager.h"

// ============================================================
// SUNRISE CONFIGURATION
// ============================================================

namespace SunriseConfig
{
    // Общая длительность рассвета: 5 минут.
    static constexpr uint32_t DURATION_MS =
        5UL * 60UL * 1000UL;

    // Границы фаз.
    static constexpr uint32_t PHASE_1_END_MS =
        4UL * 60UL * 1000UL;

    static constexpr uint32_t PHASE_2_END_MS =
        4UL * 60UL * 1000UL + 30UL * 1000UL;

    // Скорость изменения цвета.
    static constexpr float COLOR_SPEED = 1.5f;

    // --------------------------------------------------------
    // PHASE 1: RED -> ORANGE
    // --------------------------------------------------------

    static constexpr uint8_t PHASE_1_START_R = 255;
    static constexpr uint8_t PHASE_1_START_G = 0;
    static constexpr uint8_t PHASE_1_START_B = 0;

    static constexpr uint8_t PHASE_1_END_R = 255;
    static constexpr uint8_t PHASE_1_END_G = 140;
    static constexpr uint8_t PHASE_1_END_B = 0;

    // --------------------------------------------------------
    // PHASE 2: ORANGE -> WARM WHITE
    // --------------------------------------------------------

    static constexpr uint8_t PHASE_2_END_R = 255;
    static constexpr uint8_t PHASE_2_END_G = 60;
    static constexpr uint8_t PHASE_2_END_B = 40;

    // --------------------------------------------------------
    // PHASE 3 / PEAK
    // --------------------------------------------------------

    static constexpr uint8_t PEAK_R = 60;
    static constexpr uint8_t PEAK_G = 90;
    static constexpr uint8_t PEAK_B = 255;

    // --------------------------------------------------------
    // MATRIX BRIGHTNESS
    // --------------------------------------------------------

    static constexpr uint8_t PHASE_1_START_BRIGHTNESS = 0;
    static constexpr uint8_t PHASE_1_END_BRIGHTNESS = 15;
    static constexpr uint8_t PHASE_2_END_BRIGHTNESS = 30;
    static constexpr uint8_t PHASE_3_END_BRIGHTNESS = 50;
    static constexpr uint8_t PEAK_BRIGHTNESS = 100;

    // --------------------------------------------------------
    // COB
    // --------------------------------------------------------

    static constexpr uint8_t PEAK_COB_BRIGHTNESS = 15;

    static constexpr uint32_t COB_FLASH_FREQUENCY_HZ = 40;
    static constexpr uint8_t COB_DUTY_PERCENT = 50;

    // --------------------------------------------------------
    // MUSIC
    // --------------------------------------------------------

    static constexpr uint32_t MUSIC_START_MS =
        3UL * 60UL * 1000UL;

    static constexpr uint32_t MUSIC_FADE_IN_MS =
        30UL * 1000UL;

    static constexpr uint32_t MUSIC_RETRY_INTERVAL_MS =
        1000UL;

    static constexpr char MUSIC_PATH[] = "/alarm.wav";
}

// ============================================================
// SUNRISE ALARM
// ============================================================

class SunriseAlarm
{
public:

    enum class State : uint8_t
    {
        Idle,
        Running,
        Peak
    };

    SunriseAlarm(
        LightingManager& lighting,
        SoundManager& sound
    );

    // Управление.
    void start();
    void stop();

    // Единственный метод, который нужно вызывать в loop().
    void update();

    // Состояние.
    bool isActive() const;
    bool isPeak() const;

    State state() const;

    uint32_t elapsed() const;

private:

    // --------------------------------------------------------
    // UPDATE
    // --------------------------------------------------------

    void updateLighting(uint32_t elapsedMs);

    void updatePhase1(uint32_t elapsedMs);
    void updatePhase2(uint32_t elapsedMs);
    void updatePhase3(uint32_t elapsedMs);

    void updatePeak();

    void updateAuxiliary();
    void updateMusic(uint32_t elapsedMs);

    // --------------------------------------------------------
    // MATRIX / COB
    // --------------------------------------------------------

    void applyMatrix(
        uint8_t brightness,
        uint8_t red,
        uint8_t green,
        uint8_t blue
    );

    // --------------------------------------------------------
    // MUSIC
    // --------------------------------------------------------

    void startMusic(
        bool fadeIn,
        uint32_t elapsedMs
    );

    // --------------------------------------------------------
    // HELPERS
    // --------------------------------------------------------

    static uint8_t interpolate(
        uint8_t start,
        uint8_t end,
        float progress
    );

    static float normalizeProgress(
        uint32_t value,
        uint32_t duration
    );

    // --------------------------------------------------------
    // DEPENDENCIES
    // --------------------------------------------------------

    LightingManager& _lighting;
    SoundManager& _sound;

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    State _state = State::Idle;

    bool _active = false;

    uint32_t _startTimeMs = 0;
    uint32_t _elapsedMs = 0;

    // --------------------------------------------------------
    // MATRIX CACHE
    // --------------------------------------------------------

    bool _matrixApplied = false;

    uint8_t _lastMatrixBrightness = 0;

    uint8_t _lastRed = 0;
    uint8_t _lastGreen = 0;
    uint8_t _lastBlue = 0;

    // --------------------------------------------------------
    // COB
    // --------------------------------------------------------

    int16_t _lastCobBrightness = -1;

    uint32_t _lastAuxiliaryToggleUs = 0;

    bool _auxiliaryOn = false;

    // --------------------------------------------------------
    // MUSIC
    // --------------------------------------------------------

    bool _musicEverStarted = false;
    bool _musicRetryPending = false;

    uint32_t _lastMusicAttemptMs = 0;
};