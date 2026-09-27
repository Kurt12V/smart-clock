#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "Settings.h"

// ============================================================
// SETTINGS MANAGER
// ============================================================

class SettingsManager
{
public:

    SettingsManager();

    // ========================================================
    // LIFECYCLE
    // ========================================================

    bool begin();
    void load();
    void save();
    void reset();


    // ========================================================
    // DISPLAY
    // ========================================================

    void setDisplayBrightness(uint8_t brightness);
    uint8_t displayBrightness() const;


    // ========================================================
    // MATRIX
    // ========================================================

    void setMatrixEnabled(bool enabled);
    bool matrixEnabled() const;

    void setMatrixBrightness(uint8_t brightness);
    uint8_t matrixBrightness() const;


    // ========================================================
    // COB LED
    // ========================================================

    void setCobEnabled(bool enabled);
    bool cobEnabled() const;

    void setCobBrightness1(uint8_t brightness);
    uint8_t cobBrightness1() const;

    void setCobBrightness2(uint8_t brightness);
    uint8_t cobBrightness2() const;

    void setCobBrightness3(uint8_t brightness);
    uint8_t cobBrightness3() const;

    void setCobBrightness4(uint8_t brightness);
    uint8_t cobBrightness4() const;


    // ========================================================
    // AUDIO
    // ========================================================

    void setVolume(uint8_t volume);
    uint8_t volume() const;


    // ========================================================
    // MICROPHONE
    // ========================================================

    void setMicrophoneEnabled(bool enabled);
    bool microphoneEnabled() const;


    // ========================================================
    // CLOCK
    // ========================================================

    void setUtcOffset(Constants::UtcOffset offset);
    Constants::UtcOffset utcOffset() const;


    // ========================================================
    // WIFI
    // ========================================================

    void setWiFiSSID(const char* ssid);
    const char* wifiSSID() const;

    void setWiFiPassword(const char* password);
    const char* wifiPassword() const;


    // ========================================================
    // DATA
    // ========================================================

    Settings::Data& data();
    const Settings::Data& data() const;


private:

    // ========================================================
    // STORAGE
    // ========================================================

    Preferences _preferences;

    bool _initialized;

    Settings::Data _settings;


    // ========================================================
    // HELPERS
    // ========================================================

    void loadDefaults();
};