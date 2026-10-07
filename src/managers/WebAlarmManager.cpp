#include "WebAlarmManager.h"

#include <ArduinoJson.h>


namespace
{
    constexpr uint32_t MAX_SNOOZE_MS =
        24UL * 60UL * 60UL * 1000UL;
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
// ROUTES
// ============================================================

void WebAlarmManager::setupRoutes(
    WebServer& server
)
{
    // ========================================================
    // MODERN API
    // ========================================================

    server.on(
        "/api/alarms",
        HTTP_GET,
        [&server, this]()
        {
            handleGetAlarms(server);
        }
    );

    server.on(
        "/api/alarms",
        HTTP_POST,
        [&server, this]()
        {
            handleCreateAlarm(server);
        }
    );


    // ========================================================
    // LEGACY API
    // ========================================================

    server.on(
        "/api/alarm",
        HTTP_GET,
        [&server, this]()
        {
            handleGetAlarms(server);
        }
    );

    server.on(
        "/api/alarm",
        HTTP_POST,
        [&server, this]()
        {
            handleCreateAlarm(server);
        }
    );


    server.on(
        "/api/alarm/enable",
        HTTP_POST,
        [&server, this]()
        {
            handleEnableAlarm(server);
        }
    );

    server.on(
        "/api/alarm/disable",
        HTTP_POST,
        [&server, this]()
        {
            handleDisableAlarm(server);
        }
    );


    // ========================================================
    // RUNTIME
    // ========================================================

    server.on(
        "/api/alarm/runtime",
        HTTP_GET,
        [&server, this]()
        {
            handleRuntime(server);
        }
    );


    // ========================================================
    // CONTROL
    // ========================================================

    server.on(
        "/api/alarm/dismiss",
        HTTP_POST,
        [&server, this]()
        {
            handleDismiss(server);
        }
    );

    server.on(
        "/api/alarm/snooze",
        HTTP_POST,
        [&server, this]()
        {
            handleSnooze(server);
        }
    );

    server.on(
        "/api/alarm/stop",
        HTTP_POST,
        [&server, this]()
        {
            handleStop(server);
        }
    );
}


// ============================================================
// DYNAMIC REQUESTS
// ============================================================

bool WebAlarmManager::handleDynamicRequest(
    WebServer& server
)
{
    const String uri = server.uri();

    const String prefix = "/api/alarms/";


    if (!uri.startsWith(prefix))
        return false;


    String tail =
        uri.substring(prefix.length());


    // --------------------------------------------------------
    // Remove trailing slash
    // --------------------------------------------------------

    if (tail.endsWith("/"))
    {
        tail.remove(
            tail.length() - 1
        );
    }


    // ========================================================
    // /api/alarms/{id}/enabled
    // ========================================================

    const String enabledSuffix = "/enabled";


    if (tail.endsWith(enabledSuffix))
    {
        String id = tail.substring(
            0,
            tail.length() - enabledSuffix.length()
        );


        if (!isValidAlarmId(id))
        {
            sendError(
                server,
                400,
                "Invalid alarm id"
            );

            return true;
        }


        if (server.method() != HTTP_POST)
        {
            sendError(
                server,
                405,
                "Method not allowed"
            );

            return true;
        }


        // getAlarmIdFromRequest() can extract the ID from URI,
        // therefore the common handler can be used.
        handleEnableAlarm(server);

        return true;
    }


    // ========================================================
    // /api/alarms/{id}
    // ========================================================

    if (!isValidAlarmId(tail))
    {
        sendError(
            server,
            400,
            "Invalid alarm id"
        );

        return true;
    }


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
// GET ALL
// ============================================================

void WebAlarmManager::handleGetAlarms(
    WebServer& server
)
{
    sendAlarmList(server);
}


// ============================================================
// GET ONE
// ============================================================

void WebAlarmManager::handleGetAlarm(
    WebServer& server
)
{
    const String id =
        getAlarmIdFromRequest(server);


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


    if (!_alarmManager.get(
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
// CREATE
// ============================================================

void WebAlarmManager::handleCreateAlarm(
    WebServer& server
)
{
    Alarm alarm;


    if (!parseAlarmFromRequest(
        server,
        alarm
    ))
    {
        return;
    }


    if (!_alarmManager.create(
        alarm
    ))
    {
        sendError(
            server,
            500,
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
// UPDATE
// ============================================================

void WebAlarmManager::handleUpdateAlarm(
    WebServer& server
)
{
    const String id =
        getAlarmIdFromRequest(server);


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


    if (!parseAlarmFromRequest(
        server,
        alarm
    ))
    {
        return;
    }


    // The URL is authoritative.
    alarm.id = id;


    if (!_alarmManager.update(
        alarm
    ))
    {
        sendError(
            server,
            500,
            "Failed to update alarm"
        );

        return;
    }


    sendAlarm(
        server,
        alarm
    );
}


// ============================================================
// DELETE
// ============================================================

void WebAlarmManager::handleDeleteAlarm(
    WebServer& server
)
{
    const String id =
        getAlarmIdFromRequest(server);


    if (!isValidAlarmId(id))
    {
        sendError(
            server,
            400,
            "Invalid alarm id"
        );

        return;
    }


    if (!_alarmManager.remove(id))
    {
        sendError(
            server,
            404,
            "Alarm not found"
        );

        return;
    }


    server.send(
        200,
        "application/json",
        "{\"ok\":true}"
    );
}


// ============================================================
// ENABLE
// ============================================================

void WebAlarmManager::handleEnableAlarm(
    WebServer& server
)
{
    const String id =
        getAlarmIdFromRequest(server);


    if (!isValidAlarmId(id))
    {
        sendError(
            server,
            400,
            "Invalid alarm id"
        );

        return;
    }


    if (!_alarmManager.enable(id))
    {
        sendError(
            server,
            404,
            "Alarm not found"
        );

        return;
    }


    server.send(
        200,
        "application/json",
        "{\"ok\":true,\"enabled\":true}"
    );
}


// ============================================================
// DISABLE
// ============================================================

void WebAlarmManager::handleDisableAlarm(
    WebServer& server
)
{
    const String id =
        getAlarmIdFromRequest(server);


    if (!isValidAlarmId(id))
    {
        sendError(
            server,
            400,
            "Invalid alarm id"
        );

        return;
    }


    if (!_alarmManager.disable(id))
    {
        sendError(
            server,
            404,
            "Alarm not found"
        );

        return;
    }


    server.send(
        200,
        "application/json",
        "{\"ok\":true,\"enabled\":false}"
    );
}


// ============================================================
// RUNTIME
// ============================================================

void WebAlarmManager::handleRuntime(
    WebServer& server
)
{
    JsonDocument doc;


    bool active = false;

    String alarmId;

    uint32_t elapsedMs = 0;


    // ========================================================
    // AlarmController state
    // ========================================================

    if (_alarmController.isActive())
    {
        active = true;

        alarmId =
            _alarmController.alarmId();

        elapsedMs =
            _alarmManager.elapsedMs();
    }

    // ========================================================
    // AlarmManager runtime state
    // ========================================================

    else if (_alarmManager.isRunning())
    {
        active = true;


        const Alarm* alarm =
            _alarmManager.currentAlarm();


        if (alarm != nullptr)
        {
            alarmId =
                alarm->id;
        }


        elapsedMs =
            _alarmManager.elapsedMs();
    }


    // ========================================================
    // Response
    // ========================================================

    doc["active"] =
        active;

    doc["alarmId"] =
        alarmId;

    doc["elapsedMs"] =
        elapsedMs;


    String body;

    serializeJson(
        doc,
        body
    );


    server.send(
        200,
        "application/json",
        body
    );
}


// ============================================================
// DISMISS
// ============================================================

void WebAlarmManager::handleDismiss(
    WebServer& server
)
{
    bool result = false;


    // Controller owns the output state.
    if (_alarmController.isActive())
    {
        result =
            _alarmController.dismiss();
    }

    // Fallback for scheduler-only runtime.
    else if (_alarmManager.isRunning())
    {
        result =
            _alarmManager.dismiss();
    }


    if (!result)
    {
        sendError(
            server,
            409,
            "No active alarm"
        );

        return;
    }


    server.send(
        200,
        "application/json",
        "{\"ok\":true}"
    );
}


// ============================================================
// SNOOZE
// ============================================================

void WebAlarmManager::handleSnooze(
    WebServer& server
)
{
    uint32_t durationMs = 0;


    // ========================================================
    // JSON body
    // ========================================================

    if (server.hasArg("plain"))
    {
        JsonDocument doc;


        const DeserializationError error =
            deserializeJson(
                doc,
                server.arg("plain")
            );


        if (!error &&
            doc["durationMs"].is<uint32_t>())
        {
            durationMs =
                doc["durationMs"].as<uint32_t>();
        }
    }


    // ========================================================
    // Query parameter fallback
    // ========================================================

    if (
        durationMs == 0 &&
        server.hasArg("durationMs")
    )
    {
        durationMs =
            static_cast<uint32_t>(
                server.arg("durationMs").toInt()
            );
    }


    // ========================================================
    // Validate
    // ========================================================

    if (
        durationMs == 0 ||
        durationMs > MAX_SNOOZE_MS
    )
    {
        sendError(
            server,
            400,
            "Invalid durationMs"
        );

        return;
    }


    bool result = false;


    // Controller first.
    if (_alarmController.isActive())
    {
        result =
            _alarmController.snooze(
                durationMs
            );
    }

    // Scheduler fallback.
    else if (_alarmManager.isRunning())
    {
        result =
            _alarmManager.snooze(
                durationMs
            );
    }


    if (!result)
    {
        sendError(
            server,
            409,
            "No active alarm"
        );

        return;
    }


    server.send(
        200,
        "application/json",
        "{\"ok\":true}"
    );
}


// ============================================================
// STOP
// ============================================================

void WebAlarmManager::handleStop(
    WebServer& server
)
{
    if (!_alarmController.isActive() &&
        !_alarmManager.isRunning())
    {
        sendError(
            server,
            409,
            "No active alarm"
        );

        return;
    }


    _alarmController.stop();


    server.send(
        200,
        "application/json",
        "{\"ok\":true}"
    );
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
            "Missing JSON body"
        );

        return false;
    }


    JsonDocument doc;


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


    if (!_alarmManager.deserialize(
        doc,
        alarm
    ))
    {
        sendError(
            server,
            400,
            "Invalid alarm schema"
        );

        return false;
    }


    return true;
}


// ============================================================
// SEND ALARM
// ============================================================

void WebAlarmManager::sendAlarm(
    WebServer& server,
    const Alarm& alarm
)
{
    JsonDocument doc;


    _alarmManager.serialize(
        alarm,
        doc
    );


    String body;


    serializeJson(
        doc,
        body
    );


    server.send(
        200,
        "application/json",
        body
    );
}


// ============================================================
// SEND ALARM LIST
// ============================================================

void WebAlarmManager::sendAlarmList(
    WebServer& server
)
{
    constexpr uint8_t MAX_ALARMS = 32;


    Alarm alarms[MAX_ALARMS];

    uint8_t count = 0;


    if (!_alarmManager.loadAll(
        alarms,
        MAX_ALARMS,
        count
    ))
    {
        sendError(
            server,
            500,
            "Failed to load alarms"
        );

        return;
    }


    JsonDocument doc;


    JsonArray array =
        doc["alarms"].to<JsonArray>();


    for (
        uint8_t i = 0;
        i < count;
        ++i
    )
    {
        JsonObject item =
            array.add<JsonObject>();


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


        // ====================================================
        // Actual Alarm model
        // ====================================================

        item["matrixEffect"] =
            alarms[i].matrixEffect;

        item["cobEffect"] =
            alarms[i].cobEffect;

        item["audioEffect"] =
            alarms[i].audioEffect;
    }


    String body;


    serializeJson(
        doc,
        body
    );


    server.send(
        200,
        "application/json",
        body
    );
}


// ============================================================
// GET ID
// ============================================================

String WebAlarmManager::getAlarmIdFromRequest(
    WebServer& server
) const
{
    // --------------------------------------------------------
    // Query parameters
    // --------------------------------------------------------

    if (server.hasArg("id"))
        return server.arg("id");


    if (server.hasArg("alarmId"))
        return server.arg("alarmId");


    // --------------------------------------------------------
    // REST path
    // --------------------------------------------------------

    const String prefix =
        "/api/alarms/";


    const String uri =
        server.uri();


    if (uri.startsWith(prefix))
    {
        String id =
            uri.substring(
                prefix.length()
            );


        const int slash =
            id.indexOf('/');


        if (slash >= 0)
        {
            id =
                id.substring(
                    0,
                    slash
                );
        }


        return id;
    }


    return String();
}


// ============================================================
// UUID V4 VALIDATION
// ============================================================

bool WebAlarmManager::isValidAlarmId(
    const String& id
) const
{
    if (id.length() != 36)
        return false;


    for (
        uint8_t i = 0;
        i < 36;
        ++i
    )
    {
        // ----------------------------------------------------
        // Hyphens
        // ----------------------------------------------------

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


        // ----------------------------------------------------
        // Hexadecimal character
        // ----------------------------------------------------

        const char c =
            id[i];


        const bool hex =
            (c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'f') ||
            (c >= 'A' && c <= 'F');


        if (!hex)
            return false;
    }


    // --------------------------------------------------------
    // UUID version 4
    // --------------------------------------------------------

    if (
        id[14] != '4' &&
        id[14] != '4'
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // UUID variant 10xx
    // --------------------------------------------------------

    const char variant =
        id[19];


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
// ERROR RESPONSE
// ============================================================

void WebAlarmManager::sendError(
    WebServer& server,
    int code,
    const char* message
)
{
    JsonDocument doc;


    doc["error"] =
        message;


    String body;


    serializeJson(
        doc,
        body
    );


    server.send(
        code,
        "application/json",
        body
    );
}