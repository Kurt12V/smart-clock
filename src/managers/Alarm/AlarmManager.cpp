
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
    constexpr char ALARM_DIRECTORY[] = "/alarms";

    constexpr uint16_t CURRENT_SCHEMA_VERSION = 1;
    constexpr uint8_t WEEK_MASK = 0x7F;

    constexpr uint32_t MAX_SNOOZE_MS = 0x7FFFFFFFUL;
    constexpr uint32_t MISSED_TRIGGER_GRACE_MS = 60UL * 1000UL;

    constexpr int64_t MS_PER_SECOND = 1000LL;
    constexpr int64_t SECONDS_PER_MINUTE = 60LL;
    constexpr int64_t SECONDS_PER_HOUR = 3600LL;
    constexpr int64_t SECONDS_PER_DAY_ = 86400LL;

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
    Serial0.println();
    Serial0.println("========== ALARM MANAGER BEGIN ==========");

    _initialized = false;
    _filesystemReady = false;

    clearActive();

    _runtime = Runtime{};
    _snooze = SnoozeState{};

    Serial0.println("[ALARM] Mounting LittleFS...");

    // Do not format the filesystem automatically.
    if (!LittleFS.begin(false))
    {
        Serial0.println("[ALARM] ERROR: LittleFS mount failed");
        return false;
    }

    _filesystemReady = true;

    Serial0.printf(
        "[ALARM] LittleFS mounted: total=%u used=%u\n",
        static_cast<unsigned>(LittleFS.totalBytes()),
        static_cast<unsigned>(LittleFS.usedBytes())
    );

    if (!createDirectory())
    {
        Serial0.println("[ALARM] ERROR: cannot prepare alarm directory");
        _filesystemReady = false;
        return false;
    }

    if (!reload())
    {
        Serial0.println("[ALARM] ERROR: alarm reload failed");
        return false;
    }

    _initialized = true;

    Serial0.printf(
        "[ALARM] READY active=%u stored=%u limit=%u\n",
        static_cast<unsigned>(_activeCount),
        static_cast<unsigned>(countStoredAlarms()),
        static_cast<unsigned>(MAX_ALARMS)
    );

    Serial0.println("==========================================");

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
        if (static_cast<int32_t>(millis() - _snooze.untilMs) < 0)
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
    return _runtime.active
        ? millis() - _runtime.startMs
        : 0;
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
// DIRECTORY
// ============================================================

bool AlarmManager::createDirectory()
{
    if (!_filesystemReady)
    {
        Serial0.println("[ALARM] DIRECTORY ERROR: filesystem not ready");
        return false;
    }

    File directory = LittleFS.open(ALARM_DIRECTORY, "r");

    if (directory)
    {
        const bool isDirectory = directory.isDirectory();
        directory.close();

        if (isDirectory)
        {
            Serial0.println("[ALARM] Directory /alarms exists");
            return true;
        }

        Serial0.println("[ALARM] ERROR: /alarms exists but is not a directory");
        return false;
    }

    Serial0.println("[ALARM] Directory does not exist, creating...");

    if (!LittleFS.mkdir(ALARM_DIRECTORY))
    {
        Serial0.println("[ALARM] ERROR: mkdir /alarms failed");
        return false;
    }

    File verification = LittleFS.open(ALARM_DIRECTORY, "r");

    if (!verification)
    {
        Serial0.println("[ALARM] ERROR: directory verification failed");
        return false;
    }

    const bool valid = verification.isDirectory();
    verification.close();

    if (!valid)
    {
        Serial0.println("[ALARM] ERROR: created path is not a directory");
        return false;
    }

    Serial0.println("[ALARM] Directory created successfully");

    return true;
}

String AlarmManager::pathFor(const String& id) const
{
    if (!isUuidV4(id))
        return String();

    return String(ALARM_DIRECTORY) + "/" + id + ".json";
}

// ============================================================
// COUNT STORED ALARMS
// ============================================================

uint8_t AlarmManager::countStoredAlarms() const
{
    if (!_filesystemReady)
        return 0;

    File directory = LittleFS.open(ALARM_DIRECTORY, "r");

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

            if (isUuidV4(id) && count < UINT8_MAX)
                ++count;
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
    Serial0.println("[ALARM] CREATE: begin");

    if (!_initialized || !_filesystemReady)
    {
        Serial0.println("[ALARM] CREATE FAILED: manager not initialized");
        return false;
    }

    if (alarm.id.isEmpty())
    {
        alarm.id = generateId();

        Serial0.printf(
            "[ALARM] CREATE: generated id=%s\n",
            alarm.id.c_str()
        );
    }

    if (!isUuidV4(alarm.id))
    {
        Serial0.println("[ALARM] CREATE FAILED: invalid UUID");
        return false;
    }

    if (!validate(alarm))
    {
        Serial0.println("[ALARM] CREATE FAILED: validation");

        Serial0.printf(
            "[ALARM] name length=%u time=%02u:%02u:%02u repeat=0x%02X\n",
            static_cast<unsigned>(alarm.name.length()),
            alarm.time.hour,
            alarm.time.minute,
            alarm.time.second,
            alarm.repeatMask
        );

        return false;
    }

    if (exists(alarm.id))
    {
        Serial0.println("[ALARM] CREATE FAILED: UUID already exists");
        return false;
    }

    const uint8_t stored = countStoredAlarms();

    Serial0.printf(
        "[ALARM] CREATE: stored=%u active=%u limit=%u\n",
        stored,
        _activeCount,
        MAX_ALARMS
    );

    if (stored >= MAX_ALARMS)
    {
        Serial0.println("[ALARM] CREATE FAILED: storage limit");
        return false;
    }

    if (!saveToStorage(alarm))
    {
        Serial0.println("[ALARM] CREATE FAILED: storage");
        return false;
    }

    if (alarm.enabled && !addActive(alarm))
    {
        Serial0.println("[ALARM] CREATE FAILED: RAM cache");

        deleteFromStorage(alarm.id);
        return false;
    }

    logAlarm("CREATE", alarm);

    Serial0.println("[ALARM] CREATE SUCCESS");

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

    if (!loadFromStorage(alarm.id, oldAlarm))
        return false;

    const int8_t oldIndex = findActive(alarm.id);
    const bool wasActive = oldIndex >= 0;

    if (alarm.enabled && !wasActive && _activeCount >= MAX_ALARMS)
        return false;

    if (!saveToStorage(alarm))
        return false;

    if (alarm.enabled)
    {
        if (wasActive)
        {
            ActiveAlarm& active = _activeAlarms[oldIndex];

            active.alarm = alarm;
            active.time = alarm.time;
            active.repeatMask = alarm.repeatMask;
            active.enabled = true;
        }
        else if (!addActive(alarm))
        {
            saveToStorage(oldAlarm);
            return false;
        }
    }
    else if (wasActive)
    {
        if (_runtime.active && _runtime.alarmId == alarm.id)
        {
            notifyFinish();
            _runtime = Runtime{};
        }

        if (_snooze.active && _snooze.alarmId == alarm.id)
            _snooze = SnoozeState{};

        removeActive(alarm.id);
    }

    if (_runtime.active && _runtime.alarmId == alarm.id)
    {
        notifyFinish();
        _runtime = Runtime{};
    }

    if (_snooze.active && _snooze.alarmId == alarm.id)
        _snooze = SnoozeState{};

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

    if (!deleteFromStorage(id))
        return false;

    if (_runtime.active && _runtime.alarmId == id)
    {
        notifyFinish();
        _runtime = Runtime{};
    }

    if (_snooze.active && _snooze.alarmId == id)
        _snooze = SnoozeState{};

    removeActive(id);

    Serial0.printf("[ALARM] REMOVE id=%s\n", id.c_str());

    return true;
}

// ============================================================
// GET
// ============================================================

bool AlarmManager::get(const String& id, Alarm& alarm)
{
    if (!_initialized || !isUuidV4(id))
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

    if (!_initialized || (!alarms && maxCount > 0))
        return false;

    File directory = LittleFS.open(ALARM_DIRECTORY, "r");

    if (!directory || !directory.isDirectory())
    {
        if (directory)
            directory.close();

        return false;
    }

    File file = directory.openNextFile();

    while (file)
    {
        String id;

        if (!file.isDirectory())
            id = alarmIdFromPath(file.name());

        file.close();

        if (isUuidV4(id) && count < maxCount)
        {
            Alarm alarm;

            if (loadFromStorage(id, alarm))
                alarms[count++] = alarm;
        }

        file = directory.openNextFile();
    }

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
    if (!_initialized || !isUuidV4(id))
        return false;

    Alarm alarm;

    if (!loadFromStorage(id, alarm))
        return false;

    const int8_t index = findActive(id);

    if (alarm.enabled == enabled)
    {
        if (enabled && index < 0)
            return addActive(alarm);

        if (!enabled && index >= 0)
        {
            if (_runtime.active && _runtime.alarmId == id)
            {
                notifyFinish();
                _runtime = Runtime{};
            }

            if (_snooze.active && _snooze.alarmId == id)
                _snooze = SnoozeState{};

            removeActive(id);
        }

        return true;
    }

    alarm.enabled = enabled;

    if (enabled)
    {
        if (_activeCount >= MAX_ALARMS)
            return false;

        if (!saveToStorage(alarm))
            return false;

        if (!addActive(alarm))
        {
            alarm.enabled = false;
            saveToStorage(alarm);
            return false;
        }

        Serial0.printf("[ALARM] ENABLE id=%s\n", id.c_str());

        return true;
    }

    if (!saveToStorage(alarm))
        return false;

    if (_runtime.active && _runtime.alarmId == id)
    {
        notifyFinish();
        _runtime = Runtime{};
    }

    if (_snooze.active && _snooze.alarmId == id)
        _snooze = SnoozeState{};

    removeActive(id);

    Serial0.printf("[ALARM] DISABLE id=%s\n", id.c_str());

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

    Serial0.printf(
        "[ALARM] ACTIVE RAM ADD id=%s time=%02u:%02u:%02u\n",
        active.id.c_str(),
        active.time.hour,
        active.time.minute,
        active.time.second
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
// SAVE TO STORAGE
// ============================================================

bool AlarmManager::saveToStorage(const Alarm& alarm)
{
    Serial0.println("[ALARM] STORAGE SAVE: begin");

    if (!_filesystemReady)
    {
        Serial0.println("[ALARM] STORAGE SAVE ERROR: filesystem not ready");
        return false;
    }

    if (!validate(alarm))
    {
        Serial0.println("[ALARM] STORAGE SAVE ERROR: invalid alarm");
        return false;
    }

    if (!createDirectory())
    {
        Serial0.println("[ALARM] STORAGE SAVE ERROR: directory unavailable");
        return false;
    }

    const String path = pathFor(alarm.id);

    if (path.isEmpty())
    {
        Serial0.println("[ALARM] STORAGE SAVE ERROR: invalid path");
        return false;
    }

    JsonDocument document;

    if (!serialize(alarm, document))
    {
        Serial0.println("[ALARM] STORAGE SAVE ERROR: serialization failed");
        return false;
    }

    String content;

    const size_t expected = serializeJson(document, content);

    if (expected == 0 || content.isEmpty())
    {
        Serial0.println("[ALARM] STORAGE SAVE ERROR: empty JSON");
        return false;
    }

    Serial0.printf(
        "[ALARM] STORAGE SAVE: path=%s bytes=%u\n",
        path.c_str(),
        static_cast<unsigned>(expected)
    );

    // Write directly to the final file.
    // This avoids the failed temporary-file creation path.
    File file = LittleFS.open(path, "w");

    if (!file)
    {
        Serial0.printf(
            "[ALARM] STORAGE SAVE ERROR: cannot open %s\n",
            path.c_str()
        );

        Serial0.printf(
            "[ALARM] FS total=%u used=%u\n",
            static_cast<unsigned>(LittleFS.totalBytes()),
            static_cast<unsigned>(LittleFS.usedBytes())
        );

        return false;
    }

    if (file.isDirectory())
    {
        file.close();

        Serial0.println("[ALARM] STORAGE SAVE ERROR: path is a directory");
        return false;
    }

    const size_t written = file.write(
        reinterpret_cast<const uint8_t*>(content.c_str()),
        content.length()
    );

    file.flush();
    file.close();

    if (written != expected)
    {
        Serial0.printf(
            "[ALARM] STORAGE SAVE ERROR: incomplete write %u/%u\n",
            static_cast<unsigned>(written),
            static_cast<unsigned>(expected)
        );

        return false;
    }

    // Verify the file after writing.
    File check = LittleFS.open(path, "r");

    if (!check || check.isDirectory())
    {
        if (check)
            check.close();

        Serial0.println("[ALARM] STORAGE SAVE ERROR: cannot reopen saved file");
        return false;
    }

    const size_t actualSize = check.size();

    if (actualSize != expected)
    {
        check.close();

        Serial0.printf(
            "[ALARM] STORAGE SAVE ERROR: size mismatch %u/%u\n",
            static_cast<unsigned>(actualSize),
            static_cast<unsigned>(expected)
        );

        return false;
    }

    JsonDocument verification;

    const DeserializationError error = deserializeJson(
        verification,
        check
    );

    check.close();

    if (error)
    {
        Serial0.printf(
            "[ALARM] STORAGE SAVE ERROR: JSON verification: %s\n",
            error.c_str()
        );

        return false;
    }

    const char* savedId = verification["id"] | "";

    if (alarm.id != String(savedId))
    {
        Serial0.println("[ALARM] STORAGE SAVE ERROR: saved UUID mismatch");
        return false;
    }

    Serial0.printf(
        "[ALARM] STORAGE SAVE SUCCESS: %s (%u bytes)\n",
        path.c_str(),
        static_cast<unsigned>(actualSize)
    );

    return true;
}

// ============================================================
// LOAD FROM STORAGE
// ============================================================

bool AlarmManager::loadFromStorage(
    const String& id,
    Alarm& alarm
)
{
    resetAlarm(alarm);

    if (!_filesystemReady || !isUuidV4(id))
        return false;

    const String path = pathFor(id);

    File file = LittleFS.open(path, "r");

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

    const DeserializationError error = deserializeJson(
        document,
        file
    );

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
    {
        Serial0.printf(
            "[ALARM] DESERIALIZE ERROR id=%s\n",
            id.c_str()
        );

        resetAlarm(alarm);
        return false;
    }

    if (!validate(alarm))
    {
        Serial0.printf(
            "[ALARM] INVALID DATA id=%s\n",
            id.c_str()
        );

        resetAlarm(alarm);
        return false;
    }

    if (alarm.id != id)
    {
        Serial0.printf(
            "[ALARM] UUID MISMATCH file=%s data=%s\n",
            id.c_str(),
            alarm.id.c_str()
        );

        resetAlarm(alarm);
        return false;
    }

    return true;
}

// ============================================================
// DELETE FROM STORAGE
// ============================================================

bool AlarmManager::deleteFromStorage(const String& id)
{
    if (!_filesystemReady || !isUuidV4(id))
        return false;

    const String path = pathFor(id);

    if (!LittleFS.exists(path))
    {
        Serial0.printf(
            "[ALARM] DELETE ERROR: file does not exist: %s\n",
            path.c_str()
        );

        return false;
    }

    if (!LittleFS.remove(path))
    {
        Serial0.printf(
            "[ALARM] DELETE ERROR: cannot remove %s\n",
            path.c_str()
        );

        return false;
    }

    Serial0.printf("[ALARM] STORAGE DELETE SUCCESS: %s\n", path.c_str());

    return true;
}

// ============================================================
// RELOAD
// ============================================================

bool AlarmManager::reload()
{
    clearActive();

    if (!_filesystemReady)
        return false;

    const uint8_t stored = countStoredAlarms();

    if (stored > MAX_ALARMS)
    {
        Serial0.printf(
            "[ALARM] ERROR: stored=%u maximum=%u\n",
            stored,
            MAX_ALARMS
        );

        return false;
    }

    File directory = LittleFS.open(ALARM_DIRECTORY, "r");

    if (!directory || !directory.isDirectory())
    {
        if (directory)
            directory.close();

        Serial0.println("[ALARM] RELOAD ERROR: cannot open directory");
        return false;
    }

    bool result = true;

    File file = directory.openNextFile();

    while (file)
    {
        String id;

        if (!file.isDirectory())
            id = alarmIdFromPath(file.name());

        file.close();

        if (isUuidV4(id))
        {
            Alarm alarm;

            if (!loadFromStorage(id, alarm))
            {
                Serial0.printf(
                    "[ALARM] WARNING: skipped invalid record id=%s\n",
                    id.c_str()
                );
            }
            else if (alarm.enabled && !addActive(alarm))
            {
                result = false;

                Serial0.printf(
                    "[ALARM] ERROR: cannot add active alarm id=%s\n",
                    id.c_str()
                );
            }
        }

        file = directory.openNextFile();
    }

    directory.close();

    if (!result)
        clearActive();

    Serial0.printf(
        "[ALARM] RELOAD active=%u stored=%u\n",
        _activeCount,
        countStoredAlarms()
    );

    return result;
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

    const bool oneShot = (_runtime.alarm.repeatMask == 0);
    const bool snoozed = _runtime.snoozed;

    if (oneShot && !snoozed)
    {
        Alarm disabled = _runtime.alarm;
        disabled.enabled = false;

        if (saveToStorage(disabled))
        {
            removeActive(id);

            Serial0.printf(
                "[ALARM] ONE-SHOT DISABLED id=%s\n",
                id.c_str()
            );
        }
        else
        {
            Serial0.printf(
                "[ALARM] ERROR: cannot persist one-shot disable id=%s\n",
                id.c_str()
            );
        }
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

    int64_t days = seconds / SECONDS_PER_DAY_;

    if (seconds < 0 && seconds % SECONDS_PER_DAY_
 != 0)
        --days;

    return static_cast<time_t>(days * SECONDS_PER_DAY_);
}

time_t AlarmManager::shiftLocalDays(
    time_t timestamp,
    int32_t days
) const
{
    return static_cast<time_t>(
        static_cast<int64_t>(timestamp) +
        static_cast<int64_t>(days) * SECONDS_PER_DAY_

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

    // Monday=bit 0, Sunday=bit 6.
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

        if (
            active.repeatMask != 0 &&
            !(active.repeatMask & static_cast<uint8_t>(
                1U << dayBit(date)
            ))
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

// ============================================================
// START
// ============================================================

bool AlarmManager::start(
    uint8_t index,
    const Alarm& alarm,
    time_t triggerT0
)
{
    if (
        index >= MAX_ALARMS ||
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

    notifyTrigger();

    return true;
}

void AlarmManager::updateRuntime()
{
    // Alarm remains active until dismiss(), snooze(), or finish().
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

    // UUID version 4.
    bytes[6] = static_cast<uint8_t>(
        (bytes[6] & 0x0F) | 0x40
    );

    // UUID variant.
    bytes[8] = static_cast<uint8_t>(
        (bytes[8] & 0x3F) | 0x80
    );

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
