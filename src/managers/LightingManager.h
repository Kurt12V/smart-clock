#pragma once

#include "SettingsManager.h"
#include "./hardware/light/CobLed.h"
#include "CobLedManager.h"

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

    CobLed _cob1;
    CobLed _cob2;
    CobLed _cob3;
    CobLed _cob4;

    CobLedManager _cob;

    bool _initialized;
};