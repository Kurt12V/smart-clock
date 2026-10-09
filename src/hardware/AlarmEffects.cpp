#include "AlarmEffects.h"

#include <algorithm>

// ============================================================
// CONSTRUCTOR
// ============================================================

AlarmEffects::AlarmEffects(
    LedMatrixManager& matrix,
    CobLedManager& cob,
    I2SManager& i2s
)
    : _matrix(matrix),
      _cob(cob),
      _i2s(i2s),

      _effect(EffectType::None),

      _begun(false),
      _active(false),

      _sunrise(),
      _music(),

      _audioBuffer{},

      _audioStarted(false),

      _savedMatrixOn(false),
      _savedMatrixBrightness(0),
      _savedMatrixEffect(
          LedMatrixManager::Effect::None
      ),
      _savedMatrixEffectSpeed(0),
      _savedMatrixTransitionTime(0),

      _savedCobEnabled(false),
      _savedCobEffect(0),
      _savedCobSpeed(0),
      _savedCobBrightness{0, 0, 0, 0},

      _controlTaken(false)
{
}

// ============================================================
// BEGIN
// ============================================================

void AlarmEffects::begin()
{
    if (_begun)
        return;

    _music.begin(
        AUDIO_SAMPLE_RATE
    );

    _sunrise.begin();

    _begun = true;
}

// ============================================================
// START SUNRISE
// ============================================================

void AlarmEffects::startSunrise()
{
    if (!_begun)
        begin();

    // --------------------------------------------------------
    // Stop previous effect
    // --------------------------------------------------------

    stop();

    // --------------------------------------------------------
    // Save normal system state
    // --------------------------------------------------------

    saveOutputs();

    // --------------------------------------------------------
    // Take temporary control
    // --------------------------------------------------------

    takeControl();

    // --------------------------------------------------------
    // Start sunrise logic
    // --------------------------------------------------------

    _sunrise.start();

    // --------------------------------------------------------
    // Start music generator
    //
    // Actual I2S output starts later at 21 minutes.
    // --------------------------------------------------------

    _music.start();

    _music.setVolume(
        0.0f
    );

    _audioStarted = false;

    // --------------------------------------------------------
    // State
    // --------------------------------------------------------

    _effect = EffectType::Sunrise;
    _active = true;
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

            updateSunrise(
                elapsedMs
            );

            break;

        case EffectType::None:

        default:

            break;
    }
}

// ============================================================
// SUNRISE
// ============================================================

void AlarmEffects::updateSunrise(
    uint32_t elapsedMs
)
{
    // --------------------------------------------------------
    // Sunrise calculation
    // --------------------------------------------------------

    _sunrise.update(
        elapsedMs
    );

    // --------------------------------------------------------
    // Apply matrix
    // --------------------------------------------------------

    updateSunriseLight();

    // --------------------------------------------------------
    // Apply auxiliary COB
    // --------------------------------------------------------

    applyAuxiliaryLeds();

    // --------------------------------------------------------
    // Audio
    // --------------------------------------------------------

    updateSunriseAudio(
        elapsedMs
    );
}

// ============================================================
// SUNRISE LIGHT
// ============================================================

void AlarmEffects::updateSunriseLight()
{
    applyMatrix();
}

// ============================================================
// MATRIX
// ============================================================

void AlarmEffects::applyMatrix()
{
    if (!_matrix.isOn())
        _matrix.on();

    /*
     * SunriseLightEffect already calculates gamma-corrected
     * brightness.
     */

    _matrix.setBrightness(
        _sunrise.brightness()
    );

    _matrix.fill(
        _sunrise.red(),
        _sunrise.green(),
        _sunrise.blue()
    );

    _matrix.show();
}

// ============================================================
// AUXILIARY COB
// ============================================================

void AlarmEffects::applyAuxiliaryLeds()
{
    if (!_sunrise.auxiliaryEnabled())
    {
        setAuxiliaryLeds(0);
        return;
    }

    if (!_sunrise.auxiliaryFlashState())
    {
        setAuxiliaryLeds(0);
        return;
    }

    const uint8_t brightness =
        static_cast<uint8_t>(
            (
                static_cast<uint16_t>(
                    _sunrise.auxiliaryBrightness()
                )
                *
                255U
            )
            /
            100U
        );

    setAuxiliaryLeds(
        brightness
    );
}

// ============================================================
// SET ALL AUXILIARY LEDS
// ============================================================

void AlarmEffects::setAuxiliaryLeds(
    uint8_t brightness
)
{
    _cob.set(
        1,
        brightness
    );

    _cob.set(
        2,
        brightness
    );

    _cob.set(
        3,
        brightness
    );

    _cob.set(
        4,
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
    // Before 21 minutes
    //
    // No audio output.
    // --------------------------------------------------------

    if (elapsedMs < MUSIC_START_MS)
        return;

    // --------------------------------------------------------
    // Start I2S
    // --------------------------------------------------------

    if (!_audioStarted)
    {
        if (!_i2s.isSpeakerInitialized())
        {
            if (
                !_i2s.beginSpeaker(
                    AUDIO_SAMPLE_RATE
                )
            )
            {
                return;
            }
        }

        if (
            !_i2s.startSpeaker()
        )
        {
            return;
        }

        _audioStarted = true;
    }

    // --------------------------------------------------------
    // Calculate volume
    // --------------------------------------------------------

    float volume = 0.0f;

    // --------------------------------------------------------
    // 21:00 -> 25:00
    //
    // 0 -> 30%
    // --------------------------------------------------------

    if (elapsedMs < PEAK_TIME_MS)
    {
        const uint32_t elapsed =
            elapsedMs -
            MUSIC_START_MS;

        const uint32_t duration =
            PEAK_TIME_MS -
            MUSIC_START_MS;

        float progress =
            static_cast<float>(
                elapsed
            )
            /
            static_cast<float>(
                duration
            );

        progress =
            std::max(
                0.0f,
                std::min(
                    1.0f,
                    progress
                )
            );

        volume =
            0.30f *
            progress;
    }

    // --------------------------------------------------------
    // 25:00 -> 26:00
    //
    // 30 -> 100%
    // --------------------------------------------------------

    else
    {
        const uint32_t elapsed =
            elapsedMs -
            PEAK_TIME_MS;

        float progress =
            static_cast<float>(
                elapsed
            )
            /
            static_cast<float>(
                FULL_VOLUME_RAMP_MS
            );

        progress =
            std::max(
                0.0f,
                std::min(
                    1.0f,
                    progress
                )
            );

        volume =
            0.30f +
            (
                0.70f *
                progress
            );
    }

    // --------------------------------------------------------
    // Set generator volume
    // --------------------------------------------------------

    _music.setVolume(
        volume
    );

    // --------------------------------------------------------
    // Generate PCM
    // --------------------------------------------------------

    _music.generateBlock(
        _audioBuffer,
        AUDIO_SAMPLES
    );

    // --------------------------------------------------------
    // Send PCM through I2SManager
    // --------------------------------------------------------

    size_t bytesWritten = 0;

    _i2s.writeSpeaker(
        reinterpret_cast<
            const uint8_t*
        >(
            _audioBuffer
        ),

        sizeof(
            _audioBuffer
        ),

        bytesWritten,

        20
    );
}

// ============================================================
// SAVE OUTPUTS
// ============================================================

void AlarmEffects::saveOutputs()
{
    // --------------------------------------------------------
    // MATRIX
    // --------------------------------------------------------

    _savedMatrixOn =
        _matrix.isOn();

    _savedMatrixBrightness =
        _matrix.brightness();

    _savedMatrixEffect =
        _matrix.effect();

    _savedMatrixEffectSpeed =
        _matrix.effectSpeed();

    _savedMatrixTransitionTime =
        _matrix.transitionTime();

    // --------------------------------------------------------
    // COB
    // --------------------------------------------------------

    _savedCobEnabled =
        _cob.isEnabled();

    _savedCobEffect =
        _cob.effect();

    _savedCobSpeed =
        _cob.speed();

    for (uint8_t i = 0; i < 4; ++i)
    {
        _savedCobBrightness[i] =
            _cob.get(i + 1);
    }

    _controlTaken = true;
}

// ============================================================
// TAKE CONTROL
// ============================================================

void AlarmEffects::takeControl()
{
    // --------------------------------------------------------
    // MATRIX
    // --------------------------------------------------------

    /*
     * Очень важно:
     *
     * LedMatrixManager::update()
     * продолжает выполнять обычный effect.
     *
     * Поэтому останавливаем normal effect.
     */

    _matrix.stopEffect();

    _matrix.setBrightness(
        255
    );

    _matrix.on();

    _matrix.clear();

    // --------------------------------------------------------
    // COB
    // --------------------------------------------------------

    /*
     * Отключаем обычный COB effect.
     *
     * Иначе CobEffects::update()
     * может перезаписать brightness.
     */

    _cob.setEnabled(
        false
    );

    _cob.offAll();

    // --------------------------------------------------------
    // AUDIO
    // --------------------------------------------------------

    if (_i2s.isSpeakerInitialized())
    {
        _i2s.stopSpeaker();
        _i2s.clearSpeaker();
    }
}

// ============================================================
// RESTORE
// ============================================================

void AlarmEffects::restoreOutputs()
{
    if (!_controlTaken)
        return;

    // --------------------------------------------------------
    // MATRIX
    // --------------------------------------------------------

    _matrix.clear();

    _matrix.setBrightness(
        _savedMatrixBrightness
    );

    _matrix.setEffectSpeed(
        _savedMatrixEffectSpeed
    );

    _matrix.setTransitionTime(
        _savedMatrixTransitionTime
    );

    _matrix.setEffect(
        _savedMatrixEffect
    );

    if (_savedMatrixOn)
        _matrix.on();
    else
        _matrix.off();

    // --------------------------------------------------------
    // COB
    // --------------------------------------------------------

    /*
     * Сначала восстанавливаем brightness.
     */

    _cob.setEnabled(
        false
    );

    for (uint8_t i = 0; i < 4; ++i)
    {
        _cob.set(
            i + 1,
            _savedCobBrightness[i]
        );
    }

    _cob.setSpeed(
        _savedCobSpeed
    );

    _cob.setEffect(
        _savedCobEffect
    );

    if (_savedCobEnabled)
    {
        _cob.setEnabled(
            true
        );
    }
    else
    {
        _cob.setEnabled(
            false
        );
    }

    // --------------------------------------------------------
    // CONTROL
    // --------------------------------------------------------

    _controlTaken = false;
}

// ============================================================
// STOP
// ============================================================

void AlarmEffects::stop()
{
    if (!_active)
        return;

    // --------------------------------------------------------
    // STOP MUSIC
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
    // Restore normal outputs
    // --------------------------------------------------------

    restoreOutputs();

    // --------------------------------------------------------
    // State
    // --------------------------------------------------------

    _effect =
        EffectType::None;

    _active = false;
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
        _effect ==
            EffectType::Sunrise;
}

// ============================================================

AlarmEffects::EffectType
AlarmEffects::effect() const
{
    return _effect;
}