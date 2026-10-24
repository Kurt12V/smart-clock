#pragma once

#include <Arduino.h>

#include <math.h>

#include "./managers/Setting/SettingsManager.h"
#include "./managers/Sensors/SensorsManager.h"
#include "./core/ClockSystem.h"

// ============================================================
// SCREEN BRIGHTNESS SINK
// ============================================================
//
// Интерфейс управления подсветкой дисплеев.
//
// BrightnessManager не управляет COB и WS2812.
//
// value: 0..100
//
// Если все четыре дисплея используют общую подсветку,
// реализация может применить значение ко всем одновременно.
// ============================================================

class IScreenBrightnessSink
{
public:
    virtual ~IScreenBrightnessSink() = default;

    virtual void setScreenBrightness(int value) = 0;
};

// ============================================================
// BRIGHTNESS MANAGER
// ============================================================

class BrightnessManager
{
public:

    enum class Mode : uint8_t
    {
        MANUAL = 0,
        AUTO
    };

    struct Profile
    {
        int min;
        int max;
        int base;
    };

    struct Config
    {
        // Время суток: минуты от полуночи.
        uint16_t dayStartMinute;
        uint16_t nightStartMinute;
        uint16_t eveningDurationMin;

        Profile day;
        Profile evening;
        Profile night;

        // Пользовательская поправка.
        // После поправки результат ограничивается профилем.
        int offset;

        // Мёртвая зона гистерезиса.
        int hysteresisPct;

        // Сохраняется для совместимости конфигурации.
        // Используется как коэффициент сглаживания
        // при обновлении с интервалом 1000 мс.
        float smoothingAlpha;

        // Калибровка датчика освещённости.
        float luxMin;
        float luxMax;

        // Постоянная времени сглаживания, миллисекунды.
        uint32_t smoothingTimeMs;
    };

    BrightnessManager();

    // --------------------------------------------------------
    // LIFECYCLE
    // --------------------------------------------------------

    void begin(
        SettingsManager*       settings,
        SensorManager*         sensors,
        ClockSystem*           clock,
        IScreenBrightnessSink* sink
    );

    void update();

    // --------------------------------------------------------
    // MODE
    // --------------------------------------------------------

    Mode mode() const
    {
        return _mode;
    }

    void setMode(Mode mode);

    // --------------------------------------------------------
    // MANUAL BRIGHTNESS
    // --------------------------------------------------------

    void setManualBrightness(int value);

    int manualBrightness() const
    {
        return _manualBrightness;
    }

    // --------------------------------------------------------
    // CONFIGURATION
    // --------------------------------------------------------

    Config& config()
    {
        return _config;
    }

    const Config& config() const
    {
        return _config;
    }

    // --------------------------------------------------------
    // STATE / DEBUG
    // --------------------------------------------------------

    int brightness() const
    {
        return _brightness;
    }

    int sensorTarget() const
    {
        return _sensorTarget;
    }

    int profileTarget() const
    {
        return _profileTarget;
    }

    bool initialized() const
    {
        return _initialized;
    }

private:

    // --------------------------------------------------------
    // PROFILE
    // --------------------------------------------------------

    Profile selectProfile(uint16_t minuteOfDay) const;

    Profile normalizeProfile(const Profile& profile) const;

    // --------------------------------------------------------
    // SENSOR
    // --------------------------------------------------------

    int luxToPercent(float lux) const;

    bool readLux(float& lux) const;

    // --------------------------------------------------------
    // TARGET CALCULATION
    // --------------------------------------------------------

    int calculateAutoTarget();

    int calculateSafeTarget() const;

    int clampBrightness(int value) const;

    // --------------------------------------------------------
    // FILTERING
    // --------------------------------------------------------

    bool applyHysteresis(int candidate, int current) const;

    int applySmoothing(
        int candidate,
        uint32_t deltaTimeMs
    );

    // --------------------------------------------------------
    // OUTPUT
    // --------------------------------------------------------

    void applyBrightness(int value);

    // --------------------------------------------------------
    // DEPENDENCIES
    // --------------------------------------------------------

    SettingsManager*        _settings;
    SensorManager*          _sensors;
    ClockSystem*            _clock;
    IScreenBrightnessSink*  _sink;

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    Mode   _mode;
    Config _config;

    int _manualBrightness;
    int _brightness;

    int _sensorTarget;
    int _profileTarget;

    bool _initialized;
    bool _hasAutoTarget;

    uint32_t _lastUpdateMs;
};