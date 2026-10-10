
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
      _music(music)
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

    // SoundManager владеет I2S-выходом.
    if (!_sound.isInitialized())
    {
        if (!_sound.begin())
        {
            Serial0.println(
                "[AlarmEffects][ERROR] SoundManager initialization failed"
            );

            return;
        }
    }

    // В актуальном API begin() возвращает void.
    _music.begin(AUDIO_SAMPLE_RATE);
    _music.setVolume(0.0f);
    _music.stop();

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

    if (!_begun)
    {
        Serial0.println(
            "[AlarmEffects][ERROR] Cannot start sunrise"
        );

        return;
    }

    // Останавливаем предыдущий эффект.
    stop();

    // Начальное состояние музыки.
    _music.stop();
    _music.setVolume(0.05f);

    // Передаём управление освещением будильнику.
    _lighting.beginAlarmOverride();

    // Запускаем световой сценарий.
    _sunrise.start();

    _effect = EffectType::Sunrise;
    _active = true;
    _audioStarted = false;

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
// AUXILIARY COB LIGHTS
// ============================================================

void AlarmEffects::applyAuxiliaryLeds()
{
    if (!_sunrise.auxiliaryEnabled() ||
        !_sunrise.auxiliaryFlashState())
    {
        _lighting.setAlarmCob(0);
        return;
    }

    // auxiliaryBrightness(): 0–100%.
    // setAlarmCob(): 0–255.

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
// CALCULATE MUSIC VOLUME
// ============================================================

float AlarmEffects::calculateMusicVolume(
    uint32_t elapsedMs
)
{
    if (elapsedMs < MUSIC_START_MS)
        return 0.0f;

    // От начала воспроизведения до 25-й минуты:
    // плавное увеличение громкости от 0 до 30%.

    if (elapsedMs < PEAK_TIME_MS)
    {
        const uint32_t elapsed =
            elapsedMs - MUSIC_START_MS;

        const uint32_t duration =
            PEAK_TIME_MS - MUSIC_START_MS;

        const float progress =
            static_cast<float>(elapsed) /
            static_cast<float>(duration);

        const float clampedProgress =
            std::max(
                0.0f,
                std::min(1.0f, progress)
            );

        return 0.30f * clampedProgress;
    }

    // От 25-й до 26-й минуты:
    // плавное увеличение громкости от 30 до 100%.

    const uint32_t elapsed =
        elapsedMs - PEAK_TIME_MS;

    const float progress =
        static_cast<float>(elapsed) /
        static_cast<float>(FULL_VOLUME_RAMP_MS);

    const float clampedProgress =
        std::max(
            0.0f,
            std::min(1.0f, progress)
        );

    return 0.30f + 0.70f * clampedProgress;
}

// ============================================================
// UPDATE SUNRISE AUDIO
// ============================================================

void AlarmEffects::updateSunriseAudio(
    uint32_t elapsedMs
)
{
    if (elapsedMs < MUSIC_START_MS)
        return;

    // Громкость задаётся в диапазоне 0.0–1.0.
    _music.setVolume(
        calculateMusicVolume(elapsedMs)
    );

    if (_audioStarted)
        return;

    // Запускаем генератор без Preset:
    // в текущем MusicGenerator такого API нет.
    _music.stop();
    _music.start();

    // SoundManager вызывает musicPcmSource(),
    // получает PCM и выводит его через свой I2SManager.
    const bool started = _sound.playSource(
        &AlarmEffects::musicPcmSource,
        this,
        AUDIO_SAMPLE_RATE,
        SoundManager::AudioStream::Alarm
    );

    if (!started)
    {
        _music.stop();

        _music.setVolume(0.0f);

        Serial0.println(
            "[AlarmEffects][ERROR] Failed to start music PCM source"
        );

        return;
    }

    _audioStarted = true;

    Serial0.println(
        "[AlarmEffects] Sunrise audio started"
    );
}

// ============================================================
// MUSIC PCM CALLBACK
// ============================================================

size_t AlarmEffects::musicPcmSource(
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

    auto* self =
        static_cast<AlarmEffects*>(context);

    if (!self->_active ||
        !self->_audioStarted)
    {
        return 0;
    }

    // Актуальный MusicGenerator API:
    // void generateBlock(int16_t*, size_t).
    self->_music.generateBlock(
        buffer,
        sampleCount
    );

    // Возвращаем количество сформированных моносэмплов.
    return sampleCount;
}

// ============================================================
// STOP
// ============================================================

void AlarmEffects::stop()
{
    if (!_active && !_audioStarted)
        return;

    Serial0.println("[AlarmEffects] Stopping");

    // Сначала запрещаем дальнейшую генерацию PCM.
    _active = false;

    // SoundManager владеет I2S и останавливает его.
    // Останавливаем его только если мы запускали музыку.
    if (_audioStarted)
    {
        _sound.stop();
    }

    _music.stop();
    _music.setVolume(0.0f);

    _sunrise.stop();

    _lighting.setAlarmCob(0);
    _lighting.endAlarmOverride();

    _effect = EffectType::None;
    _audioStarted = false;

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
