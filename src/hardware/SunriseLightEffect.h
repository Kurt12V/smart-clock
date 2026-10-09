#pragma once

#include <Arduino.h>
#include <stdint.h>

/**
 * ============================================================
 * SunriseLightEffect
 * ============================================================
 *
 * Чистая логика светового эффекта рассвета.
 *
 * Класс НЕ управляет:
 *   - LedMatrixManager
 *   - CobLedManager
 *   - SettingsManager
 *   - AlarmManager
 *
 * Он только рассчитывает текущее состояние эффекта.
 *
 * AlarmEffects получает значения через:
 *
 *   red()
 *   green()
 *   blue()
 *   brightness()
 *   auxiliaryEnabled()
 *   auxiliaryBrightness()
 *
 * и уже самостоятельно применяет их к железу.
 *
 * ============================================================
 */
class SunriseLightEffect
{
public:

    // --------------------------------------------------------
    // CONSTANTS
    // --------------------------------------------------------

    static constexpr uint32_t DEFAULT_DURATION_MS =
        25UL * 60UL * 1000UL;

    static constexpr uint32_t DEFAULT_AUX_FLASH_PERIOD_US =
        25000UL; // 40 Hz

    static constexpr uint8_t DEFAULT_AUX_BRIGHTNESS_PERCENT = 15;

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    enum class State : uint8_t
    {
        Stopped = 0,
        Running,
        Peak
    };

    // --------------------------------------------------------
    // CONSTRUCTOR
    // --------------------------------------------------------

    SunriseLightEffect();

    // --------------------------------------------------------
    // LIFECYCLE
    // --------------------------------------------------------

    void begin();

    void start();

    void update(uint32_t elapsedMs);

    void stop();

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    State state() const;

    bool isRunning() const;

    bool isPeak() const;

    // --------------------------------------------------------
    // TIMING
    // --------------------------------------------------------

    void setDuration(uint32_t durationMs);

    uint32_t duration() const;

    uint32_t elapsed() const;

    float progress() const;

    // --------------------------------------------------------
    // LIGHT VALUES
    // --------------------------------------------------------

    uint8_t red() const;

    uint8_t green() const;

    uint8_t blue() const;

    /**
     * Перцептивная яркость после gamma correction.
     *
     * Возвращает 0..255.
     */
    uint8_t brightness() const;

    /**
     * Яркость до gamma correction.
     *
     * Возвращает 0..100%.
     */
    uint8_t perceivedBrightness() const;

    // --------------------------------------------------------
    // AUXILIARY LEDS
    // --------------------------------------------------------

    void setAuxiliaryBrightness(uint8_t percent);

    uint8_t auxiliaryBrightness() const;

    /**
     * Дополнительные COB начинают работать
     * только после достижения пика.
     */
    bool auxiliaryEnabled() const;

    /**
     * Текущее состояние мигания:
     *
     * true  = LED включены
     * false = LED выключены
     */
    bool auxiliaryFlashState() const;

    // --------------------------------------------------------
    // RESET
    // --------------------------------------------------------

    void reset();

private:

    // --------------------------------------------------------
    // UPDATE
    // --------------------------------------------------------

    void updateSunrise();

    void updatePeak();

    // --------------------------------------------------------
    // CALCULATIONS
    // --------------------------------------------------------

    void calculateColor();

    void calculateBrightness();

    float calculateProgress(
        uint32_t startMs,
        uint32_t endMs
    ) const;

    float gammaCorrect(float brightness) const;

    uint8_t interpolate(
        uint8_t start,
        uint8_t end,
        float progress
    ) const;

    // --------------------------------------------------------
    // AUXILIARY FLASH
    // --------------------------------------------------------

    void updateAuxiliaryFlash();

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

    // --------------------------------------------------------
    // BRIGHTNESS
    // --------------------------------------------------------

    uint8_t _perceivedBrightness;
    uint8_t _brightness;

    // --------------------------------------------------------
    // AUXILIARY LEDS
    // --------------------------------------------------------

    uint8_t _auxiliaryBrightnessPercent;

    bool _auxiliaryEnabled;
    bool _auxiliaryFlashState;

    uint32_t _lastAuxiliaryToggleUs;

    // --------------------------------------------------------
    // CONSTANTS
    // --------------------------------------------------------

    static constexpr float GAMMA = 2.2f;

    static constexpr uint8_t PEAK_RED   = 60;
    static constexpr uint8_t PEAK_GREEN = 90;
    static constexpr uint8_t PEAK_BLUE  = 255;

    static constexpr uint32_t PHASE_1_END_MS =
        20UL * 60UL * 1000UL;

    static constexpr uint32_t PHASE_2_END_MS =
        22UL * 60UL * 1000UL;

    static constexpr uint32_t MUSIC_START_MS =
        21UL * 60UL * 1000UL;

    static constexpr uint32_t AUX_FLASH_HALF_PERIOD_US =
        12500UL; // 40 Hz / 50% duty
};