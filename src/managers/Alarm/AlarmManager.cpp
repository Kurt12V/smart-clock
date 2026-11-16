
#include "AlarmManager.h"

#include <Arduino.h>
#include <esp_system.h>

#include <cstring>
#include <limits>
#include <utility>

#include "./core/ClockSystem.h"

namespace
{
    constexpr uint8_t WEEK_MASK = 0x7F;

    constexpr uint32_t MAX_SNOOZE_MS = 0x7FFFFFFFUL;
    constexpr uint32_t MISSED_TRIGGER_GRACE_MS = 60UL * 1000UL;

    constexpr int64_t MS_PER_SECOND = 1000LL;
    constexpr int64_t SECONDS_PER_MINUTE = 60LL;
    constexpr int64_t SECONDS_PER_HOUR = 3600LL;
    constexpr int64_t SECONDS_PER_DAY = 86400LL;

    constexpr uint16_t CURRENT_SCHEMA_VERSION = 1;

    void resetAlarm(Alarm& alarm)
    {
        alarm = Alarm{};
    }

    void localTm(time_t timestamp, struct tm& value)
    {
        memset(&value, 0, sizeof(value));
        gmtime_r(&timestamp, &value);
    }

    bool isUuidV4(const String& id)
    {
        if (id.length() != 36)
            return false;

        for (uint8_t i = 0; i < 36; ++i)
        {
            if (i == 8 || i == 13 || i == 18 || i == 23)
            {
                if (id[i] != '-')
                    return false;

                continue;
            }

            const char c = id[i];

            const bool hex =
                (c >= '0' && c <= '9') ||
                (c >= 'a' && c <= 'f') ||
                (c >= 'A' && c <= 'F');

            if (!hex)
                return false;
        }

        if (id[14] != '4')
            return false;

        const char variant = id[19];

        return variant == '8' ||
               variant == '9' ||
               variant == 'a' ||
               variant == 'A' ||
               variant == 'b' ||
               variant == 'B';
    }

    bool snoozeExpired(uint32_t now, uint32_t deadline)
    {
        return static_cast<int32_t>(now - deadline) >= 0;
    }
}

// ============================================================
// CONSTRUCTOR
// ============================================================

AlarmManager::AlarmManager(ClockSystem& clock)
    : _clock(clock)
{
}

// ============================================================
// BEGIN
// ============================================================

bool AlarmManager::begin()
{
    Serial0.println();
    Serial0.println("========== ALARM MANAGER BEGIN ==========");

    _initialized = false;

    clearActive();

    for (uint8_t i = 0; i < MAX_ALARMS; ++i)
        resetAlarm(_storedAlarms[i]);

    _storedCount = 0;

    _runtime = Runtime{};
    _snooze = SnoozeState{};

    if (!reload())
    {
        Serial0.println("[ALARM] ERROR: RAM initialization failed");
        return false;
    }

    _initialized = true;

    Serial0.printf(
        "[ALARM] READY stored=%u active=%u limit=%u\n",
        static_cast<unsigned>(_storedCount),
        static_cast<unsigned>(_activeCount),
        static_cast<unsigned>(MAX_ALARMS)
    );

    Serial0.println("=========================================");

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void AlarmManager::update()
{
    if (!_initialized)
        return;

    // An active alarm owns the runtime until dismissed,
    // snoozed or finished.
    if (_runtime.active)
    {
        updateRuntime();
        return;
    }

    // Snooze has priority over the regular schedule.
    if (_snooze.active)
    {
        const uint32_t nowMs = millis();

        if (!snoozeExpired(nowMs, _snooze.untilMs))
            return;

        const String id = _snooze.alarmId;

        _snooze = SnoozeState{};

        const int8_t index = findActive(id);

        if (index < 0)
            return;

        const Alarm alarm = _activeAlarms[index].alarm;

        if (alarm.enabled)
        {
            start(
                static_cast<uint8_t>(index),
                alarm,
                currentLocalTimestamp()
            );
        }

        return;
    }

    const time_t now = currentLocalTimestamp();

    for (uint8_t i = 0; i < _activeCount; ++i)
    {
        const ActiveAlarm& active = _activeAlarms[i];

        if (!active.enabled || active.id.isEmpty())
            continue;

        time_t triggerT0 = 0;

        if (shouldStart(i, active, now, triggerT0))
        {
            if (start(i, active.alarm, triggerT0))
                break;
        }
    }
}

bool AlarmManager::isInitialized() const
{
    return _initialized;
}

// ============================================================
// CALLBACK REGISTRATION
// ============================================================

void AlarmManager::setMatrixCallback(MatrixCallback callback)
{
    _matrixCallback = std::move(callback);
}

void AlarmManager::setAudioCallback(AudioCallback callback)
{
    _audioCallback = std::move(callback);
}

void AlarmManager::setCobCallback(CobCallback callback)
{
    _cobCallback = std::move(callback);
}

void AlarmManager::setStopCallback(StopCallback callback)
{
    _stopCallback = std::move(callback);
}

void AlarmManager::setTriggerCallback(TriggerCallback callback)
{
    _triggerCallback = std::move(callback);
}

void AlarmManager::setFinishCallback(FinishCallback callback)
{
    _finishCallback = std::move(callback);
}

// ============================================================
// CREATE
// ============================================================

bool AlarmManager::create(Alarm& alarm)
{
    if (!_initialized)
        return false;

    if (_storedCount >= MAX_ALARMS)
    {
        Serial0.println("[ALARM] CREATE FAILED: RAM limit reached");
        return false;
    }

    // Generate UUID if the caller did not provide one.
    if (alarm.id.isEmpty())
        alarm.id = generateId();

    if (!isUuidV4(alarm.id))
        return false;

    if (!validate(alarm))
        return false;

    if (findStored(alarm.id) >= 0)
        return false;

    if (!saveToMemory(alarm))
        return false;

    if (alarm.enabled && !addActive(alarm))
    {
        deleteFromMemory(alarm.id);
        return false;
    }

    logAlarm("CREATE", alarm);

    return true;
}

// ============================================================
// UPDATE
// ============================================================

bool AlarmManager::update(const Alarm& alarm)
{
    if (!_initialized || !validate(alarm))
        return false;

    const int8_t storedIndex = findStored(alarm.id);

    if (storedIndex < 0)
        return false;

    const int8_t activeIndex = findActive(alarm.id);

    // Stop the currently running alarm if its configuration
    // is being changed.
    if (_runtime.active && _runtime.alarmId == alarm.id)
    {
        onFinished(_runtime.alarmId);
        _runtime = Runtime{};
    }

    if (_snooze.active && _snooze.alarmId == alarm.id)
        _snooze = SnoozeState{};

    if (!saveToMemory(alarm))
        return false;

    if (alarm.enabled)
    {
        if (activeIndex >= 0)
        {
            ActiveAlarm& active = _activeAlarms[activeIndex];

            active.id = alarm.id;
            active.alarm = alarm;
            active.time = alarm.time;
            active.repeatMask = alarm.repeatMask;
            active.enabled = true;
        }
        else
        {
            if (!addActive(alarm))
            {
                // Restore the previous record.
                // The stored slot is still available.
                return false;
            }
        }
    }
    else
    {
        removeActive(alarm.id);
    }

    logAlarm("UPDATE", alarm);

    return true;
}

// ============================================================
// REMOVE
// ============================================================

bool AlarmManager::remove(const String& id)
{
    if (!_initialized || !isUuidV4(id))
        return false;

    if (findStored(id) < 0)
        return false;

    if (_runtime.active && _runtime.alarmId == id)
    {
        onFinished(_runtime.alarmId);
        _runtime = Runtime{};
    }

    if (_snooze.active && _snooze.alarmId == id)
        _snooze = SnoozeState{};

    removeActive(id);

    if (!deleteFromMemory(id))
        return false;

    Serial0.printf(
        "[ALARM] REMOVE id=%s stored=%u\n",
        id.c_str(),
        static_cast<unsigned>(_storedCount)
    );

    return true;
}

// ============================================================
// GET / EXISTS
// ============================================================

bool AlarmManager::get(const String& id, Alarm& alarm)
{
    if (!_initialized || !isUuidV4(id))
        return false;

    return loadFromMemory(id, alarm);
}

bool AlarmManager::exists(const String& id)
{
    if (!isUuidV4(id))
        return false;

    return findStored(id) >= 0;
}

// ============================================================
// LOAD ALL
// ============================================================

bool AlarmManager::loadAll(
    Alarm* alarms,
    uint8_t maxCount,
    uint8_t& count
)
{
    count = 0;

    if (!_initialized)
        return false;

    if (!alarms && maxCount > 0)
        return false;

    const uint8_t amount =
        (_storedCount < maxCount) ? _storedCount : maxCount;

    for (uint8_t i = 0; i < amount; ++i)
        alarms[i] = _storedAlarms[i];

    count = amount;

    return true;
}

// ============================================================
// ENABLE / DISABLE
// ============================================================

bool AlarmManager::enable(const String& id)
{
    return setEnabled(id, true);
}

bool AlarmManager::disable(const String& id)
{
    return setEnabled(id, false);
}

bool AlarmManager::setEnabled(const String& id, bool enabled)
{
    if (!_initialized || !isUuidV4(id))
        return false;

    const int8_t storedIndex = findStored(id);

    if (storedIndex < 0)
        return false;

    Alarm alarm = _storedAlarms[storedIndex];

    if (alarm.enabled == enabled)
    {
        if (enabled)
        {
            if (findActive(id) < 0)
                return addActive(alarm);

            return true;
        }

        if (_runtime.active && _runtime.alarmId == id)
        {
            onFinished(_runtime.alarmId);
            _runtime = Runtime{};
        }

        if (_snooze.active && _snooze.alarmId == id)
            _snooze = SnoozeState{};

        removeActive(id);

        return true;
    }

    alarm.enabled = enabled;

    if (enabled)
    {
        if (!addActive(alarm))
            return false;

        _storedAlarms[storedIndex] = alarm;

        Serial0.printf("[ALARM] ENABLE id=%s\n", id.c_str());

        return true;
    }

    if (_runtime.active && _runtime.alarmId == id)
    {
        onFinished(_runtime.alarmId);
        _runtime = Runtime{};
    }

    if (_snooze.active && _snooze.alarmId == id)
        _snooze = SnoozeState{};

    removeActive(id);

    _storedAlarms[storedIndex] = alarm;

    Serial0.printf("[ALARM] DISABLE id=%s\n", id.c_str());

    return true;
}

// ============================================================
// ACTIVE INFO
// ============================================================

uint8_t AlarmManager::activeCount() const
{
    return _activeCount;
}

bool AlarmManager::getActiveInfo(
    uint8_t index,
    String& id,
    AlarmTime& time,
    uint8_t& repeatMask
) const
{
    if (index >= _activeCount)
        return false;

    const ActiveAlarm& active = _activeAlarms[index];

    id = active.id;
    time = active.time;
    repeatMask = active.repeatMask;

    return true;
}

// ============================================================
// RUNTIME STATE
// ============================================================

bool AlarmManager::isRunning() const
{
    return _runtime.active;
}

bool AlarmManager::isActive() const
{
    return _runtime.active;
}

const Alarm* AlarmManager::currentAlarm() const
{
    return _runtime.active ? &_runtime.alarm : nullptr;
}

const String& AlarmManager::alarmId() const
{
    return _runtime.alarmId;
}

uint32_t AlarmManager::elapsedMs() const
{
    if (!_runtime.active)
        return 0;

    return static_cast<uint32_t>(millis() - _runtime.startMs);
}

// ============================================================
// RUNTIME CONTROL
// ============================================================

bool AlarmManager::dismiss()
{
    if (!_initialized || !_runtime.active)
        return false;

    Serial0.printf(
        "[ALARM] DISMISS id=%s\n",
        _runtime.alarmId.c_str()
    );

    _runtime.dismissed = true;

    finish();

    return true;
}

bool AlarmManager::snooze(uint32_t durationMs)
{
    if (!_initialized || !_runtime.active)
        return false;

    if (durationMs == 0 || durationMs > MAX_SNOOZE_MS)
        return false;

    const uint32_t now = millis();

    _runtime.snoozed = true;

    _snooze.active = true;
    _snooze.alarmId = _runtime.alarmId;
    _snooze.untilMs = now + durationMs;

    Serial0.printf(
        "[ALARM] SNOOZE id=%s duration=%lu\n",
        _runtime.alarmId.c_str(),
        static_cast<unsigned long>(durationMs)
    );

    finish();

    return true;
}

void AlarmManager::finish()
{
    if (!_runtime.active)
        return;

    const String id = _runtime.alarmId;

    const bool oneShot = (_runtime.alarm.repeatMask == 0);
    const bool snoozed = _runtime.snoozed;

    // A one-shot alarm is disabled only after a normal finish.
    // Snoozing must not disable it.
    if (oneShot && !snoozed)
    {
        const int8_t index = findStored(id);

        if (index >= 0)
        {
            Alarm& stored = _storedAlarms[index];
            stored.enabled = false;

            removeActive(id);

            Serial0.printf(
                "[ALARM] ONE-SHOT DISABLED id=%s\n",
                id.c_str()
            );
        }
    }

    onFinished(id);

    _runtime = Runtime{};
}

void AlarmManager::stop()
{
    if (!_initialized)
        return;

    if (_runtime.active)
    {
        finish();
        return;
    }

    if (_snooze.active)
        _snooze = SnoozeState{};

    stopOutputs();
}

// ============================================================
// RUNTIME HOOKS
// ============================================================

void AlarmManager::onTriggered(const Alarm& alarm)
{
    Serial0.printf(
        "[ALARM][MANAGER] TRIGGER id=%s matrix=%s cob=%s audio=%s\n",
        alarm.id.c_str(),
        alarm.matrixEffect.c_str(),
        alarm.cobEffect.c_str(),
        alarm.audioEffect.c_str()
    );

    executeAlarm(alarm);

    if (_triggerCallback)
        _triggerCallback(alarm);
}

void AlarmManager::onFinished(const String& id)
{
    Serial0.printf(
        "[ALARM][MANAGER] FINISH id=%s\n",
        id.c_str()
    );

    stopOutputs();

    if (_finishCallback)
        _finishCallback(id);
}

void AlarmManager::executeAlarm(const Alarm& alarm)
{
    if (!alarm.matrixEffect.isEmpty() && _matrixCallback)
        _matrixCallback(alarm.matrixEffect);

    if (!alarm.cobEffect.isEmpty() && _cobCallback)
        _cobCallback(alarm.cobEffect);

    if (!alarm.audioEffect.isEmpty() && _audioCallback)
        _audioCallback(alarm.audioEffect);
}

void AlarmManager::stopOutputs()
{
    Serial0.println("[ALARM][MANAGER] STOP OUTPUTS");

    if (_stopCallback)
        _stopCallback();
}

// ============================================================
// RAM STORAGE
// ============================================================

int8_t AlarmManager::findStored(const String& id)
{
    for (uint8_t i = 0; i < _storedCount; ++i)
    {
        if (_storedAlarms[i].id == id)
            return static_cast<int8_t>(i);
    }

    return -1;
}

bool AlarmManager::saveToMemory(const Alarm& alarm)
{
    if (!validate(alarm))
        return false;

    const int8_t existing = findStored(alarm.id);

    if (existing >= 0)
    {
        _storedAlarms[existing] = alarm;
        return true;
    }

    if (_storedCount >= MAX_ALARMS)
        return false;

    _storedAlarms[_storedCount] = alarm;
    ++_storedCount;

    return true;
}

bool AlarmManager::loadFromMemory(
    const String& id,
    Alarm& alarm
)
{
    resetAlarm(alarm);

    const int8_t index = findStored(id);

    if (index < 0)
        return false;

    alarm = _storedAlarms[index];

    return true;
}

bool AlarmManager::deleteFromMemory(const String& id)
{
    const int8_t found = findStored(id);

    if (found < 0)
        return false;

    const uint8_t index = static_cast<uint8_t>(found);

    for (uint8_t i = index; i + 1 < _storedCount; ++i)
        _storedAlarms[i] = _storedAlarms[i + 1];

    resetAlarm(_storedAlarms[_storedCount - 1]);

    --_storedCount;

    return true;
}

uint8_t AlarmManager::countStoredAlarms() const
{
    return _storedCount;
}

bool AlarmManager::reload()
{
    clearActive();

    // RAM-only initialization.
    // No filesystem access and no persistent storage.
    for (uint8_t i = 0; i < _storedCount; ++i)
    {
        const Alarm& alarm = _storedAlarms[i];

        if (!validate(alarm))
            return false;

        if (alarm.enabled && !addActive(alarm))
        {
            clearActive();
            return false;
        }
    }

    return true;
}

// ============================================================
// JSON SERIALIZE / DESERIALIZE
// Kept for compatibility with the header.
// Not used for persistent storage.
// ============================================================

bool AlarmManager::serialize(
    const Alarm& alarm,
    JsonDocument& document
) const
{
    if (!validate(alarm))
        return false;

    document.clear();

    document["schemaVersion"] = alarm.schemaVersion;
    document["id"] = alarm.id;
    document["name"] = alarm.name;
    document["enabled"] = alarm.enabled;

    JsonObject time = document["time"].to<JsonObject>();

    time["hour"] = alarm.time.hour;
    time["minute"] = alarm.time.minute;
    time["second"] = alarm.time.second;

    document["repeatMask"] = alarm.repeatMask;

    document["matrixEffect"] = alarm.matrixEffect;
    document["cobEffect"] = alarm.cobEffect;
    document["audioEffect"] = alarm.audioEffect;

    return true;
}

bool AlarmManager::deserialize(
    JsonDocument& document,
    Alarm& alarm
) const
{
    resetAlarm(alarm);

    JsonVariantConst id = document["id"];

    if (!id.is<const char*>())
        return false;

    alarm.id = id.as<String>();

    alarm.schemaVersion =
        document["schemaVersion"] | CURRENT_SCHEMA_VERSION;

    alarm.name = document["name"] | String();
    alarm.enabled = document["enabled"] | false;

    alarm.repeatMask = document["repeatMask"] | 0;

    JsonObjectConst time = document["time"].as<JsonObjectConst>();

    if (time.isNull())
        return false;

    alarm.time.hour = time["hour"] | 0;
    alarm.time.minute = time["minute"] | 0;
    alarm.time.second = time["second"] | 0;

    alarm.matrixEffect = document["matrixEffect"] | String();
    alarm.cobEffect = document["cobEffect"] | String();
    alarm.audioEffect = document["audioEffect"] | String();

    return validate(alarm);
}

// ============================================================
// VALIDATE
// ============================================================

bool AlarmManager::validate(const Alarm& alarm) const
{
    if (!isUuidV4(alarm.id))
        return false;

    if (alarm.name.length() > 64)
        return false;

    if (alarm.time.hour > 23)
        return false;

    if (alarm.time.minute > 59)
        return false;

    if (alarm.time.second > 59)
        return false;

    if (alarm.repeatMask & static_cast<uint8_t>(~WEEK_MASK))
        return false;

    if (
        alarm.matrixEffect.length() > 64 ||
        alarm.cobEffect.length() > 64 ||
        alarm.audioEffect.length() > 64
    )
    {
        return false;
    }

    return true;
}

// ============================================================
// ACTIVE CACHE
// ============================================================

int8_t AlarmManager::findActive(const String& id)
{
    for (uint8_t i = 0; i < _activeCount; ++i)
    {
        if (_activeAlarms[i].id == id)
            return static_cast<int8_t>(i);
    }

    return -1;
}

bool AlarmManager::addActive(const Alarm& alarm)
{
    if (!alarm.enabled || !isUuidV4(alarm.id))
        return false;

    const int8_t existing = findActive(alarm.id);

    if (existing >= 0)
    {
        ActiveAlarm& active = _activeAlarms[existing];

        active.id = alarm.id;
        active.alarm = alarm;
        active.time = alarm.time;
        active.repeatMask = alarm.repeatMask;
        active.enabled = true;

        return true;
    }

    if (_activeCount >= MAX_ALARMS)
        return false;

    ActiveAlarm& active = _activeAlarms[_activeCount];

    active.id = alarm.id;
    active.alarm = alarm;
    active.time = alarm.time;
    active.repeatMask = alarm.repeatMask;
    active.enabled = true;

    _lastTriggerT0[_activeCount] = 0;

    ++_activeCount;

    return true;
}

bool AlarmManager::removeActive(const String& id)
{
    const int8_t found = findActive(id);

    if (found < 0)
        return false;

    const uint8_t index = static_cast<uint8_t>(found);

    for (uint8_t i = index; i + 1 < _activeCount; ++i)
    {
        _activeAlarms[i] = _activeAlarms[i + 1];
        _lastTriggerT0[i] = _lastTriggerT0[i + 1];
    }

    _activeAlarms[_activeCount - 1] = ActiveAlarm{};
    _lastTriggerT0[_activeCount - 1] = 0;

    --_activeCount;

    return true;
}

void AlarmManager::clearActive()
{
    for (uint8_t i = 0; i < MAX_ALARMS; ++i)
    {
        _activeAlarms[i] = ActiveAlarm{};
        _lastTriggerT0[i] = 0;
    }

    _activeCount = 0;
}

// ============================================================
// TIME
// ============================================================

time_t AlarmManager::currentLocalTimestamp() const
{
    return _clock.getLocalTime();
}

time_t AlarmManager::localMidnight(time_t timestamp) const
{
    const int64_t seconds = static_cast<int64_t>(timestamp);

    int64_t days = seconds / SECONDS_PER_DAY;

    if (
        seconds < 0 &&
        seconds % SECONDS_PER_DAY != 0
    )
    {
        --days;
    }

    return static_cast<time_t>(days * SECONDS_PER_DAY);
}

time_t AlarmManager::shiftLocalDays(
    time_t timestamp,
    int32_t days
) const
{
    return static_cast<time_t>(
        static_cast<int64_t>(timestamp) +
        static_cast<int64_t>(days) * SECONDS_PER_DAY
    );
}

time_t AlarmManager::makeLocalT0(
    time_t date,
    const AlarmTime& time
) const
{
    return static_cast<time_t>(
        static_cast<int64_t>(localMidnight(date)) +
        static_cast<int64_t>(time.hour) * SECONDS_PER_HOUR +
        static_cast<int64_t>(time.minute) * SECONDS_PER_MINUTE +
        static_cast<int64_t>(time.second)
    );
}

uint8_t AlarmManager::dayBit(time_t timestamp) const
{
    struct tm value{};

    localTm(timestamp, value);

    // Monday = bit 0, Sunday = bit 6.
    if (value.tm_wday == 0)
        return 6;

    return static_cast<uint8_t>(value.tm_wday - 1);
}

bool AlarmManager::isRepeatDay(
    const Alarm& alarm,
    time_t timestamp
) const
{
    if (alarm.repeatMask == 0)
        return true;

    return (
        alarm.repeatMask &
        static_cast<uint8_t>(1U << dayBit(timestamp))
    ) != 0;
}

// ============================================================
// SCHEDULER
// ============================================================

bool AlarmManager::shouldStart(
    uint8_t index,
    const ActiveAlarm& active,
    time_t now,
    time_t& triggerT0
) const
{
    if (
        index >= MAX_ALARMS ||
        !active.enabled ||
        active.id.isEmpty()
    )
    {
        return false;
    }

    const time_t today = localMidnight(now);

    const time_t candidates[3] =
    {
        shiftLocalDays(today, -1),
        today,
        shiftLocalDays(today, 1)
    };

    const int64_t nowMs =
        static_cast<int64_t>(now) * MS_PER_SECOND;

    bool found = false;

    time_t bestT0 = 0;

    int64_t bestAge = std::numeric_limits<int64_t>::max();

    for (uint8_t i = 0; i < 3; ++i)
    {
        const time_t date = candidates[i];

        if (!isRepeatDay(active.alarm, date))
            continue;

        const time_t t0 = makeLocalT0(date, active.time);

        const int64_t ageMs =
            nowMs - static_cast<int64_t>(t0) * MS_PER_SECOND;

        if (ageMs < 0)
            continue;

        if (_lastTriggerT0[index] == t0)
            continue;

        if (
            static_cast<uint64_t>(ageMs) >
            MISSED_TRIGGER_GRACE_MS
        )
        {
            continue;
        }

        if (!found || ageMs < bestAge)
        {
            found = true;
            bestAge = ageMs;
            bestT0 = t0;
        }
    }

    if (!found)
        return false;

    triggerT0 = bestT0;

    logSchedule("MATCH", active.id, triggerT0);

    return true;
}

bool AlarmManager::start(
    uint8_t index,
    const Alarm& alarm,
    time_t triggerT0
)
{
    if (
        index >= _activeCount ||
        !alarm.enabled ||
        _runtime.active
    )
    {
        return false;
    }

    _runtime = Runtime{};

    _runtime.active = true;
    _runtime.alarmId = alarm.id;
    _runtime.alarm = alarm;
    _runtime.triggerT0 = triggerT0;
    _runtime.startMs = millis();

    _lastTriggerT0[index] = triggerT0;

    logAlarm("START", alarm);
    logSchedule("START", alarm.id, triggerT0);

    onTriggered(alarm);

    return true;
}

void AlarmManager::updateRuntime()
{
    // The alarm remains active until dismiss(), snooze()
    // or finish() is called.
}

// ============================================================
// LOGGING
// ============================================================

void AlarmManager::logSchedule(
    const char* event,
    const String& id,
    time_t timestamp
) const
{
    struct tm value{};

    localTm(timestamp, value);

    Serial0.printf(
        "[ALARM] %s id=%s local=%04d-%02d-%02d %02d:%02d:%02d "
        "ts=%lld\n",
        event,
        id.c_str(),
        value.tm_year + 1900,
        value.tm_mon + 1,
        value.tm_mday,
        value.tm_hour,
        value.tm_min,
        value.tm_sec,
        static_cast<long long>(timestamp)
    );
}

void AlarmManager::logAlarm(
    const char* event,
    const Alarm& alarm
) const
{
    Serial0.printf(
        "[ALARM] %s id=%s name=%s enabled=%s "
        "time=%02u:%02u:%02u repeat=0x%02X "
        "matrix=%s cob=%s audio=%s\n",
        event,
        alarm.id.c_str(),
        alarm.name.c_str(),
        alarm.enabled ? "true" : "false",
        alarm.time.hour,
        alarm.time.minute,
        alarm.time.second,
        alarm.repeatMask,
        alarm.matrixEffect.c_str(),
        alarm.cobEffect.c_str(),
        alarm.audioEffect.c_str()
    );
}

// ============================================================
// UUID GENERATION
// ============================================================

String AlarmManager::generateId() const
{
    uint8_t bytes[16];

    for (uint8_t i = 0; i < sizeof(bytes); i += 4)
    {
        const uint32_t value = esp_random();

        bytes[i]     = static_cast<uint8_t>(value);
        bytes[i + 1] = static_cast<uint8_t>(value >> 8);
        bytes[i + 2] = static_cast<uint8_t>(value >> 16);
        bytes[i + 3] = static_cast<uint8_t>(value >> 24);
    }

    bytes[6] = static_cast<uint8_t>((bytes[6] & 0x0F) | 0x40);
    bytes[8] = static_cast<uint8_t>((bytes[8] & 0x3F) | 0x80);

    char buffer[37];

    snprintf(
        buffer,
        sizeof(buffer),
        "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        bytes[0], bytes[1], bytes[2], bytes[3],
        bytes[4], bytes[5],
        bytes[6], bytes[7],
        bytes[8], bytes[9],
        bytes[10], bytes[11],
        bytes[12], bytes[13], bytes[14], bytes[15]
    );

    return String(buffer);
}
