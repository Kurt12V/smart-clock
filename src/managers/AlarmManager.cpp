#include "AlarmManager.h"

#include <cstring>
#include <ctime>

// ============================================================
// CONSTRUCTOR
// ============================================================

AlarmManager::AlarmManager(
    ClockSystem& clockSystem,
    SDManager& sdManager
)
    : _clockSystem(clockSystem),
      _sdManager(sdManager),
      _scheduleCount(0),
      _activeAlarm(),
      _initialized(false),
      _triggerCallback(nullptr)
{
}

// ============================================================
// BEGIN
// ============================================================

bool AlarmManager::begin()
{
    if (_initialized)
        return true;

    Serial.println();
    Serial.println("[AlarmManager] Starting...");

    if (!_sdManager.isReady())
    {
        Serial.println(
            "[AlarmManager] SD card is not ready"
        );

        return false;
    }

    if (!ensureFileExists())
    {
        Serial.println(
            "[AlarmManager] Failed to prepare alarms file"
        );

        return false;
    }

    if (!loadSchedule())
    {
        Serial.println(
            "[AlarmManager] Failed to load alarm schedule"
        );

        return false;
    }

    _activeAlarm = ActiveAlarmData();

    _initialized = true;

    Serial.print(
        "[AlarmManager] Loaded alarms: "
    );

    Serial.println(
        _scheduleCount
    );

    Serial.println(
        "[AlarmManager] Ready"
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

    if (_activeAlarm.active)
        return;

    checkAlarms();
}

// ============================================================
// ENSURE FILE EXISTS
// ============================================================

bool AlarmManager::ensureFileExists()
{
    if (!_sdManager.isReady())
        return false;

    SDCard& card =
        _sdManager.card();

    if (!card.exists("/config"))
    {
        fs::FS& filesystem =
            card.fs();

        if (!filesystem.mkdir("/config"))
        {
            Serial.println(
                "[AlarmManager] Failed to create /config"
            );

            return false;
        }
    }

    if (!card.exists(ALARMS_PATH))
    {
        return createEmptyFile();
    }

    return true;
}

// ============================================================
// CREATE EMPTY FILE
// ============================================================

bool AlarmManager::createEmptyFile()
{
    if (!_sdManager.isReady())
        return false;

    fs::FS& filesystem =
        _sdManager.card().fs();

    File file =
        filesystem.open(
            ALARMS_PATH,
            FILE_WRITE
        );

    if (!file)
    {
        Serial.println(
            "[AlarmManager] Failed to create alarms.json"
        );

        return false;
    }

    JsonDocument document;

    document["schemaVersion"] =
        SCHEMA_VERSION;

    document["alarms"]
        .to<JsonArray>();

    const size_t written =
        serializeJson(
            document,
            file
        );

    file.close();

    if (written == 0)
    {
        Serial.println(
            "[AlarmManager] Failed to write alarms.json"
        );

        return false;
    }

    return true;
}

// ============================================================
// LOAD SCHEDULE
// ============================================================

bool AlarmManager::loadSchedule()
{
    JsonDocument document;

    if (!readDocument(document))
        return false;

    return parseSchedule(document);
}

// ============================================================
// PARSE SCHEDULE
// ============================================================

bool AlarmManager::parseSchedule(
    JsonDocument& document
)
{
    _scheduleCount = 0;

    JsonArrayConst alarms =
        document["alarms"]
            .as<JsonArrayConst>();

    if (alarms.isNull())
    {
        return true;
    }

    const time_t now =
        time(nullptr);

    for (
        JsonObjectConst object : alarms
    )
    {
        if (
            _scheduleCount >=
            AlarmLimits::MAX_ALARMS
        )
        {
            break;
        }

        AlarmScheduleData& schedule =
            _schedule[_scheduleCount];

        if (!jsonToSchedule(
                object,
                schedule
            ))
        {
            Serial.println(
                "[AlarmManager] Invalid alarm schedule"
            );

            continue;
        }

        schedule.nextTrigger =
            calculateNextTrigger(
                schedule,
                now
            );

        ++_scheduleCount;
    }

    return true;
}

// ============================================================
// JSON -> SCHEDULE
// ============================================================

bool AlarmManager::jsonToSchedule(
    JsonObjectConst object,
    AlarmScheduleData& schedule
)
{
    schedule =
        AlarmScheduleData();

    const char* id =
        object["id"] | "";

    if (!id || id[0] == '\0')
        return false;

    copyString(
        schedule.id,
        sizeof(schedule.id),
        id
    );

    schedule.enabled =
        object["enabled"] | false;

    JsonObjectConst timeObject =
        object["time"]
            .as<JsonObjectConst>();

    if (!timeObject.isNull())
    {
        schedule.hour =
            timeObject["hour"] | 0;

        schedule.minute =
            timeObject["minute"] | 0;

        schedule.second =
            timeObject["second"] | 0;
    }

    schedule.repeatMask =
        object["repeatMask"] | 0;

    schedule.nextTrigger = 0;

    return true;
}

// ============================================================
// JSON -> ALARM
// ============================================================

bool AlarmManager::jsonToAlarm(
    JsonObjectConst object,
    AlarmData& alarm
)
{
    alarm =
        AlarmData();

    // --------------------------------------------------------
    // BASIC
    // --------------------------------------------------------

    const char* id =
        object["id"] | "";

    if (!id || id[0] == '\0')
        return false;

    copyString(
        alarm.id,
        sizeof(alarm.id),
        id
    );

    const char* name =
        object["name"] | "";

    copyString(
        alarm.name,
        sizeof(alarm.name),
        name
    );

    alarm.enabled =
        object["enabled"] | false;

    // --------------------------------------------------------
    // TIME
    // --------------------------------------------------------

    JsonObjectConst timeObject =
        object["time"]
            .as<JsonObjectConst>();

    if (!timeObject.isNull())
    {
        alarm.time.hour =
            timeObject["hour"] | 0;

        alarm.time.minute =
            timeObject["minute"] | 0;

        alarm.time.second =
            timeObject["second"] | 0;
    }

    // --------------------------------------------------------
    // REPEAT
    // --------------------------------------------------------

    alarm.repeatMask =
        object["repeatMask"] | 0;

    // --------------------------------------------------------
    // PHASES
    // --------------------------------------------------------

    JsonArrayConst phases =
        object["phases"]
            .as<JsonArrayConst>();

    alarm.phaseCount = 0;

    if (phases.isNull())
        return true;

    for (
        JsonObjectConst phaseObject : phases
    )
    {
        if (
            alarm.phaseCount >=
            AlarmLimits::MAX_PHASES
        )
        {
            break;
        }

        AlarmPhase& phase =
            alarm.phases[
                alarm.phaseCount
            ];

        phase =
            AlarmPhase();

        // ----------------------------------------------------
        // PHASE
        // ----------------------------------------------------

        phase.startOffsetMs =
            phaseObject["startOffsetMs"] | 0LL;

        phase.durationMs =
            phaseObject["durationMs"] | 0;

        const char* condition =
            phaseObject["condition"] | "always";

        copyString(
            phase.condition,
            sizeof(phase.condition),
            condition
        );

        // ----------------------------------------------------
        // MATRIX
        // ----------------------------------------------------

        JsonObjectConst matrix =
            phaseObject["matrix"]
                .as<JsonObjectConst>();

        if (!matrix.isNull())
        {
            phase.matrix.enabled =
                matrix["enabled"] | false;

            const char* effectId =
                matrix["effectId"] | "";

            copyString(
                phase.matrix.effectId,
                sizeof(phase.matrix.effectId),
                effectId
            );

            phase.matrix.start =
                matrix["start"] | 0;

            phase.matrix.end =
                matrix["end"] | 0;

            phase.matrix.speedMs =
                matrix["speedMs"] | 0;

            phase.matrix.durationMs =
                matrix["durationMs"] | 0;

            phase.matrix.loop =
                matrix["loop"] | false;

            phase.matrix.maxDurationMs =
                matrix["maxDurationMs"] | 0;
        }

        // ----------------------------------------------------
        // AUDIO
        // ----------------------------------------------------

        JsonObjectConst audio =
            phaseObject["audio"]
                .as<JsonObjectConst>();

        if (!audio.isNull())
        {
            phase.audio.enabled =
                audio["enabled"] | false;

            const char* effectId =
                audio["effectId"] | "";

            copyString(
                phase.audio.effectId,
                sizeof(phase.audio.effectId),
                effectId
            );

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

            phase.audio.maxDurationMs =
                audio["maxDurationMs"] | 0;
        }

        // ----------------------------------------------------
        // COB
        // ----------------------------------------------------

        JsonObjectConst cob =
            phaseObject["cob"]
                .as<JsonObjectConst>();

        if (!cob.isNull())
        {
            phase.cob.enabled =
                cob["enabled"] | false;

            const char* effectId =
                cob["effectId"] | "";

            copyString(
                phase.cob.effectId,
                sizeof(phase.cob.effectId),
                effectId
            );

            phase.cob.start =
                cob["start"] | 0;

            phase.cob.end =
                cob["end"] | 0;

            phase.cob.speedMs =
                cob["speedMs"] | 0;

            phase.cob.durationMs =
                cob["durationMs"] | 0;

            phase.cob.loop =
                cob["loop"] | false;

            phase.cob.maxDurationMs =
                cob["maxDurationMs"] | 0;
        }

        ++alarm.phaseCount;
    }

    return true;
}

// ============================================================
// ALARM -> JSON
// ============================================================

bool AlarmManager::alarmToJson(
    JsonObject object,
    const AlarmData& alarm
)
{
    object["schemaVersion"] =
        SCHEMA_VERSION;

    object["id"] =
        alarm.id;

    object["name"] =
        alarm.name;

    object["enabled"] =
        alarm.enabled;

    // --------------------------------------------------------
    // TIME
    // --------------------------------------------------------

    JsonObject timeObject =
        object["time"]
            .to<JsonObject>();

    timeObject["hour"] =
        alarm.time.hour;

    timeObject["minute"] =
        alarm.time.minute;

    timeObject["second"] =
        alarm.time.second;

    object["repeatMask"] =
        alarm.repeatMask;

    // --------------------------------------------------------
    // PHASES
    // --------------------------------------------------------

    JsonArray phases =
        object["phases"]
            .to<JsonArray>();

    for (
        uint8_t i = 0;
        i < alarm.phaseCount;
        ++i
    )
    {
        const AlarmPhase& phase =
            alarm.phases[i];

        JsonObject phaseObject =
            phases.add<JsonObject>();

        phaseObject["startOffsetMs"] =
            phase.startOffsetMs;

        phaseObject["durationMs"] =
            phase.durationMs;

        phaseObject["condition"] =
            phase.condition;

        // ----------------------------------------------------
        // MATRIX
        // ----------------------------------------------------

        JsonObject matrix =
            phaseObject["matrix"]
                .to<JsonObject>();

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

        matrix["loop"] =
            phase.matrix.loop;

        matrix["maxDurationMs"] =
            phase.matrix.maxDurationMs;

        // ----------------------------------------------------
        // AUDIO
        // ----------------------------------------------------

        JsonObject audio =
            phaseObject["audio"]
                .to<JsonObject>();

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

        audio["maxDurationMs"] =
            phase.audio.maxDurationMs;

        // ----------------------------------------------------
        // COB
        // ----------------------------------------------------

        JsonObject cob =
            phaseObject["cob"]
                .to<JsonObject>();

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

        cob["loop"] =
            phase.cob.loop;

        cob["maxDurationMs"] =
            phase.cob.maxDurationMs;
    }

    return true;
}

// ============================================================
// READ DOCUMENT
// ============================================================

bool AlarmManager::readDocument(
    JsonDocument& document
)
{
    if (!_sdManager.isReady())
        return false;

    fs::FS& filesystem =
        _sdManager.card().fs();

    File file =
        filesystem.open(
            ALARMS_PATH,
            FILE_READ
        );

    if (!file)
    {
        Serial.println(
            "[AlarmManager] Failed to open alarms.json"
        );

        return false;
    }

    DeserializationError error =
        deserializeJson(
            document,
            file
        );

    file.close();

    if (error)
    {
        Serial.print(
            "[AlarmManager] JSON error: "
        );

        Serial.println(
            error.c_str()
        );

        return false;
    }

    return true;
}

// ============================================================
// WRITE DOCUMENT
// ============================================================

bool AlarmManager::writeDocument(
    const JsonDocument& document
)
{
    if (!_sdManager.isReady())
        return false;

    fs::FS& filesystem =
        _sdManager.card().fs();

    /*
     * FILE_WRITE в ESP32 SD может открыть существующий
     * файл в append-режиме. Поэтому сначала удаляем старый
     * файл, затем создаём новый.
     */

    if (filesystem.exists(ALARMS_PATH))
    {
        filesystem.remove(ALARMS_PATH);
    }

    File file =
        filesystem.open(
            ALARMS_PATH,
            FILE_WRITE
        );

    if (!file)
    {
        Serial.println(
            "[AlarmManager] Failed to create alarms.json"
        );

        return false;
    }

    const size_t written =
        serializeJson(
            document,
            file
        );

    file.close();

    return written > 0;
}

// ============================================================
// CREATE
// ============================================================

bool AlarmManager::create(
    const AlarmData& alarm
)
{
    if (!_initialized)
        return false;

    if (alarm.id[0] == '\0')
        return false;

    if (_scheduleCount >= AlarmLimits::MAX_ALARMS)
        return false;

    if (findSchedule(alarm.id))
        return false;

    JsonDocument document;

    if (!readDocument(document))
        return false;

    JsonArray alarms =
        document["alarms"]
            .to<JsonArray>();

    JsonObject object =
        alarms.add<JsonObject>();

    if (!alarmToJson(
            object,
            alarm
        ))
    {
        return false;
    }

    if (!writeDocument(document))
        return false;

    return loadSchedule();
}

// ============================================================
// UPDATE ALARM
// ============================================================

bool AlarmManager::updateAlarm(
    const AlarmData& alarm
)
{
    if (!_initialized)
        return false;

    if (alarm.id[0] == '\0')
        return false;

    JsonDocument document;

    if (!readDocument(document))
        return false;

    JsonArray alarms =
        document["alarms"]
            .to<JsonArray>();

    for (
        JsonObject object : alarms
    )
    {
        const char* id =
            object["id"] | "";

        if (
            strcmp(
                id,
                alarm.id
            ) == 0
        )
        {
            object.clear();

            return alarmToJson(
                       object,
                       alarm
                   ) &&
                   writeDocument(document) &&
                   loadSchedule();
        }
    }

    return false;
}

// ============================================================
// REMOVE
// ============================================================

bool AlarmManager::remove(
    const char* id
)
{
    if (!_initialized)
        return false;

    if (!id || id[0] == '\0')
        return false;

    JsonDocument document;

    if (!readDocument(document))
        return false;

    JsonArray alarms =
        document["alarms"]
            .to<JsonArray>();

    for (
        size_t i = 0;
        i < alarms.size();
        ++i
    )
    {
        JsonObject object =
            alarms[i].as<JsonObject>();

        if (object.isNull())
            continue;

        const char* alarmId =
            object["id"] | "";

        if (
            strcmp(
                alarmId,
                id
            ) == 0
        )
        {
            alarms.remove(i);

            if (!writeDocument(document))
                return false;

            return loadSchedule();
        }
    }

    return false;
}

// ============================================================
// SET ENABLED
// ============================================================

bool AlarmManager::setEnabled(
    const char* id,
    bool enabled
)
{
    if (!_initialized)
        return false;

    if (!id || id[0] == '\0')
        return false;

    JsonDocument document;

    if (!readDocument(document))
        return false;

    JsonArray alarms =
        document["alarms"]
            .to<JsonArray>();

    for (
        JsonObject object : alarms
    )
    {
        const char* alarmId =
            object["id"] | "";

        if (
            strcmp(
                alarmId,
                id
            ) == 0
        )
        {
            object["enabled"] =
                enabled;

            if (!writeDocument(document))
                return false;

            return loadSchedule();
        }
    }

    return false;
}

// ============================================================
// LOAD ALARM
// ============================================================

bool AlarmManager::loadAlarm(
    const char* id,
    AlarmData& alarm
)
{
    if (!_initialized)
        return false;

    if (!id || id[0] == '\0')
        return false;

    JsonDocument document;

    if (!readDocument(document))
        return false;

    JsonArrayConst alarms =
        document["alarms"]
            .as<JsonArrayConst>();

    if (alarms.isNull())
        return false;

    for (
        JsonObjectConst object : alarms
    )
    {
        const char* alarmId =
            object["id"] | "";

        if (
            strcmp(
                alarmId,
                id
            ) == 0
        )
        {
            return jsonToAlarm(
                object,
                alarm
            );
        }
    }

    return false;
}

// ============================================================
// FIND SCHEDULE
// ============================================================

AlarmScheduleData*
AlarmManager::findSchedule(
    const char* id
)
{
    if (!id)
        return nullptr;

    for (
        uint8_t i = 0;
        i < _scheduleCount;
        ++i
    )
    {
        if (
            strcmp(
                _schedule[i].id,
                id
            ) == 0
        )
        {
            return &_schedule[i];
        }
    }

    return nullptr;
}

// ============================================================
// FIND SCHEDULE CONST
// ============================================================

const AlarmScheduleData*
AlarmManager::findSchedule(
    const char* id
) const
{
    if (!id)
        return nullptr;

    for (
        uint8_t i = 0;
        i < _scheduleCount;
        ++i
    )
    {
        if (
            strcmp(
                _schedule[i].id,
                id
            ) == 0
        )
        {
            return &_schedule[i];
        }
    }

    return nullptr;
}

// ============================================================
// GET SCHEDULE
// ============================================================

const AlarmScheduleData*
AlarmManager::getSchedule(
    uint8_t index
) const
{
    if (index >= _scheduleCount)
        return nullptr;

    return &_schedule[index];
}

// ============================================================
// COUNT
// ============================================================

uint8_t AlarmManager::count() const
{
    return _scheduleCount;
}

// ============================================================
// ACTIVE
// ============================================================

bool AlarmManager::hasActiveAlarm() const
{
    return _activeAlarm.active;
}

// ============================================================
// GET ACTIVE ALARM
// ============================================================

const ActiveAlarmData&
AlarmManager::getActiveAlarm() const
{
    return _activeAlarm;
}

// ============================================================
// DISMISS
// ============================================================

void AlarmManager::dismiss()
{
    _activeAlarm.active = false;
    _activeAlarm.id[0] = '\0';
    _activeAlarm.startedAtMs = 0;
    _activeAlarm.currentPhase = 0;
    _activeAlarm.data = AlarmData();
}

// ============================================================
// CALLBACK
// ============================================================

void AlarmManager::setTriggerCallback(
    TriggerCallback callback
)
{
    _triggerCallback =
        callback;
}

// ============================================================
// CALCULATE NEXT TRIGGER
// ============================================================

time_t AlarmManager::calculateNextTrigger(
    const AlarmScheduleData& alarm,
    time_t from
) const
{
    if (!alarm.enabled)
        return 0;

    if (from <= 0)
        return 0;

    struct tm baseTime;

    localtime_r(
        &from,
        &baseTime
    );

    /*
     * Search the next 8 calendar days.
     */

    for (
        uint8_t dayOffset = 0;
        dayOffset <= 7;
        ++dayOffset
    )
    {
        struct tm candidate =
            baseTime;

        candidate.tm_hour =
            alarm.hour;

        candidate.tm_min =
            alarm.minute;

        candidate.tm_sec =
            alarm.second;

        candidate.tm_mday +=
            dayOffset;

        time_t candidateTime =
            mktime(&candidate);

        if (
            candidateTime <= from
        )
        {
            continue;
        }

        /*
         * ESP32 tm_wday:
         *
         * 0 = Sunday
         * 1 = Monday
         * ...
         * 6 = Saturday
         *
         * AlarmData mask:
         *
         * bit 0 = Monday
         * bit 1 = Tuesday
         * ...
         * bit 6 = Sunday
         */

        if (alarm.repeatMask != 0)
        {
            struct tm candidateLocal;

            localtime_r(
                &candidateTime,
                &candidateLocal
            );

            uint8_t alarmWeekday;

            if (candidateLocal.tm_wday == 0)
            {
                alarmWeekday = 6;
            }
            else
            {
                alarmWeekday =
                    candidateLocal.tm_wday - 1;
            }

            if (
                (
                    alarm.repeatMask &
                    (1U << alarmWeekday)
                ) == 0
            )
            {
                continue;
            }
        }
        else
        {
            /*
             * One-shot alarm.
             *
             * If today's time has already passed,
             * don't move it to tomorrow.
             */

            if (dayOffset > 0)
                return 0;
        }

        return candidateTime;
    }

    return 0;
}

// ============================================================
// CHECK ALARMS
// ============================================================

void AlarmManager::checkAlarms()
{
    const time_t now =
        time(nullptr);

    if (now <= 0)
        return;

    for (
        uint8_t i = 0;
        i < _scheduleCount;
        ++i
    )
    {
        AlarmScheduleData& schedule =
            _schedule[i];

        if (!schedule.enabled)
            continue;

        if (schedule.nextTrigger <= 0)
        {
            schedule.nextTrigger =
                calculateNextTrigger(
                    schedule,
                    now
                );

            continue;
        }

        if (
            now >= schedule.nextTrigger
        )
        {
            trigger(schedule);
            return;
        }
    }
}

// ============================================================
// TRIGGER
// ============================================================

void AlarmManager::trigger(
    AlarmScheduleData& schedule
)
{
    Serial.print(
        "[AlarmManager] Triggering: "
    );

    Serial.println(
        schedule.id
    );

    AlarmData alarm;

    if (!loadAlarm(
            schedule.id,
            alarm
        ))
    {
        Serial.println(
            "[AlarmManager] Failed to load alarm"
        );

        return;
    }

    // --------------------------------------------------------
    // ACTIVE DATA
    // --------------------------------------------------------

    _activeAlarm.active =
        true;

    copyString(
        _activeAlarm.id,
        sizeof(_activeAlarm.id),
        alarm.id
    );

    _activeAlarm.startedAtMs =
        millis();

    _activeAlarm.currentPhase =
        0;

    _activeAlarm.data =
        alarm;

    // --------------------------------------------------------
    // NEXT TRIGGER
    // --------------------------------------------------------

    const time_t now =
        time(nullptr);

    if (schedule.repeatMask != 0)
    {
        /*
         * Repeating alarm:
         * calculate next occurrence.
         */
        schedule.nextTrigger =
            calculateNextTrigger(
                schedule,
                now
            );
    }
    else
    {
        /*
         * One-shot alarm:
         * disable it.
         */

        schedule.enabled = false;
        schedule.nextTrigger = 0;

        /*
         * Persist disabled state.
         */
        setEnabled(
            schedule.id,
            false
        );
    }

    // --------------------------------------------------------
    // CALLBACK
    // --------------------------------------------------------

    if (_triggerCallback)
    {
        _triggerCallback(
            _activeAlarm.data
        );
    }
}

// ============================================================
// COPY STRING
// ============================================================

void AlarmManager::copyString(
    char* destination,
    size_t destinationSize,
    const char* source
)
{
    if (
        !destination ||
        destinationSize == 0
    )
    {
        return;
    }

    if (!source)
        source = "";

    strncpy(
        destination,
        source,
        destinationSize - 1
    );

    destination[
        destinationSize - 1
    ] = '\0';
}