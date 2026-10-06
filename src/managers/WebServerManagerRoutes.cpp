#include "WebServerManager.h"

#include <cstring>

// ============================================================
// URI HELPERS
// ============================================================

namespace
{
    bool isAlarmPath(
        const String& uri
    )
    {
        constexpr const char* PREFIX =
            "/api/alarms/";

        if (!uri.startsWith(PREFIX))
            return false;

        const String rest =
            uri.substring(
                strlen(PREFIX)
            );

        return !rest.isEmpty();
    }

    bool isAlarmEnabledPath(
        const String& uri
    )
    {
        return
            isAlarmPath(uri) &&
            uri.endsWith("/enabled");
    }

    String alarmIdFromUri(
        const String& uri
    )
    {
        constexpr const char* PREFIX =
            "/api/alarms/";

        String id =
            uri.substring(
                strlen(PREFIX)
            );

        const int slash =
            id.indexOf('/');

        if (slash >= 0)
            id =
                id.substring(
                    0,
                    slash
                );

        id.trim();

        return id;
    }

    String alarmIdFromEnabledUri(
        const String& uri
    )
    {
        constexpr const char* PREFIX =
            "/api/alarms/";

        constexpr const char* SUFFIX =
            "/enabled";

        String id =
            uri.substring(
                strlen(PREFIX)
            );

        if (id.endsWith(SUFFIX))
        {
            id.remove(
                id.length() -
                strlen(SUFFIX)
            );
        }

        id.trim();

        return id;
    }

    const char* methodName(
        HTTPMethod method
    )
    {
        switch (method)
        {
            case HTTP_GET:
                return "GET";

            case HTTP_POST:
                return "POST";

            case HTTP_PUT:
                return "PUT";

            case HTTP_DELETE:
                return "DELETE";

#ifdef HTTP_PATCH
            case HTTP_PATCH:
                return "PATCH";
#endif

            default:
                return "UNKNOWN";
        }
    }
}

// ============================================================
// SETUP ROUTES
// ============================================================

void WebServerManager::setupRoutes()
{
    Serial0.println(
        "[WEB][ROUTES] Registering routes"
    );

    // ========================================================
    // ROOT
    // ========================================================

    _server.on(
        "/",
        HTTP_GET,
        [this]()
        {
            handleRoot();
        }
    );

    // ========================================================
    // SETTINGS
    // ========================================================

    _server.on(
        "/api/param",
        HTTP_GET,
        [this]()
        {
            handleGetParam();
        }
    );

    _server.on(
        "/api/param",
        HTTP_POST,
        [this]()
        {
            handleSetParam();
        }
    );

    _server.on(
        "/api/params",
        HTTP_GET,
        [this]()
        {
            handleGetAllParams();
        }
    );

    _server.on(
        "/api/reset",
        HTTP_POST,
        [this]()
        {
            handleReset();
        }
    );



    // ========================================================
    // SD
    // ========================================================

    _server.on(
        "/api/sd",
        HTTP_GET,
        [this]()
        {
            handleSD();
        }
    );

    // ========================================================
    // AUDIO
    // ========================================================

    _server.on(
        "/api/audio/play",
        HTTP_POST,
        [this]()
        {
            handleAudioPlay();
        }
    );

    _server.on(
        "/api/audio/pause",
        HTTP_POST,
        [this]()
        {
            handleAudioPause();
        }
    );

    _server.on(
        "/api/audio/resume",
        HTTP_POST,
        [this]()
        {
            handleAudioResume();
        }
    );

    _server.on(
        "/api/audio/stop",
        HTTP_POST,
        [this]()
        {
            handleAudioStop();
        }
    );

    _server.on(
        "/api/audio/status",
        HTTP_GET,
        [this]()
        {
            handleAudioStatus();
        }
    );

    // ========================================================
    // ALARMS
    // ========================================================

    _server.on(
        "/api/alarms",
        HTTP_GET,
        [this]()
        {
            handleGetAlarms();
        }
    );

    _server.on(
        "/api/alarms",
        HTTP_POST,
        [this]()
        {
            handleCreateAlarm();
        }
    );

    // ========================================================
    // LEGACY ALARM API
    // ========================================================

    _server.on(
        "/api/alarm",
        HTTP_GET,
        [this]()
        {
            handleGetAlarm();
        }
    );

    _server.on(
        "/api/alarm",
        HTTP_POST,
        [this]()
        {
            handleCreateAlarm();
        }
    );

    _server.on(
        "/api/alarm",
        HTTP_PUT,
        [this]()
        {
            handleUpdateAlarm();
        }
    );

    _server.on(
        "/api/alarm",
        HTTP_DELETE,
        [this]()
        {
            handleDeleteAlarm();
        }
    );

    _server.on(
        "/api/alarm/enable",
        HTTP_POST,
        [this]()
        {
            handleEnableAlarm();
        }
    );

    _server.on(
        "/api/alarm/disable",
        HTTP_POST,
        [this]()
        {
            handleDisableAlarm();
        }
    );

    _server.on(
        "/api/alarm/runtime",
        HTTP_GET,
        [this]()
        {
            handleAlarmRuntime();
        }
    );

    _server.on(
        "/api/alarm/dismiss",
        HTTP_POST,
        [this]()
        {
            handleAlarmDismiss();
        }
    );

    _server.on(
        "/api/alarm/snooze",
        HTTP_POST,
        [this]()
        {
            handleAlarmSnooze();
        }
    );

    _server.onNotFound(
        [this]()
        {
            handleNotFound();
        }
    );

    Serial0.println(
        "[WEB][ROUTES] Registration completed"
    );
}

// ============================================================
// ROOT
// ============================================================

void WebServerManager::handleRoot()
{
    if (!LittleFS.exists(
            "/index.html"))
    {
        Serial0.println(
            "[WEB][ROOT][ERROR] index.html not found"
        );

        sendError(
            404,
            "index.html not found"
        );

        return;
    }

    File file =
        LittleFS.open(
            "/index.html",
            "r"
        );

    if (!file)
    {
        Serial0.println(
            "[WEB][ROOT][ERROR] Cannot open index.html"
        );

        sendError(
            500,
            "Failed to open index.html"
        );

        return;
    }

    Serial0.printf(
        "[WEB][ROOT] size=%u\n",
        static_cast<unsigned>(
            file.size()
        )
    );

    _server.streamFile(
        file,
        "text/html"
    );

    file.close();
}

// ============================================================
// NOT FOUND / DYNAMIC ROUTES
// ============================================================

void WebServerManager::handleNotFound()
{
    const String uri =
        _server.uri();

    const HTTPMethod method =
        _server.method();

    Serial0.printf(
        "[WEB][NOT_FOUND] %s %s\n",
        methodName(method),
        uri.c_str()
    );

    // --------------------------------------------------------
    // /api/alarms/{id}/enabled
    // --------------------------------------------------------

    if (isAlarmEnabledPath(uri))
    {
        if (method != HTTP_POST)
        {
            sendError(
                404,
                "Route not found"
            );

            return;
        }

        if (!_alarmManager)
        {
            sendError(
                503,
                "AlarmManager unavailable"
            );

            return;
        }

        const String id =
            alarmIdFromEnabledUri(uri);

        if (!isValidAlarmId(id))
        {
            sendError(
                400,
                "Invalid alarm id"
            );

            return;
        }

        JsonDocument doc;

        if (!parseJson(doc))
            return;

        JsonObjectConst object =
            doc.as<JsonObjectConst>();

        bool enabled = false;

        if (!getBoolean(
                object,
                "enabled",
                enabled))
        {
            sendError(
                400,
                "enabled is required"
            );

            return;
        }

        if (!_alarmManager->setEnabled(
                id,
                enabled))
        {
            sendError(
                404,
                "Alarm not found"
            );

            return;
        }

        JsonDocument response;

        response["id"] =
            id;

        response["enabled"] =
            enabled;

        String output;

        serializeJson(
            response,
            output
        );

        sendJson(
            200,
            output
        );

        return;
    }

    // --------------------------------------------------------
    // /api/alarms/{id}
    // --------------------------------------------------------

    if (isAlarmPath(uri))
    {
        const String id =
            alarmIdFromUri(uri);

        if (!isValidAlarmId(id))
        {
            sendError(
                400,
                "Invalid alarm id"
            );

            return;
        }

        if (method == HTTP_GET)
        {
            handleGetAlarm();
            return;
        }

        if (method == HTTP_PUT)
        {
            handleUpdateAlarm();
            return;
        }

        if (method == HTTP_DELETE)
        {
            handleDeleteAlarm();
            return;
        }
    }

    sendError(
        404,
        "Request handler not found"
    );
}