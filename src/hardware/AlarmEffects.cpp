
#include "AlarmEffects.h"

#include <algorithm>
#include <Arduino.h>

// ============================================================
// CONSTRUCTOR
// ============================================================

AlarmEffects::AlarmEffects(
    LightingManager& lighting,
    I2SManager& i2s
)
    : _lighting(lighting),
      _i2s(i2s),

      _effect(EffectType::None),

      _begun(false),
      _active(false),

      _sunrise(),
      _music(),

      _monoAudioBuffer{},
      _stereoAudioBuffer{},

      _audioStarted(false)
{
}

// ============================================================
// BEGIN
// ============================================================

void AlarmEffects::begin()
{
    if (_begun)
        return;

    Serial0.printf(
        "[AlarmEffects] begin()\n"
    );

    _music.begin(
        AUDIO_SAMPLE_RATE
    );

    _sunrise.begin();

    _begun = true;

    Serial0.printf(
        "[AlarmEffects] begin() done\n"
    );
}

// ============================================================
// START SUNRISE
// ============================================================

void AlarmEffects::startSunrise()
{
    Serial0.printf(
        "[AlarmEffects] startSunrise()\n"
    );

    if (!_begun)
        begin();

    // Останавливаем предыдущий эффект.
    stop();

    // --------------------------------------------------------
    // STOP PREVIOUS AUDIO
    // --------------------------------------------------------

    if (_i2s.isSpeakerInitialized())
    {
        _i2s.stopSpeaker();
        _i2s.clearSpeaker();
    }

    _audioStarted = false;

    // --------------------------------------------------------
    // TAKE LIGHTING CONTROL
    // --------------------------------------------------------

    _lighting.beginAlarmOverride();

    // --------------------------------------------------------
    // START SUNRISE
    // --------------------------------------------------------

    _sunrise.start();

    // --------------------------------------------------------
    // START MUSIC GENERATOR
    //
    // PCM генерируется с нулевой громкостью.
    // Физический вывод включается на 21-й минуте.
    // --------------------------------------------------------

    _music.setVolume(0.0f);
    _music.start();

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    _effect = EffectType::Sunrise;
    _active = true;

    Serial0.printf(
        "[AlarmEffects] Sunrise started\n"
    );
}

// ============================================================
// UPDATE
// ============================================================

void AlarmEffects::update(
    uint32_t elapsedMs
)
{
    if (!_active)
        return;

    switch (_effect)
    {
        case EffectType::Sunrise:
            updateSunrise(elapsedMs);
            break;

        case EffectType::None:
        default:
            break;
    }
}

// ============================================================
// UPDATE SUNRISE
// ============================================================

void AlarmEffects::updateSunrise(
    uint32_t elapsedMs
)
{
    // Сохраняем исходный контракт SunriseLightEffect:
    // ему передаётся elapsedMs, как и в предыдущем коде.
    _sunrise.update(
        elapsedMs
    );

    updateSunriseLight();

    applyAuxiliaryLeds();

    updateSunriseAudio(
        elapsedMs
    );
}

// ============================================================
// SUNRISE LIGHT
// ============================================================

void AlarmEffects::updateSunriseLight()
{
    _lighting.setAlarmMatrix(
        _sunrise.brightness(),
        _sunrise.red(),
        _sunrise.green(),
        _sunrise.blue()
    );
}

// ============================================================
// AUXILIARY COB
// ============================================================

void AlarmEffects::applyAuxiliaryLeds()
{
    if (!_sunrise.auxiliaryEnabled())
    {
        _lighting.setAlarmCob(0);
        return;
    }

    if (!_sunrise.auxiliaryFlashState())
    {
        _lighting.setAlarmCob(0);
        return;
    }

    // auxiliaryBrightness() возвращает проценты 0–100.
    // CobLedManager получает яркость 0–255.
    const uint8_t brightness =
        static_cast<uint8_t>(
            (
                static_cast<uint16_t>(
                    _sunrise.auxiliaryBrightness()
                ) * 255U
            ) / 100U
        );

    _lighting.setAlarmCob(
        brightness
    );
}

// ============================================================
// SUNRISE AUDIO
// ============================================================

void AlarmEffects::updateSunriseAudio(
    uint32_t elapsedMs
)
{
    // --------------------------------------------------------
    // WAIT FOR MUSIC START
    // --------------------------------------------------------

    if (elapsedMs < MUSIC_START_MS)
        return;

    // --------------------------------------------------------
    // START I2S
    // --------------------------------------------------------

    if (!_audioStarted)
    {
        Serial0.printf(
            "[AlarmEffects] Starting I2S speaker\n"
        );

        if (!_i2s.isSpeakerInitialized())
        {
            if (!_i2s.beginSpeaker(AUDIO_SAMPLE_RATE))
            {
                Serial0.printf(
                    "[AlarmEffects] beginSpeaker() failed\n"
                );

                return;
            }
        }

        if (!_i2s.startSpeaker())
        {
            Serial0.printf(
                "[AlarmEffects] startSpeaker() failed\n"
            );

            return;
        }

        _audioStarted = true;

        Serial0.printf(
            "[AlarmEffects] I2S speaker started\n"
        );
    }

    // --------------------------------------------------------
    // CALCULATE VOLUME
    // --------------------------------------------------------

    float volume = 0.0f;

    // 21:00 -> 25:00
    // 0% -> 30%
    if (elapsedMs < PEAK_TIME_MS)
    {
        const uint32_t elapsed =
            elapsedMs - MUSIC_START_MS;

        const uint32_t duration =
            PEAK_TIME_MS - MUSIC_START_MS;

        float progress =
            static_cast<float>(elapsed) /
            static_cast<float>(duration);

        progress = std::max(
            0.0f,
            std::min(1.0f, progress)
        );

        volume = 0.30f * progress;
    }
    // 25:00 -> 26:00
    // 30% -> 100%
    else
    {
        const uint32_t elapsed =
            elapsedMs - PEAK_TIME_MS;

        float progress =
            static_cast<float>(elapsed) /
            static_cast<float>(FULL_VOLUME_RAMP_MS);

        progress = std::max(
            0.0f,
            std::min(1.0f, progress)
        );

        volume = 0.30f + 0.70f * progress;
    }

    // --------------------------------------------------------
    // UPDATE GENERATOR VOLUME
    // --------------------------------------------------------

    _music.setVolume(
        volume
    );

    // --------------------------------------------------------
    // GENERATE MONO PCM
    // --------------------------------------------------------

    _music.generateBlock(
        _monoAudioBuffer,
        AUDIO_SAMPLES
    );

    // --------------------------------------------------------
    // MONO -> STEREO
    //
    // I2S stereo expects interleaved L/R samples:
    // L0, R0, L1, R1, ...
    // --------------------------------------------------------

    for (size_t i = 0; i < AUDIO_SAMPLES; ++i)
    {
        const int16_t sample =
            _monoAudioBuffer[i];

        _stereoAudioBuffer[i * 2] = sample;
        _stereoAudioBuffer[i * 2 + 1] = sample;
    }

    // --------------------------------------------------------
    // WRITE STEREO PCM TO I2S
    // --------------------------------------------------------

    size_t bytesWritten = 0;

    const bool writeResult =
        _i2s.writeSpeaker(
            reinterpret_cast<const uint8_t*>(
                _stereoAudioBuffer
            ),
            sizeof(_stereoAudioBuffer),
            bytesWritten,
            20
        );

    if (!writeResult)
    {
        Serial0.printf(
            "[AlarmEffects] I2S write failed\n"
        );
    }
}

// ============================================================
// STOP
// ============================================================

void AlarmEffects::stop()
{
    if (!_active)
        return;

    Serial0.printf(
        "[AlarmEffects] stop()\n"
    );

    // --------------------------------------------------------
    // STOP MUSIC GENERATOR
    // --------------------------------------------------------

    _music.stop();

    // --------------------------------------------------------
    // STOP I2S
    // --------------------------------------------------------

    if (_audioStarted)
    {
        _i2s.stopSpeaker();
        _i2s.clearSpeaker();

        _audioStarted = false;
    }

    // --------------------------------------------------------
    // STOP SUNRISE
    // --------------------------------------------------------

    _sunrise.stop();

    // --------------------------------------------------------
    // RESTORE LIGHTING SETTINGS
    // --------------------------------------------------------

    _lighting.endAlarmOverride();

    // --------------------------------------------------------
    // RESET STATE
    // --------------------------------------------------------

    _effect = EffectType::None;
    _active = false;

    Serial0.printf(
        "[AlarmEffects] stop() done\n"
    );
}

// ============================================================
// STATE
// ============================================================

bool AlarmEffects::isActive() const
{
    return _active;
}

// ============================================================

bool AlarmEffects::isSunriseActive() const
{
    return
        _active &&
        _effect == EffectType::Sunrise;
}

// ============================================================

AlarmEffects::EffectType
AlarmEffects::effect() const
{
    return _effect;
}