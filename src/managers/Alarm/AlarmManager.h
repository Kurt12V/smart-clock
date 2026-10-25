#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <FS.h>
#include <time.h>
#include <functional>

#include "models/Alarm.h"

class ClockSystem;

class AlarmManager
{
public:
    static constexpr uint8_t MAX_ALARMS = 3;

    using TriggerCallback =
        std::function<void(const Alarm&)>;

    using FinishCallback =
        std::function<void(const String&)>;

    explicit AlarmManager(ClockSystem& clock);

    // Lifecycle
    bool begin();
    void update();
    bool isInitialized() const;

    // Callbacks
    void setTriggerCallback(TriggerCallback callback);
    void setFinishCallback(FinishCallback callback);

    // CRUD
    bool create(Alarm& alarm);
    bool update(const Alarm& alarm);
    bool remove(const String& id);
    bool get(const String& id, Alarm& alarm);
    bool exists(const String& id);
    bool loadAll(Alarm* alarms, uint8_t maxCount, uint8_t& count);

    // Enable / disable
    bool enable(const String& id);
    bool disable(const String& id);
    bool setEnabled(const String& id, bool enabled);

    // Active alarms
    uint8_t activeCount() const;

    bool getActiveInfo(
        uint8_t index,
        String& id,
        AlarmTime& time,
        uint8_t& repeatMask
    ) const;

    int8_t findActive(const String& id);
    bool addActive(const Alarm& alarm);
    bool removeActive(const String& id);
    void clearActive();

    // LittleFS
    bool createDirectory();
    String pathFor(const String& id) const;

    bool saveToStorage(const Alarm& alarm);
    bool loadFromStorage(const String& id, Alarm& alarm);
    bool deleteFromStorage(const String& id);
    bool reload();

    // JSON
    bool serialize(const Alarm& alarm, JsonDocument& document) const;
    bool deserialize(JsonDocument& document, Alarm& alarm) const;

    // Validation
    bool validate(const Alarm& alarm) const;

    // Runtime
    bool isRunning() const;
    const Alarm* currentAlarm() const;
    uint32_t elapsedMs() const;

    // Actions
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
    };

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

    struct SnoozeState
    {
        bool active = false;
        String alarmId;
        uint32_t untilMs = 0;
    };

    TriggerCallback _triggerCallback;
    FinishCallback _finishCallback;

    ClockSystem& _clock;

    ActiveAlarm _activeAlarms[MAX_ALARMS];
    time_t _lastTriggerT0[MAX_ALARMS]{};

    uint8_t _activeCount = 0;

    Runtime _runtime;
    SnoozeState _snooze;

    bool _initialized = false;
    bool _filesystemReady = false;

    // Counts every saved alarm, including disabled alarms.
    uint8_t countStoredAlarms() const;

    // Time
    time_t currentLocalTimestamp() const;
    time_t makeLocalT0(time_t date, const AlarmTime& time) const;
    time_t localMidnight(time_t timestamp) const;
    time_t shiftLocalDays(time_t timestamp, int32_t days) const;

    // Weekday
    uint8_t dayBit(time_t timestamp) const;
    bool isRepeatDay(const Alarm& alarm, time_t timestamp) const;

    // Scheduler
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

    // Notifications
    void notifyTrigger();
    void notifyFinish();

    // Logging
    void logAlarm(const char* event, const Alarm& alarm) const;

    void logSchedule(
        const char* event,
        const String& id,
        time_t timestamp
    ) const;

    // UUID
    String generateId() const;
};