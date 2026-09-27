#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "Config.h"

// ============================================================
// PARAM ID
// ============================================================

enum class Param : uint8_t
{
    DisplayBrightness1 = 0,
    DisplayBrightness2,
    DisplayBrightness3,
    DisplayBrightness4,

    MatrixEnabled,
    MatrixBrightness,

    CobEnabled,
    CobBrightness1,
    CobBrightness2,
    CobBrightness3,
    CobBrightness4,

    AudioVolume,
    MicrophoneEnabled,
    UtcOffset,

    COUNT
};

// ============================================================
// SETTINGS MANAGER
// ============================================================

class SettingsManager
{
public:

    SettingsManager();

    bool begin();

    // ------------------------------------
    // generic API
    // ------------------------------------

    int  get(Param p) const;
    bool set(Param p, int value);        // clamp по Config
    bool save(Param p);                  // RAM -> Preferences
    bool load(Param p);                  // Preferences -> RAM

    bool loadAll();
    bool saveAll();
    void resetAll();

    // ------------------------------------
    // names (для web API)
    // ------------------------------------

    static const char* paramName(Param p);
    static bool        paramFromName(const char* name, Param& out);

private:

    Preferences _preferences;
    int         _values[static_cast<size_t>(Param::COUNT)];
    bool        _initialized;

    void applyDefaults();
};