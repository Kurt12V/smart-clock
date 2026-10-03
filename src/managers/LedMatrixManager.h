#pragma once

#include <Arduino.h>

#include "./hardware/light/LedMatrix.h"
#include "./hardware/light/LedMatrixEffects.h"
#include "./managers/SettingsManager.h"

class LedMatrixManager
{
public:

    // ============================================================
    // MODE
    // ============================================================

    enum class Mode : uint8_t
    {
        Lighting = 0,
        Effect
    };

    // ============================================================
    // CONSTRUCTOR
    // ============================================================

    LedMatrixManager(
        uint8_t dataPin,
        SettingsManager& settings
    );

    // ============================================================
    // LIFECYCLE
    // ============================================================

    void begin();

    void update();

    // ============================================================
    // POWER
    // ============================================================

    void on();
    void off();

    bool isOn() const;

    // ============================================================
    // BRIGHTNESS
    // ============================================================

    void setBrightness(
        uint8_t brightness
    );

    uint8_t brightness() const;

    // ============================================================
    // LIGHTING
    // ============================================================

    void setLighting(
        uint8_t r,
        uint8_t g,
        uint8_t b
    );

    uint8_t red() const;
    uint8_t green() const;
    uint8_t blue() const;

    // ============================================================
    // EFFECT
    // ============================================================

    void setEffect(
        uint8_t effect
    );

    uint8_t effect() const;

    // ============================================================
    // SPEED
    // ============================================================

    void setSpeed(
        uint8_t speed
    );

    uint8_t speed() const;

    // ============================================================
    // FIRE
    // ============================================================

    void setFireDirection(
        bool forward
    );

    void setFireDirection(
        LedMatrixEffects::FireDirection direction
    );

    // ============================================================
    // MODE
    // ============================================================

    void setMode(
        Mode mode
    );

    Mode mode() const;

    // ============================================================
    // EFFECT ENGINE
    // ============================================================

    LedMatrixEffects& effects();

    const LedMatrixEffects& effects() const;

    // ============================================================
    // HARDWARE
    // ============================================================

    LedMatrix& matrix();

    const LedMatrix& matrix() const;

private:

    // ============================================================
    // HARDWARE
    // ============================================================

    LedMatrix _matrix;

    LedMatrixEffects _effects;

    // ============================================================
    // SETTINGS
    // ============================================================

    SettingsManager& _settings;

    // ============================================================
    // STATE
    // ============================================================

    Mode _mode;

    bool _isOn;
    bool _initialized;

    uint8_t _brightness;

    uint8_t _effect;
    uint8_t _speed;

    uint8_t _r;
    uint8_t _g;
    uint8_t _b;

    // ============================================================
    // SETTINGS CACHE
    // ============================================================

    bool _lastSettingsEnabled;

    uint8_t _lastSettingsBrightness;
    uint8_t _lastSettingsEffect;
    uint8_t _lastSettingsSpeed;

    // ============================================================
    // INTERNAL
    // ============================================================

    void synchronizeSettings();

    void renderLighting();

    void clear();

    void show();

    LedMatrixEffects::Type effectToType(
        uint8_t effect
    ) const;
};