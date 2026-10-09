#include "SunriseLightEffect.h"

#include <math.h>

// ============================================================
// CONSTRUCTOR
// ============================================================

SunriseLightEffect::SunriseLightEffect()
    : _state(State::Stopped),
      _begun(false),
      _durationMs(DEFAULT_DURATION_MS),
      _elapsedMs(0),

      _red(0),
      _green(0),
      _blue(0),

      _perceivedBrightness(0),
      _brightness(0),

      _auxiliaryBrightnessPercent(
          DEFAULT_AUX_BRIGHTNESS_PERCENT
      ),

      _auxiliaryEnabled(false),
      _auxiliaryFlashState(false),

      _lastAuxiliaryToggleUs(0)
{
}

// ============================================================
// BEGIN
// ============================================================

void SunriseLightEffect::begin()
{
    reset();

    _begun = true;
}

// ============================================================
// START
// ============================================================

void SunriseLightEffect::start()
{
    if (!_begun)
        begin();

    _elapsedMs = 0;

    _state = State::Running;

    _red = 0;
    _green = 0;
    _blue = 0;

    _perceivedBrightness = 0;
    _brightness = 0;

    _auxiliaryEnabled = false;
    _auxiliaryFlashState = false;

    _lastAuxiliaryToggleUs = micros();
}

// ============================================================
// UPDATE
// ============================================================

void SunriseLightEffect::update(uint32_t elapsedMs)
{
    if (_state == State::Stopped)
        return;

    _elapsedMs = elapsedMs;

    // --------------------------------------------------------
    // Sunrise
    // --------------------------------------------------------

    if (_elapsedMs < _durationMs)
    {
        _state = State::Running;

        updateSunrise();

        return;
    }

    // --------------------------------------------------------
    // Peak
    // --------------------------------------------------------

    _elapsedMs = _durationMs;

    _state = State::Peak;

    updatePeak();
}

// ============================================================
// STOP
// ============================================================

void SunriseLightEffect::stop()
{
    _state = State::Stopped;

    _elapsedMs = 0;

    _red = 0;
    _green = 0;
    _blue = 0;

    _perceivedBrightness = 0;
    _brightness = 0;

    _auxiliaryEnabled = false;
    _auxiliaryFlashState = false;
}

// ============================================================
// STATE
// ============================================================

SunriseLightEffect::State
SunriseLightEffect::state() const
{
    return _state;
}

// ============================================================

bool SunriseLightEffect::isRunning() const
{
    return _state == State::Running ||
           _state == State::Peak;
}

// ============================================================

bool SunriseLightEffect::isPeak() const
{
    return _state == State::Peak;
}

// ============================================================
// TIMING
// ============================================================

void SunriseLightEffect::setDuration(uint32_t durationMs)
{
    if (durationMs == 0)
        durationMs = DEFAULT_DURATION_MS;

    _durationMs = durationMs;
}

// ============================================================

uint32_t SunriseLightEffect::duration() const
{
    return _durationMs;
}

// ============================================================

uint32_t SunriseLightEffect::elapsed() const
{
    return _elapsedMs;
}

// ============================================================

float SunriseLightEffect::progress() const
{
    if (_durationMs == 0)
        return 1.0f;

    float value =
        static_cast<float>(_elapsedMs) /
        static_cast<float>(_durationMs);

    if (value < 0.0f)
        value = 0.0f;

    if (value > 1.0f)
        value = 1.0f;

    return value;
}

// ============================================================
// LIGHT VALUES
// ============================================================

uint8_t SunriseLightEffect::red() const
{
    return _red;
}

// ============================================================

uint8_t SunriseLightEffect::green() const
{
    return _green;
}

// ============================================================

uint8_t SunriseLightEffect::blue() const
{
    return _blue;
}

// ============================================================

uint8_t SunriseLightEffect::brightness() const
{
    return _brightness;
}

// ============================================================

uint8_t SunriseLightEffect::perceivedBrightness() const
{
    return _perceivedBrightness;
}

// ============================================================
// AUXILIARY LEDS
// ============================================================

void SunriseLightEffect::setAuxiliaryBrightness(
    uint8_t percent
)
{
    if (percent > 100)
        percent = 100;

    _auxiliaryBrightnessPercent = percent;
}

// ============================================================

uint8_t SunriseLightEffect::auxiliaryBrightness() const
{
    return _auxiliaryBrightnessPercent;
}

// ============================================================

bool SunriseLightEffect::auxiliaryEnabled() const
{
    return _auxiliaryEnabled;
}

// ============================================================

bool SunriseLightEffect::auxiliaryFlashState() const
{
    return _auxiliaryFlashState;
}

// ============================================================
// RESET
// ============================================================

void SunriseLightEffect::reset()
{
    _state = State::Stopped;

    _elapsedMs = 0;

    _red = 0;
    _green = 0;
    _blue = 0;

    _perceivedBrightness = 0;
    _brightness = 0;

    _auxiliaryEnabled = false;
    _auxiliaryFlashState = false;

    _lastAuxiliaryToggleUs = micros();
}

// ============================================================
// SUNRISE
// ============================================================

void SunriseLightEffect::updateSunrise()
{
    calculateColor();

    calculateBrightness();

    _auxiliaryEnabled = false;
    _auxiliaryFlashState = false;
}

// ============================================================
// PEAK
// ============================================================

void SunriseLightEffect::updatePeak()
{
    _red = PEAK_RED;
    _green = PEAK_GREEN;
    _blue = PEAK_BLUE;

    _perceivedBrightness = 100;

    /*
     * At peak the matrix brightness must be maximum.
     *
     * AlarmEffects can directly pass this value to:
     *
     *     LedMatrixManager::setBrightness()
     */

    _brightness = 255;

    _auxiliaryEnabled = true;

    updateAuxiliaryFlash();
}

// ============================================================
// COLOR CALCULATION
// ============================================================

void SunriseLightEffect::calculateColor()
{
    // --------------------------------------------------------
    // Phase 1
    //
    // 00:00 -> 20:00
    //
    // RGB:
    // 0,0,0
    //      ↓
    // 255,140,0
    // --------------------------------------------------------

    if (_elapsedMs < PHASE_1_END_MS)
    {
        const float p =
            calculateProgress(
                0,
                PHASE_1_END_MS
            );

        _red = interpolate(
            0,
            255,
            p
        );

        _green = interpolate(
            0,
            140,
            p
        );

        _blue = 0;

        return;
    }

    // --------------------------------------------------------
    // Phase 2
    //
    // 20:00 -> 22:00
    //
    // RGB:
    // 255,140,0
    //      ↓
    // 255,60,40
    // --------------------------------------------------------

    if (_elapsedMs < PHASE_2_END_MS)
    {
        const float p =
            calculateProgress(
                PHASE_1_END_MS,
                PHASE_2_END_MS
            );

        _red = 255;

        _green = interpolate(
            140,
            60,
            p
        );

        _blue = interpolate(
            0,
            40,
            p
        );

        return;
    }

    // --------------------------------------------------------
    // Phase 3
    //
    // 22:00 -> 25:00
    //
    // RGB:
    // 255,60,40
    //      ↓
    // 60,90,255
    // --------------------------------------------------------

    const float p =
        calculateProgress(
            PHASE_2_END_MS,
            _durationMs
        );

    _red = interpolate(
        255,
        PEAK_RED,
        p
    );

    _green = interpolate(
        60,
        PEAK_GREEN,
        p
    );

    _blue = interpolate(
        40,
        PEAK_BLUE,
        p
    );
}

// ============================================================
// BRIGHTNESS CALCULATION
// ============================================================

void SunriseLightEffect::calculateBrightness()
{
    float perceived = 0.0f;

    // --------------------------------------------------------
    // 00:00 -> 20:00
    //
    // 0 -> 60%
    // --------------------------------------------------------

    if (_elapsedMs < PHASE_1_END_MS)
    {
        const float p =
            calculateProgress(
                0,
                PHASE_1_END_MS
            );

        perceived =
            60.0f * p;
    }

    // --------------------------------------------------------
    // 20:00 -> 22:00
    //
    // 60 -> 75%
    // --------------------------------------------------------

    else if (_elapsedMs < PHASE_2_END_MS)
    {
        const float p =
            calculateProgress(
                PHASE_1_END_MS,
                PHASE_2_END_MS
            );

        perceived =
            60.0f +
            (15.0f * p);
    }

    // --------------------------------------------------------
    // 22:00 -> 25:00
    //
    // 75 -> 100%
    // --------------------------------------------------------

    else
    {
        const float p =
            calculateProgress(
                PHASE_2_END_MS,
                _durationMs
            );

        perceived =
            75.0f +
            (25.0f * p);
    }

    if (perceived < 0.0f)
        perceived = 0.0f;

    if (perceived > 100.0f)
        perceived = 100.0f;

    _perceivedBrightness =
        static_cast<uint8_t>(roundf(perceived));

    // --------------------------------------------------------
    // Gamma 2.2
    //
    // coefficient =
    // pow(brightness / 100, 2.2)
    // --------------------------------------------------------

    const float corrected =
        gammaCorrect(perceived);

    _brightness =
        static_cast<uint8_t>(
            roundf(corrected * 255.0f)
        );
}

// ============================================================
// PROGRESS
// ============================================================

float SunriseLightEffect::calculateProgress(
    uint32_t startMs,
    uint32_t endMs
) const
{
    if (endMs <= startMs)
        return 1.0f;

    if (_elapsedMs <= startMs)
        return 0.0f;

    if (_elapsedMs >= endMs)
        return 1.0f;

    return static_cast<float>(
               _elapsedMs - startMs
           ) /
           static_cast<float>(
               endMs - startMs
           );
}

// ============================================================
// GAMMA
// ============================================================

float SunriseLightEffect::gammaCorrect(
    float brightness
) const
{
    if (brightness <= 0.0f)
        return 0.0f;

    if (brightness >= 100.0f)
        return 1.0f;

    const float normalized =
        brightness / 100.0f;

    return powf(
        normalized,
        GAMMA
    );
}

// ============================================================
// INTERPOLATION
// ============================================================

uint8_t SunriseLightEffect::interpolate(
    uint8_t start,
    uint8_t end,
    float progress
) const
{
    if (progress <= 0.0f)
        return start;

    if (progress >= 1.0f)
        return end;

    const float value =
        static_cast<float>(start) +
        (
            static_cast<float>(end) -
            static_cast<float>(start)
        ) * progress;

    if (value <= 0.0f)
        return 0;

    if (value >= 255.0f)
        return 255;

    return static_cast<uint8_t>(
        roundf(value)
    );
}

// ============================================================
// AUXILIARY FLASH
// ============================================================

void SunriseLightEffect::updateAuxiliaryFlash()
{
    const uint32_t now = micros();

    if (
        static_cast<uint32_t>(
            now - _lastAuxiliaryToggleUs
        ) >= AUX_FLASH_HALF_PERIOD_US
    )
    {
        _lastAuxiliaryToggleUs = now;

        _auxiliaryFlashState =
            !_auxiliaryFlashState;
    }
}