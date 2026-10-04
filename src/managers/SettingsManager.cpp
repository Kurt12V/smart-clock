#include "SettingsManager.h"

#include "Config.h"

#include <cstring>

// ============================================================
// SAVE DELAY
// ============================================================

namespace
{
    constexpr uint32_t SAVE_DELAY_MS = 1000;
}


// ============================================================
// PARAMETER TABLE
// ============================================================

namespace
{
    const SettingsManager::ParamDesc PARAMS[] =
    {
        // ----------------------------------------------------
        // DISPLAY
        // ----------------------------------------------------

        {
            SettingsManager::Param::BRIGHTNESS,
            "brightness",
            "brightness",
            Config::DISPLAY_MIN_BRIGHTNESS,
            Config::DISPLAY_MAX_BRIGHTNESS,
            Config::DISPLAY_DEFAULT_BRIGHTNESS
        },


        // ----------------------------------------------------
        // MATRIX
        // ----------------------------------------------------

        {
            SettingsManager::Param::MATRIX_ENABLED,
            "mx_on",
            "mx_on",
            0,
            1,
            Config::MATRIX_ENABLED_DEFAULT
        },

        {
            SettingsManager::Param::MATRIX_BRIGHTNESS,
            "mx_br",
            "mx_br",
            Config::MATRIX_BRIGHTNESS_MIN,
            Config::MATRIX_BRIGHTNESS_MAX,
            Config::MATRIX_BRIGHTNESS_DEFAULT
        },

        {
            SettingsManager::Param::MATRIX_EFFECT,
            "mx_eff",
            "mx_eff",
            Config::MATRIX_EFFECT_MIN,
            Config::MATRIX_EFFECT_MAX,
            Config::MATRIX_EFFECT_DEFAULT
        },

        {
            SettingsManager::Param::MATRIX_SPEED,
            "mx_spd",
            "mx_spd",
            Config::MATRIX_SPEED_MIN,
            Config::MATRIX_SPEED_MAX,
            Config::MATRIX_SPEED_DEFAULT
        },


        // ----------------------------------------------------
        // COB
        // ----------------------------------------------------

        {
            SettingsManager::Param::COB_ENABLED,
            "cob_on",
            "cob_on",
            0,
            1,
            Config::COB_ENABLED_DEFAULT
        },

        {
            SettingsManager::Param::COB_BRIGHTNESS,
            "cob_br",
            "cob_br",
            Config::COB_BRIGHTNESS_MIN,
            Config::COB_BRIGHTNESS_MAX,
            Config::COB_BRIGHTNESS_DEFAULT
        },

        {
            SettingsManager::Param::COB_EFFECT,
            "cob_eff",
            "cob_eff",
            Config::COB_EFFECT_MIN,
            Config::COB_EFFECT_MAX,
            Config::COB_EFFECT_DEFAULT
        },

        {
            SettingsManager::Param::COB_SPEED,
            "cob_spd",
            "cob_spd",
            Config::COB_SPEED_MIN,
            Config::COB_SPEED_MAX,
            Config::COB_SPEED_DEFAULT
        },


        // ----------------------------------------------------
        // AUDIO
        // ----------------------------------------------------

        {
            SettingsManager::Param::VOLUME_MEDIA,
            "vol_media",
            "vol_media",
            Config::AUDIO_MIN_VOLUME,
            Config::AUDIO_MAX_VOLUME,
            Config::MEDIA_VOLUME_DEFAULT
        },

        {
            SettingsManager::Param::VOLUME_ALARM,
            "vol_alarm",
            "vol_alarm",
            Config::AUDIO_MIN_VOLUME,
            Config::AUDIO_MAX_VOLUME,
            Config::ALARM_VOLUME_DEFAULT
        },

        {
            SettingsManager::Param::VOLUME_SYSTEM,
            "vol_system",
            "vol_system",
            Config::AUDIO_MIN_VOLUME,
            Config::AUDIO_MAX_VOLUME,
            Config::SYSTEM_VOLUME_DEFAULT
        },


        // ----------------------------------------------------
        // MICROPHONE
        // ----------------------------------------------------

        {
            SettingsManager::Param::MICROPHONE_ENABLED,
            "mic_on",
            "mic_on",
            0,
            1,
            Config::MIC_ENABLED_DEFAULT
        },


        // ----------------------------------------------------
        // TIMEZONE
        // ----------------------------------------------------

        {
            SettingsManager::Param::UTC_OFFSET,
            "utc",
            "utc",
            Config::UTC_OFFSET_MIN,
            Config::UTC_OFFSET_MAX,
            Config::UTC_OFFSET_DEFAULT
        }
    };


    constexpr size_t PARAM_COUNT =
        sizeof(PARAMS) / sizeof(PARAMS[0]);


    static_assert(
        PARAM_COUNT ==
        static_cast<size_t>(SettingsManager::Param::COUNT),
        "SettingsManager: PARAMS and Param enum are out of sync"
    );
}


// ============================================================
// CONSTRUCTOR
// ============================================================

SettingsManager::SettingsManager()
    : _preferences(),
      _values{},
      _dirty{},
      _lastChangeTime(0),
      _initialized(false)
{
}


// ============================================================
// BEGIN
// ============================================================

bool SettingsManager::begin()
{
    if (_initialized)
        return true;

    if (!_preferences.begin(
            "smartclock",
            false))
    {
        return false;
    }

    if (!loadAll())
        return false;

    _initialized = true;

    return true;
}


// ============================================================
// UPDATE
// ============================================================

void SettingsManager::update()
{
    if (!_initialized)
        return;

    if (_lastChangeTime == 0)
        return;

    if (millis() - _lastChangeTime <
        SAVE_DELAY_MS)
    {
        return;
    }

    saveDirty();

    _lastChangeTime = 0;
}


// ============================================================
// GET
// ============================================================

int SettingsManager::get(
    Param param
) const
{
    const size_t index =
        static_cast<size_t>(param);

    if (index >=
        static_cast<size_t>(Param::COUNT))
    {
        return 0;
    }

    return _values[index];
}


// ============================================================
// SET
// ============================================================

bool SettingsManager::set(
    Param param,
    int value
)
{
    const size_t index =
        static_cast<size_t>(param);

    if (index >=
        static_cast<size_t>(Param::COUNT))
    {
        return false;
    }

    const ParamDesc& desc =
        getDesc(param);

    value = constrain(
        value,
        desc.minValue,
        desc.maxValue
    );

    if (_values[index] == value)
        return false;

    _values[index] = value;

    _dirty[index] = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// LOAD PARAMETER
// ============================================================

bool SettingsManager::load(
    Param param
)
{
    const size_t index =
        static_cast<size_t>(param);

    if (index >=
        static_cast<size_t>(Param::COUNT))
    {
        return false;
    }

    return loadValue(
        getDesc(param)
    );
}


// ============================================================
// LOAD ALL
// ============================================================

bool SettingsManager::loadAll()
{
    for (size_t i = 0;
         i < PARAM_COUNT;
         ++i)
    {
        if (!loadValue(PARAMS[i]))
            return false;
    }

    return true;
}


// ============================================================
// SAVE PARAMETER
// ============================================================

bool SettingsManager::save(
    Param param
)
{
    const size_t index =
        static_cast<size_t>(param);

    if (index >=
        static_cast<size_t>(Param::COUNT))
    {
        return false;
    }

    if (!saveValue(
            getDesc(param)))
    {
        return false;
    }

    _dirty[index] = false;

    return true;
}


// ============================================================
// SAVE DIRTY
// ============================================================

bool SettingsManager::saveDirty()
{
    bool result = true;

    for (size_t i = 0;
         i < PARAM_COUNT;
         ++i)
    {
        if (!_dirty[i])
            continue;

        if (saveValue(PARAMS[i]))
        {
            _dirty[i] = false;
        }
        else
        {
            result = false;
        }
    }

    return result;
}


// ============================================================
// SAVE ALL
// ============================================================

bool SettingsManager::saveAll()
{
    bool result = true;

    for (size_t i = 0;
         i < PARAM_COUNT;
         ++i)
    {
        if (!saveValue(PARAMS[i]))
        {
            result = false;
            continue;
        }

        _dirty[i] = false;
    }

    return result;
}


// ============================================================
// FLUSH
// ============================================================

void SettingsManager::flush()
{
    if (!_initialized)
        return;

    saveDirty();

    _lastChangeTime = 0;
}


// ============================================================
// RESET ALL
// ============================================================

void SettingsManager::resetAll()
{
    for (size_t i = 0;
         i < PARAM_COUNT;
         ++i)
    {
        _values[i] =
            PARAMS[i].defaultValue;

        _dirty[i] = true;
    }

    _lastChangeTime = millis();
}


// ============================================================
// PARAM NAME
// ============================================================

const char* SettingsManager::paramName(
    Param param
) const
{
    const size_t index =
        static_cast<size_t>(param);

    if (index >= PARAM_COUNT)
        return "";

    return PARAMS[index].name;
}


// ============================================================
// PARAM FROM NAME
// ============================================================

SettingsManager::Param
SettingsManager::paramFromName(
    const char* name
) const
{
    if (name == nullptr)
        return Param::COUNT;

    for (size_t i = 0;
         i < PARAM_COUNT;
         ++i)
    {
        if (std::strcmp(
                PARAMS[i].name,
                name) == 0)
        {
            return PARAMS[i].param;
        }
    }

    return Param::COUNT;
}


// ============================================================
// GET DESCRIPTION
// ============================================================

const SettingsManager::ParamDesc&
SettingsManager::getDesc(
    Param param
) const
{
    const size_t index =
        static_cast<size_t>(param);

    return PARAMS[index];
}


// ============================================================
// LOAD VALUE
// ============================================================

bool SettingsManager::loadValue(
    const ParamDesc& desc
)
{
    const size_t index =
        static_cast<size_t>(desc.param);

    const int value =
        _preferences.getInt(
            desc.key,
            desc.defaultValue
        );

    _values[index] =
        constrain(
            value,
            desc.minValue,
            desc.maxValue
        );

    _dirty[index] = false;

    return true;
}


// ============================================================
// SAVE VALUE
// ============================================================

bool SettingsManager::saveValue(
    const ParamDesc& desc
)
{
    const size_t index =
        static_cast<size_t>(desc.param);

    return _preferences.putInt(
        desc.key,
        _values[index]
    ) > 0;
}
