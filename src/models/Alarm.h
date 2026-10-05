#pragma once

#include <Arduino.h>
#include <stdint.h>

// ============================================================
// ALARM CONSTANTS
// ============================================================

namespace AlarmConfig
{
    static constexpr uint8_t MAX_PHASES = 16;
    static constexpr uint8_t MAX_ALARMS = 8;
}


// ============================================================
// TIME
// ============================================================

struct AlarmTime
{
    uint8_t hour   = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
};


// ============================================================
// CONDITION
// ============================================================

enum class AlarmCondition : uint8_t
{
    Always = 0,
    IfNotDismissed,
    IfNotSnoozed,
    IfNoMotion
};


// ============================================================
// MATRIX EFFECT
// ============================================================

struct AlarmMatrix
{
    bool enabled = false;

    // Имя эффекта из реестра.
    String effectId;

    // Универсальные параметры эффекта.
    // Обычно 0..100.
    uint8_t start = 0;
    uint8_t end   = 0;

    // Скорость / период эффекта.
    uint32_t speedMs = 0;

    // Длительность работы параметра.
    // 0 = без ограничения.
    uint32_t durationMs = 0;
};


// ============================================================
// AUDIO
// ============================================================

struct AlarmAudio
{
    bool enabled = false;

    // Имя аудиоэффекта из реестра.
    String effectId;

    // Обычно громкость 0..100.
    uint8_t start = 0;
    uint8_t end   = 0;

    // Скорость / период эффекта.
    uint32_t speedMs = 0;

    // Время изменения громкости.
    // 0 = без перехода.
    uint32_t durationMs = 0;

    // Повторять звук.
    bool loop = false;
};
struct AlarmRuntime
{
    bool active = false;
    bool dismissed = false;
    bool snoozed = false;

    String alarmId;

    uint8_t currentPhase = 0;

    uint32_t triggerTimeMs = 0;
    uint32_t elapsedMs = 0;
    uint32_t phaseElapsedMs = 0;
};

// ============================================================
// COB
// ============================================================

struct AlarmCob
{
    bool enabled = false;

    // Имя эффекта из реестра.
    String effectId;

    // Яркость 0..100.
    uint8_t start = 0;
    uint8_t end   = 0;

    // Период эффекта.
    uint32_t speedMs = 0;

    // Длительность.
    // 0 = до выключения будильника.
    uint32_t durationMs = 0;

    // Максимальное время работы COB.
    // 0 = без ограничения.
    uint32_t maxDurationMs = 0;
};


// ============================================================
// ALARM PHASE
// ============================================================

struct AlarmPhase
{
    // Смещение относительно времени T0.
    //
    // Например:
    // -600000 = за 10 минут
    // 0       = момент срабатывания
    // 30000   = через 30 секунд
    int32_t startOffsetMs = 0;

    // Длительность фазы.
    // 0 = до выключения будильника.
    uint32_t durationMs = 0;

    AlarmCondition condition =
        AlarmCondition::Always;

    AlarmMatrix matrix;
    AlarmAudio audio;
    AlarmCob cob;
};


// ============================================================
// ALARM
// ============================================================

struct Alarm
{
    // Версия структуры/схемы.
    uint16_t schemaVersion = 1;

    // UUID будильника.
    String id;

    // Название.
    String name;

    // Включен ли будильник.
    bool enabled = false;

    // Время T0.
    AlarmTime time;

    // --------------------------------------------------------
    // REPEAT MASK
    // --------------------------------------------------------
    //
    // bit 0 = Monday
    // bit 1 = Tuesday
    // bit 2 = Wednesday
    // bit 3 = Thursday
    // bit 4 = Friday
    // bit 5 = Saturday
    // bit 6 = Sunday
    //
    // 0   = одноразовый
    // 31  = Пн-Пт
    // 127 = каждый день
    //
    // --------------------------------------------------------

    uint8_t repeatMask = 0;

    // --------------------------------------------------------
    // PHASES
    // --------------------------------------------------------

    AlarmPhase phases[AlarmConfig::MAX_PHASES];

    uint8_t phaseCount = 0;
};
