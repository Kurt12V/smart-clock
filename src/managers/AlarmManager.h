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

    bool begin();
    void update();
    bool isInitialized() const;

    void setTriggerCallback(TriggerCallback callback);
    void setPhaseCallback(PhaseCallback callback);
    void setFinishCallback(FinishCallback callback);

    bool create(const Alarm& alarm);
    bool update(const Alarm& alarm);
    bool remove(const String& id);
    bool get(const String& id, Alarm& alarm);
    bool exists(const String& id);

    bool loadAll(Alarm* alarms, uint8_t maxCount, uint8_t& count);

    bool enable(const String& id);
    bool disable(const String& id);
    bool setEnabled(const String& id, bool enabled);

    uint8_t activeCount() const;
    bool getActiveInfo(
        uint8_t index,
        String& id,
        AlarmTime& time,
        uint8_t& repeatMask
    ) const;

    int8_t findActive(const String& id) const;
    bool addActive(const Alarm& alarm);
    bool removeActive(const String& id);
    void clearActive();

    bool createDirectory();
    String pathFor(const String& id) const;
    bool saveToSD(const Alarm& alarm);
    bool loadFromSD(const String& id, Alarm& alarm);
    bool deleteFromSD(const String& id);
    bool reload();

    bool serialize(
        const Alarm& alarm,
        JsonDocument& document
    ) const;

    bool deserialize(
        JsonDocument& document,
        Alarm& alarm
    ) const;

    bool validate(const Alarm& alarm) const;

    bool isRunning() const;
    const Alarm* currentAlarm() const;
    const AlarmPhase* currentPhase() const;
    uint8_t currentPhaseIndex() const;
    uint32_t elapsedMs() const;

    bool dismiss();
    bool snooze(uint32_t durationMs);
    void finish();

private:
    struct ActiveAlarm
    {
        String id;
        AlarmTime time;
        uint8_t repeatMask = 0;
        bool enabled = false;
        int32_t earliestOffsetMs = 0;
    };

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

    struct SnoozeState
    {
        bool active = false;
        String alarmId;
        uint32_t untilMs = 0;
    };

    TriggerCallback _triggerCallback;
    PhaseCallback _phaseCallback;
    FinishCallback _finishCallback;

    SDManager& _sd;
    ClockSystem& _clock;

    ActiveAlarm _activeAlarms[AlarmConfig::MAX_ALARMS];
    uint8_t _activeCount = 0;

    time_t _lastTriggerT0[AlarmConfig::MAX_ALARMS];

    Runtime _runtime;
    SnoozeState _snooze;

    bool _initialized = false;

    time_t currentLocalTimestamp() const;

    // localNow is a "local epoch": UTC epoch + configured offset.
    // Calendar extraction therefore uses gmtime_r(), not the ESP system TZ.
    time_t makeLocalT0(
        time_t localDateTimestamp,
        const AlarmTime& time
    ) const;

    int32_t earliestOffset(const Alarm& alarm) const;

    uint8_t dayBit(time_t localTimestamp) const;
    bool isRepeatDay(
        const Alarm& alarm,
        time_t localTimestamp
    ) const;

    bool shouldStart(
        uint8_t index,
        const ActiveAlarm& active,
        time_t now,
        time_t& t0
    ) const;

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

    void logAlarm(
        const char* event,
        const Alarm& alarm
    ) const;

    void logSchedule(
        const char* event,
        const String& id,
        time_t timestamp
    ) const;

    void notifyTrigger();
    void notifyPhase(uint8_t phaseIndex);
    void notifyFinish();

    String generateId() const;
    void scanFile(const String& path);
};
