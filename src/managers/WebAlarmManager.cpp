#include "WebAlarmManager.h"

namespace
{
    constexpr uint32_t MAX_SNOOZE_MS =
        24UL * 60UL * 60UL * 1000UL;

    constexpr uint8_t MAX_ALARMS =
        AlarmConfig::MAX_ALARMS;
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
    // COLLECTION
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
    // RUNTIME
    // ========================================================

    server.on(
        "/api/alarms/runtime",
        HTTP_GET,
        [&server, this]()
        {
            handleRuntime(server);
        }
    );

    server.on(
        "/api/alarms/runtime/dismiss",
        HTTP_POST,
        [&server, this]()
        {
            handleDismiss(server);
        }
    );

    server.on(
        "/api/alarms/runtime/snooze",
        HTTP_POST,
        [&server, this]()
        {
            handleSnooze(server);
        }
    );

    server.on(
        "/api/alarms/runtime/stop",
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

    constexpr const char* PREFIX =
        "/api/alarms/";

    if (!uri.startsWith(PREFIX))
        return false;


    String tail =
        uri.substring(strlen(PREFIX));


    // --------------------------------------------------------
    // Remove trailing slash
    // --------------------------------------------------------

    while (tail.endsWith("/"))
    {
        tail.remove(
            tail.length() - 1
        );
    }


    if (tail.isEmpty())
    {
        return false;
    }


    // ========================================================
    // /api/alarms/runtime/*
    //
    // These routes are handled explicitly by setupRoutes().
    // Do not treat "runtime" as an alarm UUID.
    // ========================================================

    if (tail == "runtime")
    {
        sendError(
            server,
            405,
            "Method not allowed"
        );

        return true;
    }


    // ========================================================
    // /api/alarms/{id}/enabled
    // ========================================================

    constexpr const char* ENABLED_SUFFIX =
        "/enabled";

    if (tail.endsWith(ENABLED_SUFFIX))
    {
        const String id =
            tail.substring(
                0,
                tail.length() - strlen(ENABLED_SUFFIX)
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


        handleSetEnabled(server);

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
// POST /api/alarms
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


    // ========================================================
    // IMPORTANT
    //
    // The web layer does NOT generate an ID.
    //
    // AlarmManager::create() generates UUID v4 when id is
    // empty.
    // ========================================================

    alarm.id = String();


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


    // AlarmManager has now generated the final UUID.

    server.send(
        201,
        "application/json",
        [&]()
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

            return body;
        }()
    );
}


// ============================================================
// PUT /api/alarms/{id}
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


    // ========================================================
    // URL ID is authoritative.
    // ========================================================

    alarm.id = id;


    if (!_alarmManager.update(
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
// DELETE /api/alarms/{id}
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


    if (!_alarmManager.exists(id))
    {
        sendError(
            server,
            404,
            "Alarm not found"
        );

        return;
    }


    if (!_alarmManager.remove(id))
    {
        sendError(
            server,
            500,
            "Failed to delete alarm"
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


    if (!server.hasArg("plain"))
    {
        sendError(
            server,
            400,
            "Missing JSON body"
        );

        return;
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

        return;
    }


    if (!doc["enabled"].is<bool>())
    {
        sendError(
            server,
            400,
            "Missing enabled"
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


    // --------------------------------------------------------
    // Return the actual stored alarm.
    // --------------------------------------------------------

    Alarm alarm;


    if (!_alarmManager.get(
        id,
        alarm
    ))
    {
        sendError(
            server,
            500,
            "Failed to read alarm"
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
    JsonDocument doc;


    bool active = false;

    String alarmId;

    uint32_t elapsedMs = 0;


    // ========================================================
    // Controller state
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
    // AlarmManager state
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
// POST /api/alarms/runtime/dismiss
// ============================================================

void WebAlarmManager::handleDismiss(
    WebServer& server
)
{
    bool result = false;


    if (_alarmController.isActive())
    {
        result =
            _alarmController.dismiss();
    }
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


    sendOk(server);
}


// ============================================================
// POST /api/alarms/runtime/snooze
// ============================================================

void WebAlarmManager::handleSnooze(
    WebServer& server
)
{
    uint32_t durationMs = 0;


    // ========================================================
    // JSON
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
    // Query fallback
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
    // Validation
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


    if (_alarmController.isActive())
    {
        result =
            _alarmController.snooze(
                durationMs
            );
    }
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


    sendOk(server);
}


// ============================================================
// POST /api/alarms/runtime/stop
// ============================================================

void WebAlarmManager::handleStop(
    WebServer& server
)
{
    if (
        !_alarmController.isActive() &&
        !_alarmManager.isRunning()
    )
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


    if (!_alarmManager.serialize(
        alarm,
        doc
    ))
    {
        sendError(
            server,
            500,
            "Failed to serialize alarm"
        );

        return;
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
// SEND ALARM LIST
// ============================================================

void WebAlarmManager::sendAlarmList(
    WebServer& server
)
{
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
// GET ALARM ID
// ============================================================

String WebAlarmManager::getAlarmIdFromRequest(
    WebServer& server
) const
{
    // --------------------------------------------------------
    // REST URI
    // --------------------------------------------------------

    constexpr const char* PREFIX =
        "/api/alarms/";

    const String uri =
        server.uri();


    if (uri.startsWith(PREFIX))
    {
        String id =
            uri.substring(
                strlen(PREFIX)
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


    // ========================================================
    // Format
    // xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    // ========================================================

    for (
        uint8_t i = 0;
        i < 36;
        ++i
    )
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


        const char c =
            id[i];


        const bool hex =
            (c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'f') ||
            (c >= 'A' && c <= 'F');


        if (!hex)
            return false;
    }


    // ========================================================
    // UUID version 4
    // ========================================================

    if (id[14] != '4')
        return false;


    // ========================================================
    // UUID variant 10xx
    // ========================================================

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
// OK RESPONSE
// ============================================================

void WebAlarmManager::sendOk(
    WebServer& server
)
{
    server.send(
        200,
        "application/json",
        "{\"ok\":true}"
    );
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