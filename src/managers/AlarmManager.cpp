#include "AlarmManager.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <cstring>
#include <limits>
#include <utility>

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

    constexpr uint8_t WEEK_MASK =
        0x7F;

    constexpr uint32_t MAX_SNOOZE_MS =
        0x7FFFFFFFUL;

    // Если основной loop был задержан,
    // допускаем максимум 60 секунд пропуска.
    constexpr uint32_t MISSED_TRIGGER_GRACE_MS =
        60UL * 1000UL;

    constexpr int64_t ALARM_MS_PER_SECOND =
        1000LL;

    constexpr int64_t ALARM_SECONDS_PER_MINUTE =
        60LL;

    constexpr int64_t ALARM_MINUTES_PER_HOUR =
        60LL;

    constexpr int64_t ALARM_HOURS_PER_DAY =
        24LL;

    constexpr int64_t ALARM_SECONDS_PER_HOUR =
        ALARM_SECONDS_PER_MINUTE *
        ALARM_MINUTES_PER_HOUR;

    constexpr int64_t ALARM_SECONDS_PER_DAY =
        ALARM_HOURS_PER_DAY *
        ALARM_SECONDS_PER_HOUR;

    // ========================================================
    // RESET
    // ========================================================

    void resetAlarm(
        Alarm& alarm
    )
    {
        alarm = Alarm{};
    }

    // ========================================================
    // LOCAL TIME
    // ========================================================

    void localTm(
        time_t timestamp,
        struct tm& value
    )
    {
        memset(
            &value,
            0,
            sizeof(value)
        );

        gmtime_r(
            &timestamp,
            &value
        );
    }

    // ========================================================
    // ALARM ID FROM FILE
    // ========================================================

    String alarmIdFromPath(
        const String& path
    )
    {
        String id = path;

        const int slash =
            id.lastIndexOf('/');

        if (slash >= 0)
        {
            id = id.substring(
                slash + 1
            );
        }

        if (!id.endsWith(".json"))
            return String();

        id.remove(
            id.length() - 5
        );

        return id;
    }

    // ========================================================
    // UUID CHARACTER VALIDATION
    // ========================================================

    bool isUuidV4(
        const String& id
    )
    {
        if (id.length() != 36)
            return false;

        for (uint8_t i = 0; i < 36; ++i)
        {
            if (
                i == 8 ||
                i == 13 ||
                i == 18 ||
                i == 23
            )
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

        // UUID version 4
        if (
            id[14] != '4'
        )
        {
            return false;
        }

        // UUID variant: 8, 9, A, B
        const char variant =
            id[19];

        if (
            !(
                variant == '8' ||
                variant == '9' ||
                variant == 'a' ||
                variant == 'A' ||
                variant == 'b' ||
                variant == 'B'
            )
        )
        {
            return false;
        }

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
    Serial0.println(
        "============================================"
    );

    Serial0.println(
        "[ALARM] AlarmManager::begin()"
    );

    Serial0.println(
        "============================================"
    );

    _initialized = false;

    clearActive();

    _runtime =
        Runtime{};

    _snooze =
        SnoozeState{};

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
        static_cast<unsigned>(
            _activeCount
        ),
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
    // Если будильник уже работает
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
            ) < 0
        )
        {
            return;
        }

        const String alarmId =
            _snooze.alarmId;

        Serial0.printf(
            "[ALARM] SNOOZE EXPIRED id=%s\n",
            alarmId.c_str()
        );

        _snooze =
            SnoozeState{};

        Alarm alarm;

        if (
            !loadFromSD(
                alarmId,
                alarm
            )
        )
        {
            Serial0.printf(
                "[ALARM] SNOOZE ERROR id=%s "
                "reason=load-failed\n",
                alarmId.c_str()
            );

            return;
        }

        if (!alarm.enabled)
        {
            Serial0.printf(
                "[ALARM] SNOOZE CANCELLED id=%s "
                "reason=disabled\n",
                alarmId.c_str()
            );

            return;
        }

        const int8_t index =
            findActive(alarmId);

        if (index < 0)
        {
            Serial0.printf(
                "[ALARM] SNOOZE ERROR id=%s "
                "reason=not-active\n",
                alarmId.c_str()
            );

            return;
        }

        const time_t now =
            currentLocalTimestamp();

        start(
            static_cast<uint8_t>(index),
            alarm,
            now
        );

        return;
    }

    // --------------------------------------------------------
    // NORMAL SCHEDULER
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

        time_t triggerT0 = 0;

        if (
            !shouldStart(
                i,
                active,
                now,
                triggerT0
            )
        )
        {
            continue;
        }

        Alarm alarm;

        if (
            !loadFromSD(
                active.id,
                alarm
            )
        )
        {
            Serial0.printf(
                "[ALARM] WARNING: "
                "cannot load id=%s\n",
                active.id.c_str()
            );

            continue;
        }

        if (!alarm.enabled)
            continue;

        if (
            start(
                i,
                alarm,
                triggerT0
            )
        )
        {
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
    Alarm& alarm
)
{
    if (!_initialized)
        return false;

    // --------------------------------------------------------
    // Generate UUID automatically
    // --------------------------------------------------------

    if (alarm.id.isEmpty())
    {
        do
        {
            alarm.id =
                generateId();

        } while (exists(alarm.id));
    }

    logAlarm(
        "CREATE",
        alarm
    );

    if (!validate(alarm))
    {
        Serial0.println(
            "[ALARM] CREATE rejected: "
            "validation failed"
        );

        return false;
    }

    if (exists(alarm.id))
    {
        Serial0.printf(
            "[ALARM] CREATE rejected: "
            "already exists id=%s\n",
            alarm.id.c_str()
        );

        return false;
    }

    if (
        alarm.enabled &&
        _activeCount >=
            AlarmConfig::MAX_ALARMS
    )
    {
        Serial0.println(
            "[ALARM] CREATE rejected: "
            "active alarm limit"
        );

        return false;
    }

    if (!saveToSD(alarm))
    {
        Serial0.println(
            "[ALARM] CREATE failed: save"
        );

        return false;
    }

    if (alarm.enabled)
    {
        if (!addActive(alarm))
        {
            deleteFromSD(
                alarm.id
            );

            return false;
        }
    }

    Serial0.printf(
        "[ALARM] CREATE OK "
        "id=%s\n",
        alarm.id.c_str()
    );

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

    if (!validate(alarm))
    {
        Serial0.println(
            "[ALARM] UPDATE rejected: "
            "validation failed"
        );

        return false;
    }

    Alarm oldAlarm;

    if (
        !get(
            alarm.id,
            oldAlarm
        )
    )
    {
        Serial0.printf(
            "[ALARM] UPDATE failed: "
            "not found id=%s\n",
            alarm.id.c_str()
        );

        return false;
    }

    if (
        !oldAlarm.enabled &&
        alarm.enabled &&
        _activeCount >=
            AlarmConfig::MAX_ALARMS
    )
    {
        return false;
    }

    const bool runtimeSame =
        _runtime.active &&
        _runtime.alarmId == alarm.id;

    if (runtimeSame)
    {
        finish();
    }

    if (!saveToSD(alarm))
        return false;

    removeActive(
        alarm.id
    );

    if (alarm.enabled)
    {
        if (!addActive(alarm))
        {
            Serial0.println(
                "[ALARM] UPDATE failed: "
                "addActive"
            );

            return false;
        }
    }

    Serial0.printf(
        "[ALARM] UPDATE OK "
        "id=%s\n",
        alarm.id.c_str()
    );

    return true;
}

// ============================================================
// REMOVE
// ============================================================

bool AlarmManager::remove(
    const String& id
)
{
    if (
        !_initialized ||
        id.isEmpty()
    )
    {
        return false;
    }

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
        _snooze =
            SnoozeState{};
    }

    if (!deleteFromSD(id))
        return false;

    removeActive(id);

    Serial0.printf(
        "[ALARM] REMOVE OK "
        "id=%s\n",
        id.c_str()
    );

    return true;
}

// ============================================================
// GET
// ============================================================

bool AlarmManager::get(
    const String& id,
    Alarm& alarm
)
{
    if (
        !_initialized ||
        id.isEmpty()
    )
    {
        return false;
    }

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
    if (
        !_sd.isReady() ||
        id.isEmpty()
    )
    {
        return false;
    }

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

        const String id =
            alarmIdFromPath(
                files[i].path
            );

        if (id.isEmpty())
            continue;

        Alarm alarm;

        if (
            loadFromSD(
                id,
                alarm
            )
        )
        {
            alarms[count++] =
                alarm;
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
    if (
        !_initialized ||
        id.isEmpty()
    )
    {
        return false;
    }

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
        _activeCount >=
            AlarmConfig::MAX_ALARMS
    )
    {
        Serial0.println(
            "[ALARM] ENABLE rejected: "
            "active limit"
        );

        return false;
    }

    if (
        _runtime.active &&
        _runtime.alarmId == id
    )
    {
        finish();
    }

    alarm.enabled =
        enabled;

    if (!saveToSD(alarm))
        return false;

    removeActive(id);

    if (enabled)
    {
        if (!addActive(alarm))
            return false;
    }

    Serial0.printf(
        "[ALARM] %s id=%s\n",
        enabled
            ? "ENABLED"
            : "DISABLED",
        id.c_str()
    );

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
// ACTIVE INFO
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

    if (!isUuidV4(alarm.id))
        return false;

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

    _lastTriggerT0[
        _activeCount
    ] = 0;

    ++_activeCount;

    Serial0.printf(
        "[ALARM] ACTIVE ADD "
        "id=%s "
        "time=%02u:%02u:%02u "
        "repeat=0x%02X\n",
        active.id.c_str(),
        active.time.hour,
        active.time.minute,
        active.time.second,
        active.repeatMask
    );

    return true;
}

// ============================================================
// REMOVE ACTIVE
// ============================================================

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

        _lastTriggerT0[i] =
            _lastTriggerT0[i + 1];
    }

    _activeAlarms[
        _activeCount - 1
    ] = ActiveAlarm{};

    _lastTriggerT0[
        _activeCount - 1
    ] = 0;

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
    if (!isUuidV4(id))
        return String();

    return String(ALARM_DIRECTORY) +
           "/" +
           id +
           ".json";
}

// ============================================================
// SAVE
// ============================================================

bool AlarmManager::saveToSD(
    const Alarm& alarm
)
{
    if (
        !_sd.isReady() ||
        !isUuidV4(alarm.id)
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

    if (
        serializeJson(
            document,
            content
        ) == 0
    )
    {
        return false;
    }

    return _sd.writeFile(
        pathFor(alarm.id),
        content
    );
}

// ============================================================
// LOAD
// ============================================================

bool AlarmManager::loadFromSD(
    const String& id,
    Alarm& alarm
)
{
    resetAlarm(alarm);

    if (
        !_sd.isReady() ||
        !isUuidV4(id)
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

    if (content.isEmpty())
        return false;

    JsonDocument document;

    const DeserializationError error =
        deserializeJson(
            document,
            content
        );

    if (error)
    {
        Serial0.printf(
            "[ALARM] JSON ERROR "
            "id=%s error=%s\n",
            id.c_str(),
            error.c_str()
        );

        return false;
    }

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
        Serial0.printf(
            "[ALARM] JSON ERROR "
            "id=%s reason=id-mismatch\n",
            id.c_str()
        );

        resetAlarm(alarm);

        return false;
    }

    return true;
}

// ============================================================
// DELETE
// ============================================================

bool AlarmManager::deleteFromSD(
    const String& id
)
{
    if (
        !_sd.isReady() ||
        !isUuidV4(id)
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

    Serial0.printf(
        "[ALARM] reload files=%u\n",
        static_cast<unsigned>(
            fileCount
        )
    );

    for (
        size_t i = 0;
        i < fileCount;
        ++i
    )
    {
        if (files[i].isDir)
            continue;

        const String id =
            alarmIdFromPath(
                files[i].path
            );

        if (!isUuidV4(id))
            continue;

        Alarm alarm;

        if (
            !loadFromSD(
                id,
                alarm
            )
        )
        {
            Serial0.printf(
                "[ALARM] reload "
                "failed id=%s\n",
                id.c_str()
            );

            continue;
        }

        if (!alarm.enabled)
            continue;

        if (
            !addActive(alarm)
        )
        {
            Serial0.println(
                "[ALARM] reload "
                "active limit reached"
            );

            break;
        }
    }

    Serial0.printf(
        "[ALARM] reload complete "
        "active=%u\n",
        static_cast<unsigned>(
            _activeCount
        )
    );

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

    document["matrixEffect"] =
        alarm.matrixEffect;

    document["cobEffect"] =
        alarm.cobEffect;

    document["audioEffect"] =
        alarm.audioEffect;

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
    resetAlarm(alarm);

    // --------------------------------------------------------
    // ID
    // --------------------------------------------------------

    JsonVariantConst id =
        document["id"];

    // null допустим только при CREATE.
    // При загрузке с SD id должен быть UUID.
    if (
        !id.isNull()
    )
    {
        if (!id.is<const char*>())
            return false;

        alarm.id =
            id.as<String>();
    }

    // --------------------------------------------------------
    // BASIC
    // --------------------------------------------------------

    alarm.schemaVersion =
        document["schemaVersion"] |
        CURRENT_SCHEMA_VERSION;

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
    // EFFECTS
    // --------------------------------------------------------

    alarm.matrixEffect =
        document["matrixEffect"] |
        String();

    alarm.cobEffect =
        document["cobEffect"] |
        String();

    alarm.audioEffect =
        document["audioEffect"] |
        String();

    // --------------------------------------------------------
    // Validate fields except UUID.
    // Empty ID is allowed here because POST may use null.
    // --------------------------------------------------------

    if (
        alarm.time.hour > 23 ||
        alarm.time.minute > 59 ||
        alarm.time.second > 59
    )
    {
        return false;
    }

    if (
        alarm.repeatMask &
        static_cast<uint8_t>(
            ~WEEK_MASK
        )
    )
    {
        return false;
    }

    return true;
}

// ============================================================
// VALIDATE
// ============================================================

bool AlarmManager::validate(
    const Alarm& alarm
) const
{
    // UUID is mandatory after CREATE.
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

    if (
        alarm.repeatMask &
        static_cast<uint8_t>(
            ~WEEK_MASK
        )
    )
    {
        return false;
    }

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
// RUNTIME
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
// ELAPSED
// ============================================================

uint32_t AlarmManager::elapsedMs() const
{
    if (!_runtime.active)
        return 0;

    return millis() -
           _runtime.startMs;
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

    if (
        durationMs == 0 ||
        durationMs > MAX_SNOOZE_MS
    )
    {
        return false;
    }

    _runtime.snoozed =
        true;

    _snooze.active =
        true;

    _snooze.alarmId =
        _runtime.alarmId;

    _snooze.untilMs =
        millis() +
        durationMs;

    Serial0.printf(
        "[ALARM] SNOOZE "
        "id=%s "
        "duration=%lu\n",
        _runtime.alarmId.c_str(),
        static_cast<unsigned long>(
            durationMs
        )
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

    const String alarmId =
        _runtime.alarmId;

    const bool oneShot =
        _runtime.alarm.repeatMask == 0;

    const bool snoozed =
        _runtime.snoozed;

    Serial0.printf(
        "[ALARM] FINISH "
        "id=%s "
        "oneShot=%s "
        "snoozed=%s\n",
        alarmId.c_str(),
        oneShot
            ? "true"
            : "false",
        snoozed
            ? "true"
            : "false"
    );

    // Одноразовый будильник после
    // dismiss/finish отключается.
    //
    // При snooze оставляем включённым.
    if (
        oneShot &&
        !snoozed
    )
    {
        Alarm alarm;

        if (
            loadFromSD(
                alarmId,
                alarm
            )
        )
        {
            alarm.enabled =
                false;

            if (
                saveToSD(alarm)
            )
            {
                Serial0.printf(
                    "[ALARM] "
                    "one-shot disabled "
                    "id=%s\n",
                    alarmId.c_str()
                );
            }
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
// CURRENT LOCAL TIME
// ============================================================

time_t AlarmManager::currentLocalTimestamp() const
{
    return _clock.getLocalTime();
}

// ============================================================
// LOCAL MIDNIGHT
// ============================================================

time_t AlarmManager::localMidnight(
    time_t timestamp
) const
{
    const int64_t seconds =
        static_cast<int64_t>(
            timestamp
        );

    int64_t days =
        seconds /
        ALARM_SECONDS_PER_DAY;

    if (
        seconds < 0 &&
        seconds %
            ALARM_SECONDS_PER_DAY != 0
    )
    {
        --days;
    }

    return static_cast<time_t>(
        days *
        ALARM_SECONDS_PER_DAY
    );
}

// ============================================================
// SHIFT DAYS
// ============================================================

time_t AlarmManager::shiftLocalDays(
    time_t timestamp,
    int32_t days
) const
{
    return static_cast<time_t>(
        static_cast<int64_t>(
            timestamp
        ) +
        static_cast<int64_t>(
            days
        ) *
        ALARM_SECONDS_PER_DAY
    );
}

// ============================================================
// MAKE T0
// ============================================================

time_t AlarmManager::makeLocalT0(
    time_t localDateTimestamp,
    const AlarmTime& time
) const
{
    const time_t midnight =
        localMidnight(
            localDateTimestamp
        );

    return static_cast<time_t>(
        static_cast<int64_t>(
            midnight
        ) +
        static_cast<int64_t>(
            time.hour
        ) *
        ALARM_SECONDS_PER_HOUR +
        static_cast<int64_t>(
            time.minute
        ) *
        ALARM_SECONDS_PER_MINUTE +
        static_cast<int64_t>(
            time.second
        )
    );
}

// ============================================================
// DAY BIT
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
    // Sunday = 0
    // Monday = 1
    // ...
    // Saturday = 6

    if (value.tm_wday == 0)
        return 6;

    return static_cast<uint8_t>(
        value.tm_wday - 1
    );
}

// ============================================================
// REPEAT DAY
// ============================================================

bool AlarmManager::isRepeatDay(
    const Alarm& alarm,
    time_t localTimestamp
) const
{
    // repeatMask == 0
    // означает одноразовый будильник.
    //
    // Для scheduler это означает:
    // ближайшее наступление времени.

    if (alarm.repeatMask == 0)
        return true;

    const uint8_t bit =
        dayBit(
            localTimestamp
        );

    return (
        alarm.repeatMask &
        static_cast<uint8_t>(
            1U << bit
        )
    ) != 0;
}

// ============================================================
// SHOULD START
// ============================================================

bool AlarmManager::shouldStart(
    uint8_t index,
    const ActiveAlarm& active,
    time_t now,
    time_t& triggerT0
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

    const time_t today =
        localMidnight(now);

    const time_t candidates[3] =
    {
        shiftLocalDays(
            today,
            -1
        ),

        today,

        shiftLocalDays(
            today,
            1
        )
    };

    const int64_t nowMs =
        static_cast<int64_t>(
            now
        ) *
        ALARM_MS_PER_SECOND;

    bool found = false;

    time_t bestT0 = 0;

    int64_t bestAge =
        std::numeric_limits<
            int64_t
        >::max();

    for (
        uint8_t i = 0;
        i < 3;
        ++i
    )
    {
        const time_t occurrenceDate =
            candidates[i];

        // Для повторяющегося будильника
        // день относится к самому будильнику.
        if (
            active.repeatMask != 0 &&
            !(
                active.repeatMask &
                static_cast<uint8_t>(
                    1U <<
                    dayBit(
                        occurrenceDate
                    )
                )
            )
        )
        {
            continue;
        }

        const time_t t0 =
            makeLocalT0(
                occurrenceDate,
                active.time
            );

        const int64_t triggerMs =
            static_cast<int64_t>(
                t0
            ) *
            ALARM_MS_PER_SECOND;

        const int64_t ageMs =
            nowMs -
            triggerMs;

        // Время ещё не наступило.
        if (ageMs < 0)
            continue;

        // Уже запускали именно это
        // календарное срабатывание.
        if (
            _lastTriggerT0[index] ==
            t0
        )
        {
            continue;
        }

        // Если loop был заблокирован слишком долго,
        // не запускаем старый будильник.
        if (
            static_cast<uint64_t>(
                ageMs
            ) >
            MISSED_TRIGGER_GRACE_MS
        )
        {
            continue;
        }

        // Берём ближайшее прошедшее
        // срабатывание.
        if (
            !found ||
            ageMs < bestAge
        )
        {
            found = true;

            bestAge =
                ageMs;

            bestT0 =
                t0;
        }
    }

    if (!found)
        return false;

    triggerT0 =
        bestT0;

    logSchedule(
        "MATCH",
        active.id,
        triggerT0
    );

    Serial0.printf(
        "[ALARM][MATCH] "
        "id=%s "
        "alarm=%02u:%02u:%02u "
        "now=%lld "
        "t0=%lld "
        "age=%lldms\n",
        active.id.c_str(),
        active.time.hour,
        active.time.minute,
        active.time.second,
        static_cast<long long>(
            now
        ),
        static_cast<long long>(
            triggerT0
        ),
        static_cast<long long>(
            bestAge
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
    time_t triggerT0
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

    _runtime =
        Runtime{};

    _runtime.active =
        true;

    _runtime.alarmId =
        alarm.id;

    _runtime.alarm =
        alarm;

    _runtime.triggerT0 =
        triggerT0;

    _runtime.startMs =
        millis();

    _lastTriggerT0[index] =
        triggerT0;

    logAlarm(
        "START",
        alarm
    );

    Serial0.printf(
        "[ALARM][START] "
        "id=%s "
        "t0=%lld\n",
        alarm.id.c_str(),
        static_cast<long long>(
            triggerT0
        )
    );

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

    // Сейчас у будильника нет фаз
    // и нет встроенного времени завершения.
    //
    // Он считается активным до:
    //
    // dismiss()
    // snooze()
    // finish()
    //
    // Поэтому здесь пока только
    // контролируем runtime.
}

// ============================================================
// LOG SCHEDULE
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
        "[ALARM] %s "
        "id=%s "
        "local=%04d-%02d-%02d "
        "%02d:%02d:%02d "
        "ts=%lld\n",
        event,
        id.c_str(),
        value.tm_year + 1900,
        value.tm_mon + 1,
        value.tm_mday,
        value.tm_hour,
        value.tm_min,
        value.tm_sec,
        static_cast<long long>(
            timestamp
        )
    );
}

// ============================================================
// LOG ALARM
// ============================================================

void AlarmManager::logAlarm(
    const char* event,
    const Alarm& alarm
) const
{
    Serial0.printf(
        "[ALARM] %s "
        "id=%s "
        "name=%s "
        "enabled=%s "
        "time=%02u:%02u:%02u "
        "repeat=0x%02X "
        "matrix=%s "
        "cob=%s "
        "audio=%s\n",

        event,

        alarm.id.c_str(),

        alarm.name.c_str(),

        alarm.enabled
            ? "true"
            : "false",

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
// NOTIFY FINISH
// ============================================================

void AlarmManager::notifyFinish()
{
    if (_finishCallback)
    {
        _finishCallback(
            _runtime.alarmId
        );
    }
}

// ============================================================
// UUID V4
// ============================================================

String AlarmManager::generateId() const
{
    uint8_t bytes[16];

    for (
        uint8_t i = 0;
        i < sizeof(bytes);
        i += 4
    )
    {
        const uint32_t value =
            esp_random();

        bytes[i + 0] =
            static_cast<uint8_t>(
                value
            );

        bytes[i + 1] =
            static_cast<uint8_t>(
                value >> 8
            );

        bytes[i + 2] =
            static_cast<uint8_t>(
                value >> 16
            );

        bytes[i + 3] =
            static_cast<uint8_t>(
                value >> 24
            );
    }

    // UUID version 4
    bytes[6] =
        static_cast<uint8_t>(
            (bytes[6] & 0x0F) |
            0x40
        );

    // RFC 4122 variant
    bytes[8] =
        static_cast<uint8_t>(
            (bytes[8] & 0x3F) |
            0x80
        );

    char buffer[37];

    snprintf(
        buffer,
        sizeof(buffer),

        "%02X%02X%02X%02X-"
        "%02X%02X-"
        "%02X%02X-"
        "%02X%02X-"
        "%02X%02X%02X%02X%02X%02X",

        bytes[0],
        bytes[1],
        bytes[2],
        bytes[3],

        bytes[4],
        bytes[5],

        bytes[6],
        bytes[7],

        bytes[8],
        bytes[9],

        bytes[10],
        bytes[11],
        bytes[12],
        bytes[13],
        bytes[14],
        bytes[15]
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
    const String id =
        alarmIdFromPath(path);

    if (!isUuidV4(id))
        return;

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