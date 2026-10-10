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
      _audioAttempted(false),
      _elapsedMs(0),
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

    // Остановить предыдущий сценарий, если он запущен.
    stop();

    // Остановить предыдущее воспроизведение.
    _sound.stop();

    // Сбросить генератор перед новым сценарием.
    _music.stop();
    _music.end();

    _elapsedMs = 0;

    _audioStarted = false;
    _audioAttempted = false;

    // Получить временный контроль над освещением.
    _lighting.beginAlarmOverride();

    // Перезапустить сценарий рассвета.
    _sunrise.start();

    _effect = EffectType::Sunrise;
    _active = true;

    Serial0.println("[AlarmEffects] Sunrise started");
    Serial0.println("[AlarmEffects] Generated music starts at 21:00");
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
    // Накапливаем длительность сценария.
    // elapsedMs — дельта времени текущего обновления.
    if (UINT32_MAX - _elapsedMs < elapsedMs)
        _elapsedMs = UINT32_MAX;
    else
        _elapsedMs += elapsedMs;

    // Обновляем внутренние фазы рассвета.
    _sunrise.update(elapsedMs);

    // Применяем состояние освещения.
    updateSunriseLight();

    // Обновляем вспомогательные COB-светодиоды.
    applyAuxiliaryLeds();

    // Запускаем музыку и передаём ей текущую громкость.
    updateSunriseAudio();
}

// ============================================================
// UPDATE MATRIX
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
// UPDATE AUXILIARY COB
// ============================================================

void AlarmEffects::applyAuxiliaryLeds()
{
    if (!_sunrise.auxiliaryEnabled() ||
        !_sunrise.auxiliaryFlashState())
    {
        _lighting.setAlarmCob(0);
        return;
    }

    const uint8_t percent =
        _sunrise.auxiliaryBrightness();

    const uint8_t brightness =
        static_cast<uint8_t>(
            (static_cast<uint16_t>(percent) * 255U) / 100U
        );

    _lighting.setAlarmCob(brightness);
}

// ============================================================
// UPDATE AUDIO
// ============================================================

void AlarmEffects::updateSunriseAudio()
{
    // До начала музыкальной фазы звук не запускаем.
    if (_elapsedMs < MUSIC_START_MS)
        return;

    // Запуск генерации выполняется только один раз.
    if (!_audioAttempted)
    {
        _audioAttempted = true;
        startGeneratedMusic();
    }

    if (!_audioStarted)
        return;

    // Источник громкости — SunriseLightEffect.
    // SoundManager применит этот процент при выводе PCM.
    _sound.setLocalPercent(_sunrise.soundPercent());
}

// ============================================================
// START GENERATED MUSIC
// ============================================================

void AlarmEffects::startGeneratedMusic()
{
    Serial0.println("[AlarmEffects] Starting generated music");

    // Подготовить генератор на частоте, совпадающей с I2S.
    _music.begin(MUSIC_SAMPLE_RATE);
    _music.start();

    // Передавать сгенерированные PCM-сэмплы в SoundManager.
    const bool started = _sound.playSource(
        &AlarmEffects::readGeneratedMusic,
        &_music,
        MUSIC_SAMPLE_RATE,
        SoundManager::AudioStream::Alarm,
        &AlarmEffects::isGeneratedMusicFinished
    );

    if (!started)
    {
        _music.stop();
        _audioStarted = false;

        Serial0.println(
            "[AlarmEffects] ERROR: generated audio could not start"
        );

        return;
    }

    _audioStarted = true;

    // Начальная громкость берётся из текущей фазы рассвета.
    _sound.setLocalPercent(_sunrise.soundPercent());

    Serial0.printf(
        "[AlarmEffects] Generated audio started, volume=%u%%\n",
        _sunrise.soundPercent()
    );
}

// ============================================================
// MUSIC GENERATOR CALLBACK
// ============================================================

size_t AlarmEffects::readGeneratedMusic(
    void* context,
    int16_t* buffer,
    size_t sampleCount
)
{
    auto* music = static_cast<MusicGenerator*>(context);

    if (music == nullptr ||
        buffer == nullptr ||
        sampleCount == 0 ||
        !music->isPlaying())
    {
        return 0;
    }

    // MusicGenerator записывает sampleCount монофонических
    // сэмплов int16_t, а не количество байтов.
    music->generateBlock(buffer, sampleCount);

    return sampleCount;
}

// ============================================================
// MUSIC FINISHED CALLBACK
// ============================================================

bool AlarmEffects::isGeneratedMusicFinished(void* context)
{
    auto* music = static_cast<MusicGenerator*>(context);

    return music == nullptr || !music->isPlaying();
}

// ============================================================
// STOP AUDIO
// ============================================================

void AlarmEffects::stopAudio()
{
    if (_audioStarted || _audioAttempted)
        _sound.stop();

    _music.stop();
    _music.end();

    _audioStarted = false;
    _audioAttempted = false;
}

// ============================================================
// STOP
// ============================================================

void AlarmEffects::stop()
{
    if (!_active)
        return;

    Serial0.println("[AlarmEffects] Stopping");

    stopAudio();

    _sunrise.stop();

    // Выключить вспомогательные светодиоды перед
    // возвратом управления обычному освещению.
    _lighting.setAlarmCob(0);
    _lighting.endAlarmOverride();

    _elapsedMs = 0;

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
