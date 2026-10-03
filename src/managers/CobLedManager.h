#pragma once

#include <Arduino.h>

#include "./hardware/light/CobLed.h"
#include "./hardware/light/CobEffects.h"
#include "./managers/SettingsManager.h"

// ============================================================
// COB LED MANAGER
//
// Central controller for 4 COB LEDs.
//
// Normal mode:
//     SettingsManager -> CobLedManager -> CobLed
//
// Effect mode:
//     Alarm/Web/etc -> CobLedManager -> CobEffects -> CobLed
// ============================================================

class CobLedManager
{
public:

    // ============================================================
    // MODE
    // ============================================================

    enum class Mode : uint8_t
    {
        Normal = 0,
        Effect
    };

    // ============================================================
    // CONSTRUCTOR
    // ============================================================

    CobLedManager(
        CobLed& cob1,
        CobLed& cob2,
        CobLed& cob3,
        CobLed& cob4,
        SettingsManager& settings
    );

    // ============================================================
    // LIFECYCLE
    // ============================================================

    bool begin();

    void update();

    // ============================================================
    // INDIVIDUAL COB
    //
    // cob = 1..4
    // brightness = 0..255
    // ============================================================

    void set(
        uint8_t cob,
        uint8_t brightness
    );

    uint8_t get(
        uint8_t cob
    ) const;

    void increase(
        uint8_t cob,
        uint8_t value = 5
    );

    void decrease(
        uint8_t cob,
        uint8_t value = 5
    );

    void fade(
        uint8_t cob,
        uint8_t target,
        uint32_t durationMs
    );

    void on(
        uint8_t cob
    );

    void off(
        uint8_t cob
    );

    void toggle(
        uint8_t cob
    );

    bool isOn(
        uint8_t cob
    ) const;

    // ============================================================
    // ALL COB
    // ============================================================

    void setAll(
        uint8_t brightness
    );

    void onAll();
    void offAll();

    // ============================================================
    // EFFECTS
    // ============================================================

    void setEffect(
        CobEffectType effect
    );

    CobEffectType getEffect() const;

    void setSpeed(
        uint8_t speed
    );

    uint8_t getSpeed() const;

    void enableEffects();
    void disableEffects();

    bool effectsEnabled() const;

    // ============================================================
    // EFFECT ENGINE
    // ============================================================

    CobEffects& effects();
    const CobEffects& effects() const;

    // ============================================================
    // MODE
    // ============================================================

    void setMode(
        Mode mode
    );

    Mode getMode() const;

    bool isEffectMode() const;

    // ============================================================
    // SETTINGS
    // ============================================================

    void applySettings();

    // ============================================================
    // RETURN TO NORMAL SETTINGS
    // ============================================================

    void restoreNormalMode();

private:

    // ============================================================
    // HARDWARE
    // ============================================================

    CobLed& _cob1;
    CobLed& _cob2;
    CobLed& _cob3;
    CobLed& _cob4;

    SettingsManager& _settings;

    // ============================================================
    // EFFECT ENGINE
    // ============================================================

    CobEffects _effects;

    // ============================================================
    // STATE
    // ============================================================

    Mode _mode;

    bool _initialized;

    // ============================================================
    // SETTINGS CACHE
    // ============================================================

    uint8_t _lastBrightness;
    bool _lastEnabled;
    uint8_t _lastEffect;
    uint8_t _lastSpeed;

    // ============================================================
    // INTERNAL
    // ============================================================

    CobLed& getCob(
        uint8_t cob
    );

    const CobLed& getCob(
        uint8_t cob
    ) const;

    void updateNormal();
    void updateEffects();
};