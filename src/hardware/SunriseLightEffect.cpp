
#include "SunriseLightEffect.h"

#include <math.h>

namespace
{
    constexpr uint8_t PHASE_1 = 1;
    constexpr uint8_t PHASE_2 = 2;
    constexpr uint8_t PHASE_3 = 3;

    constexpr uint32_t MICROSECONDS_PER_SECOND = 1000000UL;

    float easeOut(float progress)
    {
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;

        const float inverse = 1.0f - progress;
        return 1.0f - inverse * inverse;
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
    , _lightPercent(0)
    , _brightness(0)
    , _soundPercent(0)
    , _auxiliaryBrightnessPercent(
        SunriseConfig::DEFAULT_AUX_BRIGHTNESS_PERCENT)
    , _auxiliaryEnabled(false)
    , _auxiliaryFlashState(false)
    , _lastAuxiliaryToggleUs(0)
    , _currentPhase(0)
{
}

// ============================================================
// LIFECYCLE
// ============================================================

void SunriseLightEffect::begin()
{
    if (_begun)
        return;

    _begun = true;
    reset();
}

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

    _lightPercent =
        SunriseConfig::PHASE_1_LIGHT_START_PERCENT;

    _brightness = lightPercentToOutput(_lightPercent);

    _soundPercent =
        SunriseConfig::PHASE_1_SOUND_START_PERCENT;

    _auxiliaryEnabled = false;
    _auxiliaryFlashState = false;
    _lastAuxiliaryToggleUs = 0;
}

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

void SunriseLightEffect::stop()
{
    reset();
}

void SunriseLightEffect::reset()
{
    _state = State::Stopped;
    _elapsedMs = 0;
    _currentPhase = 0;

    _red = 0;
    _green = 0;
    _blue = 0;

    _lightPercent = 0;
    _brightness = 0;
    _soundPercent = 0;

    _auxiliaryEnabled = false;
    _auxiliaryFlashState = false;
    _lastAuxiliaryToggleUs = 0;
}

// ============================================================
// CONFIGURATION
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

void SunriseLightEffect::setAuxiliaryBrightness(uint8_t percent)
{
    _auxiliaryBrightnessPercent = clampPercent(percent);
}

uint8_t SunriseLightEffect::auxiliaryBrightness() const
{
    return _auxiliaryBrightnessPercent;
}

// ============================================================
// STATE GETTERS
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
// OUTPUT GETTERS
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

uint8_t SunriseLightEffect::lightPercent() const
{
    return _lightPercent;
}

uint8_t SunriseLightEffect::brightness() const
{
    return _brightness;
}

uint8_t SunriseLightEffect::soundPercent() const
{
    return _soundPercent;
}

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
    calculateOutputs();

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

    _lightPercent = clampPercent(
        SunriseConfig::PEAK_LIGHT_PERCENT);

    _brightness = lightPercentToOutput(_lightPercent);

    _soundPercent = clampPercent(
        SunriseConfig::PEAK_SOUND_PERCENT);

    _auxiliaryEnabled = true;

    updateAuxiliaryFlash();
}

// ============================================================
// COLOR CALCULATION
// ============================================================

void SunriseLightEffect::calculateColor()
{
    uint32_t startMs = 0;
    uint32_t endMs = 0;

    uint8_t startRed = 0;
    uint8_t startGreen = 0;
    uint8_t startBlue = 0;

    uint8_t endRed = 0;
    uint8_t endGreen = 0;
    uint8_t endBlue = 0;

    switch (_currentPhase)
    {
        case PHASE_1:
            startMs = 0;
            endMs = SunriseConfig::PHASE_1_END_MS;

            startRed = SunriseConfig::PHASE_1_START_RED;
            startGreen = SunriseConfig::PHASE_1_START_GREEN;
            startBlue = SunriseConfig::PHASE_1_START_BLUE;

            endRed = SunriseConfig::PHASE_1_END_RED;
            endGreen = SunriseConfig::PHASE_1_END_GREEN;
            endBlue = SunriseConfig::PHASE_1_END_BLUE;
            break;

        case PHASE_2:
            startMs = SunriseConfig::PHASE_1_END_MS;
            endMs = SunriseConfig::PHASE_2_END_MS;

            startRed = SunriseConfig::PHASE_2_START_RED;
            startGreen = SunriseConfig::PHASE_2_START_GREEN;
            startBlue = SunriseConfig::PHASE_2_START_BLUE;

            endRed = SunriseConfig::PHASE_2_END_RED;
            endGreen = SunriseConfig::PHASE_2_END_GREEN;
            endBlue = SunriseConfig::PHASE_2_END_BLUE;
            break;

        case PHASE_3:
            startMs = SunriseConfig::PHASE_2_END_MS;
            endMs = _durationMs;

            startRed = SunriseConfig::PHASE_3_START_RED;
            startGreen = SunriseConfig::PHASE_3_START_GREEN;
            startBlue = SunriseConfig::PHASE_3_START_BLUE;

            endRed = SunriseConfig::PEAK_RED;
            endGreen = SunriseConfig::PEAK_GREEN;
            endBlue = SunriseConfig::PEAK_BLUE;
            break;

        default:
            return;
    }

    const float progress = calculateProgress(startMs, endMs);

    _red = interpolate(startRed, endRed, progress);
    _green = interpolate(startGreen, endGreen, progress);
    _blue = interpolate(startBlue, endBlue, progress);
}

// ============================================================
// LIGHT AND SOUND PERCENTAGES
// ============================================================

void SunriseLightEffect::calculateOutputs()
{
    uint32_t startMs = 0;
    uint32_t endMs = 0;

    uint8_t lightStart = 0;
    uint8_t lightEnd = 0;

    uint8_t soundStart = 0;
    uint8_t soundEnd = 0;

    switch (_currentPhase)
    {
        case PHASE_1:
            startMs = 0;
            endMs = SunriseConfig::PHASE_1_END_MS;

            lightStart = SunriseConfig::PHASE_1_LIGHT_START_PERCENT;
            lightEnd = SunriseConfig::PHASE_1_LIGHT_END_PERCENT;

            soundStart = SunriseConfig::PHASE_1_SOUND_START_PERCENT;
            soundEnd = SunriseConfig::PHASE_1_SOUND_END_PERCENT;
            break;

        case PHASE_2:
            startMs = SunriseConfig::PHASE_1_END_MS;
            endMs = SunriseConfig::PHASE_2_END_MS;

            lightStart = SunriseConfig::PHASE_2_LIGHT_START_PERCENT;
            lightEnd = SunriseConfig::PHASE_2_LIGHT_END_PERCENT;

            soundStart = SunriseConfig::PHASE_2_SOUND_START_PERCENT;
            soundEnd = SunriseConfig::PHASE_2_SOUND_END_PERCENT;
            break;

        case PHASE_3:
            startMs = SunriseConfig::PHASE_2_END_MS;
            endMs = _durationMs;

            lightStart = SunriseConfig::PHASE_3_LIGHT_START_PERCENT;
            lightEnd = SunriseConfig::PHASE_3_LIGHT_END_PERCENT;

            soundStart = SunriseConfig::PHASE_3_SOUND_START_PERCENT;
            soundEnd = SunriseConfig::PHASE_3_SOUND_END_PERCENT;
            break;

        default:
            return;
    }

    const float progress = easeOut(
        calculateProgress(startMs, endMs));

    _lightPercent = interpolate(
        lightStart, lightEnd, progress);

    _soundPercent = interpolate(
        soundStart, soundEnd, progress);

    _lightPercent = clampPercent(_lightPercent);
    _soundPercent = clampPercent(_soundPercent);

    _brightness = lightPercentToOutput(_lightPercent);
}

// ============================================================
// AUXILIARY FLASH
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
        SunriseConfig::AUX_FLASH_DUTY_PERCENT);

    const uint32_t onTimeUs =
        static_cast<uint32_t>(
            (static_cast<uint64_t>(periodUs) * duty) / 100UL);

    const uint32_t nowUs = micros();

    if (_lastAuxiliaryToggleUs == 0)
    {
        _lastAuxiliaryToggleUs = nowUs;
        _auxiliaryFlashState = (duty > 0);
        return;
    }

    const uint32_t elapsedUs =
        nowUs - _lastAuxiliaryToggleUs;

    _auxiliaryFlashState =
        (elapsedUs % periodUs) < onTimeUs;
}

// ============================================================
// HELPERS
// ============================================================

float SunriseLightEffect::calculateProgress(
    uint32_t startMs,
    uint32_t endMs) const
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

uint8_t SunriseLightEffect::interpolate(
    uint8_t start,
    uint8_t end,
    float progress)
{
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    const float value =
        static_cast<float>(start) +
        (static_cast<float>(end) - start) * progress;

    return static_cast<uint8_t>(lroundf(value));
}

uint8_t SunriseLightEffect::clampPercent(uint8_t value)
{
    return value > 100 ? 100 : value;
}

uint8_t SunriseLightEffect::lightPercentToOutput(float percent)
{
    if (percent <= 0.0f)
        return 0;

    if (percent > 100.0f)
        percent = 100.0f;

    float gamma = SunriseConfig::GAMMA;

    if (gamma <= 0.0001f)
        gamma = 1.0f;

    const float corrected = powf(percent / 100.0f, 1.0f / gamma);

    const int output = static_cast<int>(
        lroundf(corrected * SunriseConfig::MAX_OUTPUT_BRIGHTNESS));

    if (output < 0)
        return 0;

    if (output > SunriseConfig::MAX_OUTPUT_BRIGHTNESS)
        return SunriseConfig::MAX_OUTPUT_BRIGHTNESS;

    return static_cast<uint8_t>(output);
}