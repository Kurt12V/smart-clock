#include "WebServerManager.h"

// ============================================================
// CONSTANTS
// ============================================================

namespace
{
    constexpr uint32_t MAX_SNOOZE_MS =
        24UL * 60UL * 60UL * 1000UL;
}

// ============================================================
// GET ALARMS
// ============================================================

void WebServerManager::handleGetAlarms()
{
    if (!_alarmManager)
    {
        sendError(
            503,
            "AlarmManager unavailable"
        );

        return;
    }

    sendAlarmList();
}

// ============================================================
// GET ALARM
// ============================================================

void WebServerManager::handleGetAlarm()
{
    if (!_alarmManager)
    {
        sendError(
            503,
            "AlarmManager unavailable"
        );

        return;
    }

    const String id =
        getAlarmIdFromRequest();

    if (!isValidAlarmId(id))
    {
        sendError(
            400,
            "Invalid alarm id"
        );

        return;
    }

    Alarm alarm;

    if (!_alarmManager->loadFromSD(
            id,
            alarm))
    {
        sendError(
            404,
            "Alarm not found"
        );

        return;
    }

    sendAlarm(
        alarm
    );
}

// ============================================================
// CREATE ALARM
// ============================================================

void WebServerManager::handleCreateAlarm()
{
    if (!_alarmManager)
    {
        sendError(
            503,
            "AlarmManager unavailable"
        );

        return;
    }

    Alarm alarm;

    if (!parseAlarmFromRequest(
            alarm))
    {
        return;
    }

    // --------------------------------------------------------
    // ID
    // --------------------------------------------------------

    /*
     * ID is intentionally NOT generated here.
     *
     * AlarmManager generates UUID v4 when
     * alarm.id is empty.
     */

    if (!alarm.id.isEmpty() &&
        !isValidAlarmId(alarm.id))
    {
        sendError(
            400,
            "Invalid alarm id"
        );

        return;
    }

    // --------------------------------------------------------
    // CREATE
    // --------------------------------------------------------

    if (!_alarmManager->create(
            alarm))
    {
        sendError(
            500,
            "Failed to create alarm"
        );

        return;
    }

    /*
     * AlarmManager may have generated the UUID.
     *
     * Therefore alarm.id now contains the
     * final persistent UUID.
     */

    sendAlarm(
        alarm
    );
}

// ============================================================
// UPDATE ALARM
// ============================================================

void WebServerManager::handleUpdateAlarm()
{
    if (!_alarmManager)
    {
        sendError(
            503,
            "AlarmManager unavailable"
        );

        return;
    }

    const String id =
        getAlarmIdFromRequest();

    if (!isValidAlarmId(id))
    {
        sendError(
            400,
            "Invalid alarm id"
        );

        return;
    }

    Alarm existing;

    if (!_alarmManager->get(
            id,
            existing))
    {
        sendError(
            404,
            "Alarm not found"
        );

        return;
    }

    Alarm updated =
        existing;

    if (!parseAlarmFromRequest(
            updated))
    {
        return;
    }

    updated.id =
        id;

    if (!_alarmManager->update(
            updated))
    {
        sendError(
            500,
            "Failed to update alarm"
        );

        return;
    }

    sendAlarm(
        updated
    );
}

// ============================================================
// DELETE ALARM
// ============================================================

void WebServerManager::handleDeleteAlarm()
{
    if (!_alarmManager)
    {
        sendError(
            503,
            "AlarmManager unavailable"
        );

        return;
    }

    const String id =
        getAlarmIdFromRequest();

    if (!isValidAlarmId(id))
    {
        sendError(
            400,
            "Invalid alarm id"
        );

        return;
    }

    if (!_alarmManager->remove(id))
    {
        sendError(
            404,
            "Alarm not found"
        );

        return;
    }

    sendOk();
}

// ============================================================
// ENABLE
// ============================================================

void WebServerManager::handleEnableAlarm()
{
    if (!_alarmManager)
    {
        sendError(
            503,
            "AlarmManager unavailable"
        );

        return;
    }

    const String id =
        getAlarmIdFromRequest();

    if (!isValidAlarmId(id))
    {
        sendError(
            400,
            "Invalid alarm id"
        );

        return;
    }

    if (!_alarmManager->setEnabled(
            id,
            true))
    {
        sendError(
            404,
            "Alarm not found"
        );

        return;
    }

    sendOk();
}

// ============================================================
// DISABLE
// ============================================================

void WebServerManager::handleDisableAlarm()
{
    if (!_alarmManager)
    {
        sendError(
            503,
            "AlarmManager unavailable"
        );

        return;
    }

    const String id =
        getAlarmIdFromRequest();

    if (!isValidAlarmId(id))
    {
        sendError(
            400,
            "Invalid alarm id"
        );

        return;
    }

    if (!_alarmManager->setEnabled(
            id,
            false))
    {
        sendError(
            404,
            "Alarm not found"
        );

        return;
    }

    sendOk();
}

// ============================================================
// ALARM RUNTIME
// ============================================================

void WebServerManager::handleAlarmRuntime()
{
    JsonDocument doc;

    bool active = false;

    String alarmId;

    uint32_t elapsedMs = 0;

    // --------------------------------------------------------
    // CONTROLLER
    // --------------------------------------------------------

    if (_alarmController)
    {
        active =
            _alarmController->isActive();

        if (active)
        {
            alarmId =
                _alarmController->alarmId();
        }

        if (_alarmManager)
        {
            elapsedMs =
                _alarmManager->elapsedMs();
        }
    }

    // --------------------------------------------------------
    // MANAGER FALLBACK
    // --------------------------------------------------------

    else if (_alarmManager)
    {
        active =
            _alarmManager->isRunning();

        if (active)
        {
            const Alarm* alarm =
                _alarmManager->currentAlarm();

            if (alarm)
            {
                alarmId =
                    alarm->id;
            }

            elapsedMs =
                _alarmManager->elapsedMs();
        }
    }

    // --------------------------------------------------------
    // RESPONSE
    // --------------------------------------------------------

    doc["active"] =
        active;

    doc["alarmId"] =
        alarmId;

    doc["elapsedMs"] =
        elapsedMs;

    String output;

    serializeJson(
        doc,
        output
    );

    sendJson(
        200,
        output
    );
}

// ============================================================
// DISMISS
// ============================================================

void WebServerManager::handleAlarmDismiss()
{
    bool result = false;

    if (_alarmController)
    {
        result =
            _alarmController->dismiss();
    }
    else if (_alarmManager)
    {
        result =
            _alarmManager->dismiss();
    }
    else
    {
        sendError(
            503,
            "Alarm system unavailable"
        );

        return;
    }

    Serial0.printf(
        "[WEB][ALARM][DISMISS] result=%s\n",
        result
            ? "true"
            : "false"
    );

    if (!result)
    {
        sendError(
            409,
            "Alarm dismiss failed"
        );

        return;
    }

    sendOk();
}

// ============================================================
// SNOOZE
// ============================================================

void WebServerManager::handleAlarmSnooze()
{
    uint32_t durationMs = 0;

    if (_server.hasArg(
            "durationMs"))
    {
        durationMs =
            _server.arg(
                "durationMs"
            ).toInt();
    }
    else
    {
        JsonDocument doc;

        if (!parseJson(doc))
            return;

        JsonObjectConst object =
            doc.as<JsonObjectConst>();

        if (!getUnsigned32(
                object,
                "durationMs",
                durationMs))
        {
            sendError(
                400,
                "durationMs is required"
            );

            return;
        }
    }

    if (durationMs == 0)
    {
        sendError(
            400,
            "durationMs is required"
        );

        return;
    }

    if (durationMs > MAX_SNOOZE_MS)
    {
        sendError(
            400,
            "Snooze duration too large"
        );

        return;
    }

    bool result = false;

    if (_alarmController)
    {
        result =
            _alarmController->snooze(
                durationMs
            );
    }
    else if (_alarmManager)
    {
        result =
            _alarmManager->snooze(
                durationMs
            );
    }
    else
    {
        sendError(
            503,
            "Alarm system unavailable"
        );

        return;
    }

    if (!result)
    {
        sendError(
            409,
            "Alarm snooze failed"
        );

        return;
    }

    sendOk();
}

// ============================================================
// PARSE ALARM
// ============================================================

bool WebServerManager::parseAlarmFromRequest(
    Alarm& alarm
)
{
    JsonDocument doc;

    if (!parseJson(doc))
        return false;

    if (!_alarmManager)
    {
        sendError(
            503,
            "AlarmManager unavailable"
        );

        return false;
    }

    if (!_alarmManager->deserialize(
            doc,
            alarm))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] Alarm deserialize failed"
        );

        sendError(
            400,
            "Invalid alarm JSON"
        );

        return false;
    }

    Serial0.printf(
        "[WEB][ALARM] id=%s name=%s enabled=%s "
        "time=%02u:%02u:%02u repeatMask=0x%02X\n",
        alarm.id.c_str(),
        alarm.name.c_str(),
        alarm.enabled
            ? "true"
            : "false",
        static_cast<unsigned>(
            alarm.time.hour
        ),
        static_cast<unsigned>(
            alarm.time.minute
        ),
        static_cast<unsigned>(
            alarm.time.second
        ),
        static_cast<unsigned>(
            alarm.repeatMask
        )
    );

    Serial0.printf(
        "[WEB][ALARM] matrixEffect=%s "
        "cobEffect=%s "
        "audioEffect=%s\n",
        alarm.matrixEffect.c_str(),
        alarm.cobEffect.c_str(),
        alarm.audioEffect.c_str()
    );

    return true;
}

// ============================================================
// SEND ALARM
// ============================================================

void WebServerManager::sendAlarm(
    const Alarm& alarm
)
{
    if (!_alarmManager)
    {
        sendError(
            503,
            "AlarmManager unavailable"
        );

        return;
    }

    JsonDocument doc;

    _alarmManager->serialize(
        alarm,
        doc
    );

    String output;

    serializeJson(
        doc,
        output
    );

    sendJson(
        200,
        output
    );
}

// ============================================================
// SEND ALARM LIST
// ============================================================

void WebServerManager::sendAlarmList()
{
    if (!_alarmManager)
    {
        sendError(
            503,
            "AlarmManager unavailable"
        );

        return;
    }

    Alarm alarms[
        AlarmConfig::MAX_ALARMS
    ];

    uint8_t count = 0;

    if (!_alarmManager->loadAll(
            alarms,
            AlarmConfig::MAX_ALARMS,
            count))
    {
        sendError(
            500,
            "Failed to load alarms"
        );

        return;
    }

    Serial0.printf(
        "[WEB][ALARM][LIST] count=%u\n",
        static_cast<unsigned>(
            count
        )
    );

    JsonDocument doc;

    JsonArray list =
        doc["alarms"].to<JsonArray>();

    for (
        uint8_t i = 0;
        i < count;
        ++i)
    {
        JsonObject item =
            list.add<JsonObject>();

        item["schemaVersion"] =
            alarms[i].schemaVersion;

        item["id"] =
            alarms[i].id;

        item["name"] =
            alarms[i].name;

        item["enabled"] =
            alarms[i].enabled;

        JsonObject time =
            item["time"].to<JsonObject>();

        time["hour"] =
            alarms[i].time.hour;

        time["minute"] =
            alarms[i].time.minute;

        time["second"] =
            alarms[i].time.second;

        item["repeatMask"] =
            alarms[i].repeatMask;

        item["matrixEffect"] =
            alarms[i].matrixEffect;

        item["cobEffect"] =
            alarms[i].cobEffect;

        item["audioEffect"] =
            alarms[i].audioEffect;
    }

    String output;

    serializeJson(
        doc,
        output
    );

    sendJson(
        200,
        output
    );
}