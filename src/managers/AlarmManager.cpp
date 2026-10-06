#include "AlarmManager.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <cstring>
#include <memory>
#include <new>
#include <time.h>

#include "SDManager.h"
#include "./core/ClockSystem.h"

// ============================================================
// CONSTANTS
// ============================================================

namespace
{
    constexpr const char* ALARM_DIRECTORY = "/alarms";
    constexpr uint16_t CURRENT_SCHEMA_VERSION = 1;

    constexpr uint32_t MAX_SNOOZE_MS = 0x7FFFFFFFUL;
    constexpr uint8_t WEEK_MASK = 0x7F;

    // If the main loop was blocked, allow a recently missed trigger to start.
    // The allowance also includes the negative phase offset.
    constexpr uint32_t MIN_MISSED_TRIGGER_GRACE_MS = 60UL * 1000UL;

    const char* conditionToString(
        AlarmCondition condition
    )
    {
        switch (condition)
        {
            case AlarmCondition::Always:
                return "always";

            case AlarmCondition::IfNotDismissed:
                return "if_not_dismissed";

            case AlarmCondition::IfNotSnoozed:
                return "if_not_snoozed";

            case AlarmCondition::IfNoMotion:
                return "if_no_motion";
        }

        return "always";
    }

    bool conditionFromJson(
        JsonVariantConst value,
        AlarmCondition& condition
    )
    {
        if (value.is<const char*>())
        {
            const char* name =
                value.as<const char*>();

            if (strcmp(name, "always") == 0)
            {
                condition = AlarmCondition::Always;
                return true;
            }

            if (strcmp(name, "if_not_dismissed") == 0)
            {
                condition = AlarmCondition::IfNotDismissed;
                return true;
            }

            if (strcmp(name, "if_not_snoozed") == 0)
            {
                condition = AlarmCondition::IfNotSnoozed;
                return true;
            }

            if (strcmp(name, "if_no_motion") == 0)
            {
                condition = AlarmCondition::IfNoMotion;
                return true;
            }

            return false;
        }

        if (!value.is<uint8_t>())
            return false;

        const uint8_t raw =
            value.as<uint8_t>();

        if (
            raw >
            static_cast<uint8_t>(
                AlarmCondition::IfNoMotion
            )
        )
        {
            return false;
        }

        condition =
            static_cast<AlarmCondition>(raw);

        return true;
    }

    void resetAlarm(Alarm& alarm)
    {
        alarm.~Alarm();
        new (&alarm) Alarm();
    }

    // --------------------------------------------------------
    // Local-calendar helpers
    //
    // ClockSystem's local timestamp is UTC epoch + configured
    // UTC_OFFSET. It is NOT an ESP timezone timestamp.
    //
    // Therefore all calendar operations in this manager use
    // gmtime_r(), never localtime_r()/mktime().
    // --------------------------------------------------------

    void localTm(
        time_t localTimestamp,
        struct tm& value
    )
    {
        gmtime_r(
            &localTimestamp,
            &value
        );
    }

    time_t localMidnight(
        time_t localTimestamp
    )
    {
        // ClockSystem::getLocalTime() returns a local-epoch value:
        // UTC epoch + configured offset. Treat it as a plain epoch and
        // remove the time-of-day without applying the ESP timezone.
        const int64_t seconds =
            static_cast<int64_t>(localTimestamp);

        int64_t days = seconds / 86400LL;

        if (seconds < 0 && (seconds % 86400LL) != 0)
            --days;

        return static_cast<time_t>(days * 86400LL);
    }

    time_t shiftLocalDays(
        time_t localTimestamp,
        int32_t days
    )
    {
        return localTimestamp +
               static_cast<time_t>(days) * 86400;
    }

    uint32_t missedTriggerGraceMs(
        int32_t earliestOffsetMs
    )
    {
        const int64_t negativeOffset =
            earliestOffsetMs < 0
                ? -static_cast<int64_t>(earliestOffsetMs)
                : 0;

        const int64_t grace =
            negativeOffset +
            static_cast<int64_t>(
                MIN_MISSED_TRIGGER_GRACE_MS
            );

        return grace >
                static_cast<int64_t>(0xFFFFFFFFUL)
            ? 0xFFFFFFFFUL
            : static_cast<uint32_t>(grace);
    }
}

// ============================================================
// CONSTRUCTOR
// ============================================================

AlarmManager::AlarmManager(
    SDManager& sd,
    ClockSystem& clock
)
    : _sd(sd)
    , _clock(clock)
{
}

// ============================================================
// BEGIN
// ============================================================

bool AlarmManager::begin()
{
    Serial0.println(
        "[ALARM] Initializing AlarmManager"
    );

    _initialized = false;
    _activeCount = 0;

    _runtime.~Runtime();
    new (&_runtime) Runtime();

    _snooze = SnoozeState{};

    for (
        uint8_t i = 0;
        i < AlarmConfig::MAX_ALARMS;
        ++i
    )
    {
        _activeAlarms[i] = ActiveAlarm{};
        _lastTriggerT0[i] = 0;
    }

    if (!_sd.isReady())
    {
        Serial0.println(
            "[ALARM] ERROR: SD is not ready"
        );
        return false;
    }

    if (!createDirectory())
    {
        Serial0.println(
            "[ALARM] ERROR: cannot create /alarms"
        );
        return false;
    }

    if (!reload())
    {
        Serial0.println(
            "[ALARM] ERROR: reload failed"
        );
        return false;
    }

    _initialized = true;

    Serial0.printf(
        "[ALARM] READY active=%u localNow=%lld\n",
        static_cast<unsigned>(_activeCount),
        static_cast<long long>(
            currentLocalTimestamp()
        )
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

    // --------------------------------------------------------
    // Existing runtime
    // --------------------------------------------------------

    if (_runtime.active)
    {
        updateRuntime();
        return;
    }

    // --------------------------------------------------------
    // Snooze
    // --------------------------------------------------------

    if (_snooze.active)
    {
        const uint32_t nowMs = millis();

        if (
            static_cast<int32_t>(
                nowMs - _snooze.untilMs
            ) >= 0
        )
        {
            const String alarmId =
                _snooze.alarmId;

            _snooze = SnoozeState{};

            std::unique_ptr<Alarm> alarm(
                new (std::nothrow) Alarm()
            );

            if (!alarm)
            {
                Serial0.println(
                    "[ALARM] ERROR: snooze Alarm allocation failed"
                );
                return;
            }

            if (
                loadFromSD(
                    alarmId,
                    *alarm
                )
            )
            {
                const int8_t index =
                    findActive(alarm->id);

                if (
                    index >= 0 &&
                    alarm->enabled
                )
                {
                    const time_t now =
                        currentLocalTimestamp();

                    Serial0.printf(
                        "[ALARM] SNOOZE EXPIRED id=%s localNow=%lld\n",
                        alarm->id.c_str(),
                        static_cast<long long>(now)
                    );

                    start(
                        static_cast<uint8_t>(index),
                        *alarm,
                        now,
                        0
                    );
                }
            }
        }

        return;
    }

    // --------------------------------------------------------
    // Normal scheduler
    // --------------------------------------------------------

    const time_t now =
        currentLocalTimestamp();

    for (
        uint8_t i = 0;
        i < _activeCount;
        ++i
    )
    {
        ActiveAlarm& active =
            _activeAlarms[i];

        if (!active.enabled)
            continue;

        if (active.id.isEmpty())
            continue;

        std::unique_ptr<Alarm> alarm(
            new (std::nothrow) Alarm()
        );

        if (!alarm)
        {
            Serial0.println(
                "[ALARM] ERROR: scheduler Alarm allocation failed"
            );
            return;
        }

        if (
            !loadFromSD(
                active.id,
                *alarm
            )
        )
        {
            continue;
        }

        if (!alarm->enabled)
            continue;

        time_t t0 = 0;

        if (
            !shouldStart(
                i,
                active,
                now,
                t0
            )
        )
        {
            continue;
        }

        const int64_t elapsed =
            (
                static_cast<int64_t>(now) -
                static_cast<int64_t>(t0)
            ) * 1000LL;

        start(
            i,
            *alarm,
            t0,
            elapsed
        );

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

// ============================================================
// CALLBACKS
// ============================================================

void AlarmManager::setTriggerCallback(
    TriggerCallback callback
)
{
    _triggerCallback = std::move(callback);
}

void AlarmManager::setPhaseCallback(
    PhaseCallback callback
)
{
    _phaseCallback = std::move(callback);
}

void AlarmManager::setFinishCallback(
    FinishCallback callback
)
{
    _finishCallback = std::move(callback);
}

// ============================================================
// CREATE
// ============================================================

bool AlarmManager::create(
    const Alarm& alarm
)
{
    logAlarm(
        "CREATE",
        alarm
    );

    if (!_initialized)
        return false;

    if (alarm.id.isEmpty())
        return false;

    if (!validate(alarm))
        return false;

    if (exists(alarm.id))
        return false;

    if (
        alarm.enabled &&
        _activeCount >= AlarmConfig::MAX_ALARMS
    )
    {
        return false;
    }

    if (!saveToSD(alarm))
        return false;

    if (alarm.enabled)
    {
        if (!addActive(alarm))
        {
            deleteFromSD(alarm.id);
            return false;
        }
    }

    return true;
}

// ============================================================
// UPDATE ALARM
// ============================================================

bool AlarmManager::update(
    const Alarm& alarm
)
{
    logAlarm(
        "UPDATE",
        alarm
    );

    if (!_initialized)
        return false;

    if (!validate(alarm))
        return false;

    std::unique_ptr<Alarm> oldAlarm(
        new (std::nothrow) Alarm()
    );

    if (!oldAlarm)
        return false;

    if (
        !get(
            alarm.id,
            *oldAlarm
        )
    )
    {
        return false;
    }

    const bool wasEnabled =
        oldAlarm->enabled;

    const bool willBeEnabled =
        alarm.enabled;

    const bool runtimeSameAlarm =
        _runtime.active &&
        _runtime.alarmId == alarm.id;

    if (
        !wasEnabled &&
        willBeEnabled &&
        _activeCount >= AlarmConfig::MAX_ALARMS
    )
    {
        return false;
    }

    if (runtimeSameAlarm)
        finish();

    if (!saveToSD(alarm))
        return false;

    removeActive(alarm.id);

    if (willBeEnabled)
    {
        if (!addActive(alarm))
            return false;
    }

    // The next occurrence of a changed alarm must be allowed
    // to trigger even if its T0 equals an old occurrence.
    const int8_t index =
        findActive(alarm.id);

    if (index >= 0)
        _lastTriggerT0[index] = 0;

    return true;
}

// ============================================================
// REMOVE
// ============================================================

bool AlarmManager::remove(
    const String& id
)
{
    if (!_initialized || id.isEmpty())
        return false;

    if (!exists(id))
        return false;

    if (
        _runtime.active &&
        _runtime.alarmId == id
    )
    {
        finish();
    }

    if (
        _snooze.active &&
        _snooze.alarmId == id
    )
    {
        _snooze = SnoozeState{};
    }

    removeActive(id);

    return deleteFromSD(id);
}

// ============================================================
// GET
// ============================================================

bool AlarmManager::get(
    const String& id,
    Alarm& alarm
)
{
    if (!_initialized || id.isEmpty())
        return false;

    return loadFromSD(
        id,
        alarm
    );
}

// ============================================================
// EXISTS
// ============================================================

bool AlarmManager::exists(
    const String& id
)
{
    if (!_initialized || id.isEmpty())
        return false;

    return _sd.fileExists(
        pathFor(id)
    );
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

    if (!alarms)
        return false;

    if (maxCount == 0)
        return true;

    SDFileEntry files[
        AlarmConfig::MAX_ALARMS
    ];

    const size_t fileCount =
        _sd.listFiles(
            files,
            AlarmConfig::MAX_ALARMS,
            1,
            ALARM_DIRECTORY
        );

    for (
        size_t i = 0;
        i < fileCount &&
        count < maxCount;
        ++i
    )
    {
        if (files[i].isDir)
            continue;

        if (!files[i].path.endsWith(".json"))
            continue;

        String id =
            files[i].path;

        const int slash =
            id.lastIndexOf('/');

        if (slash >= 0)
            id = id.substring(slash + 1);

        if (!id.endsWith(".json"))
            continue;

        id.remove(
            id.length() - 5
        );

        std::unique_ptr<Alarm> alarm(
            new (std::nothrow) Alarm()
        );

        if (!alarm)
            return false;

        if (
            loadFromSD(
                id,
                *alarm
            )
        )
        {
            alarms[count++] = *alarm;
        }
    }

    return true;
}

// ============================================================
// ENABLE / DISABLE
// ============================================================

bool AlarmManager::enable(
    const String& id
)
{
    return setEnabled(
        id,
        true
    );
}

bool AlarmManager::disable(
    const String& id
)
{
    return setEnabled(
        id,
        false
    );
}

bool AlarmManager::setEnabled(
    const String& id,
    bool enabled
)
{
    if (!_initialized || id.isEmpty())
        return false;

    std::unique_ptr<Alarm> alarm(
        new (std::nothrow) Alarm()
    );

    if (!alarm)
        return false;

    if (
        !loadFromSD(
            id,
            *alarm
        )
    )
    {
        return false;
    }

    if (alarm->enabled == enabled)
        return true;

    if (
        enabled &&
        _activeCount >= AlarmConfig::MAX_ALARMS
    )
    {
        return false;
    }

    if (
        _runtime.active &&
        _runtime.alarmId == id
    )
    {
        finish();
    }

    alarm->enabled = enabled;

    if (!saveToSD(*alarm))
        return false;

    removeActive(id);

    if (enabled)
    {
        if (!addActive(*alarm))
            return false;
    }

    return true;
}

// ============================================================
// ACTIVE LIST
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

    const ActiveAlarm& active =
        _activeAlarms[index];

    id = active.id;
    time = active.time;
    repeatMask = active.repeatMask;

    return true;
}

int8_t AlarmManager::findActive(
    const String& id
) const
{
    if (id.isEmpty())
        return -1;

    for (
        uint8_t i = 0;
        i < _activeCount;
        ++i
    )
    {
        if (
            _activeAlarms[i].id == id
        )
        {
            return static_cast<int8_t>(i);
        }
    }

    return -1;
}

bool AlarmManager::addActive(
    const Alarm& alarm
)
{
    if (!alarm.enabled)
        return false;

    if (
        findActive(alarm.id) >= 0
    )
    {
        return true;
    }

    if (
        _activeCount >= AlarmConfig::MAX_ALARMS
    )
    {
        return false;
    }

    ActiveAlarm& active =
        _activeAlarms[_activeCount];

    active.id = alarm.id;
    active.time = alarm.time;
    active.repeatMask = alarm.repeatMask;
    active.enabled = alarm.enabled;
    active.earliestOffsetMs =
        earliestOffset(alarm);

    ++_activeCount;

    return true;
}

bool AlarmManager::removeActive(
    const String& id
)
{
    const int8_t found =
        findActive(id);

    if (found < 0)
        return false;

    const uint8_t index =
        static_cast<uint8_t>(found);

    for (
        uint8_t i = index;
        i + 1 < _activeCount;
        ++i
    )
    {
        _activeAlarms[i] =
            _activeAlarms[i + 1];
    }

    _activeAlarms[
        _activeCount - 1
    ] = ActiveAlarm{};

    --_activeCount;

    return true;
}

void AlarmManager::clearActive()
{
    for (
        uint8_t i = 0;
        i < AlarmConfig::MAX_ALARMS;
        ++i
    )
    {
        _activeAlarms[i] =
            ActiveAlarm{};
    }

    _activeCount = 0;
}

// ============================================================
// SD
// ============================================================

bool AlarmManager::createDirectory()
{
    if (!_sd.isReady())
        return false;

    return _sd.createDirectory(
        ALARM_DIRECTORY
    );
}

String AlarmManager::pathFor(
    const String& id
) const
{
    if (id.isEmpty())
        return String();

    return String(ALARM_DIRECTORY) +
           "/" +
           id +
           ".json";
}

bool AlarmManager::saveToSD(
    const Alarm& alarm
)
{
    if (
        !_sd.isReady() ||
        alarm.id.isEmpty()
    )
    {
        return false;
    }

    JsonDocument document;

    if (
        !serialize(
            alarm,
            document
        )
    )
    {
        return false;
    }

    String content;

    serializeJson(
        document,
        content
    );

    return _sd.writeFile(
        pathFor(alarm.id),
        content
    );
}

bool AlarmManager::loadFromSD(
    const String& id,
    Alarm& alarm
)
{
    resetAlarm(alarm);

    if (
        !_sd.isReady() ||
        id.isEmpty()
    )
    {
        return false;
    }

    String content;

    if (
        !_sd.readFile(
            pathFor(id),
            content
        )
    )
    {
        return false;
    }

    JsonDocument document;

    const DeserializationError error =
        deserializeJson(
            document,
            content
        );

    if (error)
        return false;

    if (
        !deserialize(
            document,
            alarm
        )
    )
    {
        return false;
    }

    if (alarm.id != id)
    {
        resetAlarm(alarm);
        return false;
    }

    return true;
}

bool AlarmManager::deleteFromSD(
    const String& id
)
{
    if (
        !_sd.isReady() ||
        id.isEmpty()
    )
    {
        return false;
    }

    return _sd.deleteFile(
        pathFor(id)
    );
}

// ============================================================
// RELOAD
// ============================================================

bool AlarmManager::reload()
{
    clearActive();

    for (
        uint8_t i = 0;
        i < AlarmConfig::MAX_ALARMS;
        ++i
    )
    {
        _lastTriggerT0[i] = 0;
    }

    if (!_sd.isReady())
        return false;

    SDFileEntry files[
        AlarmConfig::MAX_ALARMS
    ];

    const size_t fileCount =
        _sd.listFiles(
            files,
            AlarmConfig::MAX_ALARMS,
            1,
            ALARM_DIRECTORY
        );

    for (
        size_t i = 0;
        i < fileCount;
        ++i
    )
    {
        if (files[i].isDir)
            continue;

        if (!files[i].path.endsWith(".json"))
            continue;

        String id =
            files[i].path;

        const int slash =
            id.lastIndexOf('/');

        if (slash >= 0)
            id = id.substring(slash + 1);

        if (!id.endsWith(".json"))
            continue;

        id.remove(
            id.length() - 5
        );

        std::unique_ptr<Alarm> alarm(
            new (std::nothrow) Alarm()
        );

        if (!alarm)
            return false;

        if (
            !loadFromSD(
                id,
                *alarm
            )
        )
        {
            Serial0.printf(
                "[ALARM] WARNING: cannot load %s\n",
                id.c_str()
            );
            continue;
        }

        if (!alarm->enabled)
            continue;

        if (
            _activeCount >=
            AlarmConfig::MAX_ALARMS
        )
        {
            Serial0.println(
                "[ALARM] WARNING: active alarm limit reached"
            );
            break;
        }

        addActive(*alarm);
    }

    Serial0.printf(
        "[ALARM] reload complete active=%u\n",
        static_cast<unsigned>(_activeCount)
    );

    return true;
}

// ============================================================
// SERIALIZATION
// ============================================================

bool AlarmManager::serialize(
    const Alarm& alarm,
    JsonDocument& document
) const
{
    document.clear();

    document["schemaVersion"] =
        alarm.schemaVersion;

    document["id"] =
        alarm.id;

    document["name"] =
        alarm.name;

    document["enabled"] =
        alarm.enabled;

    JsonObject time =
        document["time"].to<JsonObject>();

    time["hour"] =
        alarm.time.hour;

    time["minute"] =
        alarm.time.minute;

    time["second"] =
        alarm.time.second;

    document["repeatMask"] =
        alarm.repeatMask;

    JsonArray phases =
        document["phases"].to<JsonArray>();

    for (
        uint8_t i = 0;
        i < alarm.phaseCount;
        ++i
    )
    {
        const AlarmPhase& phase =
            alarm.phases[i];

        JsonObject object =
            phases.add<JsonObject>();

        object["startOffsetMs"] =
            phase.startOffsetMs;

        object["durationMs"] =
            phase.durationMs;

        object["condition"] =
            conditionToString(
                phase.condition
            );

        JsonObject matrix =
            object["matrix"].to<JsonObject>();

        matrix["enabled"] =
            phase.matrix.enabled;

        matrix["effectId"] =
            phase.matrix.effectId;

        matrix["start"] =
            phase.matrix.start;

        matrix["end"] =
            phase.matrix.end;

        matrix["speedMs"] =
            phase.matrix.speedMs;

        matrix["durationMs"] =
            phase.matrix.durationMs;

        JsonObject audio =
            object["audio"].to<JsonObject>();

        audio["enabled"] =
            phase.audio.enabled;

        audio["effectId"] =
            phase.audio.effectId;

        audio["start"] =
            phase.audio.start;

        audio["end"] =
            phase.audio.end;

        audio["speedMs"] =
            phase.audio.speedMs;

        audio["durationMs"] =
            phase.audio.durationMs;

        audio["loop"] =
            phase.audio.loop;

        JsonObject cob =
            object["cob"].to<JsonObject>();

        cob["enabled"] =
            phase.cob.enabled;

        cob["effectId"] =
            phase.cob.effectId;

        cob["start"] =
            phase.cob.start;

        cob["end"] =
            phase.cob.end;

        cob["speedMs"] =
            phase.cob.speedMs;

        cob["durationMs"] =
            phase.cob.durationMs;

        cob["maxDurationMs"] =
            phase.cob.maxDurationMs;
    }

    return true;
}

// ============================================================
// DESERIALIZATION
// ============================================================

bool AlarmManager::deserialize(
    JsonDocument& document,
    Alarm& alarm
) const
{
    resetAlarm(alarm);

    if (!document["id"].is<String>())
        return false;

    alarm.schemaVersion =
        document["schemaVersion"] |
        CURRENT_SCHEMA_VERSION;

    alarm.id =
        document["id"].as<String>();

    alarm.name =
        document["name"] |
        String();

    alarm.enabled =
        document["enabled"] |
        false;

    alarm.repeatMask =
        document["repeatMask"] |
        0;

    JsonObject time =
        document["time"].as<JsonObject>();

    if (time.isNull())
        return false;

    alarm.time.hour =
        time["hour"] | 0;

    alarm.time.minute =
        time["minute"] | 0;

    alarm.time.second =
        time["second"] | 0;

    JsonArray phases =
        document["phases"].as<JsonArray>();

    if (phases.isNull())
    {
        alarm.phaseCount = 0;
        return validate(alarm);
    }

    if (
        phases.size() >
        AlarmConfig::MAX_PHASES
    )
    {
        return false;
    }

    alarm.phaseCount =
        static_cast<uint8_t>(
            phases.size()
        );

    uint8_t index = 0;

    for (
        JsonVariant value : phases
    )
    {
        JsonObject object =
            value.as<JsonObject>();

        if (object.isNull())
            return false;

        AlarmPhase& phase =
            alarm.phases[index++];

        phase.startOffsetMs =
            object["startOffsetMs"] |
            0;

        phase.durationMs =
            object["durationMs"] |
            0;

        JsonVariantConst condition =
            object["condition"];

        if (
            !condition.isNull() &&
            !conditionFromJson(
                condition,
                phase.condition
            )
        )
        {
            return false;
        }

        JsonObject matrix =
            object["matrix"].as<JsonObject>();

        if (!matrix.isNull())
        {
            phase.matrix.enabled =
                matrix["enabled"] | false;

            phase.matrix.effectId =
                matrix["effectId"] | String();

            phase.matrix.start =
                matrix["start"] | 0;

            phase.matrix.end =
                matrix["end"] | 0;

            phase.matrix.speedMs =
                matrix["speedMs"] | 0;

            phase.matrix.durationMs =
                matrix["durationMs"] | 0;
        }

        JsonObject audio =
            object["audio"].as<JsonObject>();

        if (!audio.isNull())
        {
            phase.audio.enabled =
                audio["enabled"] | false;

            phase.audio.effectId =
                audio["effectId"] | String();

            phase.audio.start =
                audio["start"] | 0;

            phase.audio.end =
                audio["end"] | 0;

            phase.audio.speedMs =
                audio["speedMs"] | 0;

            phase.audio.durationMs =
                audio["durationMs"] | 0;

            phase.audio.loop =
                audio["loop"] | false;
        }

        JsonObject cob =
            object["cob"].as<JsonObject>();

        if (!cob.isNull())
        {
            phase.cob.enabled =
                cob["enabled"] | false;

            phase.cob.effectId =
                cob["effectId"] | String();

            phase.cob.start =
                cob["start"] | 0;

            phase.cob.end =
                cob["end"] | 0;

            phase.cob.speedMs =
                cob["speedMs"] | 0;

            phase.cob.durationMs =
                cob["durationMs"] | 0;

            phase.cob.maxDurationMs =
                cob["maxDurationMs"] | 0;
        }
    }

    return validate(alarm);
}

// ============================================================
// VALIDATION
// ============================================================

bool AlarmManager::validate(
    const Alarm& alarm
) const
{
    if (alarm.id.isEmpty())
        return false;

    for (
        size_t i = 0;
        i < alarm.id.length();
        ++i
    )
    {
        const char c = alarm.id[i];

        const bool valid =
            (
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '-' ||
                c == '_'
            );

        if (!valid)
            return false;
    }

    if (alarm.time.hour > 23)
        return false;

    if (alarm.time.minute > 59)
        return false;

    if (alarm.time.second > 59)
        return false;

    if (
        alarm.repeatMask &
        static_cast<uint8_t>(~WEEK_MASK)
    )
    {
        return false;
    }

    if (
        alarm.phaseCount >
        AlarmConfig::MAX_PHASES
    )
    {
        return false;
    }

    for (
        uint8_t i = 0;
        i < alarm.phaseCount;
        ++i
    )
    {
        const uint8_t condition =
            static_cast<uint8_t>(
                alarm.phases[i].condition
            );

        if (
            condition >
            static_cast<uint8_t>(
                AlarmCondition::IfNoMotion
            )
        )
        {
            return false;
        }
    }

    return true;
}

// ============================================================
// RUNTIME
// ============================================================

bool AlarmManager::isRunning() const
{
    return _runtime.active;
}

const Alarm* AlarmManager::currentAlarm() const
{
    if (!_runtime.active)
        return nullptr;

    return &_runtime.alarm;
}

const AlarmPhase* AlarmManager::currentPhase() const
{
    if (!_runtime.active)
        return nullptr;

    if (
        _runtime.phaseIndex >=
        _runtime.alarm.phaseCount
    )
    {
        return nullptr;
    }

    return &_runtime.alarm.phases[
        _runtime.phaseIndex
    ];
}

uint8_t AlarmManager::currentPhaseIndex() const
{
    return _runtime.phaseIndex;
}

uint32_t AlarmManager::elapsedMs() const
{
    if (!_runtime.active)
        return 0;

    if (_runtime.elapsedMs <= 0)
        return 0;

    if (
        _runtime.elapsedMs >
        0xFFFFFFFFLL
    )
    {
        return 0xFFFFFFFFUL;
    }

    return static_cast<uint32_t>(
        _runtime.elapsedMs
    );
}

// ============================================================
// DISMISS / SNOOZE / FINISH
// ============================================================

bool AlarmManager::dismiss()
{
    if (!_runtime.active)
        return false;

    _runtime.dismissed = true;

    finish();

    return true;
}

bool AlarmManager::snooze(
    uint32_t durationMs
)
{
    if (!_runtime.active)
        return false;

    if (
        durationMs == 0 ||
        durationMs > MAX_SNOOZE_MS
    )
    {
        return false;
    }

    _runtime.snoozed = true;

    _snooze.active = true;
    _snooze.alarmId =
        _runtime.alarmId;

    _snooze.untilMs =
        millis() + durationMs;

    Serial0.printf(
        "[ALARM] SNOOZE id=%s duration=%lu untilMs=%lu\n",
        _runtime.alarmId.c_str(),
        static_cast<unsigned long>(durationMs),
        static_cast<unsigned long>(_snooze.untilMs)
    );

    finish();

    return true;
}

void AlarmManager::finish()
{
    if (!_runtime.active)
        return;

    const String alarmId =
        _runtime.alarmId;

    const bool oneShot =
        _runtime.alarm.repeatMask == 0;

    const bool snoozed =
        _runtime.snoozed;

    if (
        oneShot &&
        !snoozed
    )
    {
        std::unique_ptr<Alarm> alarm(
            new (std::nothrow) Alarm()
        );

        if (alarm)
        {
            if (
                loadFromSD(
                    alarmId,
                    *alarm
                )
            )
            {
                alarm->enabled = false;
                saveToSD(*alarm);
            }
        }

        removeActive(alarmId);
    }

    notifyFinish();

    _runtime.~Runtime();
    new (&_runtime) Runtime();
}

// ============================================================
// LOCAL TIME
// ============================================================

time_t AlarmManager::currentLocalTimestamp() const
{
    return _clock.getLocalTime();
}

time_t AlarmManager::makeLocalT0(
    time_t localDateTimestamp,
    const AlarmTime& timeValue
) const
{
    // localDateTimestamp is already expressed in ClockSystem's
    // local-epoch representation. Therefore we only need the
    // local calendar midnight plus HH:MM:SS.
    const time_t midnight =
        localMidnight(localDateTimestamp);

    return midnight +
           static_cast<time_t>(timeValue.hour) * 3600 +
           static_cast<time_t>(timeValue.minute) * 60 +
           static_cast<time_t>(timeValue.second);
}

// ============================================================
// PHASE OFFSET
// ============================================================

int32_t AlarmManager::earliestOffset(
    const Alarm& alarm
) const
{
    if (alarm.phaseCount == 0)
        return 0;

    int32_t earliest =
        alarm.phases[0].startOffsetMs;

    for (
        uint8_t i = 1;
        i < alarm.phaseCount;
        ++i
    )
    {
        if (
            alarm.phases[i].startOffsetMs <
            earliest
        )
        {
            earliest =
                alarm.phases[i].startOffsetMs;
        }
    }

    return earliest;
}

// ============================================================
// WEEKDAY
// ============================================================

uint8_t AlarmManager::dayBit(
    time_t localTimestamp
) const
{
    struct tm value{};
    localTm(
        localTimestamp,
        value
    );

    // tm_wday:
    // Sunday=0, Monday=1 ... Saturday=6.
    // Alarm mask:
    // Monday=bit0 ... Sunday=bit6.
    if (value.tm_wday == 0)
        return 6;

    return static_cast<uint8_t>(
        value.tm_wday - 1
    );
}

bool AlarmManager::isRepeatDay(
    const Alarm& alarm,
    time_t localTimestamp
) const
{
    if (alarm.repeatMask == 0)
        return true;

    const uint8_t bit =
        dayBit(localTimestamp);

    return (
        alarm.repeatMask &
        static_cast<uint8_t>(1U << bit)
    ) != 0;
}

// ============================================================
// SHOULD START
// ============================================================

bool AlarmManager::shouldStart(
    uint8_t index,
    const ActiveAlarm& active,
    time_t now,
    time_t& t0
) const
{
    if (index >= AlarmConfig::MAX_ALARMS)
        return false;

    if (!active.enabled)
        return false;

    if (active.id.isEmpty())
        return false;

    const time_t day =
        86400;

    const time_t today =
        localMidnight(now);

    const uint32_t graceMs =
        missedTriggerGraceMs(
            active.earliestOffsetMs
        );

    /*
     * We evaluate the ALARM DATE, not the trigger date.
     *
     * Candidates:
     *
     * yesterday:
     *   catches a valid occurrence whose phase extends around midnight.
     *
     * today:
     *   normal occurrence.
     *
     * tomorrow:
     *   required when a negative phase offset moves a tomorrow alarm
     *   into today's late evening.
     */
    const time_t candidates[3] =
    {
        today - day,
        today,
        today + day
    };

    const int64_t nowMs =
        static_cast<int64_t>(now) * 1000LL;

    bool found = false;
    time_t bestTrigger = 0;
    int64_t bestAge = INT64_MAX;

    for (uint8_t c = 0; c < 3; ++c)
    {
        const time_t occurrenceDate =
            candidates[c];

        // Repeat mask belongs to the alarm occurrence date.
        if (
            active.repeatMask != 0 &&
            (
                active.repeatMask &
                static_cast<uint8_t>(
                    1U << dayBit(occurrenceDate)
                )
            ) == 0
        )
        {
            continue;
        }

        const time_t alarmT0 =
            makeLocalT0(
                occurrenceDate,
                active.time
            );

        const int64_t triggerMs =
            static_cast<int64_t>(alarmT0) * 1000LL +
            static_cast<int64_t>(
                active.earliestOffsetMs
            );

        const int64_t ageMs =
            nowMs - triggerMs;

        Serial0.printf(
            "[ALARM][CHECK] id=%s occurrence=%lld alarmT0=%lld trigger=%lld "
            "now=%lld age=%lld offset=%ld repeat=0x%02X\n",
            active.id.c_str(),
            static_cast<long long>(occurrenceDate),
            static_cast<long long>(alarmT0),
            static_cast<long long>(
                triggerMs / 1000LL
            ),
            static_cast<long long>(now),
            static_cast<long long>(ageMs),
            static_cast<long>(
                active.earliestOffsetMs
            ),
            static_cast<unsigned>(
                active.repeatMask
            )
        );

        // Trigger is still in the future.
        if (ageMs < 0)
            continue;

        // Already handled this exact occurrence.
        if (
            _lastTriggerT0[index] ==
            alarmT0
        )
        {
            Serial0.printf(
                "[ALARM][SKIP] id=%s reason=already-triggered t0=%lld\n",
                active.id.c_str(),
                static_cast<long long>(alarmT0)
            );
            continue;
        }

        // Do not fire an occurrence hours later.
        if (
            static_cast<uint64_t>(ageMs) >
            static_cast<uint64_t>(graceMs)
        )
        {
            Serial0.printf(
                "[ALARM][SKIP] id=%s reason=too-old age=%lld grace=%lu\n",
                active.id.c_str(),
                static_cast<long long>(ageMs),
                static_cast<unsigned long>(graceMs)
            );
            continue;
        }

        // Select the most recent valid trigger.
        if (
            !found ||
            ageMs < bestAge
        )
        {
            found = true;
            bestAge = ageMs;
            bestTrigger = alarmT0;
        }
    }

    if (!found)
        return false;

    t0 = bestTrigger;

    logSchedule(
        "TRIGGER",
        active.id,
        t0
    );

    Serial0.printf(
        "[ALARM][MATCH] id=%s t0=%lld now=%lld elapsed=%lldms\n",
        active.id.c_str(),
        static_cast<long long>(t0),
        static_cast<long long>(now),
        static_cast<long long>(
            (
                static_cast<int64_t>(now) -
                static_cast<int64_t>(t0)
            ) * 1000LL
        )
    );

    return true;
}

// ============================================================
// START
// ============================================================

bool AlarmManager::start(
    uint8_t index,
    const Alarm& alarm,
    time_t t0,
    int64_t elapsedMs
)
{
    if (
        index >=
        AlarmConfig::MAX_ALARMS
    )
    {
        return false;
    }

    if (!alarm.enabled)
        return false;

    if (_runtime.active)
        return false;

    _runtime.~Runtime();
    new (&_runtime) Runtime();

    _runtime.active = true;
    _runtime.alarmId = alarm.id;
    _runtime.alarm = alarm;
    _runtime.triggerT0 = t0;
    _runtime.elapsedMs = elapsedMs;
    _runtime.phaseElapsedMs = 0;
    _runtime.dismissed = false;
    _runtime.snoozed = false;
    _runtime.lastUpdateMs = millis();

    _lastTriggerT0[index] = t0;

    logAlarm(
        "START",
        _runtime.alarm
    );

    Serial0.printf(
        "[ALARM][START] id=%s t0=%lld elapsed=%lldms phases=%u\n",
        _runtime.alarmId.c_str(),
        static_cast<long long>(t0),
        static_cast<long long>(elapsedMs),
        static_cast<unsigned>(
            _runtime.alarm.phaseCount
        )
    );

    notifyTrigger();

    const int8_t phaseIndex =
        findPhase(
            _runtime.alarm,
            _runtime.elapsedMs
        );

    if (phaseIndex < 0)
    {
        _runtime.phaseIndex = 0;

        Serial0.printf(
            "[ALARM][START] id=%s no active phase yet\n",
            _runtime.alarmId.c_str()
        );

        return true;
    }

    _runtime.phaseIndex =
        static_cast<uint8_t>(
            phaseIndex
        );

    const AlarmPhase& phase =
        _runtime.alarm.phases[
            _runtime.phaseIndex
        ];

    _runtime.phaseElapsedMs =
        _runtime.elapsedMs -
        static_cast<int64_t>(
            phase.startOffsetMs
        );

    notifyPhase(
        _runtime.phaseIndex
    );

    return true;
}

// ============================================================
// RUNTIME UPDATE
// ============================================================

void AlarmManager::updateRuntime()
{
    if (!_runtime.active)
        return;

    const uint32_t nowMs =
        millis();

    const uint32_t deltaMs =
        nowMs -
        _runtime.lastUpdateMs;

    _runtime.lastUpdateMs =
        nowMs;

    _runtime.elapsedMs +=
        static_cast<int64_t>(
            deltaMs
        );

    if (
        _runtime.phaseIndex <
        _runtime.alarm.phaseCount
    )
    {
        const AlarmPhase& phase =
            _runtime.alarm.phases[
                _runtime.phaseIndex
            ];

        _runtime.phaseElapsedMs =
            _runtime.elapsedMs -
            static_cast<int64_t>(
                phase.startOffsetMs
            );
    }

    const int8_t newPhase =
        findPhase(
            _runtime.alarm,
            _runtime.elapsedMs
        );

    if (newPhase >= 0)
    {
        const uint8_t phaseIndex =
            static_cast<uint8_t>(newPhase);

        if (
            phaseIndex !=
            _runtime.phaseIndex
        )
        {
            _runtime.phaseIndex =
                phaseIndex;

            const AlarmPhase& phase =
                _runtime.alarm.phases[
                    phaseIndex
                ];

            _runtime.phaseElapsedMs =
                _runtime.elapsedMs -
                static_cast<int64_t>(
                    phase.startOffsetMs
                );

            notifyPhase(
                phaseIndex
            );
        }
    }

    bool infinite = false;
    int64_t latestEnd = INT64_MIN;

    for (
        uint8_t i = 0;
        i < _runtime.alarm.phaseCount;
        ++i
    )
    {
        const AlarmPhase& phase =
            _runtime.alarm.phases[i];

        if (phase.durationMs == 0)
        {
            infinite = true;
            break;
        }

        const int64_t end =
            static_cast<int64_t>(
                phase.startOffsetMs
            ) +
            static_cast<int64_t>(
                phase.durationMs
            );

        if (end > latestEnd)
            latestEnd = end;
    }

    if (
        !infinite &&
        _runtime.alarm.phaseCount > 0 &&
        _runtime.elapsedMs >= latestEnd
    )
    {
        finish();
    }
}

// ============================================================
// PHASES
// ============================================================

int8_t AlarmManager::findPhase(
    const Alarm& alarm,
    int64_t elapsedMs
) const
{
    int8_t result = -1;

    for (
        uint8_t i = 0;
        i < alarm.phaseCount;
        ++i
    )
    {
        const AlarmPhase& phase =
            alarm.phases[i];

        if (!conditionPassed(phase))
            continue;

        if (!phaseActive(
                phase,
                elapsedMs
            ))
        {
            continue;
        }

        // Later phase wins when intervals overlap.
        result =
            static_cast<int8_t>(i);
    }

    return result;
}

bool AlarmManager::phaseActive(
    const AlarmPhase& phase,
    int64_t elapsedMs
) const
{
    const int64_t start =
        static_cast<int64_t>(
            phase.startOffsetMs
        );

    if (elapsedMs < start)
        return false;

    if (phase.durationMs == 0)
        return true;

    const int64_t end =
        start +
        static_cast<int64_t>(
            phase.durationMs
        );

    return elapsedMs < end;
}

bool AlarmManager::conditionPassed(
    const AlarmPhase& phase
) const
{
    switch (phase.condition)
    {
        case AlarmCondition::Always:
            return true;

        case AlarmCondition::IfNotDismissed:
            return !_runtime.dismissed;

        case AlarmCondition::IfNotSnoozed:
            return !_runtime.snoozed;

        case AlarmCondition::IfNoMotion:
            // Sensor callback is not yet part of AlarmManager.
            return true;
    }

    return false;
}

// ============================================================
// DIAGNOSTICS
// ============================================================

void AlarmManager::logSchedule(
    const char* event,
    const String& id,
    time_t timestamp
) const
{
    struct tm value{};
    localTm(
        timestamp,
        value
    );

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
        "time=%02u:%02u:%02u repeat=0x%02X phases=%u\n",
        event,
        alarm.id.c_str(),
        alarm.name.c_str(),
        alarm.enabled ? "true" : "false",
        alarm.time.hour,
        alarm.time.minute,
        alarm.time.second,
        alarm.repeatMask,
        alarm.phaseCount
    );

    for (
        uint8_t i = 0;
        i < alarm.phaseCount;
        ++i
    )
    {
        const AlarmPhase& phase =
            alarm.phases[i];

        Serial0.printf(
            "[ALARM]   phase=%u offset=%ldms duration=%lums "
            "condition=%u "
            "matrix=%s/%s/%u-%u "
            "audio=%s/%s/%u-%u loop=%s "
            "cob=%s/%s/%u-%u\n",
            static_cast<unsigned>(i),
            static_cast<long>(
                phase.startOffsetMs
            ),
            static_cast<unsigned long>(
                phase.durationMs
            ),
            static_cast<unsigned>(
                phase.condition
            ),
            phase.matrix.enabled
                ? "on"
                : "off",
            phase.matrix.effectId.c_str(),
            phase.matrix.start,
            phase.matrix.end,
            phase.audio.enabled
                ? "on"
                : "off",
            phase.audio.effectId.c_str(),
            phase.audio.start,
            phase.audio.end,
            phase.audio.loop
                ? "true"
                : "false",
            phase.cob.enabled
                ? "on"
                : "off",
            phase.cob.effectId.c_str(),
            phase.cob.start,
            phase.cob.end
        );
    }
}

// ============================================================
// CALLBACK NOTIFY
// ============================================================

void AlarmManager::notifyTrigger()
{
    if (_triggerCallback)
        _triggerCallback(
            _runtime.alarm
        );
}

void AlarmManager::notifyPhase(
    uint8_t phaseIndex
)
{
    if (
        !_phaseCallback ||
        phaseIndex >=
            _runtime.alarm.phaseCount
    )
    {
        return;
    }

    _phaseCallback(
        _runtime.alarm,
        phaseIndex,
        _runtime.alarm.phases[
            phaseIndex
        ]
    );
}

void AlarmManager::notifyFinish()
{
    if (_finishCallback)
        _finishCallback(
            _runtime.alarmId
        );
}

// ============================================================
// ID
// ============================================================

String AlarmManager::generateId() const
{
    char buffer[37];

    const uint32_t a = esp_random();
    const uint32_t b = esp_random();
    const uint32_t c = esp_random();
    const uint32_t d = esp_random();

    snprintf(
        buffer,
        sizeof(buffer),
        "%08lX-%04lX-%04lX-%04lX-%08lX",
        static_cast<unsigned long>(a),
        static_cast<unsigned long>(
            (b >> 16) & 0xFFFF
        ),
        static_cast<unsigned long>(
            b & 0xFFFF
        ),
        static_cast<unsigned long>(
            (c >> 16) & 0xFFFF
        ),
        static_cast<unsigned long>(
            c & 0xFFFF
        )
    );

    return String(buffer);
}

// ============================================================
// SCAN FILE
// ============================================================

void AlarmManager::scanFile(
    const String& path
)
{
    if (
        path.isEmpty() ||
        !path.endsWith(".json")
    )
    {
        return;
    }

    String id = path;

    const int slash =
        id.lastIndexOf('/');

    if (slash >= 0)
        id = id.substring(slash + 1);

    id.remove(
        id.length() - 5
    );

    std::unique_ptr<Alarm> alarm(
        new (std::nothrow) Alarm()
    );

    if (!alarm)
        return;

    if (
        !loadFromSD(
            id,
            *alarm
        )
    )
    {
        return;
    }

    if (!alarm->enabled)
        return;

    addActive(*alarm);
}
