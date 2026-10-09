
#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "SettingsManager.h"
#include "CobLedManager.h"
#include "LedMatrixManager.h"

class LightingManager
{
public:
    explicit LightingManager(SettingsManager& settings);

    bool begin();
    void update();

    // Временное управление светом для рассвета и будильника.
    void beginAlarmOverride();

    void setAlarmMatrix(
        uint8_t brightness,
        uint8_t red,
        uint8_t green,
        uint8_t blue
    );

    void setAlarmCob(uint8_t brightness);

    void endAlarmOverride();

    bool isAlarmOverrideActive() const;

private:
    void apply();

private:
    SettingsManager& _settings;

    // ========================================================
    // COB HARDWARE
    // ========================================================

    CobLed _cob1;
    CobLed _cob2;
    CobLed _cob3;
    CobLed _cob4;

    CobLedManager _cob;

    uint8_t _cobBrightness;
    uint8_t _cobEffect;
    uint8_t _cobSpeed;
    bool _cobEnabled;

    // ========================================================
    // MATRIX HARDWARE
    // ========================================================

    LedMatrixManager _matrix;

    uint8_t _matrixBrightness;
    uint8_t _matrixEffect;
    uint8_t _matrixSpeed;
    bool _matrixEnabled;

    // ========================================================
    // STATE
    // ========================================================

    bool _initialized;
    bool _firstApply;

    // Когда true, обычные настройки и эффекты
    // не должны перезаписывать освещение будильника.
    bool _alarmOverride;
};