#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "./managers/LedMatrixManager.h"
#include "./managers/CobLedManager.h"
#include "./managers/I2SManager.h"
#include "./hardware/SunriseLightEffect.h"
#include "./hardware/MusicGenerator.h"

class AlarmEffects
{
public:

    enum class EffectType : uint8_t
    {
        None = 0,
        Sunrise
    };

    AlarmEffects(
        LedMatrixManager& matrix,
        CobLedManager& cob,
        I2SManager& i2s
    );

    void begin();

    void update(
        uint32_t elapsedMs
    );

    void stop();

    // --------------------------------------------------------
    // EFFECTS
    // --------------------------------------------------------

    void startSunrise(
        MusicGenerator::Preset preset =
            MusicGenerator::Preset::SunriseSoft
    );

    bool isActive() const;

    bool isSunriseActive() const;

    EffectType effect() const;

private:

    // --------------------------------------------------------
    // SUNRISE
    // --------------------------------------------------------

    void updateSunrise(
        uint32_t elapsedMs
    );

    void updateSunriseLight();

    void updateSunriseAudio(
        uint32_t elapsedMs
    );

    // --------------------------------------------------------
    // MATRIX
    // --------------------------------------------------------

    void applyMatrix();

    // --------------------------------------------------------
    // COB
    // --------------------------------------------------------

    void applyAuxiliaryLeds();

    void setAuxiliaryLeds(
        uint8_t brightness
    );

    // --------------------------------------------------------
    // AUDIO
    // --------------------------------------------------------

    void updateAudio();

    // --------------------------------------------------------
    // CONTROL
    // --------------------------------------------------------

    void saveOutputs();

    void takeControl();

    void restoreOutputs();

private:

    // ========================================================
    // HARDWARE
    // ========================================================

    LedMatrixManager& _matrix;
    CobLedManager& _cob;
    I2SManager& _i2s;

    // ========================================================
    // EFFECT
    // ========================================================

    EffectType _effect;

    bool _begun;
    bool _active;

    // ========================================================
    // EFFECT OBJECTS
    // ========================================================

    SunriseLightEffect _sunrise;

    MusicGenerator _music;

    // ========================================================
    // AUDIO
    // ========================================================

    static constexpr uint32_t AUDIO_SAMPLE_RATE = 44100;

    static constexpr size_t AUDIO_SAMPLES =
        MusicGenerator::BLOCK_SAMPLES;

    static constexpr size_t AUDIO_CHANNELS = 2;

    static constexpr size_t AUDIO_BUFFER_SIZE =
        AUDIO_SAMPLES * AUDIO_CHANNELS;

    int16_t _audioBuffer[AUDIO_BUFFER_SIZE];

    bool _audioStarted;

    // ========================================================
    // SUNRISE TIMING
    // ========================================================

    static constexpr uint32_t MUSIC_START_MS =
        21UL * 60UL * 1000UL;

    static constexpr uint32_t PEAK_TIME_MS =
        25UL * 60UL * 1000UL;

    static constexpr uint32_t FULL_VOLUME_RAMP_MS =
        60UL * 1000UL;

    // ========================================================
    // COB
    // ========================================================

    static constexpr uint8_t AUXILIARY_BRIGHTNESS_PERCENT = 15;

    // ========================================================
    // MATRIX SAVED STATE
    // ========================================================

    bool _savedMatrixOn;

    uint8_t _savedMatrixBrightness;

    LedMatrixManager::Effect _savedMatrixEffect;

    uint8_t _savedMatrixEffectSpeed;

    uint16_t _savedMatrixTransitionTime;

    // ========================================================
    // COB SAVED STATE
    // ========================================================

    bool _savedCobEnabled;

    uint8_t _savedCobEffect;

    uint8_t _savedCobSpeed;

    uint8_t _savedCobBrightness[4];

    // ========================================================
    // CONTROL
    // ========================================================

    bool _controlTaken;
};