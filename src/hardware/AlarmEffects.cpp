
#include "AlarmEffects.h"

#include <Arduino.h>
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

    Serial0.println();
    Serial0.println("[AlarmEffects] Initializing");

    _sunrise.begin();

    _begun = true;

    Serial0.println("[AlarmEffects] Ready");
}

// ============================================================
// START SUNRISE
// ============================================================

void AlarmEffects::startSunrise()
{
    if (!_begun)
        begin();

    Serial0.println("[AlarmEffects] Starting sunrise");

    // Остановить предыдущий эффект.
    stop();

    // Остановить старый звук.
    _sound.stop();

    // Сбросить предыдущую генерацию.
    _music.stop();
    _music.end();

    _audioStarted = false;

    // Передать управление освещением эффекту будильника.
    _lighting.beginAlarmOverride();

    // Запустить рассвет.
    _sunrise.start();

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
// SUNRISE UPDATE
// ============================================================

void AlarmEffects::updateSunrise(uint32_t elapsedMs)
{
    _sunrise.update(elapsedMs);

    updateSunriseLight();
    applyAuxiliaryLeds();
    updateSunriseAudio(elapsedMs);
}

// ============================================================
// MATRIX LIGHT
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
// AUXILIARY COB LIGHT
// ============================================================

void AlarmEffects::applyAuxiliaryLeds()
{
    if (!_sunrise.auxiliaryEnabled() ||
        !_sunrise.auxiliaryFlashState())
    {
        _lighting.setAlarmCob(0);
        return;
    }

    // SunriseLightEffect: 0–100%.
    // LightingManager: 0–255.

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
    if (elapsedMs < MUSIC_START_MS)
        return;

    if (!_audioStarted)
    {
        if (!startGeneratedMusic())
            return;

        _audioStarted = true;

        Serial0.println(
            "[AlarmEffects] Generated music started"
        );
    }

    // Громкость фазы рассвета.
    const uint8_t volumePercent =
        std::min<uint8_t>(
            _sunrise.soundPercent(),
            100
        );

    _sound.setLocalPercent(volumePercent);

    static uint8_t previousPercent = 255;

    if (volumePercent != previousPercent)
    {
        previousPercent = volumePercent;

        Serial0.printf(
            "[AlarmEffects] Phase sound: %u%%\n",
            static_cast<unsigned>(volumePercent)
        );
    }
}

// ============================================================
// START GENERATED MUSIC
// ============================================================

bool AlarmEffects::startGeneratedMusic()
{
    Serial0.println("[AlarmEffects] Initializing music generator");

    // Инициализируем генератор.
    _music.begin(AUDIO_SAMPLE_RATE);

    // Запускаем генерацию.
    _music.start();

    Serial0.println("[AlarmEffects] Starting SoundManager PCM");

    const bool started = _sound.playSource(
        &AlarmEffects::readGeneratedMusic,
        &_music,
        AUDIO_SAMPLE_RATE,
        SoundManager::AudioStream::Alarm
    );

    if (!started)
    {
        Serial0.println(
            "[AlarmEffects][ERROR] playSource failed"
        );

        _music.stop();

        return false;
    }

    Serial0.println(
        "[AlarmEffects] PCM playback started successfully"
    );

    return true;
}

// ============================================================
// PCM CALLBACK
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

    Serial0.println("[AlarmEffects] Stopping");

    // Остановить звук.
    _sound.stop();

    // Остановить и сбросить генератор.
    _music.stop();
    _music.end();

    _audioStarted = false;

    // Остановить световой эффект.
    _sunrise.stop();

    // Вернуть управление настройкам освещения.
    _lighting.endAlarmOverride();

    _effect = EffectType::None;
    _active = false;

    Serial0.println("[AlarmEffects] Stopped");
}

// ============================================================
// STATE
// ============================================================

bool AlarmEffects::isActive() const
{
    return _active;
}

bool AlarmEffects::isSunriseActive() const
{
    return _active &&
           _effect == EffectType::Sunrise;
}

AlarmEffects::EffectType AlarmEffects::effect() const
{
    return _effect;
}
