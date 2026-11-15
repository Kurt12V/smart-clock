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

    // Effect outputs (former AlarmManager).
    using MatrixCallback  = std::function<void(const String&)>;
    using AudioCallback   = std::function<void(const String&)>;
    using CobCallback     = std::function<void(const String&)>;
    using StopCallback    = std::function<void()>;

    // Lifecycle notifications (former AlarmManager).
    using TriggerCallback = std::function<void(const Alarm&)>;
    using FinishCallback  = std::function<void(const String&)>;

    explicit AlarmManager(ClockSystem& clock);

    // ---- LIFECYCLE ----

    bool begin();
    void update();
    bool isInitialized() const;

    // ---- CALLBACKS ----

    void setMatrixCallback(MatrixCallback callback);
    void setAudioCallback(AudioCallback callback);
    void setCobCallback(CobCallback callback);
    void setStopCallback(StopCallback callback);
    void setTriggerCallback(TriggerCallback callback);
    void setFinishCallback(FinishCallback callback);

    // ---- CRUD ----

    bool create(Alarm& alarm);
    bool update(const Alarm& alarm);
    bool remove(const String& id);
    bool get(const String& id, Alarm& alarm);
    bool exists(const String& id);
    bool loadAll(Alarm* alarms, uint8_t maxCount, uint8_t& count);

    bool enable(const String& id);
    bool disable(const String& id);
    bool setEnabled(const String& id, bool enabled);

    // ---- ACTIVE INFO ----

    uint8_t activeCount() const;
    bool getActiveInfo(
        uint8_t index,
        String& id,
        AlarmTime& time,
        uint8_t& repeatMask
    ) const;

    // ---- RUNTIME STATE ----

    bool isRunning() const;
    bool isActive() const;
    const Alarm* currentAlarm() const;
    const String& alarmId() const;
    uint32_t elapsedMs() const;

    // ---- RUNTIME CONTROL ----

    bool dismiss();
    bool snooze(uint32_t durationMs);
    void finish();
    void stop();

private:
    struct ActiveAlarm
    {
        String id;
        Alarm alarm;
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

    // ---- CALLBACKS ----

    MatrixCallback  _matrixCallback;
    AudioCallback   _audioCallback;
    CobCallback     _cobCallback;
    StopCallback    _stopCallback;
    TriggerCallback _triggerCallback;
    FinishCallback  _finishCallback;

    // ---- STATE ----

    ClockSystem& _clock;

    ActiveAlarm _activeAlarms[MAX_ALARMS];
    time_t _lastTriggerT0[MAX_ALARMS]{};
    uint8_t _activeCount = 0;

    Runtime _runtime;
    SnoozeState _snooze;

    bool _initialized = false;
    bool _filesystemReady = false;

    // ---- STORAGE ----

    bool createDirectory();
    String pathFor(const String& id) const;
    uint8_t countStoredAlarms() const;
    bool saveToStorage(const Alarm& alarm);
    bool loadFromStorage(const String& id, Alarm& alarm);
    bool deleteFromStorage(const String& id);
    bool reload();

    // ---- JSON ----

    bool serialize(const Alarm& alarm, JsonDocument& document) const;
    bool deserialize(JsonDocument& document, Alarm& alarm) const;
    bool validate(const Alarm& alarm) const;

    // ---- ACTIVE CACHE ----

    int8_t findActive(const String& id);
    bool addActive(const Alarm& alarm);
    bool removeActive(const String& id);
    void clearActive();

    // ---- SCHEDULER ----

    time_t currentLocalTimestamp() const;
    time_t makeLocalT0(time_t date, const AlarmTime& time) const;
    time_t localMidnight(time_t timestamp) const;
    time_t shiftLocalDays(time_t timestamp, int32_t days) const;

    uint8_t dayBit(time_t timestamp) const;
    bool isRepeatDay(const Alarm& alarm, time_t timestamp) const;

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

    // ---- RUNTIME HOOKS ----

    void onTriggered(const Alarm& alarm);
    void onFinished(const String& id);

    void executeAlarm(const Alarm& alarm);
    void stopOutputs();

    // ---- LOGGING ----

    void logAlarm(const char* event, const Alarm& alarm) const;
    void logSchedule(
        const char* event,
        const String& id,
        time_t timestamp
    ) const;

    String generateId() const;
};