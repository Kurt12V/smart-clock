#pragma once

#include <Arduino.h>
#include <Preferences.h>

// ============================================================
// SETTINGS MANAGER
// ============================================================

class SettingsManager
{
public:

    // ========================================================
    // PARAMETERS
    // ========================================================

    enum class Param : uint8_t
    {
        BRIGHTNESS = 0,

        MATRIX_ENABLED,
        MATRIX_BRIGHTNESS,
        MATRIX_EFFECT,
        MATRIX_SPEED,

        COB_ENABLED,
        COB_BRIGHTNESS,
        COB_EFFECT,
        COB_SPEED,

        VOLUME_MEDIA,
        VOLUME_ALARM,
        VOLUME_SYSTEM,

        MICROPHONE_ENABLED,

        UTC_OFFSET,

        COUNT
    };


    // ========================================================
    // PARAMETER DESCRIPTION
    // ========================================================

    struct ParamDesc
    {
        Param param;

        const char* name;
        const char* key;

        int minValue;
        int maxValue;
        int defaultValue;
    };


    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    SettingsManager();


    // ========================================================
    // LIFECYCLE
    // ========================================================

    bool begin();
    void update();


    // ========================================================
    // GET / SET
    // ========================================================

    int get(Param param) const;

    bool set(
        Param param,
        int value
    );


    // ========================================================
    // LOAD / SAVE
    // ========================================================

    bool load(Param param);
    bool loadAll();

    bool save(Param param);
    bool saveDirty();
    bool saveAll();

    void flush();
    void resetAll();


    // ========================================================
    // PARAMETER INFO
    // ========================================================

    const char* paramName(
        Param param
    ) const;

    Param paramFromName(
        const char* name
    ) const;

    const ParamDesc& getDesc(
        Param param
    ) const;


    // ========================================================
    // STATE
    // ========================================================

    bool initialized() const
    {
        return _initialized;
    }


private:

    bool loadValue(
        const ParamDesc& desc
    );

    bool saveValue(
        const ParamDesc& desc
    );


    Preferences _preferences;

    int _values[
        static_cast<size_t>(Param::COUNT)
    ];

    bool _dirty[
        static_cast<size_t>(Param::COUNT)
    ];

    uint32_t _lastChangeTime;

    bool _initialized;
};


// ============================================================
// GLOBAL ALIAS
// ============================================================
//
// Позволяет старому коду использовать:
//
//     Param::UTC_OFFSET
//     Param::COB_BRIGHTNESS
//     Param::VOLUME_MEDIA
//
// вместо:
//
//     SettingsManager::Param::UTC_OFFSET
//
// ============================================================

using Param = SettingsManager::Param;
