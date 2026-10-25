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

    // ========================================================
    // MODES
    // ========================================================

    enum class Mode : uint8_t
    {
        MANUAL = 0,
        AUTO   = 1
    };

    // ========================================================
    // PROFILE
    // ========================================================

    struct Profile
    {
        uint8_t min;
        uint8_t max;
        uint8_t base;
    };

    // ========================================================
    // CONFIGURATION
    // ========================================================

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

    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    BrightnessManager();

    // ========================================================
    // LIFECYCLE
    // ========================================================

    void begin(
        SettingsManager* settings,
        SensorManager* sensors,
        ClockSystem* clock
    );

    void update();

    // ========================================================
    // MODE
    // ========================================================

    void setMode(Mode mode);

    Mode getMode() const;

    // ========================================================
    // MANUAL BRIGHTNESS
    // ========================================================

    void setManualBrightness(uint8_t brightness);

    uint8_t getManualBrightness() const;

    // ========================================================
    // CURRENT BRIGHTNESS
    // ========================================================

    uint8_t getBrightness() const;

    uint8_t getTargetBrightness() const;

    // ========================================================
    // CONFIGURATION
    // ========================================================

    void setOffset(int8_t offset);

    int8_t getOffset() const;

    void setSmoothingTime(uint32_t milliseconds);

    void setHysteresis(uint8_t percent);

    // ========================================================
    // STATUS
    // ========================================================

    bool isInitialized() const;

private:

    // ========================================================
    // DEPENDENCIES
    // ========================================================

    SettingsManager* _settings;
    SensorManager* _sensors;
    ClockSystem* _clock;

    // ========================================================
    // STATE
    // ========================================================

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

    // ========================================================
    // MODE / SETTINGS
    // ========================================================

    void loadSettings();

    void saveMode();

    // ========================================================
    // AUTO BRIGHTNESS
    // ========================================================

    uint8_t calculateAutoTarget();

    uint8_t luxToPercent(float lux) const;

    float readLux(bool& valid) const;

    const Profile& selectProfile() const;

    // ========================================================
    // FILTERING
    // ========================================================

    bool applyHysteresis(uint8_t target) const;

    uint8_t applySmoothing(uint8_t target);

    // ========================================================
    // APPLY
    // ========================================================

    void applyBrightness(uint8_t brightness);

    // ========================================================
    // LOGGING
    // ========================================================

    void logState(const char* reason) const;

    void logAutoStatus(
        float lux,
        bool luxValid,
        uint8_t target
    ) const;

    // ========================================================
    // UTILITIES
    // ========================================================

    static uint8_t clampBrightness(int value);

    static uint8_t clampToProfile(
        int value,
        const Profile& profile
    );
};