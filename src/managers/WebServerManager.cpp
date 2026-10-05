#include "WebServerManager.h"

#include <cstring>


// ============================================================
// ALARM PATH HELPERS
// ============================================================

namespace
{
    bool isAlarmCollectionPath(const String& uri)
    {
        return uri == "/api/alarms";
    }

    bool isAlarmItemPath(const String& uri)
    {
        static constexpr const char* PREFIX = "/api/alarms/";

        if (!uri.startsWith(PREFIX))
            return false;

        const String tail = uri.substring(
            strlen(PREFIX)
        );

        return !tail.isEmpty() &&
               tail.indexOf('/') < 0;
    }

    bool isAlarmEnabledPath(const String& uri)
    {
        static constexpr const char* PREFIX = "/api/alarms/";
        static constexpr const char* SUFFIX = "/enabled";

        if (!uri.startsWith(PREFIX))
            return false;

        if (!uri.endsWith(SUFFIX))
            return false;

        const size_t prefixLength =
            strlen(PREFIX);

        const size_t suffixLength =
            strlen(SUFFIX);

        if (uri.length() <=
            prefixLength + suffixLength)
        {
            return false;
        }

        const String id = uri.substring(
            prefixLength,
            uri.length() - suffixLength
        );

        return !id.isEmpty() &&
               id.indexOf('/') < 0;
    }

    String alarmIdFromUri(const String& uri)
    {
        static constexpr const char* PREFIX = "/api/alarms/";

        if (!isAlarmItemPath(uri))
            return String();

        return uri.substring(strlen(PREFIX));
    }

    String alarmIdFromEnabledUri(const String& uri)
    {
        static constexpr const char* PREFIX = "/api/alarms/";
        static constexpr const char* SUFFIX = "/enabled";

        if (!isAlarmEnabledPath(uri))
            return String();

        return uri.substring(
            strlen(PREFIX),
            uri.length() - strlen(SUFFIX)
        );
    }
}


// ============================================================
// CONSTRUCTOR
// ============================================================

WebServerManager::WebServerManager()
    : _server(80),
      _settings(nullptr),
      _sd(nullptr),
      _sound(nullptr),
      _alarmManager(nullptr),
      _alarmController(nullptr),
      _initialized(false)
{
}


// ============================================================
// BEGIN
// ============================================================

bool WebServerManager::begin(
    SettingsManager& settings,
    SDManager& sd,
    SoundManager& sound,
    const char* ssid,
    const char* password
)
{
    _settings = &settings;
    _sd       = &sd;
    _sound    = &sound;

    // --------------------------------------------------------
    // WiFi
    // --------------------------------------------------------

    WiFi.mode(WIFI_STA);

    WiFi.disconnect(
        true,
        true
    );

    delay(300);

    WiFi.begin(
        ssid,
        password
    );

    const uint32_t startTime =
        millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 20000
    )
    {
        delay(500);
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial0.println(
            "[WEB] WiFi connection failed"
        );

        return false;
    }

    Serial0.print(
        "[WEB] WiFi connected: "
    );

    Serial0.println(
        WiFi.localIP()
    );


    // --------------------------------------------------------
    // LittleFS
    // --------------------------------------------------------

    if (!LittleFS.begin(true))
    {
        Serial0.println(
            "[WEB] LittleFS mount failed"
        );

        return false;
    }


    // --------------------------------------------------------
    // Routes
    // --------------------------------------------------------

    setupRoutes();


    // --------------------------------------------------------
    // Server
    // --------------------------------------------------------

    _server.begin();

    _initialized = true;

    Serial0.print(
        "[WEB] Server: http://"
    );

    Serial0.println(
        WiFi.localIP()
    );

    return true;
}


// ============================================================
// SET ALARM MANAGER
// ============================================================

void WebServerManager::setAlarmManager(
    AlarmManager& alarmManager
)
{
    _alarmManager =
        &alarmManager;

    Serial0.println(
        "[WEB][ALARM] AlarmManager attached"
    );
}


// ============================================================
// SET ALARM CONTROLLER
// ============================================================

void WebServerManager::setAlarmController(
    AlarmController& alarmController
)
{
    _alarmController =
        &alarmController;

    Serial0.println(
        "[WEB][ALARM] AlarmController attached"
    );
}


// ============================================================
// UPDATE
// ============================================================

void WebServerManager::update()
{
    if (!_initialized)
        return;

    _server.handleClient();
}


// ============================================================
// ROUTES
// ============================================================

void WebServerManager::setupRoutes()
{
    // ========================================================
    // WEB UI
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
    // SENSORS
    // ========================================================

    _server.on(
        "/api/sensors",
        HTTP_GET,
        [this]()
        {
            handleSensors();
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


    // ========================================================
    // 404
    // ========================================================

    _server.onNotFound(
        [this]()
        {
            handleNotFound();
        }
    );
}


// ============================================================
// ROOT
// ============================================================

void WebServerManager::handleRoot()
{
    if (!LittleFS.exists("/index.html"))
    {
        _server.send(
            404,
            "text/plain",
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
        _server.send(
            500,
            "text/plain",
            "failed to open index.html"
        );

        return;
    }

    _server.streamFile(
        file,
        "text/html; charset=utf-8"
    );

    file.close();
}


// ============================================================
// 404
// ============================================================

void WebServerManager::handleNotFound()
{
    const String uri = _server.uri();

    Serial0.println();
    Serial0.println(
        "[WEB][404] Request"
    );

    Serial0.print(
        "[WEB][404] URI: "
    );

    Serial0.println(uri);

    Serial0.print(
        "[WEB][404] Method: "
    );

    Serial0.println(
        static_cast<int>(
            _server.method()
        )
    );

    // --------------------------------------------------------
    // REST alarm API
    //
    // GET    /api/alarms/<id>
    // PUT    /api/alarms/<id>
    // DELETE /api/alarms/<id>
    // POST   /api/alarms/<id>/enabled
    // --------------------------------------------------------

    if (isAlarmItemPath(uri))
    {
        const String id =
            alarmIdFromUri(uri);

        Serial0.print(
            "[WEB][ALARM] REST item ID: "
        );

        Serial0.println(id);

        if (!id.isEmpty())
        {
            if (_server.method() == HTTP_GET)
            {
                Serial0.println(
                    "[WEB][ALARM] -> GET alarm"
                );

                handleGetAlarm();

                return;
            }

            if (_server.method() == HTTP_PUT)
            {
                Serial0.println(
                    "[WEB][ALARM] -> PUT alarm"
                );

                handleUpdateAlarm();

                return;
            }

            if (_server.method() == HTTP_DELETE)
            {
                Serial0.println(
                    "[WEB][ALARM] -> DELETE alarm"
                );

                handleDeleteAlarm();

                return;
            }
        }
    }

    if (
        isAlarmEnabledPath(uri) &&
        _server.method() == HTTP_POST
    )
    {
        Serial0.println(
            "[WEB][ALARM] -> POST enabled"
        );

        if (!_alarmManager)
        {
            Serial0.println(
                "[WEB][ALARM][ERROR] AlarmManager is null"
            );

            sendError(
                503,
                "alarm manager not initialized"
            );

            return;
        }

        const String id =
            alarmIdFromEnabledUri(uri);

        Serial0.print(
            "[WEB][ALARM] Enabled ID: "
        );

        Serial0.println(id);

        if (id.isEmpty())
        {
            sendError(
                400,
                "missing id"
            );

            return;
        }

        JsonDocument doc;

        if (!parseJson(doc))
            return;

        if (doc["enabled"].isNull())
        {
            Serial0.println(
                "[WEB][ALARM][ERROR] enabled missing"
            );

            sendError(
                400,
                "missing enabled"
            );

            return;
        }

        const bool enabled =
            doc["enabled"].as<bool>();

        Serial0.print(
            "[WEB][ALARM] Set enabled = "
        );

        Serial0.println(
            enabled
                ? "true"
                : "false"
        );

        if (!_alarmManager->setEnabled(
                id,
                enabled
            ))
        {
            Serial0.println(
                "[WEB][ALARM][ERROR] setEnabled() failed"
            );

            sendError(
                404,
                "alarm not found"
            );

            return;
        }

        Serial0.println(
            "[WEB][ALARM] setEnabled() OK"
        );

        sendOk();

        return;
    }

    Serial0.println(
        "[WEB][404] Not found"
    );

    _server.send(
        404,
        "text/plain",
        "404 - Not Found"
    );
}


// ============================================================
// GET /api/param
// ============================================================

void WebServerManager::handleGetParam()
{
    if (!_settings)
    {
        sendError(
            500,
            "settings not initialized"
        );

        return;
    }

    String name =
        _server.arg("name");

    if (name.isEmpty())
        name =
            _server.arg("param");

    if (name.isEmpty())
    {
        sendError(
            400,
            "missing name"
        );

        return;
    }

    const SettingsManager::Param param =
        _settings->paramFromName(
            name.c_str()
        );

    if (
        param ==
        SettingsManager::Param::COUNT
    )
    {
        sendError(
            400,
            "unknown param"
        );

        return;
    }

    JsonDocument doc;

    doc["ok"] = true;

    doc["param"] =
        _settings->paramName(
            param
        );

    doc["value"] =
        _settings->get(
            param
        );

    String body;

    serializeJson(
        doc,
        body
    );

    sendJson(
        200,
        body
    );
}


// ============================================================
// POST /api/param
// ============================================================

void WebServerManager::handleSetParam()
{
    if (!_settings)
    {
        sendError(
            500,
            "settings not initialized"
        );

        return;
    }

    JsonDocument doc;

    if (!parseJson(doc))
        return;

    const char* name =
        doc["param"];

    if (
        !name ||
        name[0] == '\0'
    )
    {
        name =
            doc["name"];
    }

    if (
        !name ||
        name[0] == '\0'
    )
    {
        sendError(
            400,
            "missing param"
        );

        return;
    }

    if (doc["value"].isNull())
    {
        sendError(
            400,
            "missing value"
        );

        return;
    }

    const int requestedValue =
        doc["value"].as<int>();

    const SettingsManager::Param param =
        _settings->paramFromName(
            name
        );

    if (
        param ==
        SettingsManager::Param::COUNT
    )
    {
        sendError(
            400,
            "unknown param"
        );

        return;
    }

    const uint8_t oldValue =
        _settings->get(
            param
        );

    const uint8_t value =
        static_cast<uint8_t>(
            constrain(
                requestedValue,
                0,
                255
            )
        );

    _settings->set(
        param,
        value
    );

    const uint8_t actualValue =
        _settings->get(
            param
        );

    JsonDocument response;

    response["ok"] = true;

    response["param"] =
        _settings->paramName(
            param
        );

    response["value"] =
        actualValue;

    response["changed"] =
        actualValue != oldValue;

    String body;

    serializeJson(
        response,
        body
    );

    sendJson(
        200,
        body
    );
}


// ============================================================
// GET /api/params
// ============================================================

void WebServerManager::handleGetAllParams()
{
    if (!_settings)
    {
        sendError(
            500,
            "settings not initialized"
        );

        return;
    }

    JsonDocument doc;

    for (
        uint8_t i = 0;
        i < static_cast<uint8_t>(
                SettingsManager::Param::COUNT
            );
        ++i
    )
    {
        const SettingsManager::Param param =
            static_cast<SettingsManager::Param>(
                i
            );

        const char* name =
            _settings->paramName(
                param
            );

        if (
            !name ||
            name[0] == '\0'
        )
        {
            continue;
        }

        doc[name] =
            _settings->get(
                param
            );
    }

    String body;

    serializeJson(
        doc,
        body
    );

    sendJson(
        200,
        body
    );
}


// ============================================================
// POST /api/reset
// ============================================================

void WebServerManager::handleReset()
{
    if (!_settings)
    {
        sendError(
            500,
            "settings not initialized"
        );

        return;
    }

    _settings->resetAll();

    sendOk();
}


// ============================================================
// GET /api/sensors
// ============================================================

void WebServerManager::handleSensors()
{
    JsonDocument doc;

    doc["temperature"] = 0;
    doc["humidity"]    = 0;
    doc["light"]       = 0;
    doc["distance"]    = 0;

    String body;

    serializeJson(
        doc,
        body
    );

    sendJson(
        200,
        body
    );
}


// ============================================================
// GET /api/sd
// ============================================================

void WebServerManager::handleSD()
{
    if (
        !_sd ||
        !_sd->isReady()
    )
    {
        sendError(
            503,
            "SD not available"
        );

        return;
    }

    static constexpr size_t MAX_FILES = 300;
    static constexpr uint8_t MAX_DEPTH = 3;

    static SDFileEntry entries[MAX_FILES];

    const size_t count =
        _sd->listFiles(
            entries,
            MAX_FILES,
            MAX_DEPTH,
            "/"
        );

    JsonDocument doc;

    JsonArray files =
        doc["files"].to<JsonArray>();

    for (
        size_t i = 0;
        i < count;
        ++i
    )
    {
        JsonObject item =
            files.add<JsonObject>();

        item["path"] =
            entries[i].path;

        item["size"] =
            entries[i].size;

        item["type"] =
            entries[i].isDir
                ? "dir"
                : "file";
    }

    String body;

    serializeJson(
        doc,
        body
    );

    sendJson(
        200,
        body
    );
}


// ============================================================
// POST /api/audio/play
// ============================================================

void WebServerManager::handleAudioPlay()
{
    if (
        !_sound ||
        !_sound->isInitialized()
    )
    {
        sendError(
            503,
            "sound not ready"
        );

        return;
    }

    JsonDocument doc;

    if (!parseJson(doc))
        return;

    const char* path =
        doc["path"];

    if (
        !path ||
        path[0] == '\0'
    )
    {
        sendError(
            400,
            "missing path"
        );

        return;
    }

    if (
        !_sd ||
        !_sd->isReady()
    )
    {
        sendError(
            503,
            "SD not available"
        );

        return;
    }

    if (!_sd->card().exists(path))
    {
        sendError(
            404,
            "file not found"
        );

        return;
    }

    SoundManager::PlayOptions options;

    const char* stream =
        doc["stream"] | "media";

    if (
        strcmp(
            stream,
            "alarm"
        ) == 0
    )
    {
        options.stream =
            SoundManager::AudioStream::Alarm;
    }
    else if (
        strcmp(
            stream,
            "system"
        ) == 0
    )
    {
        options.stream =
            SoundManager::AudioStream::System;
    }
    else
    {
        options.stream =
            SoundManager::AudioStream::Media;
    }

    int volume = 100;

    if (!doc["localPercent"].isNull())
    {
        volume =
            doc["localPercent"].as<int>();
    }
    else if (!doc["volume"].isNull())
    {
        volume =
            doc["volume"].as<int>();
    }

    volume =
        constrain(
            volume,
            0,
            100
        );

    options.localPercent =
        static_cast<uint8_t>(
            volume
        );

    if (!doc["fadeInMs"].isNull())
    {
        options.fadeInMs =
            doc["fadeInMs"].as<uint32_t>();
    }
    else
    {
        options.fadeInMs =
            doc["fade_in"] | 0;
    }

    if (!doc["fadeOutMs"].isNull())
    {
        options.fadeOutMs =
            doc["fadeOutMs"].as<uint32_t>();
    }
    else
    {
        options.fadeOutMs =
            doc["fade_out"] | 0;
    }

    const char* curve =
        doc["curve"] | "linear";

    if (
        strcmp(
            curve,
            "exp"
        ) == 0
    )
    {
        options.curve =
            SoundManager::FadeCurve::Exponential;
    }
    else if (
        strcmp(
            curve,
            "log"
        ) == 0
    )
    {
        options.curve =
            SoundManager::FadeCurve::Logarithmic;
    }
    else
    {
        options.curve =
            SoundManager::FadeCurve::Linear;
    }

    if (!_sound->play(
            path,
            options
        ))
    {
        sendError(
            500,
            "play failed"
        );

        return;
    }

    sendOk();
}


// ============================================================
// POST /api/audio/pause
// ============================================================

void WebServerManager::handleAudioPause()
{
    if (
        !_sound ||
        !_sound->isInitialized()
    )
    {
        sendError(
            503,
            "sound not ready"
        );

        return;
    }

    if (!_sound->isPlaying())
    {
        sendError(
            409,
            "not playing"
        );

        return;
    }

    if (!_sound->pause())
    {
        sendError(
            500,
            "pause failed"
        );

        return;
    }

    sendOk();
}


// ============================================================
// POST /api/audio/resume
// ============================================================

void WebServerManager::handleAudioResume()
{
    if (
        !_sound ||
        !_sound->isInitialized()
    )
    {
        sendError(
            503,
            "sound not ready"
        );

        return;
    }

    if (!_sound->isPaused())
    {
        sendError(
            409,
            "not paused"
        );

        return;
    }

    if (!_sound->resume())
    {
        sendError(
            500,
            "resume failed"
        );

        return;
    }

    sendOk();
}


// ============================================================
// POST /api/audio/stop
// ============================================================

void WebServerManager::handleAudioStop()
{
    if (
        !_sound ||
        !_sound->isInitialized()
    )
    {
        sendError(
            503,
            "sound not ready"
        );

        return;
    }

    uint32_t fadeOut = 0;

    if (_server.hasArg("plain"))
    {
        JsonDocument doc;

        const DeserializationError error =
            deserializeJson(
                doc,
                _server.arg("plain")
            );

        if (!error)
        {
            if (!doc["fadeOutMs"].isNull())
            {
                fadeOut =
                    doc["fadeOutMs"].as<uint32_t>();
            }
            else
            {
                fadeOut =
                    doc["fade_out"] | 0;
            }
        }
    }

    if (fadeOut > 0)
    {
        _sound->stop(
            fadeOut
        );
    }
    else
    {
        _sound->stop();
    }

    sendOk();
}


// ============================================================
// GET /api/audio/status
// ============================================================

void WebServerManager::handleAudioStatus()
{
    if (
        !_sound ||
        !_sound->isInitialized()
    )
    {
        sendError(
            503,
            "sound not ready"
        );

        return;
    }

    JsonDocument doc;

    doc["state"] =
        _sound->getStateString();

    doc["path"] =
        _sound->getCurrentPath();

    doc["stream"] =
        static_cast<uint8_t>(
            _sound->getCurrentStream()
        );

    const uint32_t position =
        _sound->getPositionMs();

    doc["positionMs"] =
        position;

    doc["position"] =
        position;

    const uint32_t duration =
        _sound->getDurationMs();

    doc["durationMs"] =
        duration;

    doc["duration"] =
        duration;

    doc["volume"] =
        _sound->getEffectiveVolume();

    String body;

    serializeJson(
        doc,
        body
    );

    sendJson(
        200,
        body
    );
}


// ============================================================
// GET /api/alarms
// ============================================================

void WebServerManager::handleGetAlarms()
{
    Serial0.println(
        "[WEB][ALARM] GET /api/alarms"
    );

    if (!_alarmManager)
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager is null"
        );

        sendError(
            503,
            "alarm manager not initialized"
        );

        return;
    }

    if (
        !_sd ||
        !_sd->isReady()
    )
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] SD not available"
        );

        sendError(
            503,
            "SD not available"
        );

        return;
    }

    sendAlarmList();
}


// ============================================================
// GET /api/alarm?id=...
// ============================================================

void WebServerManager::handleGetAlarm()
{
    Serial0.println(
        "[WEB][ALARM] GET alarm"
    );

    if (!_alarmManager)
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager is null"
        );

        sendError(
            503,
            "alarm manager not initialized"
        );

        return;
    }

    String id =
        _server.arg("id");

    if (id.isEmpty())
    {
        id =
            alarmIdFromUri(
                _server.uri()
            );
    }

    Serial0.print(
        "[WEB][ALARM] ID: "
    );

    Serial0.println(id);

    if (id.isEmpty())
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] missing id"
        );

        sendError(
            400,
            "missing id"
        );

        return;
    }

    Alarm alarm;

    if (!_alarmManager->get(
            id,
            alarm
        ))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] get() failed"
        );

        sendError(
            404,
            "alarm not found"
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM] get() OK"
    );

    sendAlarm(
        alarm
    );
}


// ============================================================
// POST /api/alarm
// ============================================================

void WebServerManager::handleCreateAlarm()
{
    Serial0.println();
    Serial0.println(
        "========== ALARM CREATE =========="
    );

    Serial0.print(
        "[WEB][ALARM] URI: "
    );

    Serial0.println(
        _server.uri()
    );

    Serial0.print(
        "[WEB][ALARM] Method: "
    );

    Serial0.println(
        static_cast<int>(
            _server.method()
        )
    );

    Serial0.println(
        "[WEB][ALARM] Incoming JSON:"
    );

    if (_server.hasArg("plain"))
    {
        Serial0.println(
            _server.arg("plain")
        );
    }
    else
    {
        Serial0.println(
            "<NO BODY>"
        );
    }

    if (!_alarmManager)
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager is null"
        );

        sendError(
            503,
            "alarm manager not initialized"
        );

        return;
    }

    if (
        !_sd ||
        !_sd->isReady()
    )
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] SD not available"
        );

        sendError(
            503,
            "SD not available"
        );

        return;
    }

    Alarm alarm;

    Serial0.println(
        "[WEB][ALARM] parseAlarmFromRequest()..."
    );

    if (!parseAlarmFromRequest(
            alarm
        ))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] parseAlarmFromRequest() FAILED"
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM] deserialize OK"
    );

    Serial0.print(
        "[WEB][ALARM] ID: "
    );

    Serial0.println(
        alarm.id
    );

    Serial0.print(
        "[WEB][ALARM] Name: "
    );

    Serial0.println(
        alarm.name
    );

    Serial0.print(
        "[WEB][ALARM] Enabled: "
    );

    Serial0.println(
        alarm.enabled
            ? "true"
            : "false"
    );

    Serial0.print(
        "[WEB][ALARM] Time: "
    );

    Serial0.print(
        alarm.time.hour
    );

    Serial0.print(":");

    Serial0.print(
        alarm.time.minute
    );

    Serial0.print(":");

    Serial0.println(
        alarm.time.second
    );

    Serial0.print(
        "[WEB][ALARM] Repeat mask: "
    );

    Serial0.println(
        alarm.repeatMask
    );

    Serial0.print(
        "[WEB][ALARM] Phase count: "
    );

    Serial0.println(
        alarm.phaseCount
    );


    // --------------------------------------------------------
    // PHASE DEBUG
    // --------------------------------------------------------

    for (
        uint8_t i = 0;
        i < alarm.phaseCount &&
        i < AlarmConfig::MAX_PHASES;
        ++i
    )
    {
        const AlarmPhase& phase =
            alarm.phases[i];

        Serial0.print(
            "[WEB][ALARM] Phase "
        );

        Serial0.print(i);

        Serial0.print(
            ": offset="
        );

        Serial0.print(
            phase.startOffsetMs
        );

        Serial0.print(
            " duration="
        );

        Serial0.print(
            phase.durationMs
        );

        Serial0.print(
            " condition="
        );

        Serial0.println(
            static_cast<uint8_t>(
                phase.condition
            )
        );

        Serial0.print(
            "  matrix enabled="
        );

        Serial0.print(
            phase.matrix.enabled
                ? "true"
                : "false"
        );

        Serial0.print(
            " effect="
        );

        Serial0.print(
            phase.matrix.effectId
        );

        Serial0.print(
            " start="
        );

        Serial0.print(
            phase.matrix.start
        );

        Serial0.print(
            " end="
        );

        Serial0.print(
            phase.matrix.end
        );

        Serial0.print(
            " speedMs="
        );

        Serial0.print(
            phase.matrix.speedMs
        );

        Serial0.print(
            " durationMs="
        );

        Serial0.println(
            phase.matrix.durationMs
        );

        Serial0.print(
            "  audio enabled="
        );

        Serial0.print(
            phase.audio.enabled
                ? "true"
                : "false"
        );

        Serial0.print(
            " effect="
        );

        Serial0.print(
            phase.audio.effectId
        );

        Serial0.print(
            " start="
        );

        Serial0.print(
            phase.audio.start
        );

        Serial0.print(
            " end="
        );

        Serial0.print(
            phase.audio.end
        );

        Serial0.print(
            " speedMs="
        );

        Serial0.print(
            phase.audio.speedMs
        );

        Serial0.print(
            " durationMs="
        );

        Serial0.print(
            phase.audio.durationMs
        );

        Serial0.print(
            " loop="
        );

        Serial0.println(
            phase.audio.loop
                ? "true"
                : "false"
        );

        Serial0.print(
            "  cob enabled="
        );

        Serial0.print(
            phase.cob.enabled
                ? "true"
                : "false"
        );

        Serial0.print(
            " effect="
        );

        Serial0.print(
            phase.cob.effectId
        );

        Serial0.print(
            " start="
        );

        Serial0.print(
            phase.cob.start
        );

        Serial0.print(
            " end="
        );

        Serial0.print(
            phase.cob.end
        );

        Serial0.print(
            " speedMs="
        );

        Serial0.print(
            phase.cob.speedMs
        );

        Serial0.print(
            " durationMs="
        );

        Serial0.print(
            phase.cob.durationMs
        );

        Serial0.print(
            " maxDurationMs="
        );

        Serial0.println(
            phase.cob.maxDurationMs
        );
    }


    // --------------------------------------------------------
    // GENERATE ID
    // --------------------------------------------------------

    if (alarm.id.isEmpty())
    {
        alarm.id =
            String("alarm_") +
            String(
                millis(),
                HEX
            ) +
            String(
                random(
                    0x10000000,
                    0x7FFFFFFF
                ),
                HEX
            );

        Serial0.print(
            "[WEB][ALARM] Generated ID: "
        );

        Serial0.println(
            alarm.id
        );
    }
    else
    {
        Serial0.println(
            "[WEB][ALARM] Existing ID preserved"
        );
    }


    // --------------------------------------------------------
    // CREATE
    // --------------------------------------------------------

    Serial0.println(
        "[WEB][ALARM] Calling AlarmManager::create()..."
    );

    if (!_alarmManager->create(
            alarm
        ))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager::create() FAILED"
        );

        sendError(
            409,
            "failed to create alarm"
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM] AlarmManager::create() OK"
    );


    // --------------------------------------------------------
    // READ AFTER CREATE
    // --------------------------------------------------------

    Alarm saved;

    Serial0.println(
        "[WEB][ALARM] Calling AlarmManager::get()..."
    );

    if (!_alarmManager->get(
            alarm.id,
            saved
        ))
    {
        Serial0.println(
            "[WEB][ALARM][WARNING] get() after create FAILED"
        );

        sendAlarm(
            alarm
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM] Alarm saved and loaded successfully"
    );

    sendAlarm(
        saved
    );

    Serial0.println(
        "========== ALARM CREATE END =========="
    );
}


// ============================================================
// PUT /api/alarm
// ============================================================

void WebServerManager::handleUpdateAlarm()
{
    Serial0.println();
    Serial0.println(
        "========== ALARM UPDATE =========="
    );

    Serial0.print(
        "[WEB][ALARM] URI: "
    );

    Serial0.println(
        _server.uri()
    );

    Serial0.print(
        "[WEB][ALARM] Method: "
    );

    Serial0.println(
        static_cast<int>(
            _server.method()
        )
    );

    Serial0.println(
        "[WEB][ALARM] Incoming JSON:"
    );

    if (_server.hasArg("plain"))
    {
        Serial0.println(
            _server.arg("plain")
        );
    }
    else
    {
        Serial0.println(
            "<NO BODY>"
        );
    }

    if (!_alarmManager)
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager is null"
        );

        sendError(
            503,
            "alarm manager not initialized"
        );

        return;
    }

    if (
        !_sd ||
        !_sd->isReady()
    )
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] SD not available"
        );

        sendError(
            503,
            "SD not available"
        );

        return;
    }

    Alarm alarm;

    Serial0.println(
        "[WEB][ALARM] parseAlarmFromRequest()..."
    );

    if (!parseAlarmFromRequest(
            alarm
        ))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] parseAlarmFromRequest() FAILED"
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM] JSON deserialized"
    );

    if (alarm.id.isEmpty())
    {
        const String pathId =
            alarmIdFromUri(
                _server.uri()
            );

        if (!pathId.isEmpty())
        {
            alarm.id =
                pathId;

            Serial0.print(
                "[WEB][ALARM] ID taken from URI: "
            );

            Serial0.println(
                alarm.id
            );
        }
    }

    Serial0.print(
        "[WEB][ALARM] Final ID: "
    );

    Serial0.println(
        alarm.id
    );

    if (alarm.id.isEmpty())
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] Missing ID"
        );

        sendError(
            400,
            "missing id"
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM] Checking exists()..."
    );

    if (!_alarmManager->exists(
            alarm.id
        ))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] Alarm does not exist"
        );

        sendError(
            404,
            "alarm not found"
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM] Alarm exists"
    );

    Serial0.println(
        "[WEB][ALARM] Calling AlarmManager::update()..."
    );

    if (!_alarmManager->update(
            alarm
        ))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager::update() FAILED"
        );

        sendError(
            409,
            "failed to update alarm"
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM] AlarmManager::update() OK"
    );

    Alarm saved;

    Serial0.println(
        "[WEB][ALARM] Calling AlarmManager::get()..."
    );

    if (_alarmManager->get(
            alarm.id,
            saved
        ))
    {
        Serial0.println(
            "[WEB][ALARM] Updated alarm read successfully"
        );

        sendAlarm(
            saved
        );

        Serial0.println(
            "========== ALARM UPDATE END =========="
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM][WARNING] get() after update FAILED"
    );

    sendAlarm(
        alarm
    );

    Serial0.println(
        "========== ALARM UPDATE END =========="
    );
}


// ============================================================
// DELETE /api/alarm?id=...
// ============================================================

void WebServerManager::handleDeleteAlarm()
{
    Serial0.println();
    Serial0.println(
        "========== ALARM DELETE =========="
    );

    if (!_alarmManager)
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager is null"
        );

        sendError(
            503,
            "alarm manager not initialized"
        );

        return;
    }

    String id =
        _server.arg("id");

    if (id.isEmpty())
    {
        id =
            alarmIdFromUri(
                _server.uri()
            );
    }

    Serial0.print(
        "[WEB][ALARM] Delete ID: "
    );

    Serial0.println(id);

    if (id.isEmpty())
    {
        sendError(
            400,
            "missing id"
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM] Calling AlarmManager::remove()..."
    );

    if (!_alarmManager->remove(
            id
        ))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] remove() FAILED"
        );

        sendError(
            404,
            "alarm not found"
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM] remove() OK"
    );

    sendOk();

    Serial0.println(
        "========== ALARM DELETE END =========="
    );
}


// ============================================================
// POST /api/alarm/enable?id=...
// ============================================================

void WebServerManager::handleEnableAlarm()
{
    Serial0.println(
        "[WEB][ALARM] ENABLE"
    );

    if (!_alarmManager)
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager is null"
        );

        sendError(
            503,
            "alarm manager not initialized"
        );

        return;
    }

    const String id =
        _server.arg("id");

    Serial0.print(
        "[WEB][ALARM] Enable ID: "
    );

    Serial0.println(id);

    if (id.isEmpty())
    {
        sendError(
            400,
            "missing id"
        );

        return;
    }

    if (!_alarmManager->enable(
            id
        ))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] enable() FAILED"
        );

        sendError(
            404,
            "alarm not found"
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM] enable() OK"
    );

    sendOk();
}


// ============================================================
// POST /api/alarm/disable?id=...
// ============================================================

void WebServerManager::handleDisableAlarm()
{
    Serial0.println(
        "[WEB][ALARM] DISABLE"
    );

    if (!_alarmManager)
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager is null"
        );

        sendError(
            503,
            "alarm manager not initialized"
        );

        return;
    }

    const String id =
        _server.arg("id");

    Serial0.print(
        "[WEB][ALARM] Disable ID: "
    );

    Serial0.println(id);

    if (id.isEmpty())
    {
        sendError(
            400,
            "missing id"
        );

        return;
    }

    if (!_alarmManager->disable(
            id
        ))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] disable() FAILED"
        );

        sendError(
            404,
            "alarm not found"
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM] disable() OK"
    );

    sendOk();
}


// ============================================================
// GET /api/alarm/runtime
// ============================================================

void WebServerManager::handleAlarmRuntime()
{
    Serial0.println(
        "[WEB][ALARM] GET runtime"
    );

    if (!_alarmManager)
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager is null"
        );

        sendError(
            503,
            "alarm manager not initialized"
        );

        return;
    }

    JsonDocument doc;

    doc["active"] =
        _alarmManager->isRunning();

    if (_alarmManager->isRunning())
    {
        const Alarm* alarm =
            _alarmManager->currentAlarm();

        const AlarmPhase* phase =
            _alarmManager->currentPhase();

        if (alarm)
        {
            doc["id"] =
                alarm->id;

            doc["name"] =
                alarm->name;
        }

        doc["phase"] =
            _alarmManager->currentPhaseIndex();

        doc["elapsedMs"] =
            _alarmManager->elapsedMs();

        doc["phaseActive"] =
            phase != nullptr;
    }

    String body;

    serializeJson(
        doc,
        body
    );

    sendJson(
        200,
        body
    );
}


// ============================================================
// POST /api/alarm/dismiss
// ============================================================

void WebServerManager::handleAlarmDismiss()
{
    Serial0.println(
        "[WEB][ALARM] DISMISS"
    );

    if (!_alarmManager)
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager is null"
        );

        sendError(
            503,
            "alarm manager not initialized"
        );

        return;
    }

    if (!_alarmManager->isRunning())
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] No active alarm"
        );

        sendError(
            409,
            "no active alarm"
        );

        return;
    }

    if (!_alarmManager->dismiss())
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] dismiss() FAILED"
        );

        sendError(
            409,
            "dismiss failed"
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM] dismiss() OK"
    );

    sendOk();
}


// ============================================================
// POST /api/alarm/snooze
// ============================================================

void WebServerManager::handleAlarmSnooze()
{
    Serial0.println(
        "[WEB][ALARM] SNOOZE"
    );

    if (!_alarmManager)
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager is null"
        );

        sendError(
            503,
            "alarm manager not initialized"
        );

        return;
    }

    if (!_alarmManager->isRunning())
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] No active alarm"
        );

        sendError(
            409,
            "no active alarm"
        );

        return;
    }

    JsonDocument doc;

    if (!parseJson(doc))
        return;

    uint32_t durationMs = 0;

    if (!doc["durationMs"].isNull())
    {
        durationMs =
            doc["durationMs"].as<uint32_t>();
    }
    else if (!doc["duration"].isNull())
    {
        durationMs =
            doc["duration"].as<uint32_t>();
    }

    Serial0.print(
        "[WEB][ALARM] Snooze duration: "
    );

    Serial0.println(
        durationMs
    );

    if (durationMs == 0)
    {
        sendError(
            400,
            "missing durationMs"
        );

        return;
    }

    if (!_alarmManager->snooze(
            durationMs
        ))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] snooze() FAILED"
        );

        sendError(
            409,
            "snooze failed"
        );

        return;
    }

    Serial0.println(
        "[WEB][ALARM] snooze() OK"
    );

    sendOk();
}


// ============================================================
// PARSE ALARM
// ============================================================

bool WebServerManager::parseAlarmFromRequest(
    Alarm& alarm
)
{
    Serial0.println(
        "[WEB][ALARM] Parsing alarm JSON..."
    );

    JsonDocument doc;

    if (!parseJson(doc))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] parseJson() FAILED"
        );

        return false;
    }

    if (!_alarmManager)
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager is null"
        );

        sendError(
            503,
            "alarm manager not initialized"
        );

        return false;
    }

    Serial0.println(
        "[WEB][ALARM] Calling AlarmManager::deserialize()..."
    );

    if (!_alarmManager->deserialize(
            doc,
            alarm
        ))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager::deserialize() FAILED"
        );

        sendError(
            400,
            "invalid alarm"
        );

        return false;
    }

    Serial0.println(
        "[WEB][ALARM] AlarmManager::deserialize() OK"
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
    Serial0.println(
        "[WEB][ALARM] Sending alarm response..."
    );

    if (!_alarmManager)
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] AlarmManager is null"
        );

        sendError(
            503,
            "alarm manager not initialized"
        );

        return;
    }

    JsonDocument doc;

    if (!_alarmManager->serialize(
            alarm,
            doc
        ))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] serialize() FAILED"
        );

        sendError(
            500,
            "failed to serialize alarm"
        );

        return;
    }

    doc["ok"] = true;

    String body;

    serializeJson(
        doc,
        body
    );

    Serial0.print(
        "[WEB][ALARM] Response JSON: "
    );

    Serial0.println(
        body
    );

    sendJson(
        200,
        body
    );
}


// ============================================================
// SEND ALARM LIST
// ============================================================

void WebServerManager::sendAlarmList()
{
    Serial0.println(
        "[WEB][ALARM] Loading alarm list..."
    );

    static constexpr uint8_t MAX_ALARMS =
        AlarmConfig::MAX_ALARMS;

    Alarm alarms[MAX_ALARMS];

    uint8_t count = 0;

    if (!_alarmManager->loadAll(
            alarms,
            MAX_ALARMS,
            count
        ))
    {
        Serial0.println(
            "[WEB][ALARM][ERROR] loadAll() FAILED"
        );

        sendError(
            500,
            "failed to load alarms"
        );

        return;
    }

    Serial0.print(
        "[WEB][ALARM] Loaded alarms: "
    );

    Serial0.println(
        count
    );

    JsonDocument doc;

    doc["ok"] = true;

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

        JsonDocument alarmDoc;

        if (!_alarmManager->serialize(
                alarms[i],
                alarmDoc
            ))
        {
            Serial0.print(
                "[WEB][ALARM][ERROR] serialize alarm #"
            );

            Serial0.println(
                i
            );

            continue;
        }

        item.set(
            alarmDoc.as<JsonObject>()
        );

        Serial0.print(
            "[WEB][ALARM] Alarm #"
        );

        Serial0.print(
            i
        );

        Serial0.print(
            " ID="
        );

        Serial0.println(
            alarms[i].id
        );
    }

    doc["count"] =
        array.size();

    String body;

    serializeJson(
        doc,
        body
    );

    Serial0.print(
        "[WEB][ALARM] Alarm list response: "
    );

    Serial0.println(
        body
    );

    sendJson(
        200,
        body
    );
}


// ============================================================
// PARSE JSON
// ============================================================

bool WebServerManager::parseJson(
    JsonDocument& doc
)
{
    if (!_server.hasArg("plain"))
    {
        Serial0.println(
            "[WEB][JSON][ERROR] Body missing"
        );

        sendError(
            400,
            "body missing"
        );

        return false;
    }

    const String raw =
        _server.arg("plain");

    Serial0.print(
        "[WEB][JSON] Body size: "
    );

    Serial0.println(
        raw.length()
    );

    Serial0.println(
        "[WEB][JSON] Body:"
    );

    Serial0.println(
        raw
    );

    const DeserializationError error =
        deserializeJson(
            doc,
            raw
        );

    if (error)
    {
        Serial0.print(
            "[WEB][JSON][ERROR] deserializeJson: "
        );

        Serial0.println(
            error.c_str()
        );

        sendError(
            400,
            "invalid JSON"
        );

        return false;
    }

    Serial0.println(
        "[WEB][JSON] JSON OK"
    );

    return true;
}


// ============================================================
// SEND JSON
// ============================================================

void WebServerManager::sendJson(
    int code,
    const String& body
)
{
    Serial0.print(
        "[WEB][RESPONSE] HTTP "
    );

    Serial0.print(
        code
    );

    Serial0.print(
        " body: "
    );

    Serial0.println(
        body
    );

    _server.send(
        code,
        "application/json",
        body
    );
}


// ============================================================
// SEND OK
// ============================================================

void WebServerManager::sendOk()
{
    Serial0.println(
        "[WEB][RESPONSE] OK"
    );

    sendJson(
        200,
        "{\"ok\":true}"
    );
}


// ============================================================
// SEND ERROR
// ============================================================

void WebServerManager::sendError(
    int code,
    const char* message
)
{
    Serial0.print(
        "[WEB][RESPONSE][ERROR] HTTP "
    );

    Serial0.print(
        code
    );

    Serial0.print(
        ": "
    );

    Serial0.println(
        message
            ? message
            : "unknown error"
    );

    JsonDocument doc;

    doc["ok"] = false;

    doc["error"] =
        message
            ? message
            : "unknown error";

    String body;

    serializeJson(
        doc,
        body
    );

    sendJson(
        code,
        body
    );
}


// ============================================================
// WIFI
// ============================================================

bool WebServerManager::isConnected() const
{
    return WiFi.status() ==
        WL_CONNECTED;
}


// ============================================================
// IP
// ============================================================

String WebServerManager::getIP() const
{
    if (!isConnected())
        return "0.0.0.0";

    return WiFi.localIP().toString();
}
