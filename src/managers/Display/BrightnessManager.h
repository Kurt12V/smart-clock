#pragma once

#include <Arduino.h>

#include "./managers/Setting/SettingsManager.h"
#include "./managers/Sensors/SensorsManager.h"
#include "./core/ClockSystem.h"

// ============================================================
// BRIGHTNESS MANAGER
// ============================================================

class BrightnessManager
{
public:
    enum class Mode : uint8_t
    {
        MANUAL = 0,
        AUTO   = 1
    };

    struct Profile
    {
        uint8_t min;
        uint8_t max;
        uint8_t base;
    };

    struct Config
    {
        uint8_t dayStartHour;
        uint8_t nightStartHour;
        uint16_t eveningDurationMinutes;
        int8_t offset;
        uint8_t hysteresisPct;
        float smoothingAlpha;
        uint32_t smoothingTimeMs;
        float luxMin;
        float luxMax;
        Profile day;
        Profile evening;
        Profile night;
    };

    BrightnessManager();

    void begin(
        SettingsManager* settings,
        SensorManager* sensors,
        ClockSystem* clock
    );

    void update();

    void setMode(Mode mode);
    Mode getMode() const;

    void setManualBrightness(uint8_t brightness);
    uint8_t getManualBrightness() const;

    uint8_t getBrightness() const;
    uint8_t getTargetBrightness() const;

    void setOffset(int8_t offset);
    int8_t getOffset() const;

    void setSmoothingTime(uint32_t milliseconds);
    void setHysteresis(uint8_t percent);

    // Auto mode stability controls.
    // Default threshold: 12 brightness percentage points.
    void setTargetChangeThreshold(uint8_t percent);

    // Default confirmation delay: 3000 ms.
    void setTargetConfirmationDelay(uint32_t milliseconds);

    bool isInitialized() const;

private:
    SettingsManager* _settings;
    SensorManager* _sensors;
    ClockSystem* _clock;

    bool _initialized;
    Mode _mode;
    Config _config;

    uint8_t _manualBrightness;
    uint8_t _brightness;
    uint8_t _sensorTarget;
    bool _hasAutoTarget;

    uint32_t _lastUpdateMs;
    uint32_t _lastLogMs;
    float _smoothedBrightness;

    // Median filter for ambient light readings.
    static constexpr uint8_t LUX_SAMPLE_COUNT = 7;
    float _luxSamples[LUX_SAMPLE_COUNT];
    uint8_t _luxSampleCount;
    uint8_t _luxSampleIndex;
    uint32_t _lastLuxSampleMs;
    uint32_t _luxSampleIntervalMs;

    // Target must differ enough and remain stable before acceptance.
    uint8_t _targetCandidate;
    uint32_t _targetCandidateSinceMs;
    bool _hasTargetCandidate;
    uint8_t _targetChangeThresholdPct;
    uint32_t _targetConfirmationMs;

    void loadSettings();
    void saveMode();

    uint8_t calculateAutoTarget();
    uint8_t luxToPercent(float lux) const;
    float readLux(bool& valid) const;
    const Profile& selectProfile() const;

    bool applyHysteresis(uint8_t target) const;
    uint8_t applySmoothing(uint8_t target);
    void applyBrightness(uint8_t brightness);

    void logState(const char* reason) const;
    void logAutoStatus(float lux, bool luxValid, uint8_t target) const;

    static uint8_t clampBrightness(int value);
    static uint8_t clampToProfile(int value, const Profile& profile);
};
