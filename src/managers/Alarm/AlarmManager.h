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

    void setFinishCallback(
        FinishCallback callback
    );

    // ========================================================
    // CRUD
    // ========================================================

    // Если alarm.id пустой:
    // AlarmManager автоматически создаст UUID v4
    // и запишет его обратно в alarm.id.
    bool create(Alarm& alarm);

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

    bool loadAll(
        Alarm* alarms,
        uint8_t maxCount,
        uint8_t& count
    );

    // ========================================================
    // ENABLE
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

    uint32_t elapsedMs() const;

    // ========================================================
    // ACTIONS
    // ========================================================

    bool dismiss();

    bool snooze(
        uint32_t durationMs
    );

    void finish();

private:

    // ========================================================
    // ACTIVE ALARM
    // ========================================================

    struct ActiveAlarm
    {
        String id;

        AlarmTime time;

        uint8_t repeatMask = 0;

        bool enabled = false;
    };

    // ========================================================
    // RUNTIME
    // ========================================================

    struct Runtime
    {
        bool active = false;

        String alarmId;

        Alarm alarm;

        time_t triggerT0 = 0;

        uint32_t startMs = 0;

        bool dismissed = false;

        bool snoozed = false;
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

    FinishCallback _finishCallback;

    // ========================================================
    // REFERENCES
    // ========================================================

    SDManager& _sd;

    ClockSystem& _clock;

    // ========================================================
    // ACTIVE ALARMS
    // ========================================================

    ActiveAlarm
        _activeAlarms[AlarmConfig::MAX_ALARMS];

    uint8_t _activeCount = 0;

    // Последний T0, который был обработан
    // для каждого активного будильника.
    time_t
        _lastTriggerT0[AlarmConfig::MAX_ALARMS];

    // ========================================================
    // RUNTIME
    // ========================================================

    Runtime _runtime;

    SnoozeState _snooze;

    bool _initialized = false;

    // ========================================================
    // TIME
    // ========================================================

    time_t currentLocalTimestamp() const;

    time_t makeLocalT0(
        time_t localDateTimestamp,
        const AlarmTime& time
    ) const;

    time_t localMidnight(
        time_t timestamp
    ) const;

    time_t shiftLocalDays(
        time_t timestamp,
        int32_t days
    ) const;

    // ========================================================
    // WEEKDAY
    // ========================================================

    uint8_t dayBit(
        time_t localTimestamp
    ) const;

    bool isRepeatDay(
        const Alarm& alarm,
        time_t localTimestamp
    ) const;

    // ========================================================
    // SCHEDULER
    // ========================================================

    bool shouldStart(
        uint8_t index,
        const ActiveAlarm& active,
        time_t now,
        time_t& triggerT0
    ) const;

    bool start(
        uint8_t index,
        const Alarm& alarm,
        time_t triggerT0
    );

    void updateRuntime();

    // ========================================================
    // NOTIFY
    // ========================================================

    void notifyTrigger();

    void notifyFinish();

    // ========================================================
    // LOG
    // ========================================================

    void logAlarm(
        const char* event,
        const Alarm& alarm
    ) const;

    void logSchedule(
        const char* event,
        const String& id,
        time_t timestamp
    ) const;

    // ========================================================
    // UUID
    // ========================================================

    String generateId() const;

    // ========================================================
    // FILE SCAN
    // ========================================================

    void scanFile(
        const String& path
    );
};