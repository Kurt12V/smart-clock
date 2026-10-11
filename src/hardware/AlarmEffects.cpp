#include "AlarmEffects.h"

#include <Arduino.h>
#include <algorithm>

// ============================================================
// CONSTANTS
// ============================================================

namespace
{
    constexpr uint32_t MUSIC_START_MS =
        21UL * 60UL * 1000UL;

    constexpr uint32_t PEAK_TIME_MS =
        25UL * 60UL * 1000UL;

    constexpr uint32_t FULL_VOLUME_RAMP_MS =
        60UL * 1000UL;

    // Продолжительность alarm.wav — 40 секунд.
    constexpr uint32_t WAV_DURATION_MS =
        40UL * 1000UL;

    constexpr uint8_t MIN_VOLUME_PERCENT = 0;
    constexpr uint8_t MAX_VOLUME_PERCENT = 100;
}

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
      _nextWavStartMs(0)
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

    // Остановить предыдущий эффект, если он был активен.
    stop();

    // Подготовить состояние аудио.
    _sound.stop();
    _sound.setLocalPercent(0);

    _audioStarted = false;
    _nextWavStartMs = MUSIC_START_MS;

    // Получить управление освещением на время будильника.
    _lighting.beginAlarmOverride();

    // Запустить эффект рассвета.
    _sunrise.start();

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
    // Обновляем состояние самого эффекта рассвета.
    _sunrise.update(elapsedMs);

    // Применяем основной свет.
    updateSunriseLight();

    // Применяем вспомогательные COB-светодиоды.
    applyAuxiliaryLeds();

    // Обновляем воспроизведение WAV.
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
    // Если дополнительный свет неактивен,
    // принудительно выключаем его.
    if (!_sunrise.auxiliaryEnabled())
    {
        _lighting.setAlarmCob(0);
        return;
    }

    // Вспышка выключена в текущем кадре.
    if (!_sunrise.auxiliaryFlashState())
    {
        _lighting.setAlarmCob(0);
        return;
    }

    // auxiliaryBrightness() возвращает проценты 0–100.
    // setAlarmCob() принимает яркость 0–255.
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
    // До 21-й минуты звук не воспроизводится.
    if (elapsedMs < MUSIC_START_MS)
        return;

    // Определяем текущую громкость.
    const uint8_t volumePercent =
        calculateVolumePercent(elapsedMs);

    // SoundManager применяет локальный уровень громкости
    // к воспроизводимому аудио.
    _sound.setLocalPercent(volumePercent);

    // Запускаем WAV при первом достижении времени старта
    // и повторяем его с интервалом в 40 секунд.
    if (!_audioStarted ||
        elapsedMs >= _nextWavStartMs)
    {
        if (startAlarmWav(volumePercent))
        {
            _audioStarted = true;

            _nextWavStartMs =
                elapsedMs + WAV_DURATION_MS;
        }
    }
}

// ============================================================
// CALCULATE VOLUME
// ============================================================

uint8_t AlarmEffects::calculateVolumePercent(
    uint32_t elapsedMs
) const
{
    float volume = 0.0f;

    // --------------------------------------------------------
    // 21:00–25:00
    // 0% -> 30%
    // --------------------------------------------------------

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

        volume = 30.0f * progress;
    }
    // --------------------------------------------------------
    // 25:00–26:00
    // 30% -> 100%
    // --------------------------------------------------------
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

        volume = 30.0f + 70.0f * progress;
    }

    // После завершения нарастания сохраняем 100%.
    volume = std::max(
        0.0f,
        std::min(100.0f, volume)
    );

    return static_cast<uint8_t>(volume);
}

// ============================================================
// START ALARM WAV
// ============================================================

bool AlarmEffects::startAlarmWav(
    uint8_t volumePercent
)
{
    static constexpr char ALARM_WAV_PATH[] =
        "/audio/alarms/alarm.wav";

    const uint8_t safeVolume =
        std::max(
            MIN_VOLUME_PERCENT,
            std::min(
                MAX_VOLUME_PERCENT,
                volumePercent
            )
        );

    _sound.setLocalPercent(safeVolume);

    Serial0.printf(
        "[AlarmEffects] Playing WAV: %s, volume=%u%%\n",
        ALARM_WAV_PATH,
        static_cast<unsigned>(safeVolume)
    );

    // SoundManager отвечает за чтение WAV и передачу PCM
    // в аудиотракт. MusicGenerator здесь не используется.
    _sound.play(ALARM_WAV_PATH);

    return true;
}

// ============================================================
// STOP
// ============================================================

void AlarmEffects::stop()
{
    // Если эффект уже выключен, сбрасываем состояние
    // аудио и выходим.
    if (!_active)
    {
        _audioStarted = false;
        _nextWavStartMs = 0;
        return;
    }

    Serial0.printf(
        "[AlarmEffects] stop()\n"
    );

    // Остановить воспроизведение WAV.
    _sound.stop();
    _sound.setLocalPercent(100);

    _audioStarted = false;
    _nextWavStartMs = 0;

    // Остановить рассвет.
    _sunrise.stop();

    // Вернуть управление освещением обычной системе.
    _lighting.setAlarmCob(0);
    _lighting.endAlarmOverride();

    // Сбросить состояние эффекта.
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