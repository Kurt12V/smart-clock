#include "AlarmManager.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_system.h>

#include <cstring>
#include <limits>
#include <utility>

#include "./core/ClockSystem.h"

namespace
{
    constexpr const char* ALARM_DIRECTORY = "/alarms";

    constexpr uint16_t CURRENT_SCHEMA_VERSION = 1;
    constexpr uint8_t WEEK_MASK = 0x7F;

    constexpr uint32_t MAX_SNOOZE_MS = 0x7FFFFFFFUL;
    constexpr uint32_t MISSED_TRIGGER_GRACE_MS = 60UL * 1000UL;

    constexpr int64_t MS_PER_SECOND = 1000LL;
    constexpr int64_t SECONDS_PER_MINUTE = 60LL;
    constexpr int64_t MINUTES_PER_HOUR = 60LL;
    constexpr int64_t HOURS_PER_DAY = 24LL;

    constexpr int64_t SECONDS_PER_HOUR =
        SECONDS_PER_MINUTE * MINUTES_PER_HOUR;

    constexpr int64_t ALARM_SECONDS_PER_DAY =
        HOURS_PER_DAY * SECONDS_PER_HOUR;

    void resetAlarm(Alarm& alarm)
    {
        alarm = Alarm{};
    }

    void localTm(time_t timestamp, struct tm& value)
    {
        memset(&value, 0, sizeof(value));
        gmtime_r(&timestamp, &value);
    }

    String alarmIdFromPath(const String& path)
    {
        String id = path;

        const int slash = id.lastIndexOf('/');

        if (slash >= 0)
            id = id.substring(slash + 1);

        if (!id.endsWith(".json"))
            return String();

        id.remove(id.length() - 5);

        return id;
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
    Serial0.println("[ALARM] AlarmManager::begin");

    _initialized = false;
    _filesystemReady = false;

    clearActive();

    _runtime = Runtime{};
    _snooze = SnoozeState{};

    // Never automatically format the filesystem.
    if (!LittleFS.begin(false))
    {
        Serial0.println("[ALARM] ERROR: LittleFS mount failed");
        return false;
    }

    _filesystemReady = true;

    if (!createDirectory())
    {
        Serial0.println("[ALARM] ERROR: cannot create /alarms");
        return false;
    }

    if (!reload())
    {
        Serial0.println("[ALARM] ERROR: reload failed");
        return false;
    }

    _initialized = true;

    Serial0.printf(
        "[ALARM] READY active=%u stored=%u limit=%u\n",
        static_cast<unsigned>(_activeCount),
        static_cast<unsigned>(countStoredAlarms()),
        static_cast<unsigned>(MAX_ALARMS)
    );

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void AlarmManager::update()
{
    if (!_initialized)
        return;

    if (_runtime.active)
    {
        updateRuntime();
        return;
    }

    if (_snooze.active)
    {
        const uint32_t nowMs = millis();

        if (static_cast<int32_t>(nowMs - _snooze.untilMs) < 0)
            return;

        const String id = _snooze.alarmId;

        Serial0.printf(
            "[ALARM] SNOOZE EXPIRED id=%s\n",
            id.c_str()
        );

        _snooze = SnoozeState{};

        Alarm alarm;

        if (!loadFromStorage(id, alarm))
        {
            Serial0.printf(
                "[ALARM] SNOOZE ERROR id=%s reason=load-failed\n",
                id.c_str()
            );
            return;
        }

        if (!alarm.enabled)
            return;

        const int8_t index = findActive(id);

        if (index < 0)
            return;

        start(
            static_cast<uint8_t>(index),
            alarm,
            currentLocalTimestamp()
        );

        return;
    }

    const time_t now = currentLocalTimestamp();

    for (uint8_t i = 0; i < _activeCount; ++i)
    {
        ActiveAlarm& active = _activeAlarms[i];

        if (!active.enabled || active.id.isEmpty())
            continue;

        time_t triggerT0 = 0;

        if (!shouldStart(i, active, now, triggerT0))
            continue;

        Alarm alarm;

        if (!loadFromStorage(active.id, alarm))
        {
            Serial0.printf(
                "[ALARM] WARNING: cannot load id=%s\n",
                active.id.c_str()
            );
            continue;
        }

        if (!alarm.enabled)
            continue;

        if (start(i, alarm, triggerT0))
            break;
    }
}

// ============================================================
// STATUS
// ============================================================

bool AlarmManager::isInitialized() const
{
    return _initialized;
}

bool AlarmManager::isRunning() const
{
    return _runtime.active;
}

const Alarm* AlarmManager::currentAlarm() const
{
    return _runtime.active ? &_runtime.alarm : nullptr;
}

uint32_t AlarmManager::elapsedMs() const
{
    return _runtime.active ? millis() - _runtime.startMs : 0;
}

// ============================================================
// CALLBACKS
// ============================================================

void AlarmManager::setTriggerCallback(TriggerCallback callback)
{
    _triggerCallback = std::move(callback);
}

void AlarmManager::setFinishCallback(FinishCallback callback)
{
    _finishCallback = std::move(callback);
}

void AlarmManager::notifyTrigger()
{
    if (_triggerCallback)
        _triggerCallback(_runtime.alarm);
}

void AlarmManager::notifyFinish()
{
    if (_finishCallback)
        _finishCallback(_runtime.alarmId);
}

// ============================================================
// DIRECTORY / PATH
// ============================================================

bool AlarmManager::createDirectory()
{
    if (!_filesystemReady)
        return false;

    if (LittleFS.exists(ALARM_DIRECTORY))
        return true;

    return LittleFS.mkdir(ALARM_DIRECTORY);
}

String AlarmManager::pathFor(const String& id) const
{
    if (!isUuidV4(id))
        return String();

    return String(ALARM_DIRECTORY) + "/" + id + ".json";
}

// ============================================================
// COUNT STORED ALARMS
// Includes enabled and disabled alarms.
// ============================================================

uint8_t AlarmManager::countStoredAlarms() const
{
    if (!_filesystemReady)
        return 0;

    File directory = LittleFS.open(ALARM_DIRECTORY, FILE_READ);

    if (!directory || !directory.isDirectory())
    {
        if (directory)
            directory.close();

        return 0;
    }

    uint8_t count = 0;

    File file = directory.openNextFile();

    while (file)
    {
        if (!file.isDirectory())
        {
            const String id = alarmIdFromPath(file.name());

            if (isUuidV4(id))
            {
                if (count < UINT8_MAX)
                    ++count;
            }
        }

        file.close();
        file = directory.openNextFile();
    }

    directory.close();

    return count;
}

// ============================================================
// EXISTS
// ============================================================

bool AlarmManager::exists(const String& id)
{
    if (!_filesystemReady || !isUuidV4(id))
        return false;

    return LittleFS.exists(pathFor(id));
}

// ============================================================
// CREATE
// ============================================================

bool AlarmManager::create(Alarm& alarm)
{
    if (!_initialized)
        return false;

    if (alarm.id.isEmpty())
    {
        do
        {
            alarm.id = generateId();
        }
        while (exists(alarm.id));
    }

    if (!validate(alarm))
    {
        Serial0.println("[ALARM] CREATE rejected: validation failed");
        return false;
    }

    if (exists(alarm.id))
    {
        Serial0.println("[ALARM] CREATE rejected: ID already exists");
        return false;
    }

    // Enforce the total limit, not just the enabled limit.
    const uint8_t stored = countStoredAlarms();

    if (stored >= MAX_ALARMS)
    {
        Serial0.printf(
            "[ALARM] CREATE rejected: total limit reached (%u/%u)\n",
            static_cast<unsigned>(stored),
            static_cast<unsigned>(MAX_ALARMS)
        );
        return false;
    }

    if (alarm.enabled && _activeCount >= MAX_ALARMS)
    {
        Serial0.println("[ALARM] CREATE rejected: active limit reached");
        return false;
    }

    if (!saveToStorage(alarm))
    {
        Serial0.println("[ALARM] CREATE failed: save");
        return false;
    }

    if (alarm.enabled && !addActive(alarm))
    {
        deleteFromStorage(alarm.id);
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

    Alarm oldAlarm;

    if (!get(alarm.id, oldAlarm))
        return false;

    if (!oldAlarm.enabled && alarm.enabled && _activeCount >= MAX_ALARMS)
        return false;

    if (_runtime.active && _runtime.alarmId == alarm.id)
        finish();

    if (!saveToStorage(alarm))
        return false;

    removeActive(alarm.id);

    if (alarm.enabled && !addActive(alarm))
        return false;

    logAlarm("UPDATE", alarm);

    return true;
}

// ============================================================
// REMOVE
// ============================================================

bool AlarmManager::remove(const String& id)
{
    if (!_initialized || id.isEmpty() || !exists(id))
        return false;

    if (_runtime.active && _runtime.alarmId == id)
        finish();

    if (_snooze.active && _snooze.alarmId == id)
        _snooze = SnoozeState{};

    if (!deleteFromStorage(id))
        return false;

    removeActive(id);

    Serial0.printf("[ALARM] REMOVE OK id=%s\n", id.c_str());

    return true;
}

// ============================================================
// GET
// ============================================================

bool AlarmManager::get(const String& id, Alarm& alarm)
{
    if (!_initialized || id.isEmpty())
        return false;

    return loadFromStorage(id, alarm);
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

    if (!_initialized || !alarms)
        return false;

    File directory = LittleFS.open(ALARM_DIRECTORY, FILE_READ);

    if (!directory || !directory.isDirectory())
    {
        if (directory)
            directory.close();

        return false;
    }

    File file = directory.openNextFile();

    while (file && count < maxCount)
    {
        if (!file.isDirectory())
        {
            const String id = alarmIdFromPath(file.name());

            file.close();

            if (isUuidV4(id))
            {
                Alarm alarm;

                if (loadFromStorage(id, alarm))
                    alarms[count++] = alarm;
            }
        }
        else
        {
            file.close();
        }

        file = directory.openNextFile();
    }

    if (file)
        file.close();

    directory.close();

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
    if (!_initialized || id.isEmpty())
        return false;

    Alarm alarm;

    if (!loadFromStorage(id, alarm))
        return false;

    if (alarm.enabled == enabled)
        return true;

    if (enabled && _activeCount >= MAX_ALARMS)
        return false;

    if (_runtime.active && _runtime.alarmId == id)
        finish();

    alarm.enabled = enabled;

    if (!saveToStorage(alarm))
        return false;

    removeActive(id);

    if (enabled && !addActive(alarm))
        return false;

    Serial0.printf(
        "[ALARM] %s id=%s\n",
        enabled ? "ENABLED" : "DISABLED",
        id.c_str()
    );

    return true;
}

// ============================================================
// ACTIVE COUNT / INFO
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

    if (findActive(alarm.id) >= 0)
        return true;

    if (_activeCount >= MAX_ALARMS)
        return false;

    ActiveAlarm& active = _activeAlarms[_activeCount];

    active.id = alarm.id;
    active.time = alarm.time;
    active.repeatMask = alarm.repeatMask;
    active.enabled = alarm.enabled;

    _lastTriggerT0[_activeCount] = 0;

    ++_activeCount;

    Serial0.printf(
        "[ALARM] ACTIVE ADD id=%s time=%02u:%02u:%02u repeat=0x%02X\n",
        active.id.c_str(),
        active.time.hour,
        active.time.minute,
        active.time.second,
        active.repeatMask
    );

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
// SAVE
// ============================================================

bool AlarmManager::saveToStorage(const Alarm& alarm)
{
    if (!_filesystemReady || !isUuidV4(alarm.id))
        return false;

    JsonDocument document;

    if (!serialize(alarm, document))
        return false;

    String content;

    if (serializeJson(document, content) == 0)
        return false;

    const String path = pathFor(alarm.id);

    File file = LittleFS.open(path, FILE_WRITE);

    if (!file)
    {
        Serial0.printf(
            "[ALARM] ERROR: cannot write %s\n",
            path.c_str()
        );
        return false;
    }

    const size_t expected = content.length();
    const size_t written = file.print(content);

    file.flush();
    file.close();

    if (written != expected)
    {
        Serial0.printf(
            "[ALARM] ERROR: incomplete write expected=%u written=%u\n",
            static_cast<unsigned>(expected),
            static_cast<unsigned>(written)
        );
        return false;
    }

    return true;
}

// ============================================================
// LOAD
// ============================================================

bool AlarmManager::loadFromStorage(const String& id, Alarm& alarm)
{
    resetAlarm(alarm);

    if (!_filesystemReady || !isUuidV4(id))
        return false;

    const String path = pathFor(id);

    File file = LittleFS.open(path, FILE_READ);

    if (!file || file.isDirectory())
    {
        if (file)
            file.close();

        return false;
    }

    if (file.size() == 0)
    {
        file.close();
        return false;
    }

    JsonDocument document;

    const DeserializationError error = deserializeJson(document, file);

    file.close();

    if (error)
    {
        Serial0.printf(
            "[ALARM] JSON ERROR id=%s error=%s\n",
            id.c_str(),
            error.c_str()
        );
        return false;
    }

    if (!deserialize(document, alarm))
        return false;

    if (alarm.id != id)
    {
        resetAlarm(alarm);
        return false;
    }

    return true;
}

// ============================================================
// DELETE
// ============================================================

bool AlarmManager::deleteFromStorage(const String& id)
{
    if (!_filesystemReady || !isUuidV4(id))
        return false;

    const String path = pathFor(id);

    if (!LittleFS.exists(path))
        return false;

    return LittleFS.remove(path);
}

// ============================================================
// RELOAD
// ============================================================

bool AlarmManager::reload()
{
    clearActive();

    if (!_filesystemReady)
        return false;

    File directory = LittleFS.open(ALARM_DIRECTORY, FILE_READ);

    if (!directory || !directory.isDirectory())
    {
        if (directory)
            directory.close();

        return false;
    }

    File file = directory.openNextFile();

    while (file)
    {
        if (!file.isDirectory())
        {
            const String id = alarmIdFromPath(file.name());

            file.close();

            if (isUuidV4(id))
            {
                Alarm alarm;

                if (loadFromStorage(id, alarm) && alarm.enabled)
                {
                    if (!addActive(alarm))
                    {
                        Serial0.println(
                            "[ALARM] reload: active limit reached"
                        );
                        break;
                    }
                }
            }
        }
        else
        {
            file.close();
        }

        file = directory.openNextFile();
    }

    if (file)
        file.close();

    directory.close();

    Serial0.printf(
        "[ALARM] reload complete active=%u stored=%u limit=%u\n",
        static_cast<unsigned>(_activeCount),
        static_cast<unsigned>(countStoredAlarms()),
        static_cast<unsigned>(MAX_ALARMS)
    );

    return true;
}

// ============================================================
// JSON SERIALIZE
// ============================================================

bool AlarmManager::serialize(
    const Alarm& alarm,
    JsonDocument& document
) const
{
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

// ============================================================
// JSON DESERIALIZE
// ============================================================

bool AlarmManager::deserialize(
    JsonDocument& document,
    Alarm& alarm
) const
{
    resetAlarm(alarm);

    JsonVariantConst id = document["id"];

    if (!id.isNull())
    {
        if (!id.is<const char*>())
            return false;

        alarm.id = id.as<String>();
    }

    alarm.schemaVersion =
        document["schemaVersion"] | CURRENT_SCHEMA_VERSION;

    alarm.name = document["name"] | String();
    alarm.enabled = document["enabled"] | false;
    alarm.repeatMask = document["repeatMask"] | 0;

    JsonObject time = document["time"].as<JsonObject>();

    if (time.isNull())
        return false;

    alarm.time.hour = time["hour"] | 0;
    alarm.time.minute = time["minute"] | 0;
    alarm.time.second = time["second"] | 0;

    alarm.matrixEffect = document["matrixEffect"] | String();
    alarm.cobEffect = document["cobEffect"] | String();
    alarm.audioEffect = document["audioEffect"] | String();

    if (
        alarm.time.hour > 23 ||
        alarm.time.minute > 59 ||
        alarm.time.second > 59
    )
    {
        return false;
    }

    if (alarm.repeatMask & static_cast<uint8_t>(~WEEK_MASK))
        return false;

    return true;
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
// DISMISS
// ============================================================

bool AlarmManager::dismiss()
{
    if (!_runtime.active)
        return false;

    Serial0.printf(
        "[ALARM] DISMISS id=%s\n",
        _runtime.alarmId.c_str()
    );

    _runtime.dismissed = true;

    finish();

    return true;
}

// ============================================================
// SNOOZE
// ============================================================

bool AlarmManager::snooze(uint32_t durationMs)
{
    if (!_runtime.active)
        return false;

    if (durationMs == 0 || durationMs > MAX_SNOOZE_MS)
        return false;

    _runtime.snoozed = true;

    _snooze.active = true;
    _snooze.alarmId = _runtime.alarmId;
    _snooze.untilMs = millis() + durationMs;

    Serial0.printf(
        "[ALARM] SNOOZE id=%s duration=%lu\n",
        _runtime.alarmId.c_str(),
        static_cast<unsigned long>(durationMs)
    );

    finish();

    return true;
}

// ============================================================
// FINISH
// ============================================================

void AlarmManager::finish()
{
    if (!_runtime.active)
        return;

    const String id = _runtime.alarmId;
    const bool oneShot = _runtime.alarm.repeatMask == 0;
    const bool snoozed = _runtime.snoozed;

    if (oneShot && !snoozed)
    {
        Alarm alarm;

        if (loadFromStorage(id, alarm))
        {
            alarm.enabled = false;

            if (saveToStorage(alarm))
            {
                Serial0.printf(
                    "[ALARM] one-shot disabled id=%s\n",
                    id.c_str()
                );
            }
        }

        removeActive(id);
    }

    notifyFinish();

    _runtime = Runtime{};
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

    int64_t days = seconds / ALARM_SECONDS_PER_DAY;

    if (seconds < 0 && seconds % ALARM_SECONDS_PER_DAY != 0)
        --days;

    return static_cast<time_t>(days * ALARM_SECONDS_PER_DAY);
}

time_t AlarmManager::shiftLocalDays(time_t timestamp, int32_t days) const
{
    return static_cast<time_t>(
        static_cast<int64_t>(timestamp) +
        static_cast<int64_t>(days) * ALARM_SECONDS_PER_DAY
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

    // Sunday=0 -> bit 6; Monday=1 -> bit 0.
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
    if (index >= MAX_ALARMS || !active.enabled || active.id.isEmpty())
        return false;

    const time_t today = localMidnight(now);

    const time_t candidates[3] =
    {
        shiftLocalDays(today, -1),
        today,
        shiftLocalDays(today, 1)
    };

    const int64_t nowMs = static_cast<int64_t>(now) * MS_PER_SECOND;

    bool found = false;
    time_t bestT0 = 0;

    int64_t bestAge = std::numeric_limits<int64_t>::max();

    for (uint8_t i = 0; i < 3; ++i)
    {
        const time_t date = candidates[i];

        if (
            active.repeatMask != 0 &&
            !(active.repeatMask & static_cast<uint8_t>(1U << dayBit(date)))
        )
        {
            continue;
        }

        const time_t t0 = makeLocalT0(date, active.time);

        const int64_t ageMs =
            nowMs - static_cast<int64_t>(t0) * MS_PER_SECOND;

        if (ageMs < 0)
            continue;

        if (_lastTriggerT0[index] == t0)
            continue;

        if (static_cast<uint64_t>(ageMs) > MISSED_TRIGGER_GRACE_MS)
            continue;

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

// ============================================================
// START
// ============================================================

bool AlarmManager::start(
    uint8_t index,
    const Alarm& alarm,
    time_t triggerT0
)
{
    if (index >= MAX_ALARMS || !alarm.enabled || _runtime.active)
        return false;

    _runtime = Runtime{};

    _runtime.active = true;
    _runtime.alarmId = alarm.id;
    _runtime.alarm = alarm;
    _runtime.triggerT0 = triggerT0;
    _runtime.startMs = millis();

    _lastTriggerT0[index] = triggerT0;

    logAlarm("START", alarm);
    logSchedule("START", alarm.id, triggerT0);

    notifyTrigger();

    return true;
}

void AlarmManager::updateRuntime()
{
    // The alarm stays active until dismiss(), snooze(), or finish().
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
        "[ALARM] %s id=%s local=%04d-%02d-%02d %02d:%02d:%02d ts=%lld\n",
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

        bytes[i] = static_cast<uint8_t>(value);
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
        bytes[10], bytes[11], bytes[12],
        bytes[13], bytes[14], bytes[15]
    );

    return String(buffer);
}