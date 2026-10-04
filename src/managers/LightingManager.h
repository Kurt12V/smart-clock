#pragma once

#include "SettingsManager.h"

#include "CobLedManager.h"
#include "LedMatrixManager.h"


class LightingManager
{
public:

    explicit LightingManager(
        SettingsManager& settings
    );

    bool begin();
    void update();

private:

    void apply();

private:

    SettingsManager& _settings;

    // ========================================================
    // COB
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
    // MATRIX
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
};