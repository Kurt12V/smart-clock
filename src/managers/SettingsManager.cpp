#include "SettingsManager.h"

#include <cstring>


// ============================================================
// PARAM DESCRIPTION
// ============================================================

namespace
{

struct ParamDesc
{
    const char* key;

    int minValue;
    int maxValue;
    int defaultValue;
};


// ============================================================
// PARAMETER TABLE
// ============================================================
//
// ПОРЯДОК ДОЛЖЕН ПОЛНОСТЬЮ СОВПАДАТЬ С enum Param.
//
// ============================================================

constexpr ParamDesc PARAMS[] =
{
    // --------------------------------------------------------
    // BRIGHTNESS
    // --------------------------------------------------------

    {
        "brightness",
        Config::DISPLAY_MIN_BRIGHTNESS,
        Config::DISPLAY_MAX_BRIGHTNESS,
        Config::DISPLAY_DEFAULT_BRIGHTNESS
    },


    // --------------------------------------------------------
    // MATRIX ENABLED
    // --------------------------------------------------------

    {
        "mx_on",
        0,
        1,
        Config::MATRIX_ENABLED_DEFAULT ? 1 : 0
    },


    // --------------------------------------------------------
    // MATRIX BRIGHTNESS
    // --------------------------------------------------------

    {
        "mx_br",
        Config::MATRIX_BRIGHTNESS_MIN,
        Config::MATRIX_BRIGHTNESS_MAX,
        Config::MATRIX_BRIGHTNESS_DEFAULT
    },


    // --------------------------------------------------------
    // COB ENABLED
    // --------------------------------------------------------

    {
        "cob_on",
        0,
        1,
        Config::COB_ENABLED_DEFAULT ? 1 : 0
    },


    // --------------------------------------------------------
    // COB BRIGHTNESS 1
    // --------------------------------------------------------

    {
        "cob1",
        Config::COB_BRIGHTNESS_MIN,
        Config::COB_BRIGHTNESS_MAX,
        Config::COB_BRIGHTNESS_DEFAULT
    },


    // --------------------------------------------------------
    // COB BRIGHTNESS 2
    // --------------------------------------------------------

    {
        "cob2",
        Config::COB_BRIGHTNESS_MIN,
        Config::COB_BRIGHTNESS_MAX,
        Config::COB_BRIGHTNESS_DEFAULT
    },


    // --------------------------------------------------------
    // COB BRIGHTNESS 3
    // --------------------------------------------------------

    {
        "cob3",
        Config::COB_BRIGHTNESS_MIN,
        Config::COB_BRIGHTNESS_MAX,
        Config::COB_BRIGHTNESS_DEFAULT
    },


    // --------------------------------------------------------
    // COB BRIGHTNESS 4
    // --------------------------------------------------------

    {
        "cob4",
        Config::COB_BRIGHTNESS_MIN,
        Config::COB_BRIGHTNESS_MAX,
        Config::COB_BRIGHTNESS_DEFAULT
    },


    // --------------------------------------------------------
    // MEDIA VOLUME
    // --------------------------------------------------------

    {
        "vol_media",
        0,
        100,
        60
    },


    // --------------------------------------------------------
    // ALARM VOLUME
    // --------------------------------------------------------

    {
        "vol_alarm",
        0,
        100,
        90
    },


    // --------------------------------------------------------
    // SYSTEM VOLUME
    // --------------------------------------------------------

    {
        "vol_system",
        0,
        100,
        40
    },


    // --------------------------------------------------------
    // MICROPHONE
    // --------------------------------------------------------

    {
        "mic_on",
        0,
        1,
        Config::MIC_ENABLED_DEFAULT ? 1 : 0
    },


    // --------------------------------------------------------
    // UTC OFFSET
    // --------------------------------------------------------

    {
        "utc",
        Config::UTC_OFFSET_MIN,
        Config::UTC_OFFSET_MAX,
        Config::UTC_OFFSET_DEFAULT
    },


    // --------------------------------------------------------
    // COB EFFECT
    // --------------------------------------------------------

    {
        "cob_eff",
        0,
        3,
        0
    },


    // --------------------------------------------------------
    // COB SPEED
    // --------------------------------------------------------

    {
        "cob_spd",
        0,
        100,
        50
    }
};


// ============================================================
// TABLE SIZE CHECK
// ============================================================

static_assert(
    sizeof(PARAMS) / sizeof(PARAMS[0]) ==
    static_cast<size_t>(Param::COUNT),
    "SettingsManager: PARAMS table does not match Param::COUNT"
);

} // namespace


// ============================================================
// CONSTRUCTOR
// ============================================================

SettingsManager::SettingsManager()
    : _dirtyMask(0),
      _lastChangeMs(0),
      _initialized(false)
{
    applyDefaults();
}


// ============================================================
// DEFAULTS
// ============================================================

void SettingsManager::applyDefaults()
{
    for (
        size_t i = 0;
        i < static_cast<size_t>(Param::COUNT);
        ++i
    )
    {
        _values[i] = PARAMS[i].defaultValue;
    }
}


// ============================================================
// BEGIN
// ============================================================

bool SettingsManager::begin()
{
    Serial0.println(
        "[SETTINGS] Starting..."
    );


    // --------------------------------------------------------
    // OPEN NVS
    // --------------------------------------------------------

    if (!_preferences.begin("smartclock", false))
    {
        Serial0.println(
            "[SETTINGS] Preferences FAILED"
        );

        _initialized = false;

        return false;
    }


    // --------------------------------------------------------
    // LOAD ALL
    // --------------------------------------------------------

    if (!loadAll())
    {
        Serial0.println(
            "[SETTINGS] Load FAILED"
        );

        _initialized = false;

        return false;
    }


    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    _dirtyMask = 0;

    _lastChangeMs = millis();

    _initialized = true;


    Serial0.println(
        "[SETTINGS] Ready"
    );


    return true;
}


// ============================================================
// GET
// ============================================================

int SettingsManager::get(
    Param p
) const
{
    const size_t index =
        static_cast<size_t>(p);


    if (
        index >=
        static_cast<size_t>(Param::COUNT)
    )
    {
        return 0;
    }


    return _values[index];
}


// ============================================================
// SET
// ============================================================

bool SettingsManager::set(
    Param p,
    int value
)
{
    const size_t index =
        static_cast<size_t>(p);


    // --------------------------------------------------------
    // VALIDATE PARAM
    // --------------------------------------------------------

    if (
        index >=
        static_cast<size_t>(Param::COUNT)
    )
    {
        return false;
    }


    const ParamDesc& desc =
        PARAMS[index];


    // --------------------------------------------------------
    // CLAMP
    // --------------------------------------------------------

    const int clampedValue =
        constrain(
            value,
            desc.minValue,
            desc.maxValue
        );


    // --------------------------------------------------------
    // NO CHANGE
    // --------------------------------------------------------

    if (
        _values[index] ==
        clampedValue
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // UPDATE RAM
    // --------------------------------------------------------

    _values[index] =
        clampedValue;


    // --------------------------------------------------------
    // MARK DIRTY
    // --------------------------------------------------------

    markDirty(p);


    // --------------------------------------------------------
    // RESET SAVE TIMER
    // --------------------------------------------------------

    _lastChangeMs =
        millis();


    return true;
}


// ============================================================
// LOAD ONE
// ============================================================

bool SettingsManager::load(
    Param p
)
{
    const size_t index =
        static_cast<size_t>(p);


    // --------------------------------------------------------
    // VALIDATE PARAM
    // --------------------------------------------------------

    if (
        index >=
        static_cast<size_t>(Param::COUNT)
    )
    {
        return false;
    }


    const ParamDesc& desc =
        PARAMS[index];


    // --------------------------------------------------------
    // READ NVS
    // --------------------------------------------------------

    const int storedValue =
        _preferences.getInt(
            desc.key,
            desc.defaultValue
        );


    // --------------------------------------------------------
    // CLAMP STORED VALUE
    // --------------------------------------------------------

    _values[index] =
        constrain(
            storedValue,
            desc.minValue,
            desc.maxValue
        );


    return true;
}


// ============================================================
// LOAD ALL
// ============================================================

bool SettingsManager::loadAll()
{
    // --------------------------------------------------------
    // START WITH DEFAULTS
    // --------------------------------------------------------

    applyDefaults();


    // --------------------------------------------------------
    // LOAD EVERY PARAMETER
    // --------------------------------------------------------

    for (
        size_t i = 0;
        i < static_cast<size_t>(Param::COUNT);
        ++i
    )
    {
        if (
            !load(
                static_cast<Param>(i)
            )
        )
        {
            Serial0.printf(
                "[SETTINGS] Failed to load parameter %u\n",
                static_cast<unsigned>(i)
            );

            return false;
        }
    }


    // --------------------------------------------------------
    // NOTHING DIRTY AFTER LOAD
    // --------------------------------------------------------

    _dirtyMask = 0;


    Serial0.println(
        "[SETTINGS] Loaded"
    );


    return true;
}


// ============================================================
// SAVE ONE
// ============================================================

bool SettingsManager::save(
    Param p
)
{
    if (!_initialized)
    {
        return false;
    }


    const size_t index =
        static_cast<size_t>(p);


    // --------------------------------------------------------
    // VALIDATE PARAM
    // --------------------------------------------------------

    if (
        index >=
        static_cast<size_t>(Param::COUNT)
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // WRITE NVS
    // --------------------------------------------------------

    const size_t result =
        _preferences.putInt(
            PARAMS[index].key,
            _values[index]
        );


    // --------------------------------------------------------
    // CHECK RESULT
    // --------------------------------------------------------

    if (result == 0)
    {
        return false;
    }


    // --------------------------------------------------------
    // CLEAR DIRTY
    // --------------------------------------------------------

    clearDirty(p);


    return true;
}


// ============================================================
// MARK DIRTY
// ============================================================

void SettingsManager::markDirty(
    Param p
)
{
    const uint8_t bit =
        static_cast<uint8_t>(p);


    if (
        bit >=
        static_cast<uint8_t>(Param::COUNT)
    )
    {
        return;
    }


    _dirtyMask |=
        (uint32_t(1) << bit);
}


// ============================================================
// CLEAR DIRTY
// ============================================================

void SettingsManager::clearDirty(
    Param p
)
{
    const uint8_t bit =
        static_cast<uint8_t>(p);


    if (
        bit >=
        static_cast<uint8_t>(Param::COUNT)
    )
    {
        return;
    }


    _dirtyMask &=
        ~(uint32_t(1) << bit);
}


// ============================================================
// IS DIRTY
// ============================================================

bool SettingsManager::isDirty(
    Param p
) const
{
    const uint8_t bit =
        static_cast<uint8_t>(p);


    if (
        bit >=
        static_cast<uint8_t>(Param::COUNT)
    )
    {
        return false;
    }


    return (
        _dirtyMask &
        (uint32_t(1) << bit)
    ) != 0;
}


// ============================================================
// HAS DIRTY
// ============================================================

bool SettingsManager::hasDirty() const
{
    return _dirtyMask != 0;
}


// ============================================================
// SAVE DIRTY
// ============================================================

bool SettingsManager::saveDirty()
{
    if (!_initialized)
    {
        return false;
    }


    if (_dirtyMask == 0)
    {
        return true;
    }


    bool success = true;


    // --------------------------------------------------------
    // SAVE ONLY CHANGED PARAMETERS
    // --------------------------------------------------------

    for (
        uint8_t i = 0;
        i < static_cast<uint8_t>(Param::COUNT);
        ++i
    )
    {
        const uint32_t mask =
            uint32_t(1) << i;


        if (
            (_dirtyMask & mask) == 0
        )
        {
            continue;
        }


        const size_t result =
            _preferences.putInt(
                PARAMS[i].key,
                _values[i]
            );


        if (result == 0)
        {
            success = false;

            Serial0.printf(
                "[SETTINGS] Save failed: %s\n",
                PARAMS[i].key
            );
        }
    }


    // --------------------------------------------------------
    // CLEAR DIRTY ONLY IF EVERYTHING WORKED
    // --------------------------------------------------------

    if (success)
    {
        _dirtyMask = 0;
    }


    return success;
}


// ============================================================
// SAVE ALL
// ============================================================

bool SettingsManager::saveAll()
{
    if (!_initialized)
    {
        Serial0.println(
            "[SETTINGS] saveAll(): not initialized"
        );

        return false;
    }


    bool success = true;


    // --------------------------------------------------------
    // SAVE EVERY PARAMETER
    // --------------------------------------------------------

    for (
        uint8_t i = 0;
        i < static_cast<uint8_t>(Param::COUNT);
        ++i
    )
    {
        const size_t result =
            _preferences.putInt(
                PARAMS[i].key,
                _values[i]
            );


        if (result == 0)
        {
            success = false;

            Serial0.printf(
                "[SETTINGS] Save failed: %s\n",
                PARAMS[i].key
            );
        }
    }


    if (success)
    {
        _dirtyMask = 0;
    }


    return success;
}


// ============================================================
// UPDATE
// ============================================================

void SettingsManager::update()
{
    if (!_initialized)
    {
        return;
    }


    // --------------------------------------------------------
    // NOTHING TO SAVE
    // --------------------------------------------------------

    if (_dirtyMask == 0)
    {
        return;
    }


    // --------------------------------------------------------
    // CURRENT TIME
    // --------------------------------------------------------

    const uint32_t now =
        millis();


    // --------------------------------------------------------
    // WAIT 700 ms AFTER LAST CHANGE
    // --------------------------------------------------------

    if (
        static_cast<uint32_t>(
            now - _lastChangeMs
        ) < SAVE_DELAY_MS
    )
    {
        return;
    }


    // --------------------------------------------------------
    // SAVE
    // --------------------------------------------------------

    saveDirty();
}


// ============================================================
// FLUSH
// ============================================================

bool SettingsManager::flush()
{
    if (!_initialized)
    {
        return false;
    }


    return saveDirty();
}


// ============================================================
// RESET ALL
// ============================================================

void SettingsManager::resetAll()
{
    // --------------------------------------------------------
    // CLEAR NVS
    // --------------------------------------------------------

    if (_initialized)
    {
        _preferences.clear();
    }


    // --------------------------------------------------------
    // RESTORE DEFAULTS
    // --------------------------------------------------------

    applyDefaults();


    // --------------------------------------------------------
    // CLEAR DIRTY
    // --------------------------------------------------------

    _dirtyMask = 0;


    // --------------------------------------------------------
    // SAVE DEFAULTS
    // --------------------------------------------------------

    if (_initialized)
    {
        saveAll();
    }


    Serial0.println(
        "[SETTINGS] Reset to defaults"
    );
}


// ============================================================
// PARAM NAME
// ============================================================

const char* SettingsManager::paramName(
    Param p
)
{
    const size_t index =
        static_cast<size_t>(p);


    if (
        index >=
        static_cast<size_t>(Param::COUNT)
    )
    {
        return "";
    }


    return PARAMS[index].key;
}


// ============================================================
// PARAM FROM NAME
// ============================================================

bool SettingsManager::paramFromName(
    const char* name,
    Param& out
)
{
    // --------------------------------------------------------
    // VALIDATE
    // --------------------------------------------------------

    if (name == nullptr)
    {
        return false;
    }


    if (*name == '\0')
    {
        return false;
    }


    // --------------------------------------------------------
    // SEARCH
    // --------------------------------------------------------

    for (
        uint8_t i = 0;
        i < static_cast<uint8_t>(Param::COUNT);
        ++i
    )
    {
        if (
            strcmp(
                name,
                PARAMS[i].key
            ) == 0
        )
        {
            out =
                static_cast<Param>(i);

            return true;
        }
    }


    return false;
}