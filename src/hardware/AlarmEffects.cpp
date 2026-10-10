
#include "AlarmEffects.h"

#include <algorithm>

// ============================================================
// CONSTRUCTOR
// ============================================================

AlarmEffects::AlarmEffects(
    LightingManager& lighting,
    SoundManager& sound,
    MusicGenerator& music
)
    : _lighting(lighting),
      _sound(sound),
      _music(music),
      _effect(EffectType::None),
      _begun(false),
      _active(false),
      _audioStarted(false),
      _sunrise()
{
}

// ============================================================
// BEGIN
// ============================================================

void AlarmEffects::begin()
{
    if (_begun)
        return;

    Serial0.println("[AlarmEffects] begin()");

    _sunrise.begin();

    _begun = true;

    Serial0.println("[AlarmEffects] ready");
}

// ============================================================
// START SUNRISE
// ============================================================

void AlarmEffects::startSunrise()
{
    if (!_begun)
        begin();

    // --------------------------------------------------------
    // STOP PREVIOUS EFFECT
    // --------------------------------------------------------

    stop();

    // Stop audio owned by SoundManager.
    _sound.stop();

    // Reset generator state.
    _music.stop();
    _music.end();

    _audioStarted = false;

    // --------------------------------------------------------
    // TAKE LIGHTING CONTROL
    // --------------------------------------------------------

    _lighting.beginAlarmOverride();

    // --------------------------------------------------------
    // START LIGHT EFFECT
    // --------------------------------------------------------

    _sunrise.start();

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    _effect = EffectType::Sunrise;
    _active = true;

    Serial0.println("[AlarmEffects] Sunrise started");
}

// ============================================================
// UPDATE
// ============================================================

void AlarmEffects::update(uint32_t elapsedMs)
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

void AlarmEffects::updateSunrise(uint32_t elapsedMs)
{
    _sunrise.update(elapsedMs);

    updateSunriseLight();

    applyAuxiliaryLeds();

    updateSunriseAudio(elapsedMs);
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
    if (!_sunrise.auxiliaryEnabled() ||
        !_sunrise.auxiliaryFlashState())
    {
        _lighting.setAlarmCob(0);
        return;
    }

    // SunriseLightEffect returns brightness as 0–100%.
    // LightingManager expects brightness from 0 to 255.

    const uint8_t brightness =
        static_cast<uint8_t>(
            static_cast<uint16_t>(
                _sunrise.auxiliaryBrightness()
            ) * 255U / 100U
        );

    _lighting.setAlarmCob(brightness);
}

// ============================================================
// SUNRISE AUDIO
// ============================================================

void AlarmEffects::updateSunriseAudio(uint32_t elapsedMs)
{
    // --------------------------------------------------------
    // WAIT FOR AUDIO START
    // --------------------------------------------------------

    if (elapsedMs < MUSIC_START_MS)
        return;

    // --------------------------------------------------------
    // START GENERATED MUSIC
    // --------------------------------------------------------

    if (!_audioStarted)
    {
        if (!startGeneratedMusic())
            return;

        _audioStarted = true;

        Serial0.println(
            "[AlarmEffects] Generated music started"
        );
    }

    // --------------------------------------------------------
    // APPLY SUNRISE VOLUME
    // --------------------------------------------------------

    // soundPercent() is the percentage configured by
    // SunriseLightEffect for the current phase.

    const uint8_t volumePercent =
        std::min<uint8_t>(
            _sunrise.soundPercent(),
            100
        );

    _sound.setLocalPercent(volumePercent);
}

// ============================================================
// START GENERATED MUSIC
// ============================================================

bool AlarmEffects::startGeneratedMusic()
{
    // Initialize the generator before starting playback.

    _music.begin(AUDIO_SAMPLE_RATE);
    _music.start();

    const bool started = _sound.playSource(
        &AlarmEffects::readGeneratedMusic,
        &_music,
        AUDIO_SAMPLE_RATE,
        SoundManager::AudioStream::Alarm
    );

    if (!started)
    {
        _music.stop();

        Serial0.println(
            "[AlarmEffects] playSource() failed"
        );

        return false;
    }

    return true;
}

// ============================================================
// PCM SOURCE CALLBACK
// ============================================================

size_t AlarmEffects::readGeneratedMusic(
    void* context,
    int16_t* buffer,
    size_t sampleCount
)
{
    if (context == nullptr ||
        buffer == nullptr ||
        sampleCount == 0)
    {
        return 0;
    }

    auto* music =
        static_cast<MusicGenerator*>(context);

    if (!music->isPlaying())
        return 0;

    music->generateBlock(
        buffer,
        sampleCount
    );

    return sampleCount;
}

// ============================================================
// STOP
// ============================================================

void AlarmEffects::stop()
{
    if (!_active)
        return;

    Serial0.println("[AlarmEffects] stop()");

    // --------------------------------------------------------
    // STOP AUDIO
    // --------------------------------------------------------

    _sound.stop();

    _music.stop();
    _music.end();

    _audioStarted = false;

    // --------------------------------------------------------
    // STOP LIGHT EFFECT
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

    Serial0.println("[AlarmEffects] stopped");
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
    return _active &&
           _effect == EffectType::Sunrise;
}

// ============================================================

AlarmEffects::EffectType AlarmEffects::effect() const
{
    return _effect;
}
