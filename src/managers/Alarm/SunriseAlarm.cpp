#include "SunriseAlarm.h"

#include <Arduino.h>
#include <algorithm>
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
    {
        Serial0.println("[SunriseAlarm] Already running");
        return;
    }

    if (!_lighting.isAlarmOverrideActive())
    {
        _lighting.beginAlarmOverride();
    }

    if (!_lighting.isAlarmOverrideActive())
    {
        Serial0.println(
            "[SunriseAlarm][ERROR] Cannot start lighting override"
        );

        return;
    }

    // Сбрасываем время.
    _startTimeMs = millis();
    _elapsedMs = 0;

    // Сбрасываем состояние.
    _state = State::Running;
    _active = true;

    // Сбрасываем кеш матрицы.
    _matrixApplied = false;

    _lastMatrixBrightness = 0;

    _lastRed = 0;
    _lastGreen = 0;
    _lastBlue = 0;

    // Сбрасываем COB.
    _lastCobBrightness = -1;

    // Сбрасываем музыку.
    _musicEverStarted = false;
    _musicRetryPending = false;
    _musicPlaying = false;
    _lastMusicAttemptMs = 0;

    Serial0.println("[SunriseAlarm] Started");

    // Начальная точка рассвета.
    updateLighting(0);
}

// ============================================================
// STOP
// ============================================================

void SunriseAlarm::stop()
{
    if (!_active &&
        !_lighting.isAlarmOverrideActive())
    {
        return;
    }

    Serial0.println("[SunriseAlarm] Stopping");

    _active = false;

    _state = State::Idle;

    _elapsedMs = 0;

    // Останавливаем музыку.
    _sound.stop();

    // Выключаем COB.
    _lighting.setAlarmCob(0);

    // Возвращаем обычное управление освещением.
    _lighting.endAlarmOverride();

    // Сбрасываем состояние.
    _matrixApplied = false;

    _lastCobBrightness = -1;

    _musicEverStarted = false;
    _musicRetryPending = false;
    _musicPlaying = false;

    Serial0.println("[SunriseAlarm] Stopped");
}

// ============================================================
// UPDATE
// ============================================================

void SunriseAlarm::update()
{
    // Обслуживаем звук постоянно.
    _sound.update();

    // Обслуживаем обычную систему освещения.
    _lighting.update();

    if (!_active)
        return;

    const uint32_t now = millis();

    _elapsedMs = now - _startTimeMs;

    // Освещение.
    updateLighting(_elapsedMs);

    // Музыка.
    updateMusic(_elapsedMs);

    // Дополнительное управление COB.
    updateAuxiliary();

    // Достигли пика рассвета.
    if (_elapsedMs >= SunriseConfig::DURATION_MS)
    {
        if (_state != State::Peak)
        {
            _state = State::Peak;

            updatePeak();

            Serial0.println("[SunriseAlarm] Peak reached");
        }
    }
}

// ============================================================
// LIGHTING UPDATE
// ============================================================

void SunriseAlarm::updateLighting(uint32_t elapsedMs)
{
    if (elapsedMs < SunriseConfig::PHASE_1_END_MS)
    {
        _state = State::Running;

        updatePhase1(elapsedMs);
        return;
    }

    if (elapsedMs < SunriseConfig::PHASE_2_END_MS)
    {
        _state = State::Running;

        updatePhase2(elapsedMs);
        return;
    }

    if (elapsedMs < SunriseConfig::DURATION_MS)
    {
        _state = State::Running;

        updatePhase3(elapsedMs);
        return;
    }

    updatePeak();
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

    const float colorProgress = std::min(
        1.0f,
        progress * SunriseConfig::COLOR_SPEED
    );

    const uint8_t red = interpolate(
        SunriseConfig::PHASE_1_START_R,
        SunriseConfig::PHASE_1_END_R,
        colorProgress
    );

    const uint8_t green = interpolate(
        SunriseConfig::PHASE_1_START_G,
        SunriseConfig::PHASE_1_END_G,
        colorProgress
    );

    const uint8_t blue = interpolate(
        SunriseConfig::PHASE_1_START_B,
        SunriseConfig::PHASE_1_END_B,
        colorProgress
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
}

// ============================================================
// PHASE 2
// ORANGE -> WARM COLOR
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
}

// ============================================================
// PHASE 3
// WARM COLOR -> BLUE PEAK
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
}

// ============================================================
// PEAK
// ============================================================

void SunriseAlarm::updatePeak()
{
    applyMatrix(
        SunriseConfig::PEAK_BRIGHTNESS,
        SunriseConfig::PEAK_R,
        SunriseConfig::PEAK_G,
        SunriseConfig::PEAK_B
    );
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
        brightness == _lastMatrixBrightness &&
        red == _lastRed &&
        green == _lastGreen &&
        blue == _lastBlue)
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
// AUXILIARY COB — ПЛАВНОЕ ДЫХАНИЕ
// ============================================================

void SunriseAlarm::updateAuxiliary()
{
    // COB начинает дышать только с третьей фазы.
    if (_elapsedMs < SunriseConfig::PHASE_2_END_MS)
    {
        if (_lastCobBrightness != 0)
        {
            _lighting.setAlarmCob(0);
            _lastCobBrightness = 0;
        }

        return;
    }

    // Период дыхания.
    constexpr uint32_t PERIOD_US =
        1000000UL / SunriseConfig::COB_FLASH_FREQUENCY_HZ;

    // Фаза внутри периода: 0..PERIOD_US.
    const uint32_t nowUs = micros();
    const uint32_t phaseUs = nowUs % PERIOD_US;

    // progress 0..1 внутри периода.
    const float progress =
        static_cast<float>(phaseUs) /
        static_cast<float>(PERIOD_US);

    // Синусоидальное дыхание: 0 -> 1 -> 0 за один период.
    // sin(progress * PI) даёт плавный "горб" без резких фронтов.
    const float wave = sinf(progress * 3.1415926f); // 0..1..0

    const uint8_t brightness = static_cast<uint8_t>(
        SunriseConfig::COB_MIN_BRIGHTNESS +
        wave * (
            SunriseConfig::COB_MAX_BRIGHTNESS -
            SunriseConfig::COB_MIN_BRIGHTNESS
        )
    );

    // Не дёргаем LightingManager без изменения.
    if (_lastCobBrightness == brightness)
        return;

    _lastCobBrightness = brightness;

    _lighting.setAlarmCob(brightness);
}

// ============================================================
// MUSIC UPDATE (С ЦИКЛИЧЕСКИМ ПЕРЕЗАПУСКОМ)
// ============================================================

void SunriseAlarm::updateMusic(uint32_t elapsedMs)
{
    if (elapsedMs < SunriseConfig::MUSIC_START_MS)
        return;

    const bool playing = _sound.isPlaying();

    // Музыка играет — ничего не делаем.
    if (playing)
    {
        _musicPlaying = true;
        return;
    }

    // Музыка только что закончилась — перезапускаем с плавным входом.
    if (_musicPlaying)
    {
        _musicPlaying = false;

        Serial0.println(
            "[SunriseAlarm] Music ended, restarting"
        );

        startMusic(
            true,
            SunriseConfig::MUSIC_RESTART_FADE_MS
        );

        return;
    }

    // Музыка ещё ни разу не запускалась или предыдущая попытка не удалась.
    if (_musicRetryPending)
    {
        const uint32_t now = millis();

        if (now - _lastMusicAttemptMs <
            SunriseConfig::MUSIC_RETRY_INTERVAL_MS)
        {
            return;
        }
    }

    const uint32_t fadeInMs = _musicEverStarted
        ? SunriseConfig::MUSIC_RESTART_FADE_MS
        : SunriseConfig::MUSIC_FADE_IN_MS;

    startMusic(true, fadeInMs);
}

// ============================================================
// START MUSIC
// ============================================================

void SunriseAlarm::startMusic(
    bool fadeIn,
    uint32_t fadeInMs
)
{
    _lastMusicAttemptMs = millis();
    _musicRetryPending = true;

    SoundManager::PlayOptions options;

    options.stream = SoundManager::AudioStream::Alarm;

    options.localPercent = 100;

    options.fadeInMs = fadeIn ? fadeInMs : 0;

    options.fadeOutMs = 0;

    options.curve = SoundManager::FadeCurve::Linear;

    Serial0.println("[SunriseAlarm] Starting music");

    const bool result = _sound.play(
        SunriseConfig::MUSIC_PATH,
        options
    );

    if (result)
    {
        _musicEverStarted = true;
        _musicRetryPending = false;
        _musicPlaying = true;

        Serial0.println("[SunriseAlarm] Music started");
    }
    else
    {
        _musicPlaying = false;

        Serial0.println(
            "[SunriseAlarm][WARN] Music start failed; retry pending"
        );
    }
}

// ============================================================
// INTERPOLATION
// ============================================================

uint8_t SunriseAlarm::interpolate(
    uint8_t start,
    uint8_t end,
    float progress
)
{
    progress = std::max(
        0.0f,
        std::min(progress, 1.0f)
    );

    const float value =
        static_cast<float>(start) +
        (
            static_cast<float>(end) -
            static_cast<float>(start)
        ) * progress;

    return static_cast<uint8_t>(
        std::round(value)
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

    const float progress =
        static_cast<float>(value) /
        static_cast<float>(duration);

    return std::max(
        0.0f,
        std::min(progress, 1.0f)
    );
}

// ============================================================
// STATUS
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