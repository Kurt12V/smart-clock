
#include "SunriseAlarm.h"

#include <cmath>

// ============================================================
// CONSTRUCTOR
// ============================================================

SunriseAlarm::SunriseAlarm(
    LightingManager& lighting,
    SoundManager& sound
)
    : _lighting(lighting),
      _sound(sound)
{
}

// ============================================================
// START
// ============================================================

void SunriseAlarm::start()
{
    if (_active)
        return;

    _sound.stop();

    _active = true;
    _state = State::Running;

    _elapsedMs = 0;

    _matrixApplied = false;

    _lastCobBrightness = -1;

    _lastAuxiliaryToggleUs = micros();

    _auxiliaryOn = false;

    _musicEverStarted = false;
    _musicRetryPending = false;

    _lastMusicAttemptMs = 0;

    // Перехватываем управление освещением.
    _lighting.beginAlarmOverride();

    // Начальное состояние света.
    applyMatrix(
        SunriseConfig::PHASE_1_START_BRIGHTNESS,
        SunriseConfig::PHASE_1_START_R,
        SunriseConfig::PHASE_1_START_G,
        SunriseConfig::PHASE_1_START_B
    );

    _lighting.setAlarmCob(0);

    _lastCobBrightness = 0;

    Serial0.println("[SUNRISE] Started");
}

// ============================================================
// STOP
// ============================================================

void SunriseAlarm::stop()
{
    // Сначала останавливаем аудио.
    _sound.stop();

    _musicEverStarted = false;
    _musicRetryPending = false;

    // Выключаем дополнительное освещение.
    if (_lighting.isAlarmOverrideActive())
    {
        _lighting.setAlarmCob(0);

        // Возвращаем обычное управление освещением.
        _lighting.endAlarmOverride();
    }

    _active = false;
    _state = State::Idle;

    _elapsedMs = 0;

    _matrixApplied = false;

    _lastCobBrightness = -1;

    _auxiliaryOn = false;

    Serial0.println("[SUNRISE] Stopped");
}

// ============================================================
// UPDATE
// ============================================================

void SunriseAlarm::update(uint32_t elapsedMs)
{
    if (!_active)
        return;

    _elapsedMs = elapsedMs;

    updateLighting(elapsedMs);

    updateMusic(elapsedMs);
}

// ============================================================
// LIGHTING UPDATE
// ============================================================

void SunriseAlarm::updateLighting(uint32_t elapsedMs)
{
    if (elapsedMs >= SunriseConfig::DURATION_MS)
    {
        updatePeak();
        return;
    }

    if (elapsedMs < SunriseConfig::PHASE_1_END_MS)
    {
        updatePhase1(elapsedMs);
        return;
    }

    if (elapsedMs < SunriseConfig::PHASE_2_END_MS)
    {
        updatePhase2(elapsedMs);
        return;
    }

    updatePhase3(elapsedMs);
}

// ============================================================
// PHASE 1
// RED -> ORANGE
// ============================================================

void SunriseAlarm::updatePhase1(uint32_t elapsedMs)
{
    const float progress = normalizeProgress(
        elapsedMs,
        SunriseConfig::PHASE_1_END_MS
    );

    const uint8_t red = interpolate(
        SunriseConfig::PHASE_1_START_R,
        SunriseConfig::PHASE_1_END_R,
        progress
    );

    const uint8_t green = interpolate(
        SunriseConfig::PHASE_1_START_G,
        SunriseConfig::PHASE_1_END_G,
        progress
    );

    const uint8_t blue = interpolate(
        SunriseConfig::PHASE_1_START_B,
        SunriseConfig::PHASE_1_END_B,
        progress
    );

    const uint8_t brightness = interpolate(
        SunriseConfig::PHASE_1_START_BRIGHTNESS,
        SunriseConfig::PHASE_1_END_BRIGHTNESS,
        progress
    );

    applyMatrix(
        brightness,
        red,
        green,
        blue
    );

    _state = State::Running;
}

// ============================================================
// PHASE 2
// ORANGE -> WARM
// ============================================================

void SunriseAlarm::updatePhase2(uint32_t elapsedMs)
{
    const uint32_t phaseDuration =
        SunriseConfig::PHASE_2_END_MS -
        SunriseConfig::PHASE_1_END_MS;

    const uint32_t phaseElapsed =
        elapsedMs -
        SunriseConfig::PHASE_1_END_MS;

    const float progress = normalizeProgress(
        phaseElapsed,
        phaseDuration
    );

    const uint8_t red = interpolate(
        SunriseConfig::PHASE_1_END_R,
        SunriseConfig::PHASE_2_END_R,
        progress
    );

    const uint8_t green = interpolate(
        SunriseConfig::PHASE_1_END_G,
        SunriseConfig::PHASE_2_END_G,
        progress
    );

    const uint8_t blue = interpolate(
        SunriseConfig::PHASE_1_END_B,
        SunriseConfig::PHASE_2_END_B,
        progress
    );

    const uint8_t brightness = interpolate(
        SunriseConfig::PHASE_1_END_BRIGHTNESS,
        SunriseConfig::PHASE_2_END_BRIGHTNESS,
        progress
    );

    applyMatrix(
        brightness,
        red,
        green,
        blue
    );

    _state = State::Running;
}

// ============================================================
// PHASE 3
// WARM -> BLUE
// ============================================================

void SunriseAlarm::updatePhase3(uint32_t elapsedMs)
{
    const uint32_t phaseDuration =
        SunriseConfig::DURATION_MS -
        SunriseConfig::PHASE_2_END_MS;

    const uint32_t phaseElapsed =
        elapsedMs -
        SunriseConfig::PHASE_2_END_MS;

    const float progress = normalizeProgress(
        phaseElapsed,
        phaseDuration
    );

    const uint8_t red = interpolate(
        SunriseConfig::PHASE_2_END_R,
        SunriseConfig::PEAK_R,
        progress
    );

    const uint8_t green = interpolate(
        SunriseConfig::PHASE_2_END_G,
        SunriseConfig::PEAK_G,
        progress
    );

    const uint8_t blue = interpolate(
        SunriseConfig::PHASE_2_END_B,
        SunriseConfig::PEAK_B,
        progress
    );

    const uint8_t brightness = interpolate(
        SunriseConfig::PHASE_2_END_BRIGHTNESS,
        SunriseConfig::PHASE_3_END_BRIGHTNESS,
        progress
    );

    applyMatrix(
        brightness,
        red,
        green,
        blue
    );

    _state = State::Running;
}

// ============================================================
// PEAK
// ============================================================

void SunriseAlarm::updatePeak()
{
    _state = State::Peak;

    applyMatrix(
        SunriseConfig::PEAK_BRIGHTNESS,
        SunriseConfig::PEAK_R,
        SunriseConfig::PEAK_G,
        SunriseConfig::PEAK_B
    );

    updateAuxiliary();
}

// ============================================================
// APPLY MATRIX
// ============================================================

void SunriseAlarm::applyMatrix(
    uint8_t brightness,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    if (_matrixApplied &&
        _lastMatrixBrightness == brightness &&
        _lastRed == red &&
        _lastGreen == green &&
        _lastBlue == blue)
    {
        return;
    }

    _lighting.setAlarmMatrix(
        brightness,
        red,
        green,
        blue
    );

    _lastMatrixBrightness = brightness;

    _lastRed = red;
    _lastGreen = green;
    _lastBlue = blue;

    _matrixApplied = true;
}

// ============================================================
// AUXILIARY COB FLASH
// ============================================================

void SunriseAlarm::updateAuxiliary()
{
    const uint32_t periodUs =
        1000000UL /
        SunriseConfig::COB_FLASH_FREQUENCY_HZ;

    const uint32_t onTimeUs =
        periodUs *
        SunriseConfig::COB_DUTY_PERCENT /
        100UL;

    const uint32_t nowUs = micros();

    const uint32_t phaseUs =
        (nowUs - _lastAuxiliaryToggleUs) % periodUs;

    const bool shouldBeOn =
        phaseUs < onTimeUs;

    if (shouldBeOn == _auxiliaryOn)
        return;

    _auxiliaryOn = shouldBeOn;

    const uint8_t brightness = shouldBeOn
        ? static_cast<uint8_t>(
            (255UL * SunriseConfig::PEAK_COB_BRIGHTNESS) / 100UL
          )
        : 0;

    _lighting.setAlarmCob(brightness);

    _lastCobBrightness = brightness;
}

// ============================================================
// MUSIC UPDATE
// ============================================================

void SunriseAlarm::updateMusic(uint32_t elapsedMs)
{
    if (!_active)
        return;

    // Музыка начинается только на 21-й минуте.
    if (elapsedMs < SunriseConfig::MUSIC_START_MS)
        return;

    // Если воспроизведение активно, ничего не делаем.
    // Это защищает от запуска второго экземпляра.
    if (_sound.isActive())
        return;

    // Если предыдущая попытка запуска не удалась,
    // ждём перед следующей попыткой.
    if (_musicRetryPending)
    {
        if ((elapsedMs - _lastMusicAttemptMs) <
            SunriseConfig::MUSIC_RETRY_INTERVAL_MS)
        {
            return;
        }
    }

    // Первый запуск — с fade-in.
    // Все последующие запуски — с начала файла без fade-in.
    startMusic(
        !_musicEverStarted,
        elapsedMs
    );
}

// ============================================================
// START MUSIC
// ============================================================

void SunriseAlarm::startMusic(
    bool fadeIn,
    uint32_t elapsedMs
)
{
    if (!_active)
        return;

    SoundManager::PlayOptions options{};

    options.stream = SoundManager::AudioStream::Alarm;

    options.fadeInMs = fadeIn
        ? SunriseConfig::MUSIC_FADE_IN_MS
        : 0;

    options.fadeOutMs = 0;

    _lastMusicAttemptMs = elapsedMs;

    const bool started = _sound.play(
        SunriseConfig::MUSIC_PATH,
        options
    );

    if (!started)
    {
        _musicRetryPending = true;

        Serial0.println(
            "[SUNRISE] Music start failed, retry scheduled"
        );

        return;
    }

    _musicEverStarted = true;
    _musicRetryPending = false;

    if (fadeIn)
    {
        Serial0.println(
            "[SUNRISE] Music started with fade-in"
        );
    }
    else
    {
        Serial0.println(
            "[SUNRISE] Music restarted from beginning"
        );
    }
}

// ============================================================
// HELPERS
// ============================================================

uint8_t SunriseAlarm::interpolate(
    uint8_t start,
    uint8_t end,
    float progress
)
{
    if (progress < 0.0f)
        progress = 0.0f;

    if (progress > 1.0f)
        progress = 1.0f;

    const float value =
        static_cast<float>(start) +
        (static_cast<float>(end) - start) * progress;

    return static_cast<uint8_t>(
        value + 0.5f
    );
}

// ============================================================
// NORMALIZE PROGRESS
// ============================================================

float SunriseAlarm::normalizeProgress(
    uint32_t value,
    uint32_t duration
)
{
    if (duration == 0)
        return 1.0f;

    float progress =
        static_cast<float>(value) /
        static_cast<float>(duration);

    progress *= SunriseConfig::COLOR_SPEED;

    if (progress > 1.0f)
        progress = 1.0f;

    if (progress < 0.0f)
        progress = 0.0f;

    return progress;
}

// ============================================================
// GETTERS
// ============================================================

bool SunriseAlarm::isActive() const
{
    return _active;
}

bool SunriseAlarm::isPeak() const
{
    return _state == State::Peak;
}

SunriseAlarm::State SunriseAlarm::state() const
{
    return _state;
}

uint32_t SunriseAlarm::elapsed() const
{
    return _elapsedMs;
}
