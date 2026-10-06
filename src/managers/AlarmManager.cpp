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
    constexpr const char* ALARM_DIRECTORY =
        "/alarms";

    constexpr uint16_t CURRENT_SCHEMA_VERSION =
        1;

    constexpr uint32_t MAX_SNOOZE_MS =
        0x7FFFFFFFUL;

    constexpr uint8_t WEEK_MASK =
        0x7F;

    const char* conditionToString(AlarmCondition condition)
    {
        switch (condition)
        {
            case AlarmCondition::Always:         return "always";
            case AlarmCondition::IfNotDismissed: return "if_not_dismissed";
            case AlarmCondition::IfNotSnoozed:   return "if_not_snoozed";
            case AlarmCondition::IfNoMotion:     return "if_no_motion";
        }

        return nullptr;
    }

    bool conditionFromJson(JsonVariantConst value, AlarmCondition& condition)
    {
        if (value.is<const char*>())
        {
            const char* name = value.as<const char*>();

            if (strcmp(name, "always") == 0)
                condition = AlarmCondition::Always;
            else if (strcmp(name, "if_not_dismissed") == 0)
                condition = AlarmCondition::IfNotDismissed;
            else if (strcmp(name, "if_not_snoozed") == 0)
                condition = AlarmCondition::IfNotSnoozed;
            else if (strcmp(name, "if_no_motion") == 0)
                condition = AlarmCondition::IfNoMotion;
            else
                return false;

            return true;
        }

        if (!value.is<uint8_t>())
            return false;

        const uint8_t raw = value.as<uint8_t>();

        if (raw > static_cast<uint8_t>(AlarmCondition::IfNoMotion))
            return false;

        condition = static_cast<AlarmCondition>(raw);
        return true;
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
    _initialized = false;

    _activeCount = 0;

    _runtime = Runtime{};

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
        return false;


    if (!createDirectory())
        return false;


    if (!reload())
        return false;


    _initialized = true;

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
    // RUNNING ALARM
    // --------------------------------------------------------

    if (_runtime.active)
    {
        updateRuntime();
        return;
    }


    // --------------------------------------------------------
    // SNOOZE
    // --------------------------------------------------------

    if (_snooze.active)
    {
        const uint32_t nowMs =
            millis();

        if (
            static_cast<int32_t>(
                nowMs - _snooze.untilMs
            ) >= 0
        )
        {
            const String alarmId =
                _snooze.alarmId;

            _snooze = SnoozeState{};


            Alarm alarm;

            if (
                loadFromSD(
                    alarmId,
                    alarm
                )
            )
            {
                const int8_t index =
                    findActive(alarm.id);

                if (
                    index >= 0 &&
                    alarm.enabled
                )
                {
                    const time_t now =
                        currentLocalTimestamp();

                    start(
                        static_cast<uint8_t>(index),
                        alarm,
                        now,
                        0
                    );
                }
            }
        }

        return;
    }


    // --------------------------------------------------------
    // NORMAL SCHEDULING
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


        Alarm alarm;

        if (
            !loadFromSD(
                active.id,
                alarm
            )
        )
        {
            continue;
        }


        if (!alarm.enabled)
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
            ) *
            1000LL;


        start(
            i,
            alarm,
            t0,
            elapsed
        );


        break;
    }
}


// ============================================================
// INITIALIZED
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
    _triggerCallback =
        std::move(callback);
}


void AlarmManager::setPhaseCallback(
    PhaseCallback callback
)
{
    _phaseCallback =
        std::move(callback);
}


void AlarmManager::setFinishCallback(
    FinishCallback callback
)
{
    _finishCallback =
        std::move(callback);
}


// ============================================================
// CREATE
// ============================================================

bool AlarmManager::create(
    const Alarm& alarm
)
{
    if (!_initialized)
        return false;


    // The caller must assign an ID before creating an alarm.  Keeping the
    // argument by reference avoids copying a large Alarm onto loopTask's
    // already constrained stack.
    if (alarm.id.isEmpty())
        return false;


    if (
        !validate(alarm)
    )
    {
        return false;
    }


    if (exists(alarm.id))
        return false;


    if (
        alarm.enabled &&
        _activeCount >= AlarmConfig::MAX_ALARMS
    )
    {
        return false;
    }




    if (
        !saveToSD(alarm)
    )
    {
        return false;
    }


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
// UPDATE
// ============================================================

bool AlarmManager::update(
    const Alarm& alarm
)
{
    if (!_initialized)
        return false;


    if (
        !validate(alarm)
    )
    {
        return false;
    }


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


    // --------------------------------------------------------
    // Capacity check
    // --------------------------------------------------------

    if (
        !wasEnabled &&
        willBeEnabled &&
        _activeCount >= AlarmConfig::MAX_ALARMS
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // Stop runtime first
    //
    // This is important for one-shot alarms:
    // finish() must not operate on the newly written alarm.
    // --------------------------------------------------------

    if (runtimeSameAlarm)
    {
        finish();
    }


    // --------------------------------------------------------
    // Save new alarm
    // --------------------------------------------------------

    if (
        !saveToSD(alarm)
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // Update active list
    // --------------------------------------------------------

    removeActive(alarm.id);


    if (willBeEnabled)
    {
        if (!addActive(alarm))
        {
            return false;
        }
    }


    return true;
}


// ============================================================
// REMOVE
// ============================================================

bool AlarmManager::remove(
    const String& id
)
{
    if (!_initialized)
        return false;


    if (id.isEmpty())
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
    if (!_initialized)
        return false;


    if (id.isEmpty())
        return false;


    return loadFromSD(
        id,
        alarm
    );
}


// ============================================================
// EXISTS
// ============================================================
bool AlarmManager::exists(const String& id)
{
    if (!_initialized)
        return false;

    if (id.isEmpty())
        return false;

    return _sd.fileExists(pathFor(id));
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


    if (alarms == nullptr)
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


        String filename =
            files[i].path;


        const int slash =
            filename.lastIndexOf('/');


        if (slash >= 0)
        {
            filename =
                filename.substring(
                    slash + 1
                );
        }


        if (
            filename.endsWith(".json")
        )
        {
            filename.remove(
                filename.length() - 5
            );
        }


        std::unique_ptr<Alarm> alarm(
            new (std::nothrow) Alarm()
        );

        if (!alarm)
            return false;

        if (
            loadFromSD(
                filename,
                *alarm
            )
        )
        {
            alarms[count++] =
                *alarm;
        }
    }


    return true;
}


// ============================================================
// ENABLE
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


// ============================================================
// DISABLE
// ============================================================

bool AlarmManager::disable(
    const String& id
)
{
    return setEnabled(
        id,
        false
    );
}


// ============================================================
// SET ENABLED
// ============================================================

bool AlarmManager::setEnabled(
    const String& id,
    bool enabled
)
{
    if (!_initialized)
        return false;


    Alarm alarm;

    if (
        !loadFromSD(
            id,
            alarm
        )
    )
    {
        return false;
    }


    if (alarm.enabled == enabled)
        return true;


    if (
        enabled &&
        !alarm.enabled &&
        _activeCount >= AlarmConfig::MAX_ALARMS
    )
    {
        return false;
    }


    const bool runtimeSameAlarm =
        _runtime.active &&
        _runtime.alarmId == id;


    if (runtimeSameAlarm)
    {
        finish();
    }


    alarm.enabled =
        enabled;


    if (
        !saveToSD(alarm)
    )
    {
        return false;
    }


    removeActive(id);


    if (enabled)
    {
        if (!addActive(alarm))
            return false;
    }


    return true;
}


// ============================================================
// ACTIVE COUNT
// ============================================================

uint8_t AlarmManager::activeCount() const
{
    return _activeCount;
}


// ============================================================
// GET ACTIVE INFO
// ============================================================

bool AlarmManager::getActiveInfo(
    uint8_t index,
    String& id,
    AlarmTime& time,
    uint8_t& repeatMask
) const
{
    if (
        index >= _activeCount
    )
    {
        return false;
    }


    const ActiveAlarm& active =
        _activeAlarms[index];


    id =
        active.id;

    time =
        active.time;

    repeatMask =
        active.repeatMask;


    return true;
}


// ============================================================
// FIND ACTIVE
// ============================================================

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
            _activeAlarms[i].id ==
            id
        )
        {
            return static_cast<int8_t>(i);
        }
    }


    return -1;
}


// ============================================================
// ADD ACTIVE
// ============================================================

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


    active.id =
        alarm.id;

    active.time =
        alarm.time;

    active.repeatMask =
        alarm.repeatMask;

    active.enabled =
        alarm.enabled;

    active.earliestOffsetMs =
        earliestOffset(alarm);


    ++_activeCount;


    return true;
}


// ============================================================
// REMOVE ACTIVE
// ============================================================

bool AlarmManager::removeActive(
    const String& id
)
{
    const int8_t index =
        findActive(id);


    if (index < 0)
        return false;


    const uint8_t i =
        static_cast<uint8_t>(index);


    for (
        uint8_t j = i;
        j + 1 < _activeCount;
        ++j
    )
    {
        _activeAlarms[j] =
            _activeAlarms[j + 1];
    }


    _activeAlarms[
        _activeCount - 1
    ] = ActiveAlarm{};


    --_activeCount;


    return true;
}


// ============================================================
// CLEAR ACTIVE
// ============================================================

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
// CREATE DIRECTORY
// ============================================================

bool AlarmManager::createDirectory()
{
    if (!_sd.isReady())
        return false;


    return _sd.createDirectory(
        ALARM_DIRECTORY
    );
}


// ============================================================
// PATH
// ============================================================

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


// ============================================================
// SAVE TO SD
// ============================================================

bool AlarmManager::saveToSD(
    const Alarm& alarm
)
{
    if (!_sd.isReady())
        return false;


    if (alarm.id.isEmpty())
        return false;


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


// ============================================================
// LOAD FROM SD
// ============================================================

bool AlarmManager::loadFromSD(
    const String& id,
    Alarm& alarm
)
{
    alarm = Alarm{};


    if (!_sd.isReady())
        return false;


    if (id.isEmpty())
        return false;


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


    // --------------------------------------------------------
    // File name and JSON ID must match.
    // --------------------------------------------------------

    if (alarm.id != id)
    {
        alarm = Alarm{};
        return false;
    }


    return true;
}


// ============================================================
// DELETE FROM SD
// ============================================================

bool AlarmManager::deleteFromSD(
    const String& id
)
{
    if (!_sd.isReady())
        return false;


    if (id.isEmpty())
        return false;


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


        String filename =
            files[i].path;


        const int slash =
            filename.lastIndexOf('/');


        if (slash >= 0)
        {
            filename =
                filename.substring(
                    slash + 1
                );
        }


        if (
            !filename.endsWith(".json")
        )
        {
            continue;
        }


        filename.remove(
            filename.length() - 5
        );


        Alarm alarm;

        if (
            !loadFromSD(
                filename,
                alarm
            )
        )
        {
            continue;
        }


        if (!alarm.enabled)
            continue;


        if (
            _activeCount >=
            AlarmConfig::MAX_ALARMS
        )
        {
            break;
        }


        addActive(alarm);
    }


    return true;
}


// ============================================================
// SERIALIZE
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

        object["condition"] = conditionToString(phase.condition);


        // ----------------------------------------------------
        // MATRIX
        // ----------------------------------------------------

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


        // ----------------------------------------------------
        // AUDIO
        // ----------------------------------------------------

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


        // ----------------------------------------------------
        // COB
        // ----------------------------------------------------

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
// DESERIALIZE
// ============================================================

bool AlarmManager::deserialize(
    JsonDocument& document,
    Alarm& alarm
) const
{
    alarm = Alarm{};


    if (
        !document["id"].is<String>()
    )
    {
        return false;
    }


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


    // --------------------------------------------------------
    // TIME
    // --------------------------------------------------------

    JsonObject time =
        document["time"].as<JsonObject>();


    if (time.isNull())
        return false;


    alarm.time.hour =
        time["hour"] |
        0;

    alarm.time.minute =
        time["minute"] |
        0;

    alarm.time.second =
        time["second"] |
        0;


    // --------------------------------------------------------
    // PHASES
    // --------------------------------------------------------

    JsonArray phases =
        document["phases"].as<JsonArray>();


    if (phases.isNull())
    {
        alarm.phaseCount = 0;
        return validate(alarm);
    }


    const size_t phaseCount =
        phases.size();


    if (
        phaseCount >
        AlarmConfig::MAX_PHASES
    )
    {
        return false;
    }


    alarm.phaseCount =
        static_cast<uint8_t>(
            phaseCount
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


        JsonVariantConst condition = object["condition"];

        // A missing value is kept compatible with the old API and means
        // Always. Both the old numeric enum and the web UI's readable names
        // are accepted.
        if (!condition.isNull() &&
            !conditionFromJson(condition, phase.condition))
        {
            return false;
        }


        // ----------------------------------------------------
        // MATRIX
        // ----------------------------------------------------

        JsonObject matrix =
            object["matrix"].as<JsonObject>();


        if (!matrix.isNull())
        {
            phase.matrix.enabled =
                matrix["enabled"] |
                false;

            phase.matrix.effectId =
                matrix["effectId"] |
                String();

            phase.matrix.start =
                matrix["start"] |
                0;

            phase.matrix.end =
                matrix["end"] |
                0;

            phase.matrix.speedMs =
                matrix["speedMs"] |
                0;

            phase.matrix.durationMs =
                matrix["durationMs"] |
                0;
        }


        // ----------------------------------------------------
        // AUDIO
        // ----------------------------------------------------

        JsonObject audio =
            object["audio"].as<JsonObject>();


        if (!audio.isNull())
        {
            phase.audio.enabled =
                audio["enabled"] |
                false;

            phase.audio.effectId =
                audio["effectId"] |
                String();

            phase.audio.start =
                audio["start"] |
                0;

            phase.audio.end =
                audio["end"] |
                0;

            phase.audio.speedMs =
                audio["speedMs"] |
                0;

            phase.audio.durationMs =
                audio["durationMs"] |
                0;

            phase.audio.loop =
                audio["loop"] |
                false;
        }


        // ----------------------------------------------------
        // COB
        // ----------------------------------------------------

        JsonObject cob =
            object["cob"].as<JsonObject>();


        if (!cob.isNull())
        {
            phase.cob.enabled =
                cob["enabled"] |
                false;

            phase.cob.effectId =
                cob["effectId"] |
                String();

            phase.cob.start =
                cob["start"] |
                0;

            phase.cob.end =
                cob["end"] |
                0;

            phase.cob.speedMs =
                cob["speedMs"] |
                0;

            phase.cob.durationMs =
                cob["durationMs"] |
                0;

            phase.cob.maxDurationMs =
                cob["maxDurationMs"] |
                0;
        }
    }


    return validate(alarm);
}


// ============================================================
// VALIDATE
// ============================================================

bool AlarmManager::validate(
    const Alarm& alarm
) const
{
    if (alarm.id.isEmpty())
        return false;


    // --------------------------------------------------------
    // ID
    //
    // Только безопасные символы.
    // ID напрямую используется в пути файла.
    // --------------------------------------------------------

    for (
        size_t i = 0;
        i < alarm.id.length();
        ++i
    )
    {
        const char c =
            alarm.id[i];


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


    // --------------------------------------------------------
    // TIME
    // --------------------------------------------------------

    if (alarm.time.hour > 23)
        return false;

    if (alarm.time.minute > 59)
        return false;

    if (alarm.time.second > 59)
        return false;


    // --------------------------------------------------------
    // REPEAT MASK
    // --------------------------------------------------------

    if (
        alarm.repeatMask &
        ~WEEK_MASK
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // PHASE COUNT
    // --------------------------------------------------------

    if (
        alarm.phaseCount >
        AlarmConfig::MAX_PHASES
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // PHASE VALIDATION
    // --------------------------------------------------------

    for (
        uint8_t i = 0;
        i < alarm.phaseCount;
        ++i
    )
    {
        const AlarmPhase& phase =
            alarm.phases[i];


        const uint8_t condition =
            static_cast<uint8_t>(
                phase.condition
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
// IS RUNNING
// ============================================================

bool AlarmManager::isRunning() const
{
    return _runtime.active;
}


// ============================================================
// CURRENT ALARM
// ============================================================

const Alarm* AlarmManager::currentAlarm() const
{
    if (!_runtime.active)
        return nullptr;


    return &_runtime.alarm;
}


// ============================================================
// CURRENT PHASE
// ============================================================

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


// ============================================================
// CURRENT PHASE INDEX
// ============================================================

uint8_t AlarmManager::currentPhaseIndex() const
{
    return _runtime.phaseIndex;
}


// ============================================================
// ELAPSED
// ============================================================

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
// DISMISS
// ============================================================

bool AlarmManager::dismiss()
{
    if (!_runtime.active)
        return false;


    _runtime.dismissed =
        true;


    finish();


    return true;
}


// ============================================================
// SNOOZE
// ============================================================

bool AlarmManager::snooze(
    uint32_t durationMs
)
{
    if (!_runtime.active)
        return false;


    if (durationMs == 0)
        return false;


    if (
        durationMs >
        MAX_SNOOZE_MS
    )
    {
        return false;
    }


    const String alarmId =
        _runtime.alarmId;


    _runtime.snoozed =
        true;


    _snooze.active =
        true;

    _snooze.alarmId =
        alarmId;

    _snooze.untilMs =
        millis() +
        durationMs;


    /*
     * finish() теперь видит snoozed=true
     * и НЕ отключает one-shot alarm.
     *
     * Поэтому после истечения snooze
     * AlarmManager сможет снова загрузить
     * alarm из SD и запустить его.
     */

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


    const String alarmId =
        _runtime.alarmId;


    const bool oneShot =
        _runtime.alarm.repeatMask == 0;


    const bool snoozed =
        _runtime.snoozed;


    /*
     * One-shot alarm отключается только
     * при настоящем завершении.
     *
     * При snooze alarm остаётся enabled.
     */

    if (
        oneShot &&
        !snoozed
    )
    {
        std::unique_ptr<Alarm> alarm(
            new (std::nothrow) Alarm()
        );

        if (!alarm)
            return;

        if (
            loadFromSD(
                alarmId,
                *alarm
            )
        )
        {
            alarm->enabled =
                false;


            saveToSD(*alarm);
        }


        removeActive(
            alarmId
        );
    }


    notifyFinish();


    _runtime =
        Runtime{};
}


// ============================================================
// CURRENT LOCAL TIMESTAMP
// ============================================================

time_t AlarmManager::currentLocalTimestamp() const
{
    return time(nullptr);
}


// ============================================================
// MAKE LOCAL T0
// ============================================================

time_t AlarmManager::makeLocalT0(
    time_t localNow,
    const AlarmTime& timeValue
) const
{
    struct tm localTm{};


    localtime_r(
        &localNow,
        &localTm
    );


    localTm.tm_hour =
        timeValue.hour;

    localTm.tm_min =
        timeValue.minute;

    localTm.tm_sec =
        timeValue.second;

    localTm.tm_isdst =
        -1;


    return mktime(
        &localTm
    );
}


// ============================================================
// EARLIEST OFFSET
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
// DAY BIT
// ============================================================

uint8_t AlarmManager::dayBit(
    time_t timestamp
) const
{
    struct tm localTm{};


    localtime_r(
        &timestamp,
        &localTm
    );


    /*
     * tm_wday:
     *
     * 0 = Sunday
     * 1 = Monday
     * ...
     * 6 = Saturday
     *
     * Alarm mask:
     *
     * 0 = Monday
     * ...
     * 6 = Sunday
     */

    if (localTm.tm_wday == 0)
        return 6;


    return static_cast<uint8_t>(
        localTm.tm_wday - 1
    );
}


// ============================================================
// IS REPEAT DAY
// ============================================================

bool AlarmManager::isRepeatDay(
    const Alarm& alarm,
    time_t timestamp
) const
{
    if (alarm.repeatMask == 0)
        return true;


    const uint8_t bit =
        dayBit(timestamp);


    return (
        alarm.repeatMask &
        (1U << bit)
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
    if (
        index >=
        AlarmConfig::MAX_ALARMS
    )
    {
        return false;
    }


    if (!active.enabled)
        return false;


    if (active.id.isEmpty())
        return false;


    /*
     * Проверяем две потенциальные даты:
     *
     * 1. сегодня;
     * 2. вчера.
     *
     * Вчера необходимо для фаз вроде:
     *
     * startOffsetMs = -600000
     *
     * когда будильник на понедельник 00:05
     * должен фактически начать работать
     * в воскресенье 23:55.
     */

    const time_t daySeconds =
        24 * 60 * 60;


    const time_t candidates[2] =
    {
        now,
        now - daySeconds
    };


    for (
        uint8_t candidateIndex = 0;
        candidateIndex < 2;
        ++candidateIndex
    )
    {
        const time_t candidateDate =
            candidates[candidateIndex];


        const time_t candidateT0 =
            makeLocalT0(
                candidateDate,
                active.time
            );


       if (
    active.repeatMask != 0 &&
    (
        active.repeatMask &
        (
            1U << dayBit(candidateT0)
        )
    ) == 0
)
{
    continue;
}


        const int64_t triggerMs =
            (
                static_cast<int64_t>(
                    candidateT0
                ) *
                1000LL
            ) +
            active.earliestOffsetMs;


        const int64_t nowMs =
            static_cast<int64_t>(
                now
            ) *
            1000LL;


        if (nowMs < triggerMs)
            continue;


        /*
         * Эта конкретная T0 уже была запущена.
         */

        if (
            _lastTriggerT0[index] ==
            candidateT0
        )
        {
            continue;
        }


        /*
         * Защита от слишком старых кандидатов.
         *
         * Без неё после выключения часов/долгой паузы
         * можно запустить старую фазу.
         *
         * Для обычного расписания достаточно
         * рассматривать occurrence не старше
         * максимальной разумной длительности.
         */

        const int64_t ageMs =
            nowMs -
            (
                static_cast<int64_t>(
                    candidateT0
                ) *
                1000LL
            );


        int64_t latestEndMs =
            0;


        bool infinite =
            false;


        /*
         * Мы не загружаем Alarm здесь специально:
         * earliestOffset уже сохранён в ActiveAlarm.
         *
         * Возраст occurrence ограничивается
         * 48 часами для повторяющихся alarm.
         */

        if (
            ageMs >
            (48LL * 60LL * 60LL * 1000LL)
        )
        {
            continue;
        }


        (void)latestEndMs;
        (void)infinite;


        t0 =
            candidateT0;


        return true;
    }


    return false;
}


// ============================================================
// HELPER FOR ACTIVE REPEAT MASK
// ============================================================

// bool isRepeatDayForActive(
//     const AlarmManager::ActiveAlarm& active,
//     time_t timestamp
// )
// {
//     if (active.repeatMask == 0)
//         return true;

//     struct tm localTm{};

//     localtime_r(
//         &timestamp,
//         &localTm
//     );

//     uint8_t bit =
//         localTm.tm_wday == 0
//             ? 6
//             : static_cast<uint8_t>(
//                 localTm.tm_wday - 1
//               );

//     return (
//         active.repeatMask &
//         (1U << bit)
//     ) != 0;
// }


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


    if (
        _runtime.active
    )
    {
        return false;
    }


    _runtime =
        Runtime{};


    _runtime.active =
        true;

    _runtime.alarmId =
        alarm.id;

    _runtime.alarm =
        alarm;

    _runtime.triggerT0 =
        t0;

    _runtime.elapsedMs =
        elapsedMs;

    _runtime.phaseElapsedMs =
        0;

    _runtime.dismissed =
        false;

    _runtime.snoozed =
        false;

    _runtime.lastUpdateMs =
        millis();


    _lastTriggerT0[index] =
        t0;


    /*
     * Trigger callback идёт ПЕРЕД phase callback.
     *
     * Controller сначала переводится
     * в active state, затем получает фазу.
     */

    notifyTrigger();


    const int8_t phaseIndex =
        findPhase(
            _runtime.alarm,
            _runtime.elapsedMs
        );


    if (phaseIndex < 0)
    {
        /*
         * Нет активной фазы.
         *
         * Это не ошибка scheduler.
         * Например alarm ещё ждёт начала первой фазы.
         */

        _runtime.phaseIndex =
            0;

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
// UPDATE RUNTIME
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
            static_cast<uint8_t>(
                newPhase
            );


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


    /*
     * Проверяем конец alarm.
     *
     * Фаза duration=0 означает:
     *
     * "держать alarm до ручного
     * dismiss/finish/snooze".
     */

    bool infinite =
        false;


    int64_t latestEnd =
        INT64_MIN;


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
// FIND PHASE
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


        if (
            !conditionPassed(
                phase
            )
        )
        {
            continue;
        }


        if (
            !phaseActive(
                phase,
                elapsedMs
            )
        )
        {
            continue;
        }


        /*
         * Более поздняя фаза имеет приоритет
         * при пересечении интервалов.
         */

        result =
            static_cast<int8_t>(
                i
            );
    }


    return result;
}


// ============================================================
// PHASE ACTIVE
// ============================================================

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


// ============================================================
// CONDITION PASSED
// ============================================================

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
            /*
             * SensorManager пока не подключён
             * к AlarmManager.
             *
             * До появления sensor callback
             * условие считается выполненным.
             */
            return true;
    }


    return false;
}


// ============================================================
// NOTIFY TRIGGER
// ============================================================

void AlarmManager::notifyTrigger()
{
    if (!_triggerCallback)
        return;


    _triggerCallback(
        _runtime.alarm
    );
}


// ============================================================
// NOTIFY PHASE
// ============================================================

void AlarmManager::notifyPhase(
    uint8_t phaseIndex
)
{
    if (!_phaseCallback)
        return;


    if (
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


// ============================================================
// NOTIFY FINISH
// ============================================================

void AlarmManager::notifyFinish()
{
    if (!_finishCallback)
        return;


    _finishCallback(
        _runtime.alarmId
    );
}


// ============================================================
// GENERATE ID
// ============================================================

String AlarmManager::generateId() const
{
    char buffer[37];


    const uint32_t a =
        esp_random();

    const uint32_t b =
        esp_random();

    const uint32_t c =
        esp_random();

    const uint32_t d =
        esp_random();


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
    if (path.isEmpty())
        return;


    if (!path.endsWith(".json"))
        return;


    String id =
        path;


    const int slash =
        id.lastIndexOf('/');


    if (slash >= 0)
    {
        id =
            id.substring(
                slash + 1
            );
    }


    id.remove(
        id.length() - 5
    );


    Alarm alarm;


    if (
        !loadFromSD(
            id,
            alarm
        )
    )
    {
        return;
    }


    if (!alarm.enabled)
        return;


    addActive(alarm);
}
