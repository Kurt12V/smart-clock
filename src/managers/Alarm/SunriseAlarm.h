
#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "./managers/Led/LightingManager.h"
#include "./managers/Sound/SoundManager.h"

// ============================================================
// SUNRISE ALARM CONFIGURATION
// ============================================================

// namespace SunriseConfig
// {
//     // Общая продолжительность рассвета
//     static constexpr uint32_t DURATION_MS = 25UL * 60UL * 1000UL;

//     // Фазы освещения
//     static constexpr uint32_t PHASE_1_END_MS = 20UL * 60UL * 1000UL;
//     static constexpr uint32_t PHASE_2_END_MS = 22UL * 60UL * 1000UL;

//     // Скорость изменения цветов
//     static constexpr float COLOR_SPEED = 1.5f;

//     // Начальный красный цвет
//     static constexpr uint8_t PHASE_1_START_R = 255;
//     static constexpr uint8_t PHASE_1_START_G = 0;
//     static constexpr uint8_t PHASE_1_START_B = 0;

//     // Оранжевый цвет
//     static constexpr uint8_t PHASE_1_END_R = 255;
//     static constexpr uint8_t PHASE_1_END_G = 140;
//     static constexpr uint8_t PHASE_1_END_B = 0;

//     // Тёплый цвет
//     static constexpr uint8_t PHASE_2_END_R = 255;
//     static constexpr uint8_t PHASE_2_END_G = 60;
//     static constexpr uint8_t PHASE_2_END_B = 40;

//     // Цвет максимальной яркости
//     static constexpr uint8_t PEAK_R = 60;
//     static constexpr uint8_t PEAK_G = 90;
//     static constexpr uint8_t PEAK_B = 255;

//     // Яркость матрицы, проценты
//     static constexpr uint8_t PHASE_1_START_BRIGHTNESS = 0;
//     static constexpr uint8_t PHASE_1_END_BRIGHTNESS = 15;
//     static constexpr uint8_t PHASE_2_END_BRIGHTNESS = 30;
//     static constexpr uint8_t PHASE_3_END_BRIGHTNESS = 50;
//     static constexpr uint8_t PEAK_BRIGHTNESS = 100;

//     // COB
//     static constexpr uint8_t PEAK_COB_BRIGHTNESS = 15;

//     // Частота мигания COB
//     static constexpr uint32_t COB_FLASH_FREQUENCY_HZ = 40;
//     static constexpr uint8_t COB_DUTY_PERCENT = 50;

//     // Музыка
//     static constexpr uint32_t MUSIC_START_MS =
//         21UL * 60UL * 1000UL;

//     static constexpr uint32_t MUSIC_FADE_IN_MS =
//         2UL * 60UL * 1000UL;

//     static constexpr uint32_t MUSIC_RETRY_INTERVAL_MS = 1000UL;

//     static constexpr char MUSIC_PATH[] =
//         "/alarm.wav";
// }
namespace SunriseConfig
{
    // ========================================================
    // ОБЩАЯ ПРОДОЛЖИТЕЛЬНОСТЬ РАССВЕТА
    // ========================================================

    static constexpr uint32_t DURATION_MS =
        5UL * 60UL * 1000UL;


    // ========================================================
    // ФАЗЫ ОСВЕЩЕНИЯ
    // ========================================================

    // Первая фаза — 4 минуты
    static constexpr uint32_t PHASE_1_END_MS =
        4UL * 60UL * 1000UL;

    // Вторая фаза — 4 минуты 30 секунд
    static constexpr uint32_t PHASE_2_END_MS =
        4UL * 60UL * 1000UL + 30UL * 1000UL;


    // ========================================================
    // СКОРОСТЬ ИЗМЕНЕНИЯ ЦВЕТОВ
    // ========================================================

    static constexpr float COLOR_SPEED = 1.5f;


    // ========================================================
    // НАЧАЛЬНЫЙ КРАСНЫЙ ЦВЕТ
    // ========================================================

    static constexpr uint8_t PHASE_1_START_R = 255;
    static constexpr uint8_t PHASE_1_START_G = 0;
    static constexpr uint8_t PHASE_1_START_B = 0;


    // ========================================================
    // ОРАНЖЕВЫЙ ЦВЕТ
    // ========================================================

    static constexpr uint8_t PHASE_1_END_R = 255;
    static constexpr uint8_t PHASE_1_END_G = 140;
    static constexpr uint8_t PHASE_1_END_B = 0;


    // ========================================================
    // ТЁПЛЫЙ ЦВЕТ
    // ========================================================

    static constexpr uint8_t PHASE_2_END_R = 255;
    static constexpr uint8_t PHASE_2_END_G = 60;
    static constexpr uint8_t PHASE_2_END_B = 40;


    // ========================================================
    // ЦВЕТ МАКСИМАЛЬНОЙ ЯРКОСТИ
    // ========================================================

    static constexpr uint8_t PEAK_R = 60;
    static constexpr uint8_t PEAK_G = 90;
    static constexpr uint8_t PEAK_B = 255;


    // ========================================================
    // ЯРКОСТЬ МАТРИЦЫ
    // ========================================================

    static constexpr uint8_t PHASE_1_START_BRIGHTNESS = 0;
    static constexpr uint8_t PHASE_1_END_BRIGHTNESS = 15;
    static constexpr uint8_t PHASE_2_END_BRIGHTNESS = 30;
    static constexpr uint8_t PHASE_3_END_BRIGHTNESS = 50;
    static constexpr uint8_t PEAK_BRIGHTNESS = 100;


    // ========================================================
    // COB
    // ========================================================

    static constexpr uint8_t PEAK_COB_BRIGHTNESS = 15;

    static constexpr uint32_t COB_FLASH_FREQUENCY_HZ = 40;
    static constexpr uint8_t COB_DUTY_PERCENT = 50;


    // ========================================================
    // МУЗЫКА
    // ========================================================

    // Музыка начинается через 3 минуты
    static constexpr uint32_t MUSIC_START_MS =
        3UL * 60UL * 1000UL;

    // Плавное нарастание громкости — 30 секунд
    static constexpr uint32_t MUSIC_FADE_IN_MS =
        30UL * 1000UL;

    static constexpr uint32_t MUSIC_RETRY_INTERVAL_MS =
        1000UL;

    static constexpr char MUSIC_PATH[] =
        "/alarm.wav";
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

    void start();
    void stop();

    void update(uint32_t elapsedMs);

    bool isActive() const;
    bool isPeak() const;

    State state() const;
    uint32_t elapsed() const;

private:

    // --------------------------------------------------------
    // Lighting
    // --------------------------------------------------------

    void updateLighting(uint32_t elapsedMs);

    void updatePhase1(uint32_t elapsedMs);
    void updatePhase2(uint32_t elapsedMs);
    void updatePhase3(uint32_t elapsedMs);
    void updatePeak();

    void applyMatrix(
        uint8_t brightness,
        uint8_t red,
        uint8_t green,
        uint8_t blue
    );

    void updateAuxiliary();

    // --------------------------------------------------------
    // Audio
    // --------------------------------------------------------

    void updateMusic(uint32_t elapsedMs);

    void startMusic(
        bool fadeIn,
        uint32_t elapsedMs
    );

    // --------------------------------------------------------
    // Helpers
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

private:

    LightingManager& _lighting;
    SoundManager& _sound;

    State _state = State::Idle;

    bool _active = false;

    uint32_t _elapsedMs = 0;

    // --------------------------------------------------------
    // Matrix cache
    // --------------------------------------------------------

    bool _matrixApplied = false;

    uint8_t _lastMatrixBrightness = 0;
    uint8_t _lastRed = 0;
    uint8_t _lastGreen = 0;
    uint8_t _lastBlue = 0;

    // --------------------------------------------------------
    // COB cache
    // --------------------------------------------------------

    int16_t _lastCobBrightness = -1;

    uint32_t _lastAuxiliaryToggleUs = 0;

    bool _auxiliaryOn = false;

    // --------------------------------------------------------
    // Audio state
    // --------------------------------------------------------

    bool _musicEverStarted = false;
    bool _musicRetryPending = false;

    uint32_t _lastMusicAttemptMs = 0;
};
