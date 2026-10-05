#include "AlarmManager.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "SDManager.h"
#include "./core/ClockSystem.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

AlarmManager::AlarmManager(
    SDManager& sd,
    ClockSystem& clock
)
    : _sd(sd),
      _clock(clock)
{
}


// ============================================================
// BEGIN
// ============================================================

bool AlarmManager::begin()
{
    _initialized = false;

    clearActive();

    _runtime = Runtime{};
    _snooze = SnoozeState{};

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
    // Уже выполняется будильник
    // --------------------------------------------------------

    if (_runtime.active)
    {
        updateRuntime();
        return;
    }


    // --------------------------------------------------------
    // Проверяем snooze
    // --------------------------------------------------------

    if (_snooze.active)
    {
        const uint32_t now = millis();

        if (
            static_cast<int32_t>(
                now - _snooze.untilMs
            ) < 0
        )
        {
            return;
        }

        const String alarmId =
            _snooze.alarmId;

        _snooze.active = false;
        _snooze.alarmId = "";
        _snooze.untilMs = 0;

        Alarm alarm;

        if (
            loadFromSD(
                alarmId,
                alarm
            )
        )
        {
            const int8_t index =
                findActive(alarmId);

            if (index >= 0)
            {
                start(
                    static_cast<uint8_t>(index),
                    alarm,
                    currentLocalTimestamp(),
                    0
                );
            }
        }

        return;
    }


    // --------------------------------------------------------
    // Текущее локальное время
    // --------------------------------------------------------

    const time_t now =
        currentLocalTimestamp();


    // --------------------------------------------------------
    // Проверяем активные будильники
    // --------------------------------------------------------

    for (
        uint8_t i = 0;
        i < _activeCount;
        ++i
    )
    {
        const ActiveAlarm& active =
            _activeAlarms[i];

        if (!active.enabled)
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


        // ----------------------------------------------------
        // ВАЖНО:
        // Полный Alarm загружается с SD только сейчас.
        // ----------------------------------------------------

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


        // ----------------------------------------------------
        // Проверяем день ещё раз после загрузки
        // ----------------------------------------------------

        if (
            !isRepeatDay(
                alarm,
                now
            )
        )
        {
            continue;
        }


        // ----------------------------------------------------
        // Реальное время относительно trigger T0
        // ----------------------------------------------------

        const int64_t nowMs =
            static_cast<int64_t>(now) *
            1000LL;

        const int64_t triggerMs =
            static_cast<int64_t>(t0) *
            1000LL;

        const int64_t elapsedMs =
            nowMs - triggerMs;


        // ----------------------------------------------------
        // Если alarm имеет отрицательный startOffset,
        // он уже может находиться внутри первой фазы.
        // ----------------------------------------------------

        start(
            i,
            alarm,
            t0,
            elapsedMs
        );

        return;
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
    Alarm alarm
)
{
    if (!_initialized)
        return false;

    if (alarm.id.isEmpty())
    {
        alarm.id =
            generateId();
    }

    if (!validate(alarm))
        return false;

    if (exists(alarm.id))
        return false;


    // --------------------------------------------------------
    // Сохраняем полный Alarm на SD
    // --------------------------------------------------------

    if (!saveToSD(alarm))
        return false;


    // --------------------------------------------------------
    // В RAM добавляем только если enabled
    // --------------------------------------------------------

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

    if (alarm.id.isEmpty())
        return false;

    if (!validate(alarm))
        return false;

    if (!exists(alarm.id))
        return false;


    // --------------------------------------------------------
    // Сохраняем новый полный Alarm
    // --------------------------------------------------------

    if (!saveToSD(alarm))
        return false;


    // --------------------------------------------------------
    // Обновляем RAM metadata
    // --------------------------------------------------------

    const int8_t activeIndex =
        findActive(alarm.id);

    if (activeIndex >= 0)
    {
        removeActive(alarm.id);
    }

    if (alarm.enabled)
    {
        addActive(alarm);
    }


    // --------------------------------------------------------
    // Если редактируется текущий alarm,
    // runtime нужно остановить.
    // --------------------------------------------------------

    if (
        _runtime.active &&
        _runtime.alarmId == alarm.id
    )
    {
        finish();
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


    // --------------------------------------------------------
    // Если текущий
    // --------------------------------------------------------

    if (
        _runtime.active &&
        _runtime.alarmId == id
    )
    {
        finish();
    }


    // --------------------------------------------------------
    // Удаляем из RAM
    // --------------------------------------------------------

    removeActive(id);


    // --------------------------------------------------------
    // Удаляем файл
    // --------------------------------------------------------

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
    if (id.isEmpty())
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

    if (alarms == nullptr)
        return false;

    if (maxCount == 0)
        return false;

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
            "/alarms"
        );


    for (
        size_t i = 0;
        i < fileCount;
        ++i
    )
    {
        if (files[i].isDir)
            continue;

        if (
            !files[i].path.endsWith(
                ".json"
            )
        )
        {
            continue;
        }

        if (count >= maxCount)
            break;


        // ----------------------------------------------------
        // Получаем ID из имени файла
        // ----------------------------------------------------

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

        if (!filename.endsWith(".json"))
            continue;

        const String id =
            filename.substring(
                0,
                filename.length() - 5
            );


        // ----------------------------------------------------
        // Загружаем полный Alarm
        // ----------------------------------------------------

        Alarm alarm;

        if (
            loadFromSD(
                id,
                alarm
            )
        )
        {
            alarms[count] =
                alarm;

            ++count;
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

    alarm.enabled = enabled;

    if (!saveToSD(alarm))
        return false;


    // --------------------------------------------------------
    // Удаляем старые RAM metadata
    // --------------------------------------------------------

    removeActive(id);


    // --------------------------------------------------------
    // Добавляем если enabled
    // --------------------------------------------------------

    if (enabled)
    {
        addActive(alarm);
    }


    // --------------------------------------------------------
    // Если выключили текущий alarm
    // --------------------------------------------------------

    if (
        !enabled &&
        _runtime.active &&
        _runtime.alarmId == id
    )
    {
        finish();
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
    if (index >= _activeCount)
        return false;

    const ActiveAlarm& alarm =
        _activeAlarms[index];

    id =
        alarm.id;

    time =
        alarm.time;

    repeatMask =
        alarm.repeatMask;

    return true;
}


// ============================================================
// FIND ACTIVE
// ============================================================

int8_t AlarmManager::findActive(
    const String& id
) const
{
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


// ============================================================
// ADD ACTIVE
// ============================================================

bool AlarmManager::addActive(
    const Alarm& alarm
)
{
    if (!alarm.enabled)
        return false;

    if (alarm.id.isEmpty())
        return false;


    // Уже есть

    if (
        findActive(alarm.id) >= 0
    )
    {
        return true;
    }


    if (
        _activeCount >=
        AlarmConfig::MAX_ALARMS
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


    _lastTriggerT0[_activeCount] =
        0;


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
        uint8_t n = i;
        n + 1 < _activeCount;
        ++n
    )
    {
        _activeAlarms[n] =
            _activeAlarms[n + 1];

        _lastTriggerT0[n] =
            _lastTriggerT0[n + 1];
    }


    --_activeCount;


    _activeAlarms[
        _activeCount
    ] = ActiveAlarm{};

    _lastTriggerT0[
        _activeCount
    ] = 0;


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

        _lastTriggerT0[i] =
            0;
    }

    _activeCount = 0;
}


// ============================================================
// CREATE DIRECTORY
// ============================================================

bool AlarmManager::createDirectory()
{
    return _sd.createDirectory(
        "/alarms"
    );
}


// ============================================================
// PATH
// ============================================================

String AlarmManager::pathFor(
    const String& id
) const
{
    return String("/alarms/") +
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


    String json;

    const size_t written =
        serializeJson(
            document,
            json
        );


    if (written == 0)
        return false;


    return _sd.writeFile(
        pathFor(alarm.id),
        json
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
    if (!_sd.isReady())
        return false;

    if (id.isEmpty())
        return false;


    String json;

    if (
        !_sd.readFile(
            pathFor(id),
            json
        )
    )
    {
        return false;
    }


    if (json.isEmpty())
        return false;


    JsonDocument document;


    const DeserializationError error =
        deserializeJson(
            document,
            json
        );


    if (error)
        return false;


    return deserialize(
        document,
        alarm
    );
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
// RELOAD ACTIVE
// ============================================================

bool AlarmManager::reload()
{
    clearActive();

    if (!_sd.isReady())
        return false;


    SDFileEntry files[
        AlarmConfig::MAX_ALARMS
    ];


    const size_t count =
        _sd.listFiles(
            files,
            AlarmConfig::MAX_ALARMS,
            1,
            "/alarms"
        );


    for (
        size_t i = 0;
        i < count;
        ++i
    )
    {
        if (files[i].isDir)
            continue;

        if (
            !files[i].path.endsWith(
                ".json"
            )
        )
        {
            continue;
        }


        scanFile(
            files[i].path
        );
    }


    return true;
}


// ============================================================
// SCAN FILE
// ============================================================

void AlarmManager::scanFile(
    const String& path
)
{
    if (
        !path.endsWith(".json")
    )
    {
        return;
    }


    int slash =
        path.lastIndexOf('/');


    String filename =
        path;


    if (slash >= 0)
    {
        filename =
            path.substring(
                slash + 1
            );
    }


    if (!filename.endsWith(".json"))
        return;


    const String id =
        filename.substring(
            0,
            filename.length() - 5
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


    // --------------------------------------------------------
    // TIME
    // --------------------------------------------------------

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


    // --------------------------------------------------------
    // PHASES
    // --------------------------------------------------------

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


        JsonObject p =
            phases.add<JsonObject>();


        p["startOffsetMs"] =
            phase.startOffsetMs;

        p["durationMs"] =
            phase.durationMs;


        // ----------------------------------------------------
        // CONDITION
        // ----------------------------------------------------

        switch (phase.condition)
        {
            case AlarmCondition::Always:
                p["condition"] =
                    "always";
                break;

            case AlarmCondition::IfNotDismissed:
                p["condition"] =
                    "ifNotDismissed";
                break;

            case AlarmCondition::IfNotSnoozed:
                p["condition"] =
                    "ifNotSnoozed";
                break;

            case AlarmCondition::IfNoMotion:
                p["condition"] =
                    "ifNoMotion";
                break;
        }


        // ----------------------------------------------------
        // MATRIX
        // ----------------------------------------------------

        JsonObject matrix =
            p["matrix"].to<JsonObject>();


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
            p["audio"].to<JsonObject>();


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
            p["cob"].to<JsonObject>();

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
    alarm =
        Alarm{};


    // --------------------------------------------------------
    // BASIC
    // --------------------------------------------------------

    alarm.schemaVersion =
        document["schemaVersion"] |
        1;


    alarm.id =
        document["id"] |
        "";

    alarm.name =
        document["name"] |
        "";

    alarm.enabled =
        document["enabled"] |
        false;


    // --------------------------------------------------------
    // TIME
    // --------------------------------------------------------

    JsonObjectConst time =
        document["time"];


    alarm.time.hour =
        time["hour"] |
        0;

    alarm.time.minute =
        time["minute"] |
        0;

    alarm.time.second =
        time["second"] |
        0;


    alarm.repeatMask =
        document["repeatMask"] |
        0;


    // --------------------------------------------------------
    // PHASES
    // --------------------------------------------------------

    JsonArrayConst phases =
        document["phases"];


    alarm.phaseCount = 0;


    if (phases.isNull())
        return validate(alarm);


    for (
        JsonObjectConst p : phases
    )
    {
        if (
            alarm.phaseCount >=
            AlarmConfig::MAX_PHASES
        )
        {
            break;
        }


        AlarmPhase& phase =
            alarm.phases[
                alarm.phaseCount
            ];


        phase.startOffsetMs =
            p["startOffsetMs"] |
            0;

        phase.durationMs =
            p["durationMs"] |
            0;


        // ----------------------------------------------------
        // CONDITION
        // ----------------------------------------------------

        const String condition =
            p["condition"] |
            "always";


        if (
            condition == "ifNotDismissed"
        )
        {
            phase.condition =
                AlarmCondition::IfNotDismissed;
        }
        else if (
            condition == "ifNotSnoozed"
        )
        {
            phase.condition =
                AlarmCondition::IfNotSnoozed;
        }
        else if (
            condition == "ifNoMotion"
        )
        {
            phase.condition =
                AlarmCondition::IfNoMotion;
        }
        else
        {
            phase.condition =
                AlarmCondition::Always;
        }


        // ----------------------------------------------------
        // MATRIX
        // ----------------------------------------------------

        JsonObjectConst matrix =
            p["matrix"];


        phase.matrix.enabled =
            matrix["enabled"] |
            false;

        phase.matrix.effectId =
            matrix["effectId"] |
            "";

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


        // ----------------------------------------------------
        // AUDIO
        // ----------------------------------------------------

        JsonObjectConst audio =
            p["audio"];


        phase.audio.enabled =
            audio["enabled"] |
            false;

        phase.audio.effectId =
            audio["effectId"] |
            "";

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


        // ----------------------------------------------------
        // COB
        // ----------------------------------------------------

        JsonObjectConst cob =
            p["cob"];


        phase.cob.enabled =
            cob["enabled"] |
            false;

        phase.cob.effectId =
            cob["effectId"] |
            "";

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


        ++alarm.phaseCount;
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
    if (alarm.id.length() == 0)
        return false;


    if (alarm.time.hour > 23)
        return false;

    if (alarm.time.minute > 59)
        return false;

    if (alarm.time.second > 59)
        return false;


    if (
        alarm.phaseCount >
        AlarmConfig::MAX_PHASES
    )
    {
        return false;
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
    if (_runtime.elapsedMs <= 0)
        return 0;

    if (
        _runtime.elapsedMs >
        UINT32_MAX
    )
    {
        return UINT32_MAX;
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

    _runtime.dismissed = true;

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


    const String alarmId =
        _runtime.alarmId;


    _runtime.snoozed = true;


    _snooze.active = true;

    _snooze.alarmId =
        alarmId;

    _snooze.untilMs =
        millis() + durationMs;


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


    // --------------------------------------------------------
    // One-shot alarm после завершения отключается.
    // --------------------------------------------------------

    if (oneShot)
    {
        Alarm alarm;

        if (
            loadFromSD(
                alarmId,
                alarm
            )
        )
        {
            alarm.enabled = false;

            saveToSD(alarm);
        }

        removeActive(alarmId);
    }


    // --------------------------------------------------------
    // Callback
    // --------------------------------------------------------

    notifyFinish();


    // --------------------------------------------------------
    // Runtime очищается после callback.
    // --------------------------------------------------------

    _runtime =
        Runtime{};
}


// ============================================================
// CURRENT LOCAL TIME
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
    const AlarmTime& time
) const
{
    struct tm localTm{};

    localtime_r(
        &localNow,
        &localTm
    );


    localTm.tm_hour =
        time.hour;

    localTm.tm_min =
        time.minute;

    localTm.tm_sec =
        time.second;


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


    int32_t result =
        alarm.phases[0].startOffsetMs;


    for (
        uint8_t i = 1;
        i < alarm.phaseCount;
        ++i
    )
    {
        result =
            min(
                result,
                alarm.phases[i].startOffsetMs
            );
    }


    return result;
}


// ============================================================
// DAY BIT
//
// Monday = 0
// Tuesday = 1
// ...
// Sunday = 6
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


    // tm_wday:
    // Sunday = 0
    // Monday = 1
    // ...
    // Saturday = 6

    if (localTm.tm_wday == 0)
        return 6;


    return static_cast<uint8_t>(
        localTm.tm_wday - 1
    );
}


// ============================================================
// REPEAT DAY
// ============================================================

bool AlarmManager::isRepeatDay(
    const Alarm& alarm,
    time_t timestamp
) const
{
    // 0 = one shot

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
    if (index >= _activeCount)
        return false;


    if (!active.enabled)
        return false;


    t0 =
        makeLocalT0(
            now,
            active.time
        );


    // --------------------------------------------------------
    // Проверяем день
    // --------------------------------------------------------

    if (
        active.repeatMask != 0
    )
    {
        const uint8_t bit =
            dayBit(now);


        if (
            !(
                active.repeatMask &
                (1U << bit)
            )
        )
        {
            return false;
        }
    }


    // --------------------------------------------------------
    // Самая ранняя фаза может начаться раньше времени alarm.
    // --------------------------------------------------------

    const int64_t triggerMs =
        static_cast<int64_t>(t0) *
        1000LL +
        active.earliestOffsetMs;


    const int64_t nowMs =
        static_cast<int64_t>(now) *
        1000LL;


    if (nowMs < triggerMs)
        return false;


    // --------------------------------------------------------
    // Уже срабатывал в этот T0
    // --------------------------------------------------------

    if (
        _lastTriggerT0[index] == t0
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // One-shot больше одного раза не запускаем
    // --------------------------------------------------------

    if (
        active.repeatMask == 0 &&
        _lastTriggerT0[index] != 0
    )
    {
        return false;
    }


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
    if (index >= _activeCount)
        return false;


    if (!alarm.enabled)
        return false;


    // --------------------------------------------------------
    // Создаём runtime
    // --------------------------------------------------------

    _runtime =
        Runtime{};


    _runtime.active =
        true;

    _runtime.alarmId =
        alarm.id;

    _runtime.alarm =
        alarm;

    _runtime.phaseIndex =
        0;

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


    // --------------------------------------------------------
    // Находим текущую фазу
    // --------------------------------------------------------

    const int8_t phase =
        findPhase(
            _runtime.alarm,
            _runtime.elapsedMs
        );


    if (phase >= 0)
    {
        _runtime.phaseIndex =
            static_cast<uint8_t>(
                phase
            );


        const AlarmPhase& current =
            _runtime.alarm.phases[
                _runtime.phaseIndex
            ];


        _runtime.phaseElapsedMs =
            max<int64_t>(
                0,
                _runtime.elapsedMs -
                current.startOffsetMs
            );


        notifyPhase(
            _runtime.phaseIndex
        );
    }


    // --------------------------------------------------------
    // Trigger callback
    // --------------------------------------------------------

    notifyTrigger();


    return true;
}


// ============================================================
// UPDATE RUNTIME
// ============================================================

void AlarmManager::updateRuntime()
{
    if (!_runtime.active)
        return;


    // --------------------------------------------------------
    // Delta millis
    // --------------------------------------------------------

    const uint32_t now =
        millis();


    const uint32_t delta =
        now -
        _runtime.lastUpdateMs;


    _runtime.lastUpdateMs =
        now;


    _runtime.elapsedMs +=
        static_cast<int64_t>(
            delta
        );


    // --------------------------------------------------------
    // Нет фаз
    // --------------------------------------------------------

    if (
        _runtime.alarm.phaseCount == 0
    )
    {
        finish();
        return;
    }


    // --------------------------------------------------------
    // Ищем активную фазу
    // --------------------------------------------------------

    const int8_t phase =
        findPhase(
            _runtime.alarm,
            _runtime.elapsedMs
        );


    if (phase >= 0)
    {
        const uint8_t phaseIndex =
            static_cast<uint8_t>(
                phase
            );


        if (
            phaseIndex !=
            _runtime.phaseIndex
        )
        {
            _runtime.phaseIndex =
                phaseIndex;

            _runtime.phaseElapsedMs =
                0;


            notifyPhase(
                phaseIndex
            );
        }


        const AlarmPhase& current =
            _runtime.alarm.phases[
                _runtime.phaseIndex
            ];


        _runtime.phaseElapsedMs =
            max<int64_t>(
                0,
                _runtime.elapsedMs -
                current.startOffsetMs
            );
    }


    // --------------------------------------------------------
    // Определяем конец последней фазы
    // --------------------------------------------------------

    bool infinite = false;

    int64_t latestEnd =
        INT64_MIN;


    for (
        uint8_t i = 0;
        i < _runtime.alarm.phaseCount;
        ++i
    )
    {
        const AlarmPhase& phaseData =
            _runtime.alarm.phases[i];


        if (
            phaseData.durationMs == 0
        )
        {
            infinite = true;
            continue;
        }


        const int64_t end =
            static_cast<int64_t>(
                phaseData.startOffsetMs
            ) +
            static_cast<int64_t>(
                phaseData.durationMs
            );


        latestEnd =
            max(
                latestEnd,
                end
            );
    }


    // --------------------------------------------------------
    // Все конечные фазы завершены
    // --------------------------------------------------------

    if (
        !infinite &&
        latestEnd != INT64_MIN &&
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
            !phaseActive(
                phase,
                elapsedMs
            )
        )
        {
            continue;
        }


        if (
            !conditionPassed(phase)
        )
        {
            continue;
        }


        // Последняя подходящая фаза
        // имеет приоритет.

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
    if (
        elapsedMs <
        phase.startOffsetMs
    )
    {
        return false;
    }


    // duration == 0
    // означает бесконечную фазу.

    if (
        phase.durationMs == 0
    )
    {
        return true;
    }


    const int64_t end =
        static_cast<int64_t>(
            phase.startOffsetMs
        ) +
        static_cast<int64_t>(
            phase.durationMs
        );


    return elapsedMs < end;
}


// ============================================================
// CONDITION
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
            // Пока SensorManager не подключён.
            return true;
    }


    return false;
}


// ============================================================
// NOTIFY TRIGGER
// ============================================================

void AlarmManager::notifyTrigger()
{
    if (_triggerCallback)
    {
        _triggerCallback(
            _runtime.alarm
        );
    }
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

        "%08lx-%04lx-%04lx-%04lx-%08lx",

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

        static_cast<unsigned long>(d)
    );


    return String(buffer);
}