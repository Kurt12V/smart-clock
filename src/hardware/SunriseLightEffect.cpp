
#include "SunriseLightEffect.h"

#include <math.h>

// ============================================================
// INTERNAL CONSTANTS
// ============================================================

namespace
{
    constexpr uint8_t SUNRISE_MAX_OUTPUT_BRIGHTNESS = 150;

    constexpr uint8_t PHASE_1 = 1;
    constexpr uint8_t PHASE_2 = 2;
    constexpr uint8_t PHASE_3 = 3;

    constexpr float BRIGHTNESS_MAX_PERCENT = 100.0f;
    constexpr float FLOAT_EPSILON = 0.0001f;
    constexpr uint32_t MICROSECONDS_PER_SECOND = 1000000UL;

    // Smooth progression without abrupt changes.
    float easeOut(float progress)
    {
        if (progress < 0.0f)
            progress = 0.0f;

        if (progress > 1.0f)
            progress = 1.0f;

        const float inverse = 1.0f - progress;

        return 1.0f - inverse * inverse;
    }

    // Converts perceived brightness into the LED output value.
    // The maximum output is intentionally limited to 150/255.
    uint8_t perceivedToOutput(float percent)
    {
        if (percent <= 0.0f)
            return 0;

        if (percent > BRIGHTNESS_MAX_PERCENT)
            percent = BRIGHTNESS_MAX_PERCENT;

        float gamma = SunriseConfig::GAMMA;

        if (gamma < FLOAT_EPSILON)
            gamma = 1.0f;

        const float normalized =
            percent / BRIGHTNESS_MAX_PERCENT;

        const float corrected =
            powf(normalized, 1.0f / gamma);

        int output = static_cast<int>(
            lroundf(
                corrected *
                static_cast<float>(SUNRISE_MAX_OUTPUT_BRIGHTNESS)
            )
        );

        if (output < 0)
            output = 0;

        if (output > SUNRISE_MAX_OUTPUT_BRIGHTNESS)
            output = SUNRISE_MAX_OUTPUT_BRIGHTNESS;

        return static_cast<uint8_t>(output);
    }

    uint8_t clampByte(int value)
    {
        if (value < 0)
            value = 0;

        if (value > 255)
            value = 255;

        return static_cast<uint8_t>(value);
    }
}

// ============================================================
// CONSTRUCTOR
// ============================================================

SunriseLightEffect::SunriseLightEffect()
    : _state(State::Stopped)
    , _begun(false)
    , _durationMs(SunriseConfig::DEFAULT_DURATION_MS)
    , _elapsedMs(0)
    , _red(0)
    , _green(0)
    , _blue(0)
    , _perceivedBrightness(0)
    , _brightness(0)
    , _auxiliaryBrightnessPercent(
        SunriseConfig::DEFAULT_AUX_BRIGHTNESS_PERCENT)
    , _auxiliaryEnabled(false)
    , _auxiliaryFlashState(false)
    , _lastAuxiliaryToggleUs(0)
    , _currentPhase(0)
{
}

// ============================================================
// BEGIN
// ============================================================

void SunriseLightEffect::begin()
{
    if (_begun)
        return;

    _begun = true;

    reset();
}

// ============================================================
// START
// ============================================================

void SunriseLightEffect::start()
{
    if (!_begun)
        begin();

    _state = State::Running;
    _elapsedMs = 0;

    _currentPhase = PHASE_1;

    _red = SunriseConfig::PHASE_1_START_RED;
    _green = SunriseConfig::PHASE_1_START_GREEN;
    _blue = SunriseConfig::PHASE_1_START_BLUE;

    _perceivedBrightness =
        clampPercent(SunriseConfig::PHASE_1_START_BRIGHTNESS);

    _brightness =
        perceivedToOutput(_perceivedBrightness);

    _auxiliaryEnabled = false;
    _auxiliaryFlashState = false;
    _lastAuxiliaryToggleUs = 0;
}

// ============================================================
// UPDATE
// ============================================================

void SunriseLightEffect::update(uint32_t elapsedMs)
{
    if (_state == State::Stopped)
        return;

    if (elapsedMs >= _durationMs)
    {
        _elapsedMs = _durationMs;
        updatePeak();
        return;
    }

    _elapsedMs = elapsedMs;

    updateSunrise();
}

// ============================================================
// STOP
// ============================================================

void SunriseLightEffect::stop()
{
    reset();
}

// ============================================================
// RESET
// ============================================================

void SunriseLightEffect::reset()
{
    _state = State::Stopped;
    _elapsedMs = 0;

    _currentPhase = 0;

    _red = 0;
    _green = 0;
    _blue = 0;

    _perceivedBrightness = 0;
    _brightness = 0;

    _auxiliaryEnabled = false;
    _auxiliaryFlashState = false;
    _lastAuxiliaryToggleUs = 0;
}

// ============================================================
// CONFIGURATION: DURATION
// ============================================================

void SunriseLightEffect::setDuration(uint32_t durationMs)
{
    if (durationMs < SunriseConfig::MIN_DURATION_MS)
        durationMs = SunriseConfig::MIN_DURATION_MS;

    _durationMs = durationMs;
}

uint32_t SunriseLightEffect::duration() const
{
    return _durationMs;
}

// ============================================================
// CONFIGURATION: AUXILIARY BRIGHTNESS
// ============================================================

void SunriseLightEffect::setAuxiliaryBrightness(uint8_t percent)
{
    _auxiliaryBrightnessPercent = clampPercent(percent);
}

uint8_t SunriseLightEffect::auxiliaryBrightness() const
{
    return _auxiliaryBrightnessPercent;
}

// ============================================================
// STATE
// ============================================================

SunriseLightEffect::State SunriseLightEffect::state() const
{
    return _state;
}

bool SunriseLightEffect::isRunning() const
{
    return _state == State::Running;
}

bool SunriseLightEffect::isPeak() const
{
    return _state == State::Peak;
}

bool SunriseLightEffect::isStopped() const
{
    return _state == State::Stopped;
}

bool SunriseLightEffect::begun() const
{
    return _begun;
}

uint32_t SunriseLightEffect::elapsed() const
{
    return _elapsedMs;
}

// ============================================================
// RGB OUTPUT
// ============================================================

uint8_t SunriseLightEffect::red() const
{
    return _red;
}

uint8_t SunriseLightEffect::green() const
{
    return _green;
}

uint8_t SunriseLightEffect::blue() const
{
    return _blue;
}

// ============================================================
// PERCEIVED BRIGHTNESS
// ============================================================

uint8_t SunriseLightEffect::perceivedBrightness() const
{
    return _perceivedBrightness;
}

// ============================================================
// OUTPUT BRIGHTNESS
// ============================================================

uint8_t SunriseLightEffect::brightness() const
{
    return _brightness;
}

// ============================================================
// AUXILIARY LIGHT OUTPUT
// ============================================================

bool SunriseLightEffect::auxiliaryEnabled() const
{
    return _auxiliaryEnabled;
}

bool SunriseLightEffect::auxiliaryFlashState() const
{
    return _auxiliaryFlashState;
}

// ============================================================
// UPDATE SUNRISE
// ============================================================

void SunriseLightEffect::updateSunrise()
{
    if (_elapsedMs < SunriseConfig::PHASE_1_END_MS)
    {
        _currentPhase = PHASE_1;
    }
    else if (_elapsedMs < SunriseConfig::PHASE_2_END_MS)
    {
        _currentPhase = PHASE_2;
    }
    else
    {
        _currentPhase = PHASE_3;
    }

    calculateColor();
    calculateBrightness();

    // Auxiliary lighting remains disabled during the sunrise.
    _auxiliaryEnabled = false;
    _auxiliaryFlashState = false;
    _lastAuxiliaryToggleUs = 0;
}

// ============================================================
// UPDATE PEAK
// ============================================================

void SunriseLightEffect::updatePeak()
{
    _state = State::Peak;
    _currentPhase = 4;

    _red = SunriseConfig::PEAK_RED;
    _green = SunriseConfig::PEAK_GREEN;
    _blue = SunriseConfig::PEAK_BLUE;

    _perceivedBrightness = 100;

    // Hard output limit applies to the peak as well.
    _brightness = SUNRISE_MAX_OUTPUT_BRIGHTNESS;

    _auxiliaryEnabled = true;

    updateAuxiliaryFlash();
}

// ============================================================
// CALCULATE COLOR
// ============================================================

void SunriseLightEffect::calculateColor()
{
    switch (_currentPhase)
    {
        case PHASE_1:
        {
            const float progress = calculateProgress(
                0,
                SunriseConfig::PHASE_1_END_MS
            );

            _red = interpolate(
                SunriseConfig::PHASE_1_START_RED,
                SunriseConfig::PHASE_1_END_RED,
                progress
            );

            _green = interpolate(
                SunriseConfig::PHASE_1_START_GREEN,
                SunriseConfig::PHASE_1_END_GREEN,
                progress
            );

            _blue = interpolate(
                SunriseConfig::PHASE_1_START_BLUE,
                SunriseConfig::PHASE_1_END_BLUE,
                progress
            );

            break;
        }

        case PHASE_2:
        {
            const float progress = calculateProgress(
                SunriseConfig::PHASE_1_END_MS,
                SunriseConfig::PHASE_2_END_MS
            );

            _red = interpolate(
                SunriseConfig::PHASE_2_START_RED,
                SunriseConfig::PHASE_2_END_RED,
                progress
            );

            _green = interpolate(
                SunriseConfig::PHASE_2_START_GREEN,
                SunriseConfig::PHASE_2_END_GREEN,
                progress
            );

            _blue = interpolate(
                SunriseConfig::PHASE_2_START_BLUE,
                SunriseConfig::PHASE_2_END_BLUE,
                progress
            );

            break;
        }

        case PHASE_3:
        {
            const float progress = calculateProgress(
                SunriseConfig::PHASE_2_END_MS,
                _durationMs
            );

            _red = interpolate(
                SunriseConfig::PHASE_3_START_RED,
                SunriseConfig::PEAK_RED,
                progress
            );

            _green = interpolate(
                SunriseConfig::PHASE_3_START_GREEN,
                SunriseConfig::PEAK_GREEN,
                progress
            );

            _blue = interpolate(
                SunriseConfig::PHASE_3_START_BLUE,
                SunriseConfig::PEAK_BLUE,
                progress
            );

            break;
        }

        default:
            break;
    }
}

// ============================================================
// CALCULATE BRIGHTNESS
// ============================================================

void SunriseLightEffect::calculateBrightness()
{
    uint8_t startBrightness = 0;
    uint8_t endBrightness = 100;

    uint32_t phaseStartMs = 0;
    uint32_t phaseEndMs = _durationMs;

    switch (_currentPhase)
    {
        case PHASE_1:
        {
            phaseStartMs = 0;
            phaseEndMs = SunriseConfig::PHASE_1_END_MS;

            startBrightness =
                SunriseConfig::PHASE_1_START_BRIGHTNESS;

            endBrightness =
                SunriseConfig::PHASE_1_END_BRIGHTNESS;

            break;
        }

        case PHASE_2:
        {
            phaseStartMs = SunriseConfig::PHASE_1_END_MS;
            phaseEndMs = SunriseConfig::PHASE_2_END_MS;

            startBrightness =
                SunriseConfig::PHASE_2_START_BRIGHTNESS;

            endBrightness =
                SunriseConfig::PHASE_2_END_BRIGHTNESS;

            break;
        }

        case PHASE_3:
        {
            phaseStartMs = SunriseConfig::PHASE_2_END_MS;
            phaseEndMs = _durationMs;

            startBrightness =
                SunriseConfig::PHASE_3_START_BRIGHTNESS;

            endBrightness =
                SunriseConfig::PHASE_3_END_BRIGHTNESS;

            break;
        }

        default:
            return;
    }

    const float progress = calculateProgress(
        phaseStartMs,
        phaseEndMs
    );

    const float easedProgress = easeOut(progress);

    const float perceived =
        static_cast<float>(startBrightness) +
        (
            static_cast<float>(endBrightness) -
            static_cast<float>(startBrightness)
        ) * easedProgress;

    int roundedPercent = static_cast<int>(lroundf(perceived));

    if (roundedPercent < 0)
        roundedPercent = 0;

    if (roundedPercent > 100)
        roundedPercent = 100;

    _perceivedBrightness =
        static_cast<uint8_t>(roundedPercent);

    // Use the unrounded value for a smoother output curve.
    _brightness = perceivedToOutput(perceived);
}

// ============================================================
// UPDATE AUXILIARY FLASH
// ============================================================

void SunriseLightEffect::updateAuxiliaryFlash()
{
    if (!_auxiliaryEnabled ||
        SunriseConfig::AUX_FLASH_FREQUENCY_HZ == 0 ||
        SunriseConfig::AUX_FLASH_DUTY_PERCENT == 0)
    {
        _auxiliaryFlashState = false;
        return;
    }

    const uint32_t periodUs =
        MICROSECONDS_PER_SECOND /
        SunriseConfig::AUX_FLASH_FREQUENCY_HZ;

    if (periodUs == 0)
    {
        _auxiliaryFlashState = false;
        return;
    }

    const uint8_t duty = clampPercent(
        SunriseConfig::AUX_FLASH_DUTY_PERCENT
    );

    const uint32_t onTimeUs =
        static_cast<uint32_t>(
            (static_cast<uint64_t>(periodUs) * duty) / 100UL
        );

    const uint32_t nowUs = micros();

    // Record the start of the flashing cycle once.
    if (_lastAuxiliaryToggleUs == 0)
    {
        _lastAuxiliaryToggleUs = nowUs;

        _auxiliaryFlashState = (duty > 0);
        return;
    }

    // Unsigned subtraction safely handles micros() rollover.
    const uint32_t elapsedUs =
        nowUs - _lastAuxiliaryToggleUs;

    const uint32_t phaseUs = elapsedUs % periodUs;

    _auxiliaryFlashState = (phaseUs < onTimeUs);
}

// ============================================================
// CALCULATE PROGRESS
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

    return static_cast<float>(_elapsedMs - startMs) /
           static_cast<float>(endMs - startMs);
}

// ============================================================
// INTERPOLATION
// ============================================================

uint8_t SunriseLightEffect::interpolate(
    uint8_t start,
    uint8_t end,
    float progress
)
{
    if (progress < 0.0f)
        progress = 0.0f;

    if (progress > 1.0f)
        progress = 1.0f;

    const float value =
        static_cast<float>(start) +
        (
            static_cast<float>(end) -
            static_cast<float>(start)
        ) * progress;

    return clampByte(static_cast<int>(lroundf(value)));
}

// ============================================================
// CLAMP PERCENT
// ============================================================

uint8_t SunriseLightEffect::clampPercent(uint8_t value)
{
    if (value > 100)
        return 100;

    return value;
}