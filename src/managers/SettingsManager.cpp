#include "SettingsManager.h"

// ============================================================
// PARAM TABLE
//   key = имя в Preferences
//   min / max / def берутся из Config.h
// ============================================================

namespace
{
    struct ParamDesc
    {
        const char* key;
        int         min;
        int         max;
        int         def;
    };

const ParamDesc PARAMS[] =
{
    // DISPLAY
    { "brightness",
      Config::DISPLAY_MIN_BRIGHTNESS,
      Config::DISPLAY_MAX_BRIGHTNESS,
      Config::DISPLAY_DEFAULT_BRIGHTNESS },

    // MATRIX
    { "mx_on", 0, 1, Config::MATRIX_ENABLED_DEFAULT ? 1 : 0 },
    { "mx_br",
      Config::MATRIX_BRIGHTNESS_MIN,
      Config::MATRIX_BRIGHTNESS_MAX,
      Config::MATRIX_BRIGHTNESS_DEFAULT },

    // COB
    { "cob_on", 0, 1, Config::COB_ENABLED_DEFAULT ? 1 : 0 },
    { "cob1",
      Config::COB_BRIGHTNESS_MIN,
      Config::COB_BRIGHTNESS_MAX,
      Config::COB_BRIGHTNESS_DEFAULT },
    { "cob2",
      Config::COB_BRIGHTNESS_MIN,
      Config::COB_BRIGHTNESS_MAX,
      Config::COB_BRIGHTNESS_DEFAULT },
    { "cob3",
      Config::COB_BRIGHTNESS_MIN,
      Config::COB_BRIGHTNESS_MAX,
      Config::COB_BRIGHTNESS_DEFAULT },
    { "cob4",
      Config::COB_BRIGHTNESS_MIN,
      Config::COB_BRIGHTNESS_MAX,
      Config::COB_BRIGHTNESS_DEFAULT },

    // --------------------------------------------------------
    // VOLUMES (streams)
    // --------------------------------------------------------

    { "vol_media",  0, 100, 60 },
    { "vol_alarm",  0, 100, 90 },
    { "vol_system", 0, 100, 40 },

    // MIC
    { "mic_on", 0, 1, Config::MIC_ENABLED_DEFAULT ? 1 : 0 },

    // CLOCK
    { "utc",
      Config::UTC_OFFSET_MIN,
      Config::UTC_OFFSET_MAX,
      Config::UTC_OFFSET_DEFAULT },
    { "cob_eff", 0,   3,   0  },
    { "cob_spd", 0, 100,  50  },

};

    static_assert(
        sizeof(PARAMS) / sizeof(PARAMS[0]) ==
        static_cast<size_t>(Param::COUNT),
        "PARAMS table size mismatch"
    );
}

// ============================================================
// CONSTRUCTOR
// ============================================================

SettingsManager::SettingsManager()
    : _initialized(false)
{
    applyDefaults();
}

void SettingsManager::applyDefaults()
{
    for (size_t i = 0; i < static_cast<size_t>(Param::COUNT); ++i)
        _values[i] = PARAMS[i].def;
}

// ============================================================
// BEGIN
// ============================================================

bool SettingsManager::begin()
{
    Serial0.println("[SETTINGS] Starting...");

    if (!_preferences.begin("smartclock", false))
    {
        Serial0.println("[SETTINGS] Preferences FAILED");
        return false;
    }

    loadAll();

    _initialized = true;

    Serial0.println("[SETTINGS] Ready");
    return true;
}

// ============================================================
// GET / SET
// ============================================================

int SettingsManager::get(Param p) const
{
    size_t i = static_cast<size_t>(p);
    if (i >= static_cast<size_t>(Param::COUNT)) return 0;
    return _values[i];
}

bool SettingsManager::set(Param p, int value)
{
    size_t i = static_cast<size_t>(p);
    if (i >= static_cast<size_t>(Param::COUNT)) return false;

    const ParamDesc& d = PARAMS[i];
    _values[i] = constrain(value, d.min, d.max);

    return true;
}

// ============================================================
// LOAD / SAVE
// ============================================================

bool SettingsManager::load(Param p)
{
    size_t i = static_cast<size_t>(p);
    if (i >= static_cast<size_t>(Param::COUNT)) return false;

    const ParamDesc& d = PARAMS[i];
    int stored = _preferences.getInt(d.key, d.def);

    _values[i] = constrain(stored, d.min, d.max);

    return true;
}

bool SettingsManager::save(Param p)
{
    size_t i = static_cast<size_t>(p);
    if (i >= static_cast<size_t>(Param::COUNT)) return false;

    _preferences.putInt(PARAMS[i].key, _values[i]);

    return true;
}

// ============================================================
// BULK
// ============================================================

bool SettingsManager::loadAll()
{
    applyDefaults();

    for (size_t i = 0; i < static_cast<size_t>(Param::COUNT); ++i)
        load(static_cast<Param>(i));

    Serial0.println("[SETTINGS] Loaded");
    return true;
}

bool SettingsManager::saveAll()
{
    if (!_initialized)
    {
        Serial0.println("[SETTINGS] Not initialized");
        return false;
    }

    for (size_t i = 0; i < static_cast<size_t>(Param::COUNT); ++i)
        save(static_cast<Param>(i));

    Serial0.println("[SETTINGS] Saved");
    return true;
}

void SettingsManager::resetAll()
{
    _preferences.clear();
    applyDefaults();

    if (_initialized)
        saveAll();

    Serial0.println("[SETTINGS] Reset to defaults");
}

// ============================================================
// NAME <-> PARAM
// ============================================================

const char* SettingsManager::paramName(Param p)
{
    size_t i = static_cast<size_t>(p);
    if (i >= static_cast<size_t>(Param::COUNT)) return "";
    return PARAMS[i].key;
}

bool SettingsManager::paramFromName(const char* name, Param& out)
{
    if (!name) return false;

    for (size_t i = 0; i < static_cast<size_t>(Param::COUNT); ++i)
    {
        if (strcmp(name, PARAMS[i].key) == 0)
        {
            out = static_cast<Param>(i);
            return true;
        }
    }
    return false;
}