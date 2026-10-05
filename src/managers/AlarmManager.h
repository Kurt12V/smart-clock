#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <time.h>
#include <functional>

#include "models/Alarm.h"

class SDManager;
class ClockSystem;

class AlarmManager
{
public:

    using TriggerCallback =
        std::function<void(const Alarm& alarm)>;

    using PhaseCallback =
        std::function<void(
            const Alarm& alarm,
            uint8_t phaseIndex,
            const AlarmPhase& phase
        )>;

    using FinishCallback =
        std::function<void(const String& alarmId)>;


    explicit AlarmManager(
        SDManager& sd,
        ClockSystem& clock
    );


    // ========================================================
    // LIFECYCLE
    // ========================================================

    bool begin();

    void update();

    bool isInitialized() const;


    // ========================================================
    // CALLBACKS
    // ========================================================

    void setTriggerCallback(
        TriggerCallback callback
    );

    void setPhaseCallback(
        PhaseCallback callback
    );

    void setFinishCallback(
        FinishCallback callback
    );


    // ========================================================
    // CRUD
    // ========================================================

    bool create(
        Alarm alarm
    );

    bool update(
        const Alarm& alarm
    );

    bool remove(
        const String& id
    );

    bool get(
        const String& id,
        Alarm& alarm
    );

    bool exists(
        const String& id
    );


    // ========================================================
    // LOAD ALL
    // ========================================================

    bool loadAll(
        Alarm* alarms,
        uint8_t maxCount,
        uint8_t& count
    );


    // ========================================================
    // ENABLE / DISABLE
    // ========================================================

    bool enable(
        const String& id
    );

    bool disable(
        const String& id
    );

    bool setEnabled(
        const String& id,
        bool enabled
    );


    // ========================================================
    // ACTIVE ALARMS
    // ========================================================

    uint8_t activeCount() const;

    bool getActiveInfo(
        uint8_t index,
        String& id,
        AlarmTime& time,
        uint8_t& repeatMask
    ) const;

    int8_t findActive(
        const String& id
    ) const;

    bool addActive(
        const Alarm& alarm
    );

    bool removeActive(
        const String& id
    );

    void clearActive();


    // ========================================================
    // SD
    // ========================================================

    bool createDirectory();

    String pathFor(
        const String& id
    ) const;

    bool saveToSD(
        const Alarm& alarm
    );

    bool loadFromSD(
        const String& id,
        Alarm& alarm
    );

    bool deleteFromSD(
        const String& id
    );

    bool reload();


    // ========================================================
    // JSON
    // ========================================================

    bool serialize(
        const Alarm& alarm,
        JsonDocument& document
    ) const;

    bool deserialize(
        JsonDocument& document,
        Alarm& alarm
    ) const;


    // ========================================================
    // VALIDATION
    // ========================================================

    bool validate(
        const Alarm& alarm
    ) const;


    // ========================================================
    // RUNTIME
    // ========================================================

    bool isRunning() const;

    const Alarm* currentAlarm() const;

    const AlarmPhase* currentPhase() const;

    uint8_t currentPhaseIndex() const;

    uint32_t elapsedMs() const;


    // ========================================================
    // CONTROL
    // ========================================================

    bool dismiss();

    bool snooze(
        uint32_t durationMs
    );

    void finish();


private:

    // ========================================================
    // ACTIVE ALARM
    //
    // Только метаданные.
    // Полные phases здесь НЕ хранятся.
    // ========================================================

    struct ActiveAlarm
    {
        String id;

        AlarmTime time;

        uint8_t repeatMask = 0;

        bool enabled = false;

        int32_t earliestOffsetMs = 0;
    };


    // ========================================================
    // RUNTIME
    //
    // Только один реально выполняющийся Alarm
    // загружается полностью с SD.
    // ========================================================

    struct Runtime
    {
        bool active = false;

        String alarmId;

        Alarm alarm;

        uint8_t phaseIndex = 0;

        time_t triggerT0 = 0;

        int64_t elapsedMs = 0;

        int64_t phaseElapsedMs = 0;

        bool dismissed = false;

        bool snoozed = false;

        uint32_t lastUpdateMs = 0;
    };


    // ========================================================
    // SNOOZE
    // ========================================================

    struct SnoozeState
    {
        bool active = false;

        String alarmId;

        uint32_t untilMs = 0;
    };


    // ========================================================
    // CALLBACKS
    // ========================================================

    TriggerCallback _triggerCallback;

    PhaseCallback _phaseCallback;

    FinishCallback _finishCallback;


    // ========================================================
    // DEPENDENCIES
    // ========================================================

    SDManager& _sd;

    ClockSystem& _clock;


    // ========================================================
    // ACTIVE ALARMS
    // ========================================================

    ActiveAlarm
        _activeAlarms[
            AlarmConfig::MAX_ALARMS
        ];

    uint8_t _activeCount = 0;


    // Последнее срабатывание каждого активного будильника.

    time_t
        _lastTriggerT0[
            AlarmConfig::MAX_ALARMS
        ];


    // ========================================================
    // CURRENT RUNTIME
    // ========================================================

    Runtime _runtime;

    SnoozeState _snooze;


    // ========================================================
    // STATE
    // ========================================================

    bool _initialized = false;


    // ========================================================
    // TIME
    // ========================================================

    time_t currentLocalTimestamp() const;

    time_t makeLocalT0(
        time_t localNow,
        const AlarmTime& time
    ) const;


    // ========================================================
    // ALARM SCHEDULING
    // ========================================================

    int32_t earliestOffset(
        const Alarm& alarm
    ) const;

    uint8_t dayBit(
        time_t timestamp
    ) const;

    bool isRepeatDay(
        const Alarm& alarm,
        time_t timestamp
    ) const;

    bool shouldStart(
        uint8_t index,
        const ActiveAlarm& active,
        time_t now,
        time_t& t0
    ) const;


    // ========================================================
    // RUNTIME
    // ========================================================

    bool start(
        uint8_t index,
        const Alarm& alarm,
        time_t t0,
        int64_t elapsedMs
    );

    void updateRuntime();

    int8_t findPhase(
        const Alarm& alarm,
        int64_t elapsedMs
    ) const;

    bool phaseActive(
        const AlarmPhase& phase,
        int64_t elapsedMs
    ) const;

    bool conditionPassed(
        const AlarmPhase& phase
    ) const;


    // ========================================================
    // CALLBACK NOTIFICATIONS
    // ========================================================

    void notifyTrigger();

    void notifyPhase(
        uint8_t phaseIndex
    );

    void notifyFinish();


    // ========================================================
    // ID
    // ========================================================

    String generateId() const;


    // ========================================================
    // SD SCANNING
    // ========================================================

    void scanFile(
        const String& path
    );
};