
#include "AlarmEffects.h"

#include <cstring>

// ============================================================
// CONSTRUCTOR
// ============================================================

AlarmEffects::AlarmEffects(
    LightingManager& lighting,
    SoundManager& sound
)
    : _lighting(lighting),
      _sound(sound),
      _sunrise(),
      _effect(EffectType::None),
      _begun(false),
      _active(false),
      _audioStarted(false),
      _audioFadeCompleted(false),
      _lastAudioCheckMs(0)
{
}

// ============================================================
// BEGIN
// ============================================================

void AlarmEffects::begin()
{
    if (_begun)
        return;

    Serial0.println(
        "[AlarmEffects] begin()"
    );

    _sunrise.begin();

    _begun = true;

    Serial0.println(
        "[AlarmEffects] begin() done"
    );
}

// ============================================================
// START SUNRISE
// ============================================================

void AlarmEffects::startSunrise()
{
    Serial0.println(
        "[AlarmEffects] startSunrise()"
    );

    if (!_begun)
        begin();

    // --------------------------------------------------------
    // STOP PREVIOUS EFFECT
    // --------------------------------------------------------

    stop();

    // --------------------------------------------------------
    // RESET AUDIO STATE
    // --------------------------------------------------------

    _audioStarted = false;
    _audioFadeCompleted = false;
    _lastAudioCheckMs = 0;

    // --------------------------------------------------------
    // TAKE LIGHTING CONTROL
    // --------------------------------------------------------

    _lighting.beginAlarmOverride();

    // --------------------------------------------------------
    // START SUNRISE LIGHT EFFECT
    // --------------------------------------------------------

    _sunrise.start();

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    _effect = EffectType::Sunrise;
    _active = true;

    Serial0.println(
        "[AlarmEffects] Sunrise started"
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
    // Keep the original elapsed-time contract.
    _sunrise.update(elapsedMs);

    updateSunriseLight();

    applyAuxiliaryLeds();

    updateSunriseAudio(elapsedMs);
}

// ============================================================
// SUNRISE MATRIX
// ============================================================

void AlarmEffects::updateSunriseLight()
{
    static uint8_t lastBrightness = 255;

    const uint8_t brightness =
        _sunrise.brightness();

    if (brightness != lastBrightness)
    {
        Serial0.printf(
            "[AlarmEffects] Brightness: %u, RGB=(%u,%u,%u)\n",
            static_cast<unsigned>(brightness),
            static_cast<unsigned>(_sunrise.red()),
            static_cast<unsigned>(_sunrise.green()),
            static_cast<unsigned>(_sunrise.blue())
        );

        lastBrightness = brightness;
    }

    _lighting.setAlarmMatrix(
        brightness,
        _sunrise.red(),
        _sunrise.green(),
        _sunrise.blue()
    );
}

// ============================================================
// AUXILIARY COB LEDS
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

    // SunriseLightEffect returns brightness as 0–100%.
    // LightingManager receives an 8-bit brightness value.

    const uint8_t brightness =
        static_cast<uint8_t>(
            (
                static_cast<uint16_t>(
                    _sunrise.auxiliaryBrightness()
                ) * 255U
            ) / 100U
        );

    _lighting.setAlarmCob(brightness);
}

// ============================================================
// SUNRISE AUDIO
// ============================================================

void AlarmEffects::updateSunriseAudio(
    uint32_t elapsedMs
)
{
    // --------------------------------------------------------
    // WAIT UNTIL THE 21ST MINUTE
    // --------------------------------------------------------

    if (elapsedMs < MUSIC_START_MS)
        return;

    // --------------------------------------------------------
    // FIRST PLAYBACK: FADE FROM 0% TO 100% IN 2 MINUTES
    // --------------------------------------------------------

    if (!_audioStarted)
    {
        Serial0.println(
            "[AlarmEffects] Starting alarm WAV with 2-minute fade-in"
        );

        if (startAlarmWav(true))
        {
            _audioStarted = true;
            _lastAudioCheckMs = elapsedMs;

            Serial0.println(
                "[AlarmEffects] Alarm WAV started"
            );
        }

        return;
    }

    // --------------------------------------------------------
    // CHECK PLAYBACK PERIODICALLY
    // --------------------------------------------------------

    constexpr uint32_t AUDIO_CHECK_INTERVAL_MS = 250;

    if (
        elapsedMs - _lastAudioCheckMs <
        AUDIO_CHECK_INTERVAL_MS
    )
    {
        return;
    }

    _lastAudioCheckMs = elapsedMs;

    // --------------------------------------------------------
    // FADE-IN PERIOD
    // --------------------------------------------------------

    if (
        elapsedMs - MUSIC_START_MS <
        AUDIO_FADE_IN_MS
    )
    {
        return;
    }

    if (!_audioFadeCompleted)
    {
        _audioFadeCompleted = true;

        Serial0.println(
            "[AlarmEffects] Alarm volume ramp completed: 100%"
        );
    }

    // --------------------------------------------------------
    // RESTART WAV WHEN PLAYBACK HAS FINISHED
    // --------------------------------------------------------

    if (!_sound.isPlaying())
    {
        Serial0.println(
            "[AlarmEffects] Restarting alarm WAV at full volume"
        );

        if (startAlarmWav(false))
        {
            Serial0.println(
                "[AlarmEffects] Alarm WAV restarted"
            );
        }
    }
}

// ============================================================
// START ALARM WAV
// ============================================================

bool AlarmEffects::startAlarmWav(
    bool fadeIn
)
{
    SoundManager::PlayOptions options;

    options.stream =
        SoundManager::AudioStream::Alarm;

    // The alarm's configured base volume should be 100%.
    // localPercent controls this playback instance.

    options.localPercent = 100;

    // First playback fades in over 2 minutes.
    // Subsequent playbacks start at full volume.

    options.fadeInMs =
        fadeIn
            ? AUDIO_FADE_IN_MS
            : 0;

    options.fadeOutMs = 0;

    // A logarithmic fade-in raises the perceived volume
    // faster near the beginning than a linear fade.

    options.curve =
        SoundManager::FadeCurve::Logarithmic;

    if (!_sound.play(
            ALARM_WAV_PATH,
            options
        ))
    {
        Serial0.printf(
            "[AlarmEffects] Failed to play: %s\n",
            ALARM_WAV_PATH
        );

        return false;
    }

    return true;
}

// ============================================================
// STOP
// ============================================================

void AlarmEffects::stop()
{
    if (!_active)
        return;

    Serial0.println(
        "[AlarmEffects] stop()"
    );

    // --------------------------------------------------------
    // STOP AUDIO
    // --------------------------------------------------------

    _sound.stop();

    _audioStarted = false;
    _audioFadeCompleted = false;
    _lastAudioCheckMs = 0;

    // --------------------------------------------------------
    // STOP SUNRISE
    // --------------------------------------------------------

    _sunrise.stop();

    // --------------------------------------------------------
    // RESTORE NORMAL LIGHTING SETTINGS
    // --------------------------------------------------------

    _lighting.endAlarmOverride();

    // --------------------------------------------------------
    // RESET STATE
    // --------------------------------------------------------

    _effect = EffectType::None;
    _active = false;

    Serial0.println(
        "[AlarmEffects] stop() done"
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
