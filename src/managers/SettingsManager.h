#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "Config.h"

// ============================================================
// PARAM ID
// ============================================================

enum class Param : uint8_t
{
    DisplayBrightness = 0,

    MatrixEnabled,
    MatrixBrightness,

    CobEnabled,
    CobBrightness1,
    CobBrightness2,
    CobBrightness3,
    CobBrightness4,

    // --------------------------------------------------------
    // Громкости по стримам (как на телефоне)
    // --------------------------------------------------------

    VolumeMedia,      // музыка с SD
    VolumeAlarm,      // будильник
    VolumeSystem,     // стартовый звук, клики

    MicrophoneEnabled,
    UtcOffset,
    CobEffect,     // 0=Static, 1=Breath, 2=Strobe, 3=Wave
    CobSpeed,      // 0..100
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

    int  get(Param p) const;
    bool set(Param p, int value);
    bool save(Param p);
    bool load(Param p);

    bool loadAll();
    bool saveAll();
    void resetAll();

    static const char* paramName(Param p);
    static bool        paramFromName(const char* name, Param& out);

private:

    Preferences _preferences;
    int         _values[static_cast<size_t>(Param::COUNT)];
    bool        _initialized;

    void applyDefaults();
};