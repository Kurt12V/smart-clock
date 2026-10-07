#include "WebAlarmManager.h"

#include <ArduinoJson.h>
#include <WebServer.h>

#include "AlarmManager.h"
#include "AlarmController.h"

// ============================================================
// CONSTANTS
// ============================================================

namespace
{
    constexpr size_t JSON_DOC_SIZE = 4096;
    constexpr uint32_t MAX_SNOOZE_MS = 24UL * 60UL * 60UL * 1000UL;

    constexpr const char* API_PREFIX = "/api/alarms";
}


// ============================================================
// CONSTRUCTOR
// ============================================================

WebAlarmManager::WebAlarmManager(
    AlarmManager& alarmManager,
    AlarmController& alarmController
)
    : _alarmManager(alarmManager)
    , _alarmController(alarmController)
{
}


// ============================================================
// SETUP ROUTES
// ============================================================

void WebAlarmManager::setupRoutes(WebServer& server)
{
    // --------------------------------------------------------
    // Collection
    // --------------------------------------------------------

    server.on(
        "/api/alarms",
        HTTP_GET,
        [this, &server]()
        {
            handleGetAlarms(server);
        }
    );

    server.on(
        "/api/alarms",
        HTTP_POST,
        [this, &server]()
        {
            handleCreateAlarm(server);
        }
    );


    // --------------------------------------------------------
    // Runtime
    // --------------------------------------------------------

    server.on(
        "/api/alarms/runtime",
        HTTP_GET,
        [this, &server]()
        {
            handleRuntime(server);
        }
    );

    server.on(
        "/api/alarms/runtime/dismiss",
        HTTP_POST,
        [this, &server]()
        {
            handleDismiss(server);
        }
    );

    server.on(
        "/api/alarms/runtime/snooze",
        HTTP_POST,
        [this, &server]()
        {
            handleSnooze(server);
        }
    );

    server.on(
        "/api/alarms/runtime/stop",
        HTTP_POST,
        [this, &server]()
        {
            handleStop(server);
        }
    );
}


// ============================================================
// DYNAMIC REQUEST ROUTER
// ============================================================

bool WebAlarmManager::handleDynamicRequest(
    WebServer& server
)
{
    const String uri = server.uri();

    // --------------------------------------------------------
    // We only handle /api/alarms/*
    // --------------------------------------------------------

    if (
        uri != API_PREFIX &&
        !uri.startsWith(String(API_PREFIX) + "/")
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // Collection itself is already handled by server.on()
    // --------------------------------------------------------

    if (uri == API_PREFIX)
    {
        return false;
    }


    String tail = uri.substring(
        strlen(API_PREFIX) + 1
    );

    // Remove trailing slash.
    while (
        tail.length() > 0 &&
        tail.endsWith("/")
    )
    {
        tail.remove(
            tail.length() - 1
        );
    }


    // --------------------------------------------------------
    // Empty path
    // --------------------------------------------------------

    if (tail.isEmpty())
    {
        return false;
    }


    // --------------------------------------------------------
    // Runtime routes
    //
    // These are registered explicitly in setupRoutes().
    // Do not interpret "runtime" as an alarm UUID.
    // --------------------------------------------------------

    if (tail == "runtime")
    {
        sendError(
            server,
            405,
            "Method not allowed"
        );

        return true;
    }

    if (tail.startsWith("runtime/"))
    {
        sendError(
            server,
            405,
            "Method not allowed"
        );

        return true;
    }


    // --------------------------------------------------------
    // ENABLED
    //
    // POST /api/alarms/{id}/enabled
    // --------------------------------------------------------

    const String enabledSuffix = "/enabled";

    if (tail.endsWith(enabledSuffix))
    {
        if (server.method() != HTTP_POST)
        {
            sendError(
                server,
                405,
                "Method not allowed"
            );

            return true;
        }

        const String alarmId = tail.substring(
            0,
            tail.length() - enabledSuffix.length()
        );

        if (!isValidAlarmId(alarmId))
        {
            sendError(
                server,
                400,
                "Invalid alarm id"
            );

            return true;
        }

        handleSetEnabled(
            server
        );

        return true;
    }


    // --------------------------------------------------------
    // There must be exactly one UUID segment.
    // --------------------------------------------------------

    if (tail.indexOf('/') >= 0)
    {
        sendError(
            server,
            404,
            "Alarm endpoint not found"
        );

        return true;
    }


    // --------------------------------------------------------
    // Alarm ID
    // --------------------------------------------------------

    const String alarmId = tail;

    if (!isValidAlarmId(alarmId))
    {
        sendError(
            server,
            400,
            "Invalid alarm id"
        );

        return true;
    }


    // --------------------------------------------------------
    // Method dispatch
    // --------------------------------------------------------

    switch (server.method())
    {
        case HTTP_GET:
            handleGetAlarm(server);
            return true;

        case HTTP_PUT:
            handleUpdateAlarm(server);
            return true;

        case HTTP_DELETE:
            handleDeleteAlarm(server);
            return true;

        case HTTP_POST:
            sendError(
                server,
                405,
                "Method not allowed"
            );

            return true;

        default:
            sendError(
                server,
                405,
                "Method not allowed"
            );

            return true;
    }
}


// ============================================================
// GET /api/alarms
// ============================================================

void WebAlarmManager::handleGetAlarms(
    WebServer& server
)
{
    sendAlarmList(server);
}


// ============================================================
// GET /api/alarms/{id}
// ============================================================

void WebAlarmManager::handleGetAlarm(
    WebServer& server
)
{
    const String id = getAlarmIdFromRequest(
        server
    );

    if (!isValidAlarmId(id))
    {
        sendError(
            server,
            400,
            "Invalid alarm id"
        );

        return;
    }


    Alarm alarm;

    if (!_alarmManager.getAlarm(id, alarm))
    {
        sendError(
            server,
            404,
            "Alarm not found"
        );

        return;
    }


    sendAlarm(
        server,
        alarm
    );
}


// ============================================================
// POST /api/alarms
// ============================================================

void WebAlarmManager::handleCreateAlarm(
    WebServer& server
)
{
    Alarm alarm;

    if (!parseAlarmFromRequest(server, alarm))
    {
        return;
    }


    // --------------------------------------------------------
    // Empty ID means AlarmManager generates UUID v4.
    // --------------------------------------------------------

    if (!alarm.id.isEmpty())
    {
        if (!isValidAlarmId(alarm.id))
        {
            sendError(
                server,
                400,
                "Invalid alarm id"
            );

            return;
        }
    }


    if (!_alarmManager.createAlarm(alarm))
    {
        sendError(
            server,
            400,
            "Failed to create alarm"
        );

        return;
    }


    sendAlarm(
        server,
        alarm
    );
}


// ============================================================
// PUT /api/alarms/{id}
// ============================================================

void WebAlarmManager::handleUpdateAlarm(
    WebServer& server
)
{
    const String id = getAlarmIdFromRequest(
        server
    );

    if (!isValidAlarmId(id))
    {
        sendError(
            server,
            400,
            "Invalid alarm id"
        );

        return;
    }


    Alarm alarm;

    if (!parseAlarmFromRequest(server, alarm))
    {
        return;
    }


    // --------------------------------------------------------
    // URL ID is authoritative.
    // --------------------------------------------------------

    alarm.id = id;


    if (!_alarmManager.updateAlarm(alarm))
    {
        sendError(
            server,
            404,
            "Alarm not found"
        );

        return;
    }


    sendAlarm(
        server,
        alarm
    );
}


// ============================================================
// DELETE /api/alarms/{id}
// ============================================================

void WebAlarmManager::handleDeleteAlarm(
    WebServer& server
)
{
    const String id = getAlarmIdFromRequest(
        server
    );

    if (!isValidAlarmId(id))
    {
        sendError(
            server,
            400,
            "Invalid alarm id"
        );

        return;
    }


    if (!_alarmManager.removeAlarm(id))
    {
        sendError(
            server,
            404,
            "Alarm not found"
        );

        return;
    }


    sendOk(server);
}


// ============================================================
// POST /api/alarms/{id}/enabled
// ============================================================

void WebAlarmManager::handleSetEnabled(
    WebServer& server
)
{
    const String id = getAlarmIdFromRequest(
        server
    );

    if (!isValidAlarmId(id))
    {
        sendError(
            server,
            400,
            "Invalid alarm id"
        );

        return;
    }


    if (!server.hasArg("plain"))
    {
        sendError(
            server,
            400,
            "Request body required"
        );

        return;
    }


    DynamicJsonDocument doc(
        512
    );

    const DeserializationError error =
        deserializeJson(
            doc,
            server.arg("plain")
        );

    if (error)
    {
        sendError(
            server,
            400,
            "Invalid JSON"
        );

        return;
    }


    if (!doc["enabled"].is<bool>())
    {
        sendError(
            server,
            400,
            "Field 'enabled' must be boolean"
        );

        return;
    }


    const bool enabled =
        doc["enabled"].as<bool>();


    if (!_alarmManager.setEnabled(
            id,
            enabled
        ))
    {
        sendError(
            server,
            404,
            "Alarm not found"
        );

        return;
    }


    Alarm alarm;

    if (!_alarmManager.getAlarm(
            id,
            alarm
        ))
    {
        sendError(
            server,
            404,
            "Alarm not found"
        );

        return;
    }


    sendAlarm(
        server,
        alarm
    );
}


// ============================================================
// GET /api/alarms/runtime
// ============================================================

void WebAlarmManager::handleRuntime(
    WebServer& server
)
{
    DynamicJsonDocument doc(
        JSON_DOC_SIZE
    );

    doc["active"] =
        _alarmController.isActive();

    doc["running"] =
        _alarmManager.isRunning();


    if (_alarmManager.isRunning())
    {
        const Alarm* alarm =
            _alarmManager.currentAlarm();

        if (alarm)
        {
            doc["id"] =
                alarm->id;

            doc["name"] =
                alarm->name;

            doc["elapsedMs"] =
                _alarmManager.elapsedMs();
        }
    }
    else
    {
        doc["id"] = "";
        doc["name"] = "";
        doc["elapsedMs"] = 0;
    }


    String response;

    serializeJson(
        doc,
        response
    );


    server.send(
        200,
        "application/json",
        response
    );
}


// ============================================================
// POST /api/alarms/runtime/dismiss
// ============================================================

void WebAlarmManager::handleDismiss(
    WebServer& server
)
{
    if (!_alarmController.isActive())
    {
        sendError(
            server,
            409,
            "No active alarm"
        );

        return;
    }


    if (!_alarmController.dismiss())
    {
        sendError(
            server,
            409,
            "Failed to dismiss alarm"
        );

        return;
    }


    sendOk(server);
}


// ============================================================
// POST /api/alarms/runtime/snooze
// ============================================================

void WebAlarmManager::handleSnooze(
    WebServer& server
)
{
    if (!_alarmController.isActive())
    {
        sendError(
            server,
            409,
            "No active alarm"
        );

        return;
    }


    if (!server.hasArg("plain"))
    {
        sendError(
            server,
            400,
            "Request body required"
        );

        return;
    }


    DynamicJsonDocument doc(
        512
    );

    const DeserializationError error =
        deserializeJson(
            doc,
            server.arg("plain")
        );

    if (error)
    {
        sendError(
            server,
            400,
            "Invalid JSON"
        );

        return;
    }


    if (!doc["durationMs"].is<uint32_t>())
    {
        sendError(
            server,
            400,
            "Field 'durationMs' must be uint32"
        );

        return;
    }


    const uint32_t durationMs =
        doc["durationMs"].as<uint32_t>();


    if (
        durationMs == 0 ||
        durationMs > MAX_SNOOZE_MS
    )
    {
        sendError(
            server,
            400,
            "Invalid snooze duration"
        );

        return;
    }


    if (!_alarmController.snooze(
            durationMs
        ))
    {
        sendError(
            server,
            409,
            "Failed to snooze alarm"
        );

        return;
    }


    sendOk(server);
}


// ============================================================
// POST /api/alarms/runtime/stop
// ============================================================

void WebAlarmManager::handleStop(
    WebServer& server
)
{
    if (!_alarmController.isActive())
    {
        sendError(
            server,
            409,
            "No active alarm"
        );

        return;
    }


    _alarmController.stop();

    sendOk(server);
}


// ============================================================
// PARSE ALARM
// ============================================================

bool WebAlarmManager::parseAlarmFromRequest(
    WebServer& server,
    Alarm& alarm
)
{
    if (!server.hasArg("plain"))
    {
        sendError(
            server,
            400,
            "Request body required"
        );

        return false;
    }


    DynamicJsonDocument doc(
        JSON_DOC_SIZE
    );

    const DeserializationError error =
        deserializeJson(
            doc,
            server.arg("plain")
        );

    if (error)
    {
        sendError(
            server,
            400,
            "Invalid JSON"
        );

        return false;
    }


    // --------------------------------------------------------
    // schemaVersion
    // --------------------------------------------------------

    if (doc["schemaVersion"].is<uint16_t>())
    {
        alarm.schemaVersion =
            doc["schemaVersion"].as<uint16_t>();
    }
    else
    {
        alarm.schemaVersion = 1;
    }


    // --------------------------------------------------------
    // ID
    // --------------------------------------------------------

    if (
        doc["id"].is<const char*>() &&
        !doc["id"].isNull()
    )
    {
        alarm.id =
            doc["id"].as<const char*>();
    }
    else
    {
        alarm.id = String();
    }


    // --------------------------------------------------------
    // Name
    // --------------------------------------------------------

    if (!doc["name"].is<const char*>())
    {
        sendError(
            server,
            400,
            "Field 'name' is required"
        );

        return false;
    }

    alarm.name =
        doc["name"].as<const char*>();


    // --------------------------------------------------------
    // Enabled
    // --------------------------------------------------------

    if (!doc["enabled"].is<bool>())
    {
        sendError(
            server,
            400,
            "Field 'enabled' is required"
        );

        return false;
    }

    alarm.enabled =
        doc["enabled"].as<bool>();


    // --------------------------------------------------------
    // Time
    // --------------------------------------------------------

    JsonObject time =
        doc["time"].as<JsonObject>();

    if (time.isNull())
    {
        sendError(
            server,
            400,
            "Field 'time' is required"
        );

        return false;
    }


    if (
        !time["hour"].is<uint8_t>() ||
        !time["minute"].is<uint8_t>() ||
        !time["second"].is<uint8_t>()
    )
    {
        sendError(
            server,
            400,
            "Invalid time"
        );

        return false;
    }


    alarm.time.hour =
        time["hour"].as<uint8_t>();

    alarm.time.minute =
        time["minute"].as<uint8_t>();

    alarm.time.second =
        time["second"].as<uint8_t>();


    if (
        alarm.time.hour > 23 ||
        alarm.time.minute > 59 ||
        alarm.time.second > 59
    )
    {
        sendError(
            server,
            400,
            "Invalid time range"
        );

        return false;
    }


    // --------------------------------------------------------
    // Repeat mask
    // --------------------------------------------------------

    if (doc["repeatMask"].is<uint8_t>())
    {
        alarm.repeatMask =
            doc["repeatMask"].as<uint8_t>();
    }
    else
    {
        alarm.repeatMask = 0;
    }


    // --------------------------------------------------------
    // Effects
    //
    // Empty string = no effect.
    // --------------------------------------------------------

    if (
        doc["matrixEffect"].is<const char*>() &&
        !doc["matrixEffect"].isNull()
    )
    {
        alarm.matrixEffect =
            doc["matrixEffect"].as<const char*>();
    }
    else
    {
        alarm.matrixEffect = "";
    }


    if (
        doc["cobEffect"].is<const char*>() &&
        !doc["cobEffect"].isNull()
    )
    {
        alarm.cobEffect =
            doc["cobEffect"].as<const char*>();
    }
    else
    {
        alarm.cobEffect = "";
    }


    if (
        doc["audioEffect"].is<const char*>() &&
        !doc["audioEffect"].isNull()
    )
    {
        alarm.audioEffect =
            doc["audioEffect"].as<const char*>();
    }
    else
    {
        alarm.audioEffect = "";
    }


    // --------------------------------------------------------
    // Basic string limits
    //
    // AlarmManager performs final validation as well.
    // --------------------------------------------------------

    if (alarm.name.length() > 64)
    {
        sendError(
            server,
            400,
            "Alarm name too long"
        );

        return false;
    }

    if (alarm.matrixEffect.length() > 64)
    {
        sendError(
            server,
            400,
            "Matrix effect too long"
        );

        return false;
    }

    if (alarm.cobEffect.length() > 64)
    {
        sendError(
            server,
            400,
            "COB effect too long"
        );

        return false;
    }

    if (alarm.audioEffect.length() > 64)
    {
        sendError(
            server,
            400,
            "Audio effect too long"
        );

        return false;
    }


    return true;
}


// ============================================================
// SEND SINGLE ALARM
// ============================================================

void WebAlarmManager::sendAlarm(
    WebServer& server,
    const Alarm& alarm
)
{
    DynamicJsonDocument doc(
        JSON_DOC_SIZE
    );

    doc["schemaVersion"] =
        alarm.schemaVersion;

    doc["id"] =
        alarm.id;

    doc["name"] =
        alarm.name;

    doc["enabled"] =
        alarm.enabled;


    JsonObject time =
        doc.createNestedObject("time");

    time["hour"] =
        alarm.time.hour;

    time["minute"] =
        alarm.time.minute;

    time["second"] =
        alarm.time.second;


    doc["repeatMask"] =
        alarm.repeatMask;

    doc["matrixEffect"] =
        alarm.matrixEffect;

    doc["cobEffect"] =
        alarm.cobEffect;

    doc["audioEffect"] =
        alarm.audioEffect;


    String response;

    serializeJson(
        doc,
        response
    );


    server.send(
        200,
        "application/json",
        response
    );
}


// ============================================================
// SEND ALARM LIST
// ============================================================

void WebAlarmManager::sendAlarmList(
    WebServer& server
)
{
    Alarm alarms[
        AlarmConfig::MAX_ALARMS
    ];

    const size_t count =
        _alarmManager.listAlarms(
            alarms,
            AlarmConfig::MAX_ALARMS
        );


    DynamicJsonDocument doc(
        JSON_DOC_SIZE
    );

    JsonArray array =
        doc.createNestedArray("alarms");


    for (size_t i = 0; i < count; ++i)
    {
        JsonObject item =
            array.createNestedObject();

        item["schemaVersion"] =
            alarms[i].schemaVersion;

        item["id"] =
            alarms[i].id;

        item["name"] =
            alarms[i].name;

        item["enabled"] =
            alarms[i].enabled;


        JsonObject time =
            item.createNestedObject("time");

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


    doc["count"] =
        count;


    String response;

    serializeJson(
        doc,
        response
    );


    server.send(
        200,
        "application/json",
        response
    );
}


// ============================================================
// GET ALARM ID
// ============================================================

String WebAlarmManager::getAlarmIdFromRequest(
    WebServer& server
) const
{
    const String uri =
        server.uri();

    const String prefix =
        String(API_PREFIX) + "/";


    if (!uri.startsWith(prefix))
        return String();


    String tail =
        uri.substring(
            prefix.length()
        );


    // --------------------------------------------------------
    // Remove trailing slash.
    // --------------------------------------------------------

    while (
        tail.length() > 0 &&
        tail.endsWith("/")
    )
    {
        tail.remove(
            tail.length() - 1
        );
    }


    // --------------------------------------------------------
    // /api/alarms/{id}/enabled
    //
    // Return only ID.
    // --------------------------------------------------------

    const int slash =
        tail.indexOf('/');

    if (slash >= 0)
    {
        return tail.substring(
            0,
            slash
        );
    }


    return tail;
}


// ============================================================
// UUID VALIDATION
// ============================================================

bool WebAlarmManager::isValidAlarmId(
    const String& id
) const
{
    // UUID v4:
    //
    // xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    //

    if (id.length() != 36)
        return false;


    if (
        id.charAt(8) != '-' ||
        id.charAt(13) != '-' ||
        id.charAt(18) != '-' ||
        id.charAt(23) != '-'
    )
    {
        return false;
    }


    auto isHex =
        [](char c) -> bool
        {
            return
                (c >= '0' && c <= '9') ||
                (c >= 'a' && c <= 'f') ||
                (c >= 'A' && c <= 'F');
        };


    for (int i = 0; i < 36; ++i)
    {
        if (
            i == 8 ||
            i == 13 ||
            i == 18 ||
            i == 23
        )
        {
            continue;
        }


        if (!isHex(id.charAt(i)))
            return false;
    }


    // UUID version 4.
    if (id.charAt(14) != '4')
        return false;


    // RFC 4122 variant.
    const char variant =
        id.charAt(19);

    if (
        variant != '8' &&
        variant != '9' &&
        variant != 'a' &&
        variant != 'A' &&
        variant != 'b' &&
        variant != 'B'
    )
    {
        return false;
    }


    return true;
}


// ============================================================
// ERROR
// ============================================================

void WebAlarmManager::sendError(
    WebServer& server,
    int code,
    const char* message
)
{
    DynamicJsonDocument doc(
        512
    );

    doc["ok"] = false;
    doc["error"] = message;


    String response;

    serializeJson(
        doc,
        response
    );


    server.send(
        code,
        "application/json",
        response
    );
}


// ============================================================
// OK
// ============================================================

void WebAlarmManager::sendOk(
    WebServer& server
)
{
    DynamicJsonDocument doc(
        256
    );

    doc["ok"] = true;


    String response;

    serializeJson(
        doc,
        response
    );


    server.send(
        200,
        "application/json",
        response
    );
}