#include "SettingsManager.h"


// ============================================================
// BEGIN
// ============================================================

bool SettingsManager::begin()
{
    if (_initialized)
        return true;


    if (!_preferences.begin(NAMESPACE, false))
        return false;


    _initialized = true;

    return load();
}


// ============================================================
// UPDATE
// ============================================================

void SettingsManager::update()
{
    if (!_initialized)
        return;

    if (!_dirty)
        return;


    if (millis() - _lastChangeTime < SAVE_DELAY_MS)
        return;


    save();
}


// ============================================================
// LOAD
// ============================================================

bool SettingsManager::load()
{
    if (!_initialized)
        return false;


    Data defaults = defaultSettings();

    // ========================================================
    // DISPLAY
    // ========================================================

    _data.displayBrightness =
        _preferences.getUChar(
            "display_br",
            defaults.displayBrightness
        );


    // ========================================================
    // MATRIX
    // ========================================================

    _data.matrixEnabled =
        _preferences.getBool(
            "matrix_on",
            defaults.matrixEnabled
        );


    _data.matrixBrightness =
        _preferences.getUChar(
            "matrix_br",
            defaults.matrixBrightness
        );


    _data.matrixEffect =
        _preferences.getUChar(
            "matrix_eff",
            defaults.matrixEffect
        );


    _data.matrixSpeed =
        _preferences.getUChar(
            "matrix_spd",
            defaults.matrixSpeed
        );


    // ========================================================
    // COB
    // ========================================================

    _data.cobEnabled =
        _preferences.getBool(
            "cob_on",
            defaults.cobEnabled
        );


    _data.cobBrightness =
        _preferences.getUChar(
            "cob_br",
            defaults.cobBrightness
        );


    _data.cobEffect =
        _preferences.getUChar(
            "cob_eff",
            defaults.cobEffect
        );


    _data.cobSpeed =
        _preferences.getUChar(
            "cob_spd",
            defaults.cobSpeed
        );


    // ========================================================
    // MICROPHONE
    // ========================================================

    _data.micEnabled =
        _preferences.getBool(
            "mic_on",
            defaults.micEnabled
        );


    // ========================================================
    // AUDIO
    // ========================================================

    _data.volumeMedia =
        _preferences.getUChar(
            "vol_media",
            defaults.volumeMedia
        );


    _data.volumeAlarm =
        _preferences.getUChar(
            "vol_alarm",
            defaults.volumeAlarm
        );


    _data.volumeSystem =
        _preferences.getUChar(
            "vol_system",
            defaults.volumeSystem
        );


    // ========================================================
    // TIMEZONE
    // ========================================================

    _data.utcOffset =
        _preferences.getChar(
            "utc",
            defaults.utcOffset
        );


    // ========================================================
    // VALIDATE / CLAMP
    // ========================================================

    _data.displayBrightness =
        clampDisplayBrightness(
            _data.displayBrightness
        );


    _data.matrixBrightness =
        clampMatrixBrightness(
            _data.matrixBrightness
        );


    _data.matrixEffect =
        clampMatrixEffect(
            _data.matrixEffect
        );


    _data.matrixSpeed =
        clampMatrixSpeed(
            _data.matrixSpeed
        );


    _data.cobBrightness =
        clampCobBrightness(
            _data.cobBrightness
        );


    _data.cobEffect =
        clampCobEffect(
            _data.cobEffect
        );


    _data.cobSpeed =
        clampCobSpeed(
            _data.cobSpeed
        );


    _data.volumeMedia =
        clampMediaVolume(
            _data.volumeMedia
        );


    _data.volumeAlarm =
        clampAlarmVolume(
            _data.volumeAlarm
        );


    _data.volumeSystem =
        clampSystemVolume(
            _data.volumeSystem
        );


    _data.utcOffset =
        clampUtcOffset(
            _data.utcOffset
        );


    _dirty = false;

    return true;
}


// ============================================================
// SAVE
// ============================================================

bool SettingsManager::save()
{
    if (!_initialized)
        return false;


    bool success = true;


    // ========================================================
    // DISPLAY
    // ========================================================

    success &= (
        _preferences.putUChar(
            "display_br",
            _data.displayBrightness
        ) > 0
    );


    // ========================================================
    // MATRIX
    // ========================================================

    success &= (
        _preferences.putBool(
            "matrix_on",
            _data.matrixEnabled
        )
    );


    success &= (
        _preferences.putUChar(
            "matrix_br",
            _data.matrixBrightness
        ) > 0
    );


    success &= (
        _preferences.putUChar(
            "matrix_eff",
            _data.matrixEffect
        ) > 0
    );


    success &= (
        _preferences.putUChar(
            "matrix_spd",
            _data.matrixSpeed
        ) > 0
    );


    // ========================================================
    // COB
    // ========================================================

    success &= (
        _preferences.putBool(
            "cob_on",
            _data.cobEnabled
        )
    );


    success &= (
        _preferences.putUChar(
            "cob_br",
            _data.cobBrightness
        ) > 0
    );


    success &= (
        _preferences.putUChar(
            "cob_eff",
            _data.cobEffect
        ) > 0
    );


    success &= (
        _preferences.putUChar(
            "cob_spd",
            _data.cobSpeed
        ) > 0
    );


    // ========================================================
    // MICROPHONE
    // ========================================================

    success &= (
        _preferences.putBool(
            "mic_on",
            _data.micEnabled
        )
    );


    // ========================================================
    // AUDIO
    // ========================================================

    success &= (
        _preferences.putUChar(
            "vol_media",
            _data.volumeMedia
        ) > 0
    );


    success &= (
        _preferences.putUChar(
            "vol_alarm",
            _data.volumeAlarm
        ) > 0
    );


    success &= (
        _preferences.putUChar(
            "vol_system",
            _data.volumeSystem
        ) > 0
    );


    // ========================================================
    // TIMEZONE
    // ========================================================

    success &= (
        _preferences.putChar(
            "utc",
            _data.utcOffset
        ) > 0
    );


    if (success)
        _dirty = false;


    return success;
}


// ============================================================
// RESET
// ============================================================

void SettingsManager::reset()
{
    if (!_initialized)
        return;


    Data defaults = defaultSettings();

    if (equals(_data, defaults))
        return;


    _data = defaults;

    _dirty = true;

    _lastChangeTime = millis();
}


// ============================================================
// STATE
// ============================================================

bool SettingsManager::isInitialized() const
{
    return _initialized;
}


bool SettingsManager::isDirty() const
{
    return _dirty;
}


// ============================================================
// DATA
// ============================================================

const SettingsManager::Data&
SettingsManager::data() const
{
    return _data;
}


bool SettingsManager::setData(
    const Data& data
)
{
    if (!_initialized)
        return false;


    Data newData = data;


    // ========================================================
    // CLAMP
    // ========================================================

    newData.displayBrightness =
        clampDisplayBrightness(
            newData.displayBrightness
        );


    newData.matrixBrightness =
        clampMatrixBrightness(
            newData.matrixBrightness
        );


    newData.matrixEffect =
        clampMatrixEffect(
            newData.matrixEffect
        );


    newData.matrixSpeed =
        clampMatrixSpeed(
            newData.matrixSpeed
        );


    newData.cobBrightness =
        clampCobBrightness(
            newData.cobBrightness
        );


    newData.cobEffect =
        clampCobEffect(
            newData.cobEffect
        );


    newData.cobSpeed =
        clampCobSpeed(
            newData.cobSpeed
        );


    newData.volumeMedia =
        clampMediaVolume(
            newData.volumeMedia
        );


    newData.volumeAlarm =
        clampAlarmVolume(
            newData.volumeAlarm
        );


    newData.volumeSystem =
        clampSystemVolume(
            newData.volumeSystem
        );


    newData.utcOffset =
        clampUtcOffset(
            newData.utcOffset
        );


    // ========================================================
    // CHECK CHANGES
    // ========================================================

    if (equals(_data, newData))
        return false;


    _data = newData;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// DISPLAY
// ============================================================

uint8_t
SettingsManager::getDisplayBrightness() const
{
    return _data.displayBrightness;
}


bool SettingsManager::setDisplayBrightness(
    uint8_t value
)
{
    value = clampDisplayBrightness(value);

    if (_data.displayBrightness == value)
        return false;


    _data.displayBrightness = value;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// MATRIX ENABLED
// ============================================================

bool SettingsManager::isMatrixEnabled() const
{
    return _data.matrixEnabled;
}


bool SettingsManager::setMatrixEnabled(
    bool enabled
)
{
    if (_data.matrixEnabled == enabled)
        return false;


    _data.matrixEnabled = enabled;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// MATRIX BRIGHTNESS
// ============================================================

uint8_t
SettingsManager::getMatrixBrightness() const
{
    return _data.matrixBrightness;
}


bool SettingsManager::setMatrixBrightness(
    uint8_t value
)
{
    value = clampMatrixBrightness(value);

    if (_data.matrixBrightness == value)
        return false;


    _data.matrixBrightness = value;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// MATRIX EFFECT
// ============================================================

uint8_t
SettingsManager::getMatrixEffect() const
{
    return _data.matrixEffect;
}


bool SettingsManager::setMatrixEffect(
    uint8_t value
)
{
    value = clampMatrixEffect(value);

    if (_data.matrixEffect == value)
        return false;


    _data.matrixEffect = value;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// MATRIX SPEED
// ============================================================

uint8_t
SettingsManager::getMatrixSpeed() const
{
    return _data.matrixSpeed;
}


bool SettingsManager::setMatrixSpeed(
    uint8_t value
)
{
    value = clampMatrixSpeed(value);

    if (_data.matrixSpeed == value)
        return false;


    _data.matrixSpeed = value;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// COB ENABLED
// ============================================================

bool
SettingsManager::isCobEnabled() const
{
    return _data.cobEnabled;
}


bool
SettingsManager::setCobEnabled(
    bool enabled
)
{
    if (_data.cobEnabled == enabled)
        return false;


    _data.cobEnabled = enabled;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// COB BRIGHTNESS
// ============================================================

uint8_t
SettingsManager::getCobBrightness() const
{
    return _data.cobBrightness;
}


bool
SettingsManager::setCobBrightness(
    uint8_t value
)
{
    value = clampCobBrightness(value);

    if (_data.cobBrightness == value)
        return false;


    _data.cobBrightness = value;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// COB EFFECT
// ============================================================

uint8_t
SettingsManager::getCobEffect() const
{
    return _data.cobEffect;
}


bool
SettingsManager::setCobEffect(
    uint8_t value
)
{
    value = clampCobEffect(value);

    if (_data.cobEffect == value)
        return false;


    _data.cobEffect = value;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// COB SPEED
// ============================================================

uint8_t
SettingsManager::getCobSpeed() const
{
    return _data.cobSpeed;
}


bool
SettingsManager::setCobSpeed(
    uint8_t value
)
{
    value = clampCobSpeed(value);

    if (_data.cobSpeed == value)
        return false;


    _data.cobSpeed = value;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// MICROPHONE
// ============================================================

bool
SettingsManager::isMicEnabled() const
{
    return _data.micEnabled;
}


bool
SettingsManager::setMicEnabled(
    bool enabled
)
{
    if (_data.micEnabled == enabled)
        return false;


    _data.micEnabled = enabled;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// MEDIA VOLUME
// ============================================================

uint8_t
SettingsManager::getMediaVolume() const
{
    return _data.volumeMedia;
}


bool
SettingsManager::setMediaVolume(
    uint8_t value
)
{
    value = clampMediaVolume(value);

    if (_data.volumeMedia == value)
        return false;


    _data.volumeMedia = value;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// ALARM VOLUME
// ============================================================

uint8_t
SettingsManager::getAlarmVolume() const
{
    return _data.volumeAlarm;
}


bool
SettingsManager::setAlarmVolume(
    uint8_t value
)
{
    value = clampAlarmVolume(value);

    if (_data.volumeAlarm == value)
        return false;


    _data.volumeAlarm = value;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// SYSTEM VOLUME
// ============================================================

uint8_t
SettingsManager::getSystemVolume() const
{
    return _data.volumeSystem;
}


bool
SettingsManager::setSystemVolume(
    uint8_t value
)
{
    value = clampSystemVolume(value);

    if (_data.volumeSystem == value)
        return false;


    _data.volumeSystem = value;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// UTC OFFSET
// ============================================================

int8_t
SettingsManager::getUtcOffset() const
{
    return _data.utcOffset;
}


bool
SettingsManager::setUtcOffset(
    int8_t value
)
{
    value = clampUtcOffset(value);

    if (_data.utcOffset == value)
        return false;


    _data.utcOffset = value;

    _dirty = true;

    _lastChangeTime = millis();

    return true;
}


// ============================================================
// DEFAULT SETTINGS
// ============================================================

SettingsManager::Data
SettingsManager::defaultSettings()
{
    Data data{};


    // Display
    data.displayBrightness =
        Config::DISPLAY_DEFAULT_BRIGHTNESS;


    // Matrix
    data.matrixEnabled =
        Config::MATRIX_ENABLED_DEFAULT;

    data.matrixBrightness =
        Config::MATRIX_BRIGHTNESS_DEFAULT;

    data.matrixEffect =
        Config::MATRIX_EFFECT_DEFAULT;

    data.matrixSpeed =
        Config::MATRIX_SPEED_DEFAULT;


    // COB
    data.cobEnabled =
        Config::COB_ENABLED_DEFAULT;

    data.cobBrightness =
        Config::COB_BRIGHTNESS_DEFAULT;

    data.cobEffect =
        Config::COB_EFFECT_DEFAULT;

    data.cobSpeed =
        Config::COB_SPEED_DEFAULT;


    // Microphone
    data.micEnabled =
        Config::MIC_ENABLED_DEFAULT;


    // Audio
    data.volumeMedia =
        Config::AUDIO_VOLUME_DEFAULT;

    data.volumeAlarm =
        Config::AUDIO_MAX_VOLUME;

    data.volumeSystem =
        Config::AUDIO_VOLUME_DEFAULT;


    // Timezone
    data.utcOffset =
        Config::UTC_OFFSET_DEFAULT;


    return data;
}


// ============================================================
// CLAMP DISPLAY BRIGHTNESS
// ============================================================

uint8_t
SettingsManager::clampDisplayBrightness(
    uint8_t value
)
{
    return constrain(
        value,
        Config::DISPLAY_MIN_BRIGHTNESS,
        Config::DISPLAY_MAX_BRIGHTNESS
    );
}


// ============================================================
// CLAMP MATRIX BRIGHTNESS
// ============================================================

uint8_t
SettingsManager::clampMatrixBrightness(
    uint8_t value
)
{
    return constrain(
        value,
        Config::MATRIX_BRIGHTNESS_MIN,
        Config::MATRIX_BRIGHTNESS_MAX
    );
}


// ============================================================
// CLAMP MATRIX EFFECT
// ============================================================

uint8_t
SettingsManager::clampMatrixEffect(
    uint8_t value
)
{
    return constrain(
        value,
        Config::MATRIX_EFFECT_MIN,
        Config::MATRIX_EFFECT_MAX
    );
}


// ============================================================
// CLAMP MATRIX SPEED
// ============================================================

uint8_t
SettingsManager::clampMatrixSpeed(
    uint8_t value
)
{
    return constrain(
        value,
        Config::MATRIX_SPEED_MIN,
        Config::MATRIX_SPEED_MAX
    );
}


// ============================================================
// CLAMP COB BRIGHTNESS
// ============================================================

uint8_t
SettingsManager::clampCobBrightness(
    uint8_t value
)
{
    return constrain(
        value,
        Config::COB_BRIGHTNESS_MIN,
        Config::COB_BRIGHTNESS_MAX
    );
}


// ============================================================
// CLAMP COB EFFECT
// ============================================================

uint8_t
SettingsManager::clampCobEffect(
    uint8_t value
)
{
    return constrain(
        value,
        Config::COB_EFFECT_MIN,
        Config::COB_EFFECT_MAX
    );
}


// ============================================================
// CLAMP COB SPEED
// ============================================================

uint8_t
SettingsManager::clampCobSpeed(
    uint8_t value
)
{
    return constrain(
        value,
        Config::COB_SPEED_MIN,
        Config::COB_SPEED_MAX
    );
}


// ============================================================
// CLAMP MEDIA VOLUME
// ============================================================

uint8_t
SettingsManager::clampMediaVolume(
    uint8_t value
)
{
    return constrain(
        value,
        Config::AUDIO_MIN_VOLUME,
        Config::AUDIO_MAX_VOLUME
    );
}


// ============================================================
// CLAMP ALARM VOLUME
// ============================================================

uint8_t
SettingsManager::clampAlarmVolume(
    uint8_t value
)
{
    return constrain(
        value,
        Config::AUDIO_MIN_VOLUME,
        Config::AUDIO_MAX_VOLUME
    );
}


// ============================================================
// CLAMP SYSTEM VOLUME
// ============================================================

uint8_t
SettingsManager::clampSystemVolume(
    uint8_t value
)
{
    return constrain(
        value,
        Config::AUDIO_MIN_VOLUME,
        Config::AUDIO_MAX_VOLUME
    );
}


// ============================================================
// CLAMP UTC
// ============================================================

int8_t
SettingsManager::clampUtcOffset(
    int8_t value
)
{
    if (value < Config::UTC_OFFSET_MIN)
        return Config::UTC_OFFSET_MIN;

    if (value > Config::UTC_OFFSET_MAX)
        return Config::UTC_OFFSET_MAX;

    return value;
}


// ============================================================
// DATA COMPARISON
// ============================================================

bool
SettingsManager::equals(
    const Data& a,
    const Data& b
)
{
    return
        a.displayBrightness == b.displayBrightness &&

        a.matrixEnabled     == b.matrixEnabled &&
        a.matrixBrightness  == b.matrixBrightness &&
        a.matrixEffect      == b.matrixEffect &&
        a.matrixSpeed       == b.matrixSpeed &&

        a.cobEnabled        == b.cobEnabled &&
        a.cobBrightness     == b.cobBrightness &&
        a.cobEffect         == b.cobEffect &&
        a.cobSpeed          == b.cobSpeed &&

        a.micEnabled        == b.micEnabled &&

        a.volumeMedia       == b.volumeMedia &&
        a.volumeAlarm       == b.volumeAlarm &&
        a.volumeSystem      == b.volumeSystem &&

        a.utcOffset         == b.utcOffset;
}


// ============================================================
// PARAMETER NAMES
// ============================================================

const char*
SettingsManager::getName(Id id)
{
    switch (id)
    {
        case Id::DISPLAY_BRIGHTNESS:
            return "display_brightness";


        case Id::MATRIX_ENABLED:
            return "matrix_enabled";

        case Id::MATRIX_BRIGHTNESS:
            return "matrix_brightness";

        case Id::MATRIX_EFFECT:
            return "matrix_effect";

        case Id::MATRIX_SPEED:
            return "matrix_speed";


        case Id::COB_ENABLED:
            return "cob_enabled";

        case Id::COB_BRIGHTNESS:
            return "cob_brightness";

        case Id::COB_EFFECT:
            return "cob_effect";

        case Id::COB_SPEED:
            return "cob_speed";


        case Id::MIC_ENABLED:
            return "mic_enabled";


        case Id::VOLUME_MEDIA:
            return "volume_media";

        case Id::VOLUME_ALARM:
            return "volume_alarm";

        case Id::VOLUME_SYSTEM:
            return "volume_system";


        case Id::UTC_OFFSET:
            return "utc_offset";


        default:
            return "unknown";
    }
}