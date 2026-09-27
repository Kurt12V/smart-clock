#include "SettingsManager.h"


// ============================================================
// NAMESPACE
// ============================================================

static constexpr const char* SETTINGS_NAMESPACE =
    "smartclock";


// ============================================================
// CONSTRUCTOR
// ============================================================

SettingsManager::SettingsManager()
    : _initialized(false)
{
}


// ============================================================
// BEGIN
// ============================================================

bool SettingsManager::begin()
{
    Serial.println(
        "[SETTINGS] Starting..."
    );


    if (
        !_preferences.begin(
            SETTINGS_NAMESPACE,
            false
        )
    )
    {
        Serial.println(
            "[SETTINGS] Preferences begin failed"
        );

        return false;
    }


    _initialized = true;


    load();


    Serial.println(
        "[SETTINGS] Ready"
    );


    return true;
}


// ============================================================
// LOAD DEFAULTS
// ============================================================

void SettingsManager::loadDefaults()
{
    _settings = Settings::Data();
}


// ============================================================
// LOAD
// ============================================================

void SettingsManager::load()
{
    if (!_initialized)
        return;


    Serial.println(
        "[SETTINGS] Loading..."
    );


    // ========================================================
    // DISPLAY
    // ========================================================

    _settings.display.brightness =
        _preferences.getUChar(
            "display_brightness",
            100
        );


    // ========================================================
    // MATRIX
    // ========================================================

    _settings.matrix.enabled =
        _preferences.getBool(
            "matrix_enabled",
            true
        );


    _settings.matrix.brightness =
        _preferences.getUChar(
            "matrix_brightness",
            50
        );


    // ========================================================
    // COB LED
    // ========================================================

    _settings.cobLed.enabled =
        _preferences.getBool(
            "cob_enabled",
            true
        );


    _settings.cobLed.brightness1 =
        _preferences.getUChar(
            "cob_brightness1",
            100
        );


    _settings.cobLed.brightness2 =
        _preferences.getUChar(
            "cob_brightness2",
            100
        );


    _settings.cobLed.brightness3 =
        _preferences.getUChar(
            "cob_brightness3",
            100
        );


    _settings.cobLed.brightness4 =
        _preferences.getUChar(
            "cob_brightness4",
            100
        );


    // ========================================================
    // AUDIO
    // ========================================================

    _settings.audio.volume =
        _preferences.getUChar(
            "audio_volume",
            70
        );


    // ========================================================
    // MICROPHONE
    // ========================================================

    _settings.microphone.enabled =
        _preferences.getBool(
            "mic_enabled",
            true
        );


    // ========================================================
    // CLOCK
    // ========================================================

    int8_t offset =
        _preferences.getChar(
            "utc_offset",
            3
        );


    // Ограничиваем UTC offset
    if (offset < -12)
        offset = -12;

    if (offset > 14)
        offset = 14;


    _settings.clock.utcOffset =
        static_cast<Constants::UtcOffset>(
            offset
        );


    // ========================================================
    // WIFI
    // ========================================================

    _settings.wifi.ssid =
        _preferences.getString(
            "wifi_ssid",
            "tpl47"
        );


    _settings.wifi.password =
        _preferences.getString(
            "wifi_password",
            "12713714"
        );


    // ========================================================
    // VALIDATION
    // ========================================================

    if (
        _settings.display.brightness > 100
    )
    {
        _settings.display.brightness = 100;
    }


    if (
        _settings.matrix.brightness > 100
    )
    {
        _settings.matrix.brightness = 100;
    }


    if (
        _settings.cobLed.brightness1 > 100
    )
    {
        _settings.cobLed.brightness1 = 100;
    }


    if (
        _settings.cobLed.brightness2 > 100
    )
    {
        _settings.cobLed.brightness2 = 100;
    }


    if (
        _settings.cobLed.brightness3 > 100
    )
    {
        _settings.cobLed.brightness3 = 100;
    }


    if (
        _settings.cobLed.brightness4 > 100
    )
    {
        _settings.cobLed.brightness4 = 100;
    }


    if (
        _settings.audio.volume > 100
    )
    {
        _settings.audio.volume = 100;
    }


    Serial.println(
        "[SETTINGS] Loaded"
    );
}


// ============================================================
// SAVE
// ============================================================

void SettingsManager::save()
{
    if (!_initialized)
        return;


    Serial.println(
        "[SETTINGS] Saving..."
    );


    // ========================================================
    // DISPLAY
    // ========================================================

    _preferences.putUChar(
        "display_brightness",
        _settings.display.brightness
    );


    // ========================================================
    // MATRIX
    // ========================================================

    _preferences.putBool(
        "matrix_enabled",
        _settings.matrix.enabled
    );


    _preferences.putUChar(
        "matrix_brightness",
        _settings.matrix.brightness
    );


    // ========================================================
    // COB LED
    // ========================================================

    _preferences.putBool(
        "cob_enabled",
        _settings.cobLed.enabled
    );


    _preferences.putUChar(
        "cob_brightness1",
        _settings.cobLed.brightness1
    );


    _preferences.putUChar(
        "cob_brightness2",
        _settings.cobLed.brightness2
    );


    _preferences.putUChar(
        "cob_brightness3",
        _settings.cobLed.brightness3
    );


    _preferences.putUChar(
        "cob_brightness4",
        _settings.cobLed.brightness4
    );


    // ========================================================
    // AUDIO
    // ========================================================

    _preferences.putUChar(
        "audio_volume",
        _settings.audio.volume
    );


    // ========================================================
    // MICROPHONE
    // ========================================================

    _preferences.putBool(
        "mic_enabled",
        _settings.microphone.enabled
    );


    // ========================================================
    // CLOCK
    // ========================================================

    int8_t offset =
        static_cast<int8_t>(
            _settings.clock.utcOffset
        );


    _preferences.putChar(
        "utc_offset",
        offset
    );


    // ========================================================
    // WIFI
    // ========================================================

    _preferences.putString(
        "wifi_ssid",
        _settings.wifi.ssid
    );


    _preferences.putString(
        "wifi_password",
        _settings.wifi.password
    );


    Serial.println(
        "[SETTINGS] Saved"
    );
}


// ============================================================
// RESET
// ============================================================

void SettingsManager::reset()
{
    Serial.println(
        "[SETTINGS] Resetting..."
    );


    loadDefaults();


    if (_initialized)
    {
        _preferences.clear();
    }


    Serial.println(
        "[SETTINGS] Reset complete"
    );
}


// ============================================================
// DISPLAY
// ============================================================

void SettingsManager::setDisplayBrightness(
    uint8_t brightness
)
{
    _settings.display.brightness =
        constrain(
            brightness,
            0,
            100
        );
}


uint8_t SettingsManager::displayBrightness() const
{
    return _settings.display.brightness;
}


// ============================================================
// MATRIX
// ============================================================

void SettingsManager::setMatrixEnabled(
    bool enabled
)
{
    _settings.matrix.enabled =
        enabled;
}


bool SettingsManager::matrixEnabled() const
{
    return _settings.matrix.enabled;
}


void SettingsManager::setMatrixBrightness(
    uint8_t brightness
)
{
    _settings.matrix.brightness =
        constrain(
            brightness,
            0,
            100
        );
}


uint8_t SettingsManager::matrixBrightness() const
{
    return _settings.matrix.brightness;
}


// ============================================================
// COB LED
// ============================================================

void SettingsManager::setCobEnabled(
    bool enabled
)
{
    _settings.cobLed.enabled =
        enabled;
}


bool SettingsManager::cobEnabled() const
{
    return _settings.cobLed.enabled;
}


void SettingsManager::setCobBrightness1(
    uint8_t brightness
)
{
    _settings.cobLed.brightness1 =
        constrain(
            brightness,
            0,
            100
        );
}


uint8_t SettingsManager::cobBrightness1() const
{
    return _settings.cobLed.brightness1;
}


void SettingsManager::setCobBrightness2(
    uint8_t brightness
)
{
    _settings.cobLed.brightness2 =
        constrain(
            brightness,
            0,
            100
        );
}


uint8_t SettingsManager::cobBrightness2() const
{
    return _settings.cobLed.brightness2;
}


void SettingsManager::setCobBrightness3(
    uint8_t brightness
)
{
    _settings.cobLed.brightness3 =
        constrain(
            brightness,
            0,
            100
        );
}


uint8_t SettingsManager::cobBrightness3() const
{
    return _settings.cobLed.brightness3;
}


void SettingsManager::setCobBrightness4(
    uint8_t brightness
)
{
    _settings.cobLed.brightness4 =
        constrain(
            brightness,
            0,
            100
        );
}


uint8_t SettingsManager::cobBrightness4() const
{
    return _settings.cobLed.brightness4;
}


// ============================================================
// AUDIO
// ============================================================

void SettingsManager::setVolume(
    uint8_t volume
)
{
    _settings.audio.volume =
        constrain(
            volume,
            0,
            100
        );
}


uint8_t SettingsManager::volume() const
{
    return _settings.audio.volume;
}


// ============================================================
// MICROPHONE
// ============================================================

void SettingsManager::setMicrophoneEnabled(
    bool enabled
)
{
    _settings.microphone.enabled =
        enabled;
}


bool SettingsManager::microphoneEnabled() const
{
    return _settings.microphone.enabled;
}


// ============================================================
// CLOCK
// ============================================================

void SettingsManager::setUtcOffset(
    Constants::UtcOffset offset
)
{
    int value =
        static_cast<int>(offset);


    if (value < -12)
        value = -12;

    if (value > 14)
        value = 14;


    _settings.clock.utcOffset =
        static_cast<Constants::UtcOffset>(
            value
        );
}


Constants::UtcOffset
SettingsManager::utcOffset() const
{
    return _settings.clock.utcOffset;
}


// ============================================================
// WIFI
// ============================================================

void SettingsManager::setWiFiSSID(
    const char* ssid
)
{
    if (!ssid)
        return;


    _settings.wifi.ssid =
        ssid;
}


const char* SettingsManager::wifiSSID() const
{
    return _settings.wifi.ssid.c_str();
}


void SettingsManager::setWiFiPassword(
    const char* password
)
{
    if (!password)
        return;


    _settings.wifi.password =
        password;
}


const char* SettingsManager::wifiPassword() const
{
    return _settings.wifi.password.c_str();
}


// ============================================================
// DATA
// ============================================================

Settings::Data&
SettingsManager::data()
{
    return _settings;
}


const Settings::Data&
SettingsManager::data() const
{
    return _settings;
}