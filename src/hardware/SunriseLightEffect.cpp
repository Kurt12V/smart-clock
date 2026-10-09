
#include "SunriseLightEffect.h"

#include <math.h>

// ============================================================
// CONSTRUCTOR
// ============================================================

SunriseLightEffect::SunriseLightEffect()
    : _state(State::Stopped),
      _begun(false),
      _durationMs(SunriseConfig::DEFAULT_DURATION_MS),
      _elapsedMs(0),
      _red(0),
      _green(0),
      _blue(0),
      _perceivedBrightness(0),
      _brightness(0),
      _auxiliaryBrightnessPercent(
          SunriseConfig::DEFAULT_AUX_BRIGHTNESS_PERCENT
      ),
      _auxiliaryEnabled(false),
      _auxiliaryFlashState(false),
      _lastAuxiliaryToggleUs(0),
      _currentPhase(0)
{
    Serial.println("[SUNRISE] Object created");
}

// ============================================================
// BEGIN
// ============================================================

void SunriseLightEffect::begin()
{
    if (_begun)
    {
        Serial.println("[SUNRISE] begin(): already initialized");
        return;
    }

    reset();

    _begun = true;

    Serial.println("[SUNRISE] Initialized");
    Serial.printf(
        "[SUNRISE] Duration: %lu ms\n",
        static_cast<unsigned long>(_durationMs)
    );

    Serial.printf(
        "[SUNRISE] Phase 1 ends: %lu ms\n",
        static_cast<unsigned long>(
            SunriseConfig::PHASE_1_END_MS
        )
    );

    Serial.printf(
        "[SUNRISE] Phase 2 ends: %lu ms\n",
        static_cast<unsigned long>(
            SunriseConfig::PHASE_2_END_MS
        )
    );

    Serial.printf(
        "[SUNRISE] Gamma: %.2f\n",
        static_cast<double>(SunriseConfig::GAMMA)
    );
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
    _currentPhase = 1;

    _red = SunriseConfig::PHASE_1_START_RED;
    _green = SunriseConfig::PHASE_1_START_GREEN;
    _blue = SunriseConfig::PHASE_1_START_BLUE;

    _perceivedBrightness =
        SunriseConfig::PHASE_1_START_BRIGHTNESS;

    _brightness = 0;

    _auxiliaryEnabled = false;
    _auxiliaryFlashState = false;

    _lastAuxiliaryToggleUs = micros();

    Serial.println("[SUNRISE] START");
    Serial.println("[SUNRISE] State: Stopped -> Running");
    Serial.println("[SUNRISE] Phase: 1");

    Serial.printf(
        "[SUNRISE] RGB: (%u,%u,%u)\n",
        static_cast<unsigned>(_red),
        static_cast<unsigned>(_green),
        static_cast<unsigned>(_blue)
    );

    Serial.printf(
        "[SUNRISE] Perceived brightness: %u%%\n",
        static_cast<unsigned>(_perceivedBrightness)
    );

    Serial.printf(
        "[SUNRISE] Output brightness: %u/255\n",
        static_cast<unsigned>(_brightness)
    );

    Serial.println("[SUNRISE] Auxiliary lighting: OFF");
}

// ============================================================
// UPDATE
// ============================================================

void SunriseLightEffect::update(uint32_t elapsedMs)
{
    if (_state == State::Stopped)
        return;

    const State previousState = _state;

    const uint8_t previousRed = _red;
    const uint8_t previousGreen = _green;
    const uint8_t previousBlue = _blue;

    const uint8_t previousBrightness = _brightness;

    const uint8_t previousPerceivedBrightness =
        _perceivedBrightness;

    const uint8_t previousPhase = _currentPhase;

    const bool previousAuxiliaryEnabled = _auxiliaryEnabled;

    _elapsedMs = elapsedMs;

    if (_elapsedMs >= _durationMs)
    {
        _elapsedMs = _durationMs;
        _state = State::Peak;

        updatePeak();
    }
    else
    {
        _state = State::Running;

        updateSunrise();
    }

    // --------------------------------------------------------
    // STATE CHANGES
    // --------------------------------------------------------

    if (previousState != _state)
    {
        Serial.printf(
            "[SUNRISE] State changed: %s -> %s\n",
            previousState == State::Running ? "Running" :
            previousState == State::Peak ? "Peak" : "Stopped",
            _state == State::Running ? "Running" :
            _state == State::Peak ? "Peak" : "Stopped"
        );
    }

    // --------------------------------------------------------
    // PHASE CHANGES
    // --------------------------------------------------------

    if (previousPhase != _currentPhase)
    {
        Serial.printf(
            "[SUNRISE] Phase changed: %u -> %u\n",
            static_cast<unsigned>(previousPhase),
            static_cast<unsigned>(_currentPhase)
        );
    }

    // --------------------------------------------------------
    // RGB CHANGES
    // --------------------------------------------------------

    if (
        previousRed != _red ||
        previousGreen != _green ||
        previousBlue != _blue
    )
    {
        Serial.printf(
            "[SUNRISE] RGB changed: (%u,%u,%u) -> (%u,%u,%u)\n",
            static_cast<unsigned>(previousRed),
            static_cast<unsigned>(previousGreen),
            static_cast<unsigned>(previousBlue),
            static_cast<unsigned>(_red),
            static_cast<unsigned>(_green),
            static_cast<unsigned>(_blue)
        );
    }

    // --------------------------------------------------------
    // BRIGHTNESS CHANGES
    // --------------------------------------------------------

    if (
        previousBrightness != _brightness ||
        previousPerceivedBrightness != _perceivedBrightness
    )
    {
        Serial.printf(
            "[SUNRISE] Brightness changed: perceived %u%% -> %u%%, "
            "output %u -> %u/255\n",
            static_cast<unsigned>(previousPerceivedBrightness),
            static_cast<unsigned>(_perceivedBrightness),
            static_cast<unsigned>(previousBrightness),
            static_cast<unsigned>(_brightness)
        );
    }

    // --------------------------------------------------------
    // AUXILIARY LIGHT CHANGES
    // --------------------------------------------------------

    if (previousAuxiliaryEnabled != _auxiliaryEnabled)
    {
        Serial.printf(
            "[SUNRISE] Auxiliary lighting: %s -> %s\n",
            previousAuxiliaryEnabled ? "ON" : "OFF",
            _auxiliaryEnabled ? "ON" : "OFF"
        );
    }
}

// ============================================================
// STOP
// ============================================================

void SunriseLightEffect::stop()
{
    const State previousState = _state;

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

    _lastAuxiliaryToggleUs = micros();

    Serial.printf(
        "[SUNRISE] State changed: %s -> Stopped\n",
        previousState == State::Running ? "Running" :
        previousState == State::Peak ? "Peak" : "Stopped"
    );

    Serial.println("[SUNRISE] Stopped");
    Serial.println("[SUNRISE] RGB reset to (0,0,0)");
    Serial.println("[SUNRISE] Brightness reset to 0");
    Serial.println("[SUNRISE] Auxiliary lighting disabled");
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

    _lastAuxiliaryToggleUs = micros();

    Serial.println("[SUNRISE] Reset");
    Serial.println("[SUNRISE] State: Stopped");
    Serial.println("[SUNRISE] RGB: (0,0,0)");
    Serial.println("[SUNRISE] Brightness: 0");
    Serial.println("[SUNRISE] Auxiliary lighting: OFF");
}

// ============================================================
// SUNRISE UPDATE
// ============================================================

void SunriseLightEffect::updateSunrise()
{
    if (_elapsedMs < SunriseConfig::PHASE_1_END_MS)
    {
        _currentPhase = 1;
    }
    else if (_elapsedMs < SunriseConfig::PHASE_2_END_MS)
    {
        _currentPhase = 2;
    }
    else
    {
        _currentPhase = 3;
    }

    calculateColor();
    calculateBrightness();

    // The auxiliary lights are reserved for the peak state.
    _auxiliaryEnabled = false;
    _auxiliaryFlashState = false;
}

// ============================================================
// PEAK UPDATE
// ============================================================

void SunriseLightEffect::updatePeak()
{
    _currentPhase = 4;

    _red = SunriseConfig::PEAK_RED;
    _green = SunriseConfig::PEAK_GREEN;
    _blue = SunriseConfig::PEAK_BLUE;

    _perceivedBrightness = 100;
    _brightness = 255;

    if (!_auxiliaryEnabled)
    {
        _auxiliaryEnabled = true;
        _auxiliaryFlashState = true;

        _lastAuxiliaryToggleUs = micros();

        Serial.println("[SUNRISE] Peak reached");
        Serial.printf(
            "[SUNRISE] Peak RGB: (%u,%u,%u)\n",
            static_cast<unsigned>(_red),
            static_cast<unsigned>(_green),
            static_cast<unsigned>(_blue)
        );

        Serial.println("[SUNRISE] Peak brightness: 100%");

        Serial.printf(
            "[SUNRISE] Auxiliary flash enabled: %lu Hz, duty %u%%\n",
            static_cast<unsigned long>(
                SunriseConfig::AUX_FLASH_FREQUENCY_HZ
            ),
            static_cast<unsigned>(
                SunriseConfig::AUX_FLASH_DUTY_PERCENT
            )
        );

        Serial.printf(
            "[SUNRISE] Auxiliary brightness: %u%%\n",
            static_cast<unsigned>(_auxiliaryBrightnessPercent)
        );
    }

    updateAuxiliaryFlash();
}

// ============================================================
// RGB CALCULATION
// ============================================================

void SunriseLightEffect::calculateColor()
{
    float progress = 0.0f;

    switch (_currentPhase)
    {
        case 1:
        {
            progress = calculateProgress(
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

        case 2:
        {
            progress = calculateProgress(
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

        case 3:
        {
            progress = calculateProgress(
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
// BRIGHTNESS CALCULATION
// ============================================================

void SunriseLightEffect::calculateBrightness()
{
    float progress = 0.0f;

    uint8_t startBrightness = 0;
    uint8_t endBrightness = 0;

    switch (_currentPhase)
    {
        case 1:
            progress = calculateProgress(
                0,
                SunriseConfig::PHASE_1_END_MS
            );

            startBrightness =
                SunriseConfig::PHASE_1_START_BRIGHTNESS;

            endBrightness =
                SunriseConfig::PHASE_1_END_BRIGHTNESS;

            break;

        case 2:
            progress = calculateProgress(
                SunriseConfig::PHASE_1_END_MS,
                SunriseConfig::PHASE_2_END_MS
            );

            startBrightness =
                SunriseConfig::PHASE_2_START_BRIGHTNESS;

            endBrightness =
                SunriseConfig::PHASE_2_END_BRIGHTNESS;

            break;

        case 3:
            progress = calculateProgress(
                SunriseConfig::PHASE_2_END_MS,
                _durationMs
            );

            startBrightness =
                SunriseConfig::PHASE_3_START_BRIGHTNESS;

            endBrightness =
                SunriseConfig::PHASE_3_END_BRIGHTNESS;

            break;

        default:
            return;
    }

    _perceivedBrightness = interpolate(
        startBrightness,
        endBrightness,
        progress
    );

    const float normalized =
        static_cast<float>(_perceivedBrightness) / 100.0f;

    const float corrected = powf(
        normalized,
        SunriseConfig::GAMMA
    );

    int output = static_cast<int>(
        corrected * 255.0f + 0.5f
    );

    if (output < 0)
        output = 0;

    if (output > 255)
        output = 255;

    _brightness = static_cast<uint8_t>(output);
}

// ============================================================
// AUXILIARY FLASH
// ============================================================

void SunriseLightEffect::updateAuxiliaryFlash()
{
    if (!_auxiliaryEnabled)
        return;

    constexpr uint32_t PERIOD_US =
        1000000UL / SunriseConfig::AUX_FLASH_FREQUENCY_HZ;

    constexpr uint32_t ON_TIME_US =
        PERIOD_US * SunriseConfig::AUX_FLASH_DUTY_PERCENT / 100UL;

    constexpr uint32_t OFF_TIME_US =
        PERIOD_US - ON_TIME_US;

    const uint32_t now = micros();

    const uint32_t elapsedUs =
        static_cast<uint32_t>(now - _lastAuxiliaryToggleUs);

    const uint32_t requiredInterval =
        _auxiliaryFlashState ? ON_TIME_US : OFF_TIME_US;

    if (elapsedUs < requiredInterval)
        return;

    _lastAuxiliaryToggleUs = now;

    _auxiliaryFlashState = !_auxiliaryFlashState;

    Serial.printf(
        "[SUNRISE][COB] Flash state changed: %s\n",
        _auxiliaryFlashState ? "ON" : "OFF"
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

    const uint32_t elapsedInPhase =
        _elapsedMs - startMs;

    const uint32_t phaseDuration =
        endMs - startMs;

    return static_cast<float>(elapsedInPhase) /
           static_cast<float>(phaseDuration);
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

    int result = static_cast<int>(value + 0.5f);

    if (result < 0)
        result = 0;

    if (result > 255)
        result = 255;

    return static_cast<uint8_t>(result);
}

// ============================================================
// CLAMP
// ============================================================

uint8_t SunriseLightEffect::clampPercent(uint8_t value)
{
    return value > 100 ? 100 : value;
}

// ============================================================
// DURATION
// ============================================================

void SunriseLightEffect::setDuration(uint32_t durationMs)
{
    const uint32_t previousDuration = _durationMs;

    if (durationMs < SunriseConfig::MIN_DURATION_MS)
    {
        durationMs = SunriseConfig::MIN_DURATION_MS;

        Serial.printf(
            "[SUNRISE][WARNING] Requested duration is too short; "
            "using minimum %lu ms\n",
            static_cast<unsigned long>(durationMs)
        );
    }

    if (previousDuration == durationMs)
        return;

    _durationMs = durationMs;

    Serial.printf(
        "[SUNRISE] Duration changed: %lu -> %lu ms\n",
        static_cast<unsigned long>(previousDuration),
        static_cast<unsigned long>(_durationMs)
    );
}

uint32_t SunriseLightEffect::duration() const
{
    return _durationMs;
}

// ============================================================
// AUXILIARY BRIGHTNESS
// ============================================================

void SunriseLightEffect::setAuxiliaryBrightness(uint8_t percent)
{
    percent = clampPercent(percent);

    if (_auxiliaryBrightnessPercent == percent)
        return;

    const uint8_t previous =
        _auxiliaryBrightnessPercent;

    _auxiliaryBrightnessPercent = percent;

    Serial.printf(
        "[SUNRISE][COB] Brightness changed: %u%% -> %u%%\n",
        static_cast<unsigned>(previous),
        static_cast<unsigned>(_auxiliaryBrightnessPercent)
    );
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
// RGB GETTERS
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

uint8_t SunriseLightEffect::perceivedBrightness() const
{
    return _perceivedBrightness;
}

uint8_t SunriseLightEffect::brightness() const
{
    return _brightness;
}

// ============================================================
// AUXILIARY STATE GETTERS
// ============================================================

bool SunriseLightEffect::auxiliaryEnabled() const
{
    return _auxiliaryEnabled;
}

bool SunriseLightEffect::auxiliaryFlashState() const
{
    return _auxiliaryFlashState;
}