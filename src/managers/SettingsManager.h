#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "Config.h"


class SettingsManager
{
public:

    // ========================================================
    // PARAMETER IDs
    // ========================================================

    enum class Id : uint8_t
    {
        DISPLAY_BRIGHTNESS,

        MATRIX_ENABLED,
        MATRIX_BRIGHTNESS,
        MATRIX_EFFECT,
        MATRIX_SPEED,

        COB_ENABLED,
        COB_BRIGHTNESS,
        COB_EFFECT,
        COB_SPEED,

        MIC_ENABLED,

        VOLUME_MEDIA,
        VOLUME_ALARM,
        VOLUME_SYSTEM,

        UTC_OFFSET,

        COUNT
    };


    // ========================================================
    // SETTINGS DATA
    // ========================================================

    struct Data
    {
        // Display
        uint8_t displayBrightness;

        // Matrix
        bool    matrixEnabled;
        uint8_t matrixBrightness;
        uint8_t matrixEffect;
        uint8_t matrixSpeed;

        // COB
        bool    cobEnabled;
        uint8_t cobBrightness;
        uint8_t cobEffect;
        uint8_t cobSpeed;

        // Microphone
        bool micEnabled;

        // Audio
        uint8_t volumeMedia;
        uint8_t volumeAlarm;
        uint8_t volumeSystem;

        // Time
        int8_t utcOffset;
    };


    // ========================================================
    // LIFECYCLE
    // ========================================================

    bool begin();

    void update();

    bool load();

    bool save();

    void reset();


    // ========================================================
    // STATE
    // ========================================================

    bool isInitialized() const;

    bool isDirty() const;


    // ========================================================
    // DATA
    // ========================================================

    const Data& data() const;

    bool setData(const Data& data);


    // ========================================================
    // DISPLAY
    // ========================================================

    uint8_t getDisplayBrightness() const;

    bool setDisplayBrightness(uint8_t value);


    // ========================================================
    // MATRIX
    // ========================================================

    bool isMatrixEnabled() const;

    bool setMatrixEnabled(bool enabled);


    uint8_t getMatrixBrightness() const;

    bool setMatrixBrightness(uint8_t value);


    uint8_t getMatrixEffect() const;

    bool setMatrixEffect(uint8_t value);


    uint8_t getMatrixSpeed() const;

    bool setMatrixSpeed(uint8_t value);


    // ========================================================
    // COB
    // ========================================================

    bool isCobEnabled() const;

    bool setCobEnabled(bool enabled);


    uint8_t getCobBrightness() const;

    bool setCobBrightness(uint8_t value);


    uint8_t getCobEffect() const;

    bool setCobEffect(uint8_t value);


    uint8_t getCobSpeed() const;

    bool setCobSpeed(uint8_t value);


    // ========================================================
    // MICROPHONE
    // ========================================================

    bool isMicEnabled() const;

    bool setMicEnabled(bool enabled);


    // ========================================================
    // AUDIO
    // ========================================================

    uint8_t getMediaVolume() const;

    bool setMediaVolume(uint8_t value);


    uint8_t getAlarmVolume() const;

    bool setAlarmVolume(uint8_t value);


    uint8_t getSystemVolume() const;

    bool setSystemVolume(uint8_t value);


    // ========================================================
    // TIMEZONE
    // ========================================================

    int8_t getUtcOffset() const;

    bool setUtcOffset(int8_t value);


    // ========================================================
    // PARAMETER NAME
    // ========================================================

    static const char* getName(Id id);


private:

    // ========================================================
    // DEFAULTS
    // ========================================================

    static Data defaultSettings();


    // ========================================================
    // CLAMP
    // ========================================================

    static uint8_t clampDisplayBrightness(uint8_t value);

    static uint8_t clampMatrixBrightness(uint8_t value);

    static uint8_t clampMatrixEffect(uint8_t value);

    static uint8_t clampMatrixSpeed(uint8_t value);

    static uint8_t clampCobBrightness(uint8_t value);

    static uint8_t clampCobEffect(uint8_t value);

    static uint8_t clampCobSpeed(uint8_t value);

    static uint8_t clampMediaVolume(uint8_t value);

    static uint8_t clampAlarmVolume(uint8_t value);

    static uint8_t clampSystemVolume(uint8_t value);

    static int8_t clampUtcOffset(int8_t value);


    // ========================================================
    // COMPARISON
    // ========================================================

    static bool equals(
        const Data& a,
        const Data& b
    );


    // ========================================================
    // STORAGE
    // ========================================================

    Preferences _preferences;

    Data _data{};


    // ========================================================
    // STATE
    // ========================================================

    bool _initialized = false;

    bool _dirty = false;

    uint32_t _lastChangeTime = 0;


    // ========================================================
    // SAVE SETTINGS
    // ========================================================

    static constexpr uint32_t SAVE_DELAY_MS = 700;


    // ========================================================
    // NVS
    // ========================================================

    static constexpr const char* NAMESPACE = "smartclock";
};