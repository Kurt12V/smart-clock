#include "BrightnessManager.h"

#include <math.h>

// ============================================================
// CONSTRUCTOR
// ============================================================

BrightnessManager::BrightnessManager()
    : _settings(nullptr)
    , _sensors(nullptr)
    , _clock(nullptr)
    , _initialized(false)
    , _mode(Mode::AUTO)
    , _config{
        7,                  // dayStartHour
        22,                 // nightStartHour
        120,                // eveningDurationMinutes
        0,                  // offset
        3,                  // hysteresisPct
        0.2f,               // smoothingAlpha
        1500,               // smoothingTimeMs
        1.0f,               // luxMin
        1000.0f,            // luxMax

        {35, 100, 65},      // day
        {15, 60, 30},       // evening
        {5, 25, 10}         // night
    }
    , _manualBrightness(50)
    , _brightness(50)
    , _sensorTarget(50)
    , _hasAutoTarget(false)
    , _lastUpdateMs(0)
    , _lastLogMs(0)
    , _smoothedBrightness(50.0f)
    , _luxSamples{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
    , _luxSampleCount(0)
    , _luxSampleIndex(0)
    , _lastLuxSampleMs(0)
    , _luxSampleIntervalMs(250)
    , _targetCandidate(50)
    , _targetCandidateSinceMs(0)
    , _hasTargetCandidate(false)
    , _targetChangeThresholdPct(12)
    , _targetConfirmationMs(3000)
{
}

// ============================================================
// BEGIN
// ============================================================

void BrightnessManager::begin(
    SettingsManager* settings,
    SensorManager* sensors,
    ClockSystem* clock
)
{
    Serial0.println();
    Serial0.println(
        "[BRIGHTNESS] Initializing..."
    );

    if (settings == nullptr)
    {
        Serial0.println(
            "[BRIGHTNESS][ERROR] SettingsManager is null"
        );

        return;
    }

    if (sensors == nullptr)
    {
        Serial0.println(
            "[BRIGHTNESS][WARN] SensorManager is null"
        );
    }

    if (clock == nullptr)
    {
        Serial0.println(
            "[BRIGHTNESS][WARN] ClockSystem is null"
        );
    }

    _settings = settings;
    _sensors = sensors;
    _clock = clock;

    loadSettings();

    _lastUpdateMs = millis();
    _lastLogMs = _lastUpdateMs;

    // Reset median filter and pending target after initialization.
    _luxSampleCount = 0;
    _luxSampleIndex = 0;
    _lastLuxSampleMs = _lastUpdateMs - _luxSampleIntervalMs;
    _hasTargetCandidate = false;
    _targetCandidateSinceMs = 0;

    _smoothedBrightness = static_cast<float>(
        _brightness
    );

    _sensorTarget = _brightness;
    _hasAutoTarget = false;

    _initialized = true;

    // --------------------------------------------------------
    // INITIAL BRIGHTNESS
    // --------------------------------------------------------

    if (_mode == Mode::AUTO)
    {
        const uint8_t target = calculateAutoTarget();

        _sensorTarget = target;
        _smoothedBrightness = static_cast<float>(
            target
        );

        _hasAutoTarget = true;

        applyBrightness(target);

        logState("AUTO initialization");
    }
    else
    {
        applyBrightness(_manualBrightness);

        logState("MANUAL initialization");
    }

    Serial0.println(
        "[BRIGHTNESS] Initialization complete"
    );

    Serial0.printf(
        "[BRIGHTNESS] Mode: %s\n",
        _mode == Mode::AUTO ? "AUTO" : "MANUAL"
    );

    Serial0.printf(
        "[BRIGHTNESS] Current: %u%%\n",
        static_cast<unsigned>(_brightness)
    );

    Serial0.println();
}

// ============================================================
// UPDATE
// ============================================================

void BrightnessManager::update()
{
    if (!_initialized)
    {
        return;
    }

    const uint32_t now = millis();

    // --------------------------------------------------------
    // MANUAL MODE
    // --------------------------------------------------------

    if (_mode == Mode::MANUAL)
    {
        if (_brightness != _manualBrightness)
        {
            applyBrightness(_manualBrightness);
        }

        _lastUpdateMs = now;

        return;
    }

    // --------------------------------------------------------
    // AUTO MODE
    // --------------------------------------------------------

    const uint8_t calculatedTarget = calculateAutoTarget();

    // --------------------------------------------------------
    // LARGE-CHANGE GATE + CONFIRMATION DELAY
    // --------------------------------------------------------

    if (!_hasAutoTarget)
    {
        _sensorTarget = calculatedTarget;
        _hasAutoTarget = true;
        _hasTargetCandidate = false;

        _smoothedBrightness = static_cast<float>(_brightness);
    }
    else
    {
        const int differenceFromCurrent = abs(
            static_cast<int>(calculatedTarget)
            - static_cast<int>(_sensorTarget)
        );

        // Ignore small target fluctuations completely.
        if (differenceFromCurrent < static_cast<int>(_targetChangeThresholdPct))
        {
            _hasTargetCandidate = false;
        }
        else
        {
            const int differenceFromCandidate = abs(
                static_cast<int>(calculatedTarget)
                - static_cast<int>(_targetCandidate)
            );

            // Start or restart the confirmation timer only when the candidate
            // changes substantially. Small sensor fluctuations do not reset it.
            if (!_hasTargetCandidate || differenceFromCandidate > 5)
            {
                _targetCandidate = calculatedTarget;
                _targetCandidateSinceMs = now;
                _hasTargetCandidate = true;

                Serial0.printf(
                    "[BRIGHTNESS][PENDING] Candidate=%u%%; waiting %lu ms\n",
                    static_cast<unsigned>(_targetCandidate),
                    static_cast<unsigned long>(_targetConfirmationMs)
                );
            }
            else
            {
                // Follow minor variations while preserving the original timer.
                _targetCandidate = static_cast<uint8_t>(
                    (static_cast<unsigned>(_targetCandidate) * 3U
                     + static_cast<unsigned>(calculatedTarget)) / 4U
                );
            }

            if (_hasTargetCandidate
                && now - _targetCandidateSinceMs >= _targetConfirmationMs)
            {
                if (applyHysteresis(_targetCandidate))
                {
                    _sensorTarget = _targetCandidate;

                    Serial0.printf(
                        "[BRIGHTNESS][TARGET] Confirmed target: %u%%\n",
                        static_cast<unsigned>(_sensorTarget)
                    );
                }

                _hasTargetCandidate = false;
            }
        }
    }

    // --------------------------------------------------------
    // SMOOTHING
    // --------------------------------------------------------

    const uint8_t smoothed = applySmoothing(
        _sensorTarget
    );

    // --------------------------------------------------------
    // APPLY
    // --------------------------------------------------------

    if (smoothed != _brightness)
    {
        applyBrightness(smoothed);
    }

    _lastUpdateMs = now;

    // --------------------------------------------------------
    // PERIODIC LOGGING
    // --------------------------------------------------------

    if (now - _lastLogMs >= 2000)
    {
        bool luxValid = false;

        const float lux = readLux(luxValid);

        logAutoStatus(
            lux,
            luxValid,
            _sensorTarget
        );

        _lastLogMs = now;
    }
}

// ============================================================
// LOAD SETTINGS
// ============================================================

void BrightnessManager::loadSettings()
{
    if (_settings == nullptr)
    {
        return;
    }

    // --------------------------------------------------------
    // BRIGHTNESS
    // --------------------------------------------------------

    const int savedBrightness = _settings->get(
        SettingsManager::Param::BRIGHTNESS
    );

    _manualBrightness = clampBrightness(
        savedBrightness
    );

    _brightness = _manualBrightness;

    // --------------------------------------------------------
    // MODE
    // --------------------------------------------------------

    const int savedMode = _settings->get(
        SettingsManager::Param::BRIGHTNESS_MODE
    );

    _mode = savedMode == 1
        ? Mode::AUTO
        : Mode::MANUAL;

    Serial0.printf(
        "[BRIGHTNESS][SETTINGS] Loaded brightness: %u%%\n",
        static_cast<unsigned>(_manualBrightness)
    );

    Serial0.printf(
        "[BRIGHTNESS][SETTINGS] Loaded mode: %s\n",
        _mode == Mode::AUTO ? "AUTO" : "MANUAL"
    );
}

// ============================================================
// SAVE MODE
// ============================================================

void BrightnessManager::saveMode()
{
    if (_settings == nullptr)
    {
        return;
    }

    const int value = _mode == Mode::AUTO ? 1 : 0;

    _settings->set(
        SettingsManager::Param::BRIGHTNESS_MODE,
        value
    );

    Serial0.printf(
        "[BRIGHTNESS][SETTINGS] Mode queued for saving: %s\n",
        _mode == Mode::AUTO ? "AUTO" : "MANUAL"
    );
}

// ============================================================
// SET MODE
// ============================================================

void BrightnessManager::setMode(Mode mode)
{
    if (!_initialized)
    {
        Serial0.println(
            "[BRIGHTNESS][WARN] setMode called before begin()"
        );

        return;
    }

    if (mode != Mode::MANUAL && mode != Mode::AUTO)
    {
        Serial0.println(
            "[BRIGHTNESS][WARN] Invalid mode"
        );

        return;
    }

    if (_mode == mode)
    {
        return;
    }

    Serial0.printf(
        "[BRIGHTNESS][MODE] %s -> %s\n",
        _mode == Mode::AUTO ? "AUTO" : "MANUAL",
        mode == Mode::AUTO ? "AUTO" : "MANUAL"
    );

    _mode = mode;

    saveMode();

    _lastUpdateMs = millis();

    if (_mode == Mode::MANUAL)
    {
        // Последнее применённое значение яркости
        // используется при переходе в ручной режим.
        if (_settings != nullptr)
        {
            _manualBrightness = clampBrightness(
                _settings->get(
                    SettingsManager::Param::BRIGHTNESS
                )
            );
        }

        _smoothedBrightness = static_cast<float>(
            _manualBrightness
        );

        applyBrightness(_manualBrightness);

        logState("Switched to MANUAL");
    }
    else
    {
        _hasAutoTarget = false;

        _smoothedBrightness = static_cast<float>(
            _brightness
        );

        logState("Switched to AUTO");
    }
}

// ============================================================
// GET MODE
// ============================================================

BrightnessManager::Mode BrightnessManager::getMode() const
{
    return _mode;
}

// ============================================================
// SET MANUAL BRIGHTNESS
// ============================================================

void BrightnessManager::setManualBrightness(
    uint8_t brightness
)
{
    if (!_initialized)
    {
        Serial0.println(
            "[BRIGHTNESS][WARN] setManualBrightness called before begin()"
        );

        return;
    }

    brightness = clampBrightness(
        static_cast<int>(brightness)
    );

    _manualBrightness = brightness;

    Serial0.printf(
        "[BRIGHTNESS][MANUAL] Requested brightness: %u%%\n",
        static_cast<unsigned>(brightness)
    );

    if (_mode == Mode::MANUAL)
    {
        _smoothedBrightness = static_cast<float>(
            brightness
        );

        applyBrightness(brightness);
    }
    else
    {
        // В AUTO сохраняем ручное значение как текущее
        // заданное значение в настройках.
        // При следующем обновлении AUTO применит
        // рассчитанную по датчику яркость.
        if (_settings != nullptr)
        {
            _settings->set(
                SettingsManager::Param::BRIGHTNESS,
                static_cast<int>(brightness)
            );
        }
    }
}

// ============================================================
// GET MANUAL BRIGHTNESS
// ============================================================

uint8_t BrightnessManager::getManualBrightness() const
{
    return _manualBrightness;
}

// ============================================================
// GET CURRENT BRIGHTNESS
// ============================================================

uint8_t BrightnessManager::getBrightness() const
{
    return _brightness;
}

// ============================================================
// GET TARGET BRIGHTNESS
// ============================================================

uint8_t BrightnessManager::getTargetBrightness() const
{
    return _mode == Mode::AUTO
        ? _sensorTarget
        : _manualBrightness;
}

// ============================================================
// SET OFFSET
// ============================================================

void BrightnessManager::setOffset(int8_t offset)
{
    _config.offset = offset;

    Serial0.printf(
        "[BRIGHTNESS][CONFIG] Offset: %d%%\n",
        static_cast<int>(_config.offset)
    );
}

// ============================================================
// GET OFFSET
// ============================================================

int8_t BrightnessManager::getOffset() const
{
    return _config.offset;
}

// ============================================================
// SET SMOOTHING TIME
// ============================================================

void BrightnessManager::setSmoothingTime(
    uint32_t milliseconds
)
{
    _config.smoothingTimeMs = milliseconds;

    Serial0.printf(
        "[BRIGHTNESS][CONFIG] Smoothing time: %lu ms\n",
        static_cast<unsigned long>(milliseconds)
    );
}

// ============================================================
// SET HYSTERESIS
// ============================================================

void BrightnessManager::setHysteresis(uint8_t percent)
{
    if (percent > 25)
    {
        percent = 25;
    }

    _config.hysteresisPct = percent;

    Serial0.printf(
        "[BRIGHTNESS][CONFIG] Hysteresis: %u%%\n",
        static_cast<unsigned>(percent)
    );
}

// ============================================================
// SET TARGET CHANGE THRESHOLD
// ============================================================

void BrightnessManager::setTargetChangeThreshold(uint8_t percent)
{
    if (percent > 50)
    {
        percent = 50;
    }

    _targetChangeThresholdPct = percent;
    _hasTargetCandidate = false;

    Serial0.printf(
        "[BRIGHTNESS][CONFIG] Target change threshold: %u%%\n",
        static_cast<unsigned>(_targetChangeThresholdPct)
    );
}

// ============================================================
// SET TARGET CONFIRMATION DELAY
// ============================================================

void BrightnessManager::setTargetConfirmationDelay(uint32_t milliseconds)
{
    _targetConfirmationMs = milliseconds;
    _hasTargetCandidate = false;

    Serial0.printf(
        "[BRIGHTNESS][CONFIG] Target confirmation delay: %lu ms\n",
        static_cast<unsigned long>(_targetConfirmationMs)
    );
}

// ============================================================
// IS INITIALIZED
// ============================================================

bool BrightnessManager::isInitialized() const
{
    return _initialized;
}

// ============================================================
// CALCULATE AUTO TARGET
// ============================================================

uint8_t BrightnessManager::calculateAutoTarget()
{
    const Profile& profile = selectProfile();

    bool luxValid = false;
    const float rawLux = readLux(luxValid);
    float lux = rawLux;

    if (luxValid)
    {
        const uint32_t now = millis();

        // Sample at a fixed interval so calling update() very frequently
        // does not fill the median window with duplicate readings.
        if (_luxSampleCount == 0
            || now - _lastLuxSampleMs >= _luxSampleIntervalMs)
        {
            _luxSamples[_luxSampleIndex] = rawLux;
            _luxSampleIndex = static_cast<uint8_t>(
                (_luxSampleIndex + 1U) % 7U
            );

            if (_luxSampleCount < 7)
            {
                ++_luxSampleCount;
            }

            _lastLuxSampleMs = now;
        }

        if (_luxSampleCount > 0)
        {
            float sorted[7];

            for (uint8_t i = 0; i < _luxSampleCount; ++i)
            {
                sorted[i] = _luxSamples[i];
            }

            // Insertion sort is inexpensive for a maximum of seven samples.
            for (uint8_t i = 1; i < _luxSampleCount; ++i)
            {
                const float value = sorted[i];
                int j = static_cast<int>(i) - 1;

                while (j >= 0 && sorted[j] > value)
                {
                    sorted[j + 1] = sorted[j];
                    --j;
                }

                sorted[j + 1] = value;
            }

            lux = sorted[_luxSampleCount / 2U];
        }
    }

    // If the sensor temporarily reports invalid data, keep the last target
    // instead of jumping to the profile base brightness.
    if (!luxValid && _hasAutoTarget)
    {
        return _sensorTarget;
    }

    int target = profile.base;

    if (luxValid)
    {
        const uint8_t luxPercent = luxToPercent(lux);

        target = profile.min
            + (
                static_cast<int>(profile.max - profile.min)
                * static_cast<int>(luxPercent)
            ) / 100;
    }

    target += _config.offset;

    const uint8_t result = clampToProfile(
        target,
        profile
    );

    return result;
}

// ============================================================
// LUX TO PERCENT
// ============================================================

uint8_t BrightnessManager::luxToPercent(float lux) const
{
    if (!isfinite(lux))
    {
        return 0;
    }

    if (lux <= _config.luxMin)
    {
        return 0;
    }

    if (lux >= _config.luxMax)
    {
        return 100;
    }

    const float range = _config.luxMax - _config.luxMin;

    if (range <= 0.0f)
    {
        return 0;
    }

    float normalized = (
        lux - _config.luxMin
    ) / range;

    normalized = constrain(
        normalized,
        0.0f,
        1.0f
    );

    const float mapped = powf(
        normalized,
        0.6f
    ) * 100.0f;

    return clampBrightness(
        static_cast<int>(lroundf(mapped))
    );
}

// ============================================================
// READ LUX
// ============================================================

float BrightnessManager::readLux(bool& valid) const
{
    valid = false;

    if (_sensors == nullptr)
    {
        return 0.0f;
    }

    const SensorData& data = _sensors->getData();

    if (!data.lightValid)
    {
        return 0.0f;
    }

    if (!isfinite(data.lightLux))
    {
        return 0.0f;
    }

    if (data.lightLux < 0.0f)
    {
        return 0.0f;
    }

    valid = true;

    return data.lightLux;
}

// ============================================================
// SELECT PROFILE
// ============================================================

const BrightnessManager::Profile&
BrightnessManager::selectProfile() const
{
    // Если часы недоступны, используем безопасный
    // ночной профиль.
    if (_clock == nullptr || !_clock->isTimeValid())
    {
        return _config.night;
    }

    const int hour = _clock->hour();
    const int minute = _clock->minute();

    if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
    {
        return _config.night;
    }

    const int currentMinutes = hour * 60 + minute;

    const int dayStart = (
        static_cast<int>(_config.dayStartHour) * 60
    );

    const int nightStart = (
        static_cast<int>(_config.nightStartHour) * 60
    );

    const int eveningStart = nightStart
        - static_cast<int>(_config.eveningDurationMinutes);

    // День: 07:00–20:00 при настройках по умолчанию.
    if (
        currentMinutes >= dayStart
        && currentMinutes < eveningStart
    )
    {
        return _config.day;
    }

    // Вечер: 20:00–22:00 при настройках по умолчанию.
    if (
        currentMinutes >= eveningStart
        && currentMinutes < nightStart
    )
    {
        return _config.evening;
    }

    // Ночь: 22:00–07:00.
    return _config.night;
}

// ============================================================
// APPLY HYSTERESIS
// ============================================================

bool BrightnessManager::applyHysteresis(
    uint8_t target
) const
{
    const int difference = abs(
        static_cast<int>(target)
        - static_cast<int>(_sensorTarget)
    );

    const uint8_t requiredDifference =
        _config.hysteresisPct > _targetChangeThresholdPct
            ? _config.hysteresisPct
            : _targetChangeThresholdPct;

    return difference >= static_cast<int>(requiredDifference);
}

// ============================================================
// APPLY SMOOTHING
// ============================================================

uint8_t BrightnessManager::applySmoothing(
    uint8_t target
)
{
    const uint32_t now = millis();

    uint32_t deltaTime = now - _lastUpdateMs;

    if (deltaTime > 1000)
    {
        deltaTime = 1000;
    }

    if (
        _config.smoothingTimeMs == 0
        || deltaTime == 0
    )
    {
        if (_config.smoothingTimeMs == 0)
        {
            _smoothedBrightness = static_cast<float>(
                target
            );
        }

        return clampBrightness(
            static_cast<int>(
                lroundf(_smoothedBrightness)
            )
        );
    }

    const float alpha = 1.0f - expf(
        -static_cast<float>(deltaTime)
        / static_cast<float>(_config.smoothingTimeMs)
    );

    _smoothedBrightness += (
        static_cast<float>(target)
        - _smoothedBrightness
    ) * alpha;

    return clampBrightness(
        static_cast<int>(
            lroundf(_smoothedBrightness)
        )
    );
}

// ============================================================
// APPLY BRIGHTNESS
// ============================================================

void BrightnessManager::applyBrightness(
    uint8_t brightness
)
{
    brightness = clampBrightness(
        static_cast<int>(brightness)
    );

    const uint8_t previous = _brightness;

    _brightness = brightness;

    // Сохраняем текущее применённое значение
    // в SettingsManager.
    //
    // SettingsManager::update() отвечает за отложенную
    // запись во Flash. Здесь нет прямой записи в NVS.
    if (_settings != nullptr)
    {
        const int savedBrightness = _settings->get(
            SettingsManager::Param::BRIGHTNESS
        );

        if (savedBrightness != static_cast<int>(brightness))
        {
            _settings->set(
                SettingsManager::Param::BRIGHTNESS,
                static_cast<int>(brightness)
            );
        }
    }

    if (previous != brightness)
    {
        Serial0.printf(
            "[BRIGHTNESS][APPLY] %u%% -> %u%%\n",
            static_cast<unsigned>(previous),
            static_cast<unsigned>(brightness)
        );
    }
}

// ============================================================
// LOG STATE
// ============================================================

void BrightnessManager::logState(
    const char* reason
) const
{
    Serial0.printf(
        "[BRIGHTNESS][STATE] %s | mode=%s | brightness=%u%% | manual=%u%% | target=%u%% | offset=%d%%\n",
        reason,
        _mode == Mode::AUTO ? "AUTO" : "MANUAL",
        static_cast<unsigned>(_brightness),
        static_cast<unsigned>(_manualBrightness),
        static_cast<unsigned>(_sensorTarget),
        static_cast<int>(_config.offset)
    );
}

// ============================================================
// LOG AUTO STATUS
// ============================================================

void BrightnessManager::logAutoStatus(
    float lux,
    bool luxValid,
    uint8_t target
) const
{
    if (luxValid)
    {
        Serial0.printf(
            "[BRIGHTNESS][AUTO] lux=%.2f | target=%u%% | current=%u%% | threshold=%u%% | delay=%lu ms | offset=%d%%\n",
            static_cast<double>(lux),
            static_cast<unsigned>(target),
            static_cast<unsigned>(_brightness),
            static_cast<unsigned>(_targetChangeThresholdPct),
            static_cast<unsigned long>(_targetConfirmationMs),
            static_cast<int>(_config.offset)
        );
    }
    else
    {
        Serial0.printf(
            "[BRIGHTNESS][AUTO] lux=INVALID | profile base used | target=%u%% | current=%u%%\n",
            static_cast<unsigned>(target),
            static_cast<unsigned>(_brightness)
        );
    }
}

// ============================================================
// CLAMP BRIGHTNESS
// ============================================================

uint8_t BrightnessManager::clampBrightness(int value)
{
    if (value < 0)
    {
        return 0;
    }

    if (value > 100)
    {
        return 100;
    }

    return static_cast<uint8_t>(value);
}

// ============================================================
// CLAMP TO PROFILE
// ============================================================

uint8_t BrightnessManager::clampToProfile(
    int value,
    const Profile& profile
)
{
    if (value < static_cast<int>(profile.min))
    {
        value = profile.min;
    }

    if (value > static_cast<int>(profile.max))
    {
        value = profile.max;
    }

    return clampBrightness(value);
}