
#include "AlarmEffects.h"

#include <algorithm>

// ============================================================
// CONSTRUCTOR
// ============================================================

AlarmEffects::AlarmEffects(
    LightingManager& lighting,
    SoundManager& sound
)
    : _lighting(lighting),
      _sound(sound),
      _effect(EffectType::None),
      _begun(false),
      _active(false),
      _sunrise(),
      _musicPath("/alarms/sunrise.wav"),
      _audioStarted(false),
      _audioAttempted(false)
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
// START SUNRISE — DEFAULT FILE
// ============================================================

void AlarmEffects::startSunrise()
{
    startSunrise("/alarms/sunrise.wav");
}

// ============================================================
// START SUNRISE — CUSTOM FILE
// ============================================================

void AlarmEffects::startSunrise(const char* musicPath)
{
    if (!_begun)
        begin();

    // Завершаем предыдущий сценарий, если он активен.
    stop();

    // Если SoundManager использовался ранее для другого звука,
    // останавливаем предыдущий файл перед запуском будильника.
    _sound.stop();

    if (musicPath != nullptr && musicPath[0] != '\0')
        _musicPath = musicPath;
    else
        _musicPath = "/alarms/sunrise.wav";

    _audioStarted = false;
    _audioAttempted = false;

    // Получаем временный контроль над освещением.
    _lighting.beginAlarmOverride();

    // Запускаем отсчёт рассвета с нуля.
    _sunrise.start();

    _effect = EffectType::Sunrise;
    _active = true;

    Serial0.printf(
        "[AlarmEffects] Sunrise started\n"
        "[AlarmEffects] Music: %s\n"
        "[AlarmEffects] Music starts at 21:00\n",
        _musicPath.c_str()
    );
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
    // 1. Обновляем состояние света.
    _sunrise.update(elapsedMs);

    // 2. Применяем световые параметры.
    updateSunriseLight();

    // 3. Обновляем вспомогательные COB-светодиоды.
    applyAuxiliaryLeds();

    // 4. Запускаем музыку и управляем её громкостью.
    updateSunriseAudio(elapsedMs);

    // 5. Обслуживаем чтение WAV и вывод аудио в I2S.
    //
    // Важно: в таком варианте SoundManager::update()
    // должен вызываться только здесь, а не повторно
    // из основного цикла приложения.
    if (_audioStarted)
    {
        _sound.update();

        if (!_sound.isActive())
        {
            _audioStarted = false;

            Serial0.println(
                "[AlarmEffects] Audio playback finished"
            );
        }
    }
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
            (
                static_cast<uint16_t>(percent) * 255U
            ) / 100U
        );

    _lighting.setAlarmCob(brightness);
}

// ============================================================
// UPDATE AUDIO
// ============================================================

void AlarmEffects::updateSunriseAudio(uint32_t elapsedMs)
{
    // До 21-й минуты музыка не играет.
    if (elapsedMs < MUSIC_START_MS)
        return;

    // Запускаем файл только один раз за сценарий.
    if (!_audioAttempted)
    {
        _audioAttempted = true;

        SoundManager::PlayOptions options;

        options.stream =
            SoundManager::AudioStream::Alarm;

        // Громкость в начале — 0% от VOLUME_ALARM.
        options.localPercent = 0;

        // Без дополнительного fade-in: громкостью управляет
        // сам сценарий рассвета.
        options.fadeInMs = 0;
        options.fadeOutMs = 0;

        if (!_sound.play(
                _musicPath.c_str(),
                options))
        {
            _audioStarted = false;

            Serial0.printf(
                "[AlarmEffects] ERROR: cannot play %s\n",
                _musicPath.c_str()
            );

            return;
        }

        _audioStarted = true;

        Serial0.printf(
            "[AlarmEffects] Audio started: %s\n",
            _musicPath.c_str()
        );
    }

    if (!_audioStarted)
        return;

    // Устанавливаем громкость до следующего чтения
    // аудиоблока SoundManager::update().
    const uint8_t volume =
        calculateMusicVolume(elapsedMs);

    _sound.setLocalPercent(volume);
}

// ============================================================
// CALCULATE MUSIC VOLUME
// ============================================================

uint8_t AlarmEffects::calculateMusicVolume(
    uint32_t elapsedMs
) const
{
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

        return static_cast<uint8_t>(
            PEAK_VOLUME_PERCENT * progress + 0.5f
        );
    }

    // 25:00 -> 26:00
    // 30% -> 100%
    const uint32_t elapsed =
        elapsedMs - PEAK_TIME_MS;

    float progress =
        static_cast<float>(elapsed) /
        static_cast<float>(FULL_VOLUME_RAMP_MS);

    progress = std::max(
        0.0f,
        std::min(1.0f, progress)
    );

    const float volume =
        PEAK_VOLUME_PERCENT +
        (MAX_VOLUME_PERCENT - PEAK_VOLUME_PERCENT) *
        progress;

    return static_cast<uint8_t>(
        volume + 0.5f
    );
}

// ============================================================
// STOP AUDIO
// ============================================================

void AlarmEffects::stopAudio()
{
    if (_audioStarted || _audioAttempted)
        _sound.stop();

    _audioStarted = false;
    _audioAttempted = false;
}

// ============================================================
// STOP
// ============================================================

void AlarmEffects::stop()
{
    if (!_active)
    {
        // Не останавливаем постороннее воспроизведение,
        // если этот объект не запускал сценарий.
        return;
    }

    Serial0.println("[AlarmEffects] Stopping");

    stopAudio();

    _sunrise.stop();

    // Выключаем вспомогательные светодиоды перед
    // возвращением управления обычному освещению.
    _lighting.setAlarmCob(0);

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
