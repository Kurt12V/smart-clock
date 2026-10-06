#include "WebServerManager.h"

#include <cstring>
#include <memory>
#include <new>


namespace
{

// ============================================================
// SERIAL LOG HELPERS
// ============================================================

void serialLog(const char* message)
{
    Serial0.print("[WebServerManager] ");
    Serial0.println(message);
}

void serialLogKV(const char* key, const String& value)
{
    Serial0.print("[WebServerManager] ");
    Serial0.print(key);
    Serial0.print(": ");
    Serial0.println(value);
}

void serialLogCStr(const char* key, const char* value)
{
    Serial0.print("[WebServerManager] ");
    Serial0.print(key);
    Serial0.print(": ");
    Serial0.println(value != nullptr ? value : "(null)");
}

void serialLogUInt(const char* key, uint32_t value)
{
    Serial0.print("[WebServerManager] ");
    Serial0.print(key);
    Serial0.print(": ");
    Serial0.println(value);
}

void serialLogInt(const char* key, long value)
{
    Serial0.print("[WebServerManager] ");
    Serial0.print(key);
    Serial0.print(": ");
    Serial0.println(value);
}

void serialLogBool(const char* key, bool value)
{
    Serial0.print("[WebServerManager] ");
    Serial0.print(key);
    Serial0.print(": ");
    Serial0.println(value ? "true" : "false");
}

void serialLogError(int code, const char* message)
{
    Serial0.print("[WebServerManager] ERROR ");
    Serial0.print(code);
    Serial0.print(": ");
    Serial0.println(message != nullptr ? message : "unknown error");
}

const char* httpMethodName(HTTPMethod method)
{
    switch (method)
    {
        case HTTP_GET:     return "GET";
        case HTTP_POST:    return "POST";
        case HTTP_PUT:     return "PUT";
        case HTTP_DELETE:  return "DELETE";
        case HTTP_PATCH:   return "PATCH";
        case HTTP_OPTIONS: return "OPTIONS";
        default:           return "OTHER";
    }
}


// ============================================================
// HTTP CODES
// ============================================================

constexpr int HTTP_OK_CODE                  = 200;
constexpr int HTTP_BAD_REQUEST_CODE        = 400;
constexpr int HTTP_NOT_FOUND_CODE          = 404;
constexpr int HTTP_CONFLICT_CODE           = 409;
constexpr int HTTP_INTERNAL_ERROR_CODE     = 500;
constexpr int HTTP_SERVICE_UNAVAILABLE_CODE = 503;


// ============================================================
// SERVER CONFIG
// ============================================================

constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS =
    20000UL;

constexpr uint32_t MAX_SNOOZE_MS =
    24UL * 60UL * 60UL * 1000UL;

constexpr size_t MAX_SD_FILES =
    300;


// ============================================================
// PATH HELPERS
// ============================================================

bool isAlarmItemPath(
    const String& uri
)
{
    constexpr const char* PREFIX =
        "/api/alarms/";

    if (!uri.startsWith(PREFIX))
        return false;

    const String id =
        uri.substring(strlen(PREFIX));

    if (id.isEmpty())
        return false;

    if (id.indexOf('/') >= 0)
        return false;

    return true;
}


// ============================================================
// ALARM ENABLE PATH
// ============================================================

bool isAlarmEnabledPath(
    const String& uri
)
{
    constexpr const char* PREFIX =
        "/api/alarms/";

    constexpr const char* SUFFIX =
        "/enabled";

    if (!uri.startsWith(PREFIX))
        return false;

    if (!uri.endsWith(SUFFIX))
        return false;

    const int start =
        strlen(PREFIX);

    const int end =
        uri.length() - strlen(SUFFIX);

    if (end <= start)
        return false;

    const String id =
        uri.substring(start, end);

    if (id.isEmpty())
        return false;

    if (id.indexOf('/') >= 0)
        return false;

    return true;
}


// ============================================================
// ALARM ID FROM URI
// ============================================================

String alarmIdFromUri(
    const String& uri
)
{
    constexpr const char* PREFIX =
        "/api/alarms/";

    if (!uri.startsWith(PREFIX))
        return String();

    String id =
        uri.substring(strlen(PREFIX));

    const int slash =
        id.indexOf('/');

    if (slash >= 0)
    {
        id =
            id.substring(0, slash);
    }

    return id;
}


// ============================================================
// ALARM ID FROM ENABLE PATH
// ============================================================

String alarmIdFromEnabledUri(
    const String& uri
)
{
    constexpr const char* PREFIX =
        "/api/alarms/";

    constexpr const char* SUFFIX =
        "/enabled";

    if (!uri.startsWith(PREFIX))
        return String();

    if (!uri.endsWith(SUFFIX))
        return String();

    const int start =
        strlen(PREFIX);

    const int end =
        uri.length() - strlen(SUFFIX);

    if (end <= start)
        return String();

    return uri.substring(start, end);
}

} // namespace


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
    serialLog("constructor");
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
    Serial0.begin(115200);
    delay(50);

    serialLog("begin()");

    if (_initialized)
    {
        serialLog("already initialized");
        return true;
    }

    if (ssid == nullptr)
    {
        serialLog("ssid is null");
        return false;
    }

    _settings = &settings;
    _sd       = &sd;
    _sound    = &sound;

    serialLogCStr("ssid", ssid);

    WiFi.mode(WIFI_STA);

    WiFi.disconnect(true);

    delay(100);

    WiFi.begin(
        ssid,
        password
    );

    serialLog("WiFi connecting...");

    const uint32_t startTime =
        millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < WIFI_CONNECT_TIMEOUT_MS
    )
    {
        delay(100);
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        serialLog("WiFi connection timeout");
        return false;
    }

    serialLog("WiFi connected");
    serialLogKV("IP", WiFi.localIP().toString());

    if (!LittleFS.begin(true))
    {
        serialLog("LittleFS mount failed");
        return false;
    }

    serialLog("LittleFS mounted");

    setupRoutes();

    _server.begin();

    _initialized = true;

    serialLog("HTTP server started on port 80");

    return true;
}


// ============================================================
// SET ALARM MANAGER
// ============================================================

void WebServerManager::setAlarmManager(
    AlarmManager& alarmManager
)
{
    serialLog("setAlarmManager()");

    _alarmManager =
        &alarmManager;
}


// ============================================================
// SET ALARM CONTROLLER
// ============================================================

void WebServerManager::setAlarmController(
    AlarmController& alarmController
)
{
    serialLog("setAlarmController()");

    _alarmController =
        &alarmController;
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
// CONNECTION
// ============================================================

bool WebServerManager::isConnected() const
{
    return
        _initialized &&
        WiFi.status() == WL_CONNECTED;
}


// ============================================================
// IP
// ============================================================

String WebServerManager::getIP() const
{
    if (WiFi.status() != WL_CONNECTED)
        return String();

    return WiFi.localIP().toString();
}


// ============================================================
// ROUTES
// ============================================================

void WebServerManager::setupRoutes()
{
    serialLog("setupRoutes()");

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


    // ========================================================
    // LEGACY ALARM CONTROL
    // ========================================================

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
    // NOT FOUND / DYNAMIC ROUTES
    // ========================================================

    _server.onNotFound(
        [this]()
        {
            handleNotFound();
        }
    );

    serialLog("setupRoutes() done");
}


// ============================================================
// ROOT
// ============================================================

void WebServerManager::handleRoot()
{
    serialLog("handleRoot()");

    if (!LittleFS.exists("/index.html"))
    {
        serialLog("index.html not found");

        sendError(
            HTTP_NOT_FOUND_CODE,
            "index.html not found"
        );

        return;
    }

    File file =
        LittleFS.open(
            "/index.html",
            FILE_READ
        );

    if (!file)
    {
        serialLog("failed to open index.html");

        sendError(
            HTTP_INTERNAL_ERROR_CODE,
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
// NOT FOUND
// ============================================================

void WebServerManager::handleNotFound()
{
    const String uri =
        _server.uri();

    const HTTPMethod method =
        _server.method();

    Serial0.print("[WebServerManager] handleNotFound uri=");
    Serial0.print(uri);
    Serial0.print(" method=");
    Serial0.println(httpMethodName(method));


    // ========================================================
    // GET /api/alarms/{id}
    // ========================================================

    if (
        method == HTTP_GET &&
        isAlarmItemPath(uri)
    )
    {
        handleGetAlarm();
        return;
    }


    // ========================================================
    // PUT /api/alarms/{id}
    // ========================================================

    if (
        method == HTTP_PUT &&
        isAlarmItemPath(uri)
    )
    {
        handleUpdateAlarm();
        return;
    }


    // ========================================================
    // DELETE /api/alarms/{id}
    // ========================================================

    if (
        method == HTTP_DELETE &&
        isAlarmItemPath(uri)
    )
    {
        handleDeleteAlarm();
        return;
    }


    // ========================================================
    // POST /api/alarms/{id}/enabled
    // ========================================================

    if (
        method == HTTP_POST &&
        isAlarmEnabledPath(uri)
    )
    {
        const String id =
            alarmIdFromEnabledUri(uri);

        if (!isValidAlarmId(id))
        {
            serialLog("invalid alarm id");

            sendError(
                HTTP_BAD_REQUEST_CODE,
                "invalid alarm id"
            );

            return;
        }

        if (_alarmManager == nullptr)
        {
            serialLog("alarm manager unavailable");

            sendError(
                HTTP_SERVICE_UNAVAILABLE_CODE,
                "alarm manager unavailable"
            );

            return;
        }

        JsonDocument doc;

        if (!parseJson(doc))
            return;

        JsonObjectConst root =
            doc.as<JsonObjectConst>();

        bool enabled = false;

        if (
            !getBoolean(
                root,
                "enabled",
                enabled
            )
        )
        {
            serialLog("enabled must be boolean");

            sendError(
                HTTP_BAD_REQUEST_CODE,
                "enabled must be boolean"
            );

            return;
        }

        serialLogKV("setEnabled id", id);
        serialLogBool("setEnabled value", enabled);

        if (
            !_alarmManager->setEnabled(
                id,
                enabled
            )
        )
        {
            serialLog("failed to change alarm state");

            sendError(
                HTTP_BAD_REQUEST_CODE,
                "failed to change alarm state"
            );

            return;
        }

        sendOk();

        return;
    }


    // ========================================================
    // UNKNOWN ROUTE
    // ========================================================

    serialLog("route not found");

    sendError(
        HTTP_NOT_FOUND_CODE,
        "route not found"
    );
}


// ============================================================
// GET PARAM
// ============================================================

void WebServerManager::handleGetParam()
{
    serialLog("handleGetParam()");

    if (_settings == nullptr)
    {
        serialLog("settings unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "settings unavailable"
        );

        return;
    }

    String name =
        _server.arg("name");

    if (name.isEmpty())
        name = _server.arg("param");

    if (name.isEmpty())
    {
        serialLog("missing parameter name");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "missing parameter name"
        );

        return;
    }

    serialLogKV("param name", name);

    const SettingsManager::Param param =
        _settings->paramFromName(
            name.c_str()
        );

    if (
        param >= SettingsManager::Param::COUNT
    )
    {
        serialLog("unknown parameter");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "unknown parameter"
        );

        return;
    }

    JsonDocument doc;

    doc["ok"] = true;

    doc["param"] =
        _settings->paramName(param);

    doc["value"] =
        _settings->get(param);

    const SettingsManager::ParamDesc& desc =
        _settings->getDesc(param);

    doc["min"] =
        desc.minValue;

    doc["max"] =
        desc.maxValue;

    doc["default"] =
        desc.defaultValue;

    String body;

    serializeJson(
        doc,
        body
    );

    sendJson(
        HTTP_OK_CODE,
        body
    );
}


// ============================================================
// SET PARAM
// ============================================================

void WebServerManager::handleSetParam()
{
    serialLog("handleSetParam()");

    if (_settings == nullptr)
    {
        serialLog("settings unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "settings unavailable"
        );

        return;
    }

    JsonDocument doc;

    if (!parseJson(doc))
        return;

    JsonObjectConst root =
        doc.as<JsonObjectConst>();

    const char* name = nullptr;

    if (root["param"].is<const char*>())
        name = root["param"].as<const char*>();

    if (
        name == nullptr &&
        root["name"].is<const char*>()
    )
    {
        name =
            root["name"].as<const char*>();
    }

    if (
        name == nullptr ||
        *name == '\0'
    )
    {
        serialLog("missing parameter name");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "missing parameter name"
        );

        return;
    }

    if (!root["value"].is<int>() &&
        !root["value"].is<long>() &&
        !root["value"].is<float>() &&
        !root["value"].is<double>())
    {
        serialLog("invalid parameter value");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "invalid parameter value"
        );

        return;
    }

    const SettingsManager::Param param =
        _settings->paramFromName(name);

    if (
        param >= SettingsManager::Param::COUNT
    )
    {
        serialLog("unknown parameter");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "unknown parameter"
        );

        return;
    }

    const SettingsManager::ParamDesc& desc =
        _settings->getDesc(param);

    long value =
        root["value"].as<long>();

    if (value < desc.minValue)
        value = desc.minValue;

    if (value > desc.maxValue)
        value = desc.maxValue;

    const int oldValue =
        _settings->get(param);

    if (
        !_settings->set(
            param,
            static_cast<int>(value)
        )
    )
    {
        serialLog("failed to set parameter");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "failed to set parameter"
        );

        return;
    }

    const int actualValue =
        _settings->get(param);

    serialLogCStr("set param name", name);
    serialLogInt("set param old", oldValue);
    serialLogInt("set param new", actualValue);

    JsonDocument response;

    response["ok"] = true;

    response["param"] =
        _settings->paramName(param);

    response["value"] =
        actualValue;

    response["previous"] =
        oldValue;

    response["changed"] =
        oldValue != actualValue;

    String body;

    serializeJson(
        response,
        body
    );

    sendJson(
        HTTP_OK_CODE,
        body
    );
}


// ============================================================
// GET ALL PARAMS
// ============================================================

void WebServerManager::handleGetAllParams()
{
    serialLog("handleGetAllParams()");

    if (_settings == nullptr)
    {
        serialLog("settings unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "settings unavailable"
        );

        return;
    }

    JsonDocument doc;

    doc["ok"] = true;

    JsonObject params =
        doc["params"].to<JsonObject>();

    for (
        uint8_t i = 0;
        i < static_cast<uint8_t>(
                SettingsManager::Param::COUNT
            );
        ++i
    )
    {
        const SettingsManager::Param param =
            static_cast<SettingsManager::Param>(i);

        const char* name =
            _settings->paramName(param);

        if (
            name == nullptr ||
            *name == '\0'
        )
        {
            continue;
        }

        params[name] =
            _settings->get(param);
    }

    String body;

    serializeJson(
        doc,
        body
    );

    sendJson(
        HTTP_OK_CODE,
        body
    );
}


// ============================================================
// RESET
// ============================================================

void WebServerManager::handleReset()
{
    serialLog("handleReset()");

    if (_settings == nullptr)
    {
        serialLog("settings unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "settings unavailable"
        );

        return;
    }

    _settings->resetAll();

    sendOk();
}


// ============================================================
// SENSORS
// ============================================================

void WebServerManager::handleSensors()
{
    serialLog("handleSensors()");

    /*
     * Здесь намеренно нет фиктивных значений.
     *
     * Реальные SHT45 / VEML7700 / VL53L8CX
     * должны передаваться через соответствующий
     * SensorManager.
     *
     * Пока SensorManager не подключён к этому классу,
     * возвращаем состояние API.
     */

    JsonDocument doc;

    doc["ok"] = true;

    JsonObject sensors =
        doc["sensors"].to<JsonObject>();

    sensors["temperature"] = nullptr;
    sensors["humidity"] = nullptr;
    sensors["light"] = nullptr;
    sensors["distance"] = nullptr;

    String body;

    serializeJson(
        doc,
        body
    );

    sendJson(
        HTTP_OK_CODE,
        body
    );
}


// ============================================================
// SD
// ============================================================

void WebServerManager::handleSD()
{
    serialLog("handleSD()");

    if (_sd == nullptr)
    {
        serialLog("sd unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "sd unavailable"
        );

        return;
    }

    if (!_sd->isReady())
    {
        serialLog("sd card not ready");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "sd card not ready"
        );

        return;
    }

    std::unique_ptr<SDFileEntry[]> entries(
        new (std::nothrow) SDFileEntry[MAX_SD_FILES]
    );

    if (!entries)
    {
        serialLog("not enough memory for sd file list");

        sendError(
            HTTP_INTERNAL_ERROR_CODE,
            "not enough memory for sd file list"
        );

        return;
    }

    const size_t count =
        _sd->listFiles(
            entries.get(),
            MAX_SD_FILES,
            3,
            "/"
        );

    serialLogUInt("sd file count", static_cast<uint32_t>(count));

    JsonDocument doc;

    doc["ok"] = true;

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
                ? "directory"
                : "file";
    }

    doc["count"] =
        static_cast<uint32_t>(count);

    String body;

    serializeJson(
        doc,
        body
    );

    sendJson(
        HTTP_OK_CODE,
        body
    );
}


// ============================================================
// AUDIO PLAY
// ============================================================

void WebServerManager::handleAudioPlay()
{
    serialLog("handleAudioPlay()");

    if (_sound == nullptr)
    {
        serialLog("sound manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "sound manager unavailable"
        );

        return;
    }

    if (_sd == nullptr || !_sd->isReady())
    {
        serialLog("sd card not ready");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "sd card not ready"
        );

        return;
    }

    JsonDocument doc;

    if (!parseJson(doc))
        return;

    JsonObjectConst root =
        doc.as<JsonObjectConst>();

    const char* path = nullptr;

    if (root["path"].is<const char*>())
        path = root["path"].as<const char*>();

    if (
        path == nullptr &&
        root["file"].is<const char*>()
    )
    {
        path =
            root["file"].as<const char*>();
    }

    if (
        path == nullptr ||
        *path == '\0'
    )
    {
        serialLog("missing audio path");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "missing audio path"
        );

        return;
    }

    serialLogCStr("audio path", path);

    if (!_sd->fileExists(path))
    {
        serialLog("audio file not found");

        sendError(
            HTTP_NOT_FOUND_CODE,
            "audio file not found"
        );

        return;
    }


    // ========================================================
    // PLAY OPTIONS
    // ========================================================

    SoundManager::PlayOptions options;


    // --------------------------------------------------------
    // STREAM
    // --------------------------------------------------------

    const char* streamName =
        root["stream"] |
        "media";

    if (strcmp(streamName, "alarm") == 0)
    {
        options.stream =
            SoundManager::AudioStream::Alarm;
    }
    else if (
        strcmp(streamName, "system") == 0
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

    serialLogCStr("audio stream", streamName);


    // --------------------------------------------------------
    // LOCAL VOLUME
    // --------------------------------------------------------

    if (root["volume"].is<int>())
    {
        const int volume =
            root["volume"].as<int>();

        options.localPercent =
            static_cast<uint8_t>(
                constrain(
                    volume,
                    0,
                    100
                )
            );

        serialLogInt("audio volume", volume);
    }


    // --------------------------------------------------------
    // FADE IN
    // --------------------------------------------------------

    uint32_t fadeInMs = 0;

    if (
        getUnsigned32(
            root,
            "fadeInMs",
            fadeInMs
        )
    )
    {
        options.fadeInMs =
            fadeInMs;
    }
    else
    {
        getUnsigned32(
            root,
            "fade_in",
            options.fadeInMs
        );
    }


    // --------------------------------------------------------
    // FADE OUT
    // --------------------------------------------------------

    uint32_t fadeOutMs = 0;

    if (
        getUnsigned32(
            root,
            "fadeOutMs",
            fadeOutMs
        )
    )
    {
        options.fadeOutMs =
            fadeOutMs;
    }
    else
    {
        getUnsigned32(
            root,
            "fade_out",
            options.fadeOutMs
        );
    }


    // --------------------------------------------------------
    // FADE CURVE
    // --------------------------------------------------------

    const char* curve =
        root["curve"] |
        "linear";

    if (strcmp(curve, "exp") == 0 ||
        strcmp(curve, "exponential") == 0)
    {
        options.curve =
            SoundManager::FadeCurve::Exponential;
    }
    else if (
        strcmp(curve, "log") == 0 ||
        strcmp(curve, "logarithmic") == 0
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


    // ========================================================
    // PLAY
    // ========================================================

    if (
        !_sound->play(
            path,
            options
        )
    )
    {
        serialLog("failed to play audio");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "failed to play audio"
        );

        return;
    }

    sendOk();
}


// ============================================================
// AUDIO PAUSE
// ============================================================

void WebServerManager::handleAudioPause()
{
    serialLog("handleAudioPause()");

    if (_sound == nullptr)
    {
        serialLog("sound manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "sound manager unavailable"
        );

        return;
    }

    if (!_sound->isPlaying())
    {
        serialLog("audio is not playing");

        sendError(
            HTTP_CONFLICT_CODE,
            "audio is not playing"
        );

        return;
    }

    if (!_sound->pause())
    {
        serialLog("failed to pause audio");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "failed to pause audio"
        );

        return;
    }

    sendOk();
}


// ============================================================
// AUDIO RESUME
// ============================================================

void WebServerManager::handleAudioResume()
{
    serialLog("handleAudioResume()");

    if (_sound == nullptr)
    {
        serialLog("sound manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "sound manager unavailable"
        );

        return;
    }

    if (!_sound->isPaused())
    {
        serialLog("audio is not paused");

        sendError(
            HTTP_CONFLICT_CODE,
            "audio is not paused"
        );

        return;
    }

    if (!_sound->resume())
    {
        serialLog("failed to resume audio");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "failed to resume audio"
        );

        return;
    }

    sendOk();
}


// ============================================================
// AUDIO STOP
// ============================================================

void WebServerManager::handleAudioStop()
{
    serialLog("handleAudioStop()");

    if (_sound == nullptr)
    {
        serialLog("sound manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "sound manager unavailable"
        );

        return;
    }

    uint32_t fadeOutMs = 0;

    if (
        _server.hasArg("plain") &&
        !_server.arg("plain").isEmpty()
    )
    {
        JsonDocument doc;

        if (!parseJson(doc))
            return;

        JsonObjectConst root =
            doc.as<JsonObjectConst>();

        if (
            !getUnsigned32(
                root,
                "fadeOutMs",
                fadeOutMs
            )
        )
        {
            getUnsigned32(
                root,
                "fade_out",
                fadeOutMs
            );
        }
    }

    serialLogUInt("audio stop fadeOutMs", fadeOutMs);

    if (fadeOutMs > 0)
        _sound->stop(fadeOutMs);
    else
        _sound->stop();

    sendOk();
}


// ============================================================
// AUDIO STATUS
// ============================================================

void WebServerManager::handleAudioStatus()
{
    serialLog("handleAudioStatus()");

    if (_sound == nullptr)
    {
        serialLog("sound manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "sound manager unavailable"
        );

        return;
    }

    JsonDocument doc;

    doc["ok"] = true;

    doc["initialized"] =
        _sound->isInitialized();

    doc["active"] =
        _sound->isActive();

    doc["playing"] =
        _sound->isPlaying();

    doc["paused"] =
        _sound->isPaused();

    doc["state"] =
        _sound->getStateString();

    doc["path"] =
        _sound->getCurrentPath();

    doc["positionMs"] =
        _sound->getPositionMs();

    doc["durationMs"] =
        _sound->getDurationMs();

    doc["localPercent"] =
        _sound->getLocalPercent();

    doc["effectiveVolume"] =
        _sound->getEffectiveVolume();

    const SoundManager::AudioStream stream =
        _sound->getCurrentStream();

    switch (stream)
    {
        case SoundManager::AudioStream::Media:
            doc["stream"] = "media";
            break;

        case SoundManager::AudioStream::Alarm:
            doc["stream"] = "alarm";
            break;

        case SoundManager::AudioStream::System:
            doc["stream"] = "system";
            break;
    }

    String body;

    serializeJson(
        doc,
        body
    );

    sendJson(
        HTTP_OK_CODE,
        body
    );
}


// ============================================================
// GET ALARMS
// ============================================================

void WebServerManager::handleGetAlarms()
{
    serialLog("handleGetAlarms()");

    if (_alarmManager == nullptr)
    {
        serialLog("alarm manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "alarm manager unavailable"
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
    serialLog("handleGetAlarm()");

    if (_alarmManager == nullptr)
    {
        serialLog("alarm manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "alarm manager unavailable"
        );

        return;
    }

    const String id =
        getAlarmIdFromRequest();

    if (!isValidAlarmId(id))
    {
        serialLog("invalid alarm id");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "invalid alarm id"
        );

        return;
    }

    serialLogKV("get alarm id", id);

    // Alarm contains a fixed array of rich phases. Request handlers execute
    // inside loopTask, so allocate the transient object outside its stack.
    std::unique_ptr<Alarm> alarm(
        new (std::nothrow) Alarm()
    );

    if (!alarm)
    {
        serialLog("not enough memory for alarm");

        sendError(
            HTTP_INTERNAL_ERROR_CODE,
            "not enough memory for alarm"
        );

        return;
    }

    if (
        !_alarmManager->loadFromSD(
            id,
            *alarm
        )
    )
    {
        serialLog("alarm not found");

        sendError(
            HTTP_NOT_FOUND_CODE,
            "alarm not found"
        );

        return;
    }

    sendAlarm(*alarm);
}


// ============================================================
// CREATE ALARM
// ============================================================

void WebServerManager::handleCreateAlarm()
{
    serialLog("handleCreateAlarm()");

    if (_alarmManager == nullptr)
    {
        serialLog("alarm manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "alarm manager unavailable"
        );

        return;
    }

    JsonDocument doc;

    if (!parseJson(doc))
        return;


    // ========================================================
    // ID
    // ========================================================

    String id;

    if (doc["id"].is<const char*>())
    {
        id =
            doc["id"].as<String>();
    }

    if (id.isEmpty())
    {
        id =
            "alarm_" +
            String(millis(), HEX) +
            "_" +
            String(random(0x10000), HEX);

        doc["id"] =
            id;
    }

    if (!isValidAlarmId(id))
    {
        serialLog("invalid alarm id");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "invalid alarm id"
        );

        return;
    }

    serialLogKV("create alarm id", id);


    // ========================================================
    // DESERIALIZE
    // ========================================================

    std::unique_ptr<Alarm> alarm(
        new (std::nothrow) Alarm()
    );

    if (!alarm)
    {
        serialLog("not enough memory for alarm");

        sendError(
            HTTP_INTERNAL_ERROR_CODE,
            "not enough memory for alarm"
        );

        return;
    }

    if (
        !_alarmManager->deserialize(
            doc,
            *alarm
        )
    )
    {
        serialLog("invalid alarm data");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "invalid alarm data"
        );

        return;
    }

    alarm->id = id;


    // ========================================================
    // CREATE
    // ========================================================

    if (
        !_alarmManager->create(
            *alarm
        )
    )
    {
        serialLog("failed to create alarm");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "failed to create alarm"
        );

        return;
    }

    serialLog("alarm created");

    sendAlarm(*alarm);
}


// ============================================================
// UPDATE ALARM
// ============================================================

void WebServerManager::handleUpdateAlarm()
{
    serialLog("handleUpdateAlarm()");

    if (_alarmManager == nullptr)
    {
        serialLog("alarm manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "alarm manager unavailable"
        );

        return;
    }

    JsonDocument doc;

    if (!parseJson(doc))
        return;

    const String uriId =
        alarmIdFromUri(
            _server.uri()
        );

    String id =
        uriId;

    if (id.isEmpty() &&
        doc["id"].is<const char*>())
    {
        id =
            doc["id"].as<String>();
    }

    if (!isValidAlarmId(id))
    {
        serialLog("invalid alarm id");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "invalid alarm id"
        );

        return;
    }

    if (!_alarmManager->exists(id))
    {
        serialLog("alarm not found");

        sendError(
            HTTP_NOT_FOUND_CODE,
            "alarm not found"
        );

        return;
    }

    serialLogKV("update alarm id", id);

    doc["id"] = id;

    std::unique_ptr<Alarm> alarm(
        new (std::nothrow) Alarm()
    );

    if (!alarm)
    {
        serialLog("not enough memory for alarm");

        sendError(
            HTTP_INTERNAL_ERROR_CODE,
            "not enough memory for alarm"
        );

        return;
    }

    if (
        !_alarmManager->deserialize(
            doc,
            *alarm
        )
    )
    {
        serialLog("invalid alarm data");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "invalid alarm data"
        );

        return;
    }

    alarm->id = id;

    if (
        !_alarmManager->update(
            *alarm
        )
    )
    {
        serialLog("failed to update alarm");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "failed to update alarm"
        );

        return;
    }

    serialLog("alarm updated");

    sendAlarm(*alarm);
}


// ============================================================
// DELETE ALARM
// ============================================================

void WebServerManager::handleDeleteAlarm()
{
    serialLog("handleDeleteAlarm()");

    if (_alarmManager == nullptr)
    {
        serialLog("alarm manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "alarm manager unavailable"
        );

        return;
    }

    const String id =
        getAlarmIdFromRequest();

    if (!isValidAlarmId(id))
    {
        serialLog("invalid alarm id");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "invalid alarm id"
        );

        return;
    }

    serialLogKV("delete alarm id", id);

    if (
        !_alarmManager->remove(id)
    )
    {
        serialLog("failed to delete alarm");

        sendError(
            HTTP_NOT_FOUND_CODE,
            "failed to delete alarm"
        );

        return;
    }

    sendOk();
}


// ============================================================
// ENABLE ALARM
// ============================================================

void WebServerManager::handleEnableAlarm()
{
    serialLog("handleEnableAlarm()");

    if (_alarmManager == nullptr)
    {
        serialLog("alarm manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "alarm manager unavailable"
        );

        return;
    }

    String id =
        _server.arg("id");

    if (id.isEmpty())
    {
        serialLog("missing alarm id");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "missing alarm id"
        );

        return;
    }

    if (!isValidAlarmId(id))
    {
        serialLog("invalid alarm id");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "invalid alarm id"
        );

        return;
    }

    serialLogKV("enable alarm id", id);

    if (
        !_alarmManager->setEnabled(
            id,
            true
        )
    )
    {
        serialLog("failed to enable alarm");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "failed to enable alarm"
        );

        return;
    }

    sendOk();
}


// ============================================================
// DISABLE ALARM
// ============================================================

void WebServerManager::handleDisableAlarm()
{
    serialLog("handleDisableAlarm()");

    if (_alarmManager == nullptr)
    {
        serialLog("alarm manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "alarm manager unavailable"
        );

        return;
    }

    String id =
        _server.arg("id");

    if (id.isEmpty())
    {
        serialLog("missing alarm id");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "missing alarm id"
        );

        return;
    }

    if (!isValidAlarmId(id))
    {
        serialLog("invalid alarm id");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "invalid alarm id"
        );

        return;
    }

    serialLogKV("disable alarm id", id);

    if (
        !_alarmManager->setEnabled(
            id,
            false
        )
    )
    {
        serialLog("failed to disable alarm");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "failed to disable alarm"
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
    serialLog("handleAlarmRuntime()");

    if (_alarmManager == nullptr)
    {
        serialLog("alarm manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "alarm manager unavailable"
        );

        return;
    }

    JsonDocument doc;

    doc["ok"] = true;

    doc["active"] =
        _alarmManager->isRunning();

    doc["phase"] =
        _alarmManager->currentPhaseIndex();

    doc["elapsedMs"] =
        _alarmManager->elapsedMs();

    if (_alarmManager->isRunning())
    {
        const Alarm* alarm =
            _alarmManager->currentAlarm();

        const AlarmPhase* phase =
            _alarmManager->currentPhase();

        if (alarm != nullptr)
        {
            doc["id"] =
                alarm->id;

            doc["name"] =
                alarm->name;
        }

        if (phase != nullptr)
        {
            doc["phaseDurationMs"] =
                phase->durationMs;

            doc["phaseCondition"] =
                static_cast<uint8_t>(
                    phase->condition
                );
        }
    }

    String body;

    serializeJson(
        doc,
        body
    );

    sendJson(
        HTTP_OK_CODE,
        body
    );
}


// ============================================================
// ALARM DISMISS
// ============================================================

void WebServerManager::handleAlarmDismiss()
{
    serialLog("handleAlarmDismiss()");

    if (_alarmManager == nullptr)
    {
        serialLog("alarm manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "alarm manager unavailable"
        );

        return;
    }

    if (!_alarmManager->isRunning())
    {
        serialLog("no active alarm");

        sendError(
            HTTP_CONFLICT_CODE,
            "no active alarm"
        );

        return;
    }

    bool result = false;

    if (_alarmController != nullptr)
        result = _alarmController->dismiss();
    else
        result = _alarmManager->dismiss();

    if (!result)
    {
        serialLog("failed to dismiss alarm");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "failed to dismiss alarm"
        );

        return;
    }

    sendOk();
}


// ============================================================
// ALARM SNOOZE
// ============================================================

void WebServerManager::handleAlarmSnooze()
{
    serialLog("handleAlarmSnooze()");

    if (_alarmManager == nullptr)
    {
        serialLog("alarm manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "alarm manager unavailable"
        );

        return;
    }

    if (!_alarmManager->isRunning())
    {
        serialLog("no active alarm");

        sendError(
            HTTP_CONFLICT_CODE,
            "no active alarm"
        );

        return;
    }

    JsonDocument doc;

    if (!parseJson(doc))
        return;

    JsonObjectConst root =
        doc.as<JsonObjectConst>();

    uint32_t durationMs = 0;

    if (
        !getUnsigned32(
            root,
            "durationMs",
            durationMs
        )
    )
    {
        getUnsigned32(
            root,
            "duration",
            durationMs
        );
    }

    if (durationMs == 0)
    {
        serialLog("invalid snooze duration");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "invalid snooze duration"
        );

        return;
    }

    if (durationMs > MAX_SNOOZE_MS)
    {
        serialLog("snooze duration is too large");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "snooze duration is too large"
        );

        return;
    }

    serialLogUInt("snooze durationMs", durationMs);

    bool result = false;

    if (_alarmController != nullptr)
    {
        result =
            _alarmController->snooze(
                durationMs
            );
    }
    else
    {
        result =
            _alarmManager->snooze(
                durationMs
            );
    }

    if (!result)
    {
        serialLog("failed to snooze alarm");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "failed to snooze alarm"
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
    serialLog("parseAlarmFromRequest()");

    if (_alarmManager == nullptr)
        return false;

    JsonDocument doc;

    if (!parseJson(doc))
        return false;

    if (
        !_alarmManager->deserialize(
            doc,
            alarm
        )
    )
    {
        serialLog("invalid alarm data");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "invalid alarm data"
        );

        return false;
    }

    return true;
}


// ============================================================
// SEND ALARM
// ============================================================

void WebServerManager::sendAlarm(
    const Alarm& alarm
)
{
    serialLog("sendAlarm()");

    if (_alarmManager == nullptr)
    {
        serialLog("alarm manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "alarm manager unavailable"
        );

        return;
    }

    JsonDocument doc;

    if (
        !_alarmManager->serialize(
            alarm,
            doc
        )
    )
    {
        serialLog("failed to serialize alarm");

        sendError(
            HTTP_INTERNAL_ERROR_CODE,
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

    sendJson(
        HTTP_OK_CODE,
        body
    );
}


// ============================================================
// SEND ALARM LIST
// ============================================================

void WebServerManager::sendAlarmList()
{
    serialLog("sendAlarmList()");

    if (_alarmManager == nullptr)
    {
        serialLog("alarm manager unavailable");

        sendError(
            HTTP_SERVICE_UNAVAILABLE_CODE,
            "alarm manager unavailable"
        );

        return;
    }

    /*
     * Alarm содержит несколько String и массив фаз.
     *
     * Поэтому массив Alarm не создаём на стеке.
     */

    std::unique_ptr<Alarm[]> alarms(
        new (std::nothrow)
        Alarm[AlarmConfig::MAX_ALARMS]
    );

    if (!alarms)
    {
        serialLog("not enough memory for alarm list");

        sendError(
            HTTP_INTERNAL_ERROR_CODE,
            "not enough memory for alarm list"
        );

        return;
    }

    uint8_t count = 0;

    if (
        !_alarmManager->loadAll(
            alarms.get(),
            AlarmConfig::MAX_ALARMS,
            count
        )
    )
    {
        serialLog("failed to load alarms");

        sendError(
            HTTP_INTERNAL_ERROR_CODE,
            "failed to load alarms"
        );

        return;
    }

    serialLogUInt("alarm count", count);

    JsonDocument doc;

    doc["ok"] = true;

    JsonArray list =
        doc["alarms"].to<JsonArray>();

    for (
        uint8_t i = 0;
        i < count;
        ++i
    )
    {
        JsonObject item =
            list.add<JsonObject>();

        JsonDocument alarmDoc;

        if (
            !_alarmManager->serialize(
                alarms[i],
                alarmDoc
            )
        )
        {
            continue;
        }

        item.set(
            alarmDoc.as<JsonObjectConst>()
        );
    }

    doc["count"] =
        static_cast<uint8_t>(
            list.size()
        );

    String body;

    serializeJson(
        doc,
        body
    );

    sendJson(
        HTTP_OK_CODE,
        body
    );
}


// ============================================================
// PARSE JSON
// ============================================================

bool WebServerManager::parseJson(
    JsonDocument& document
)
{
    if (!_server.hasArg("plain"))
    {
        serialLog("request body is required");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "request body is required"
        );

        return false;
    }

    const String body =
        _server.arg("plain");

    if (body.isEmpty())
    {
        serialLog("request body is empty");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "request body is empty"
        );

        return false;
    }

    const DeserializationError error =
        deserializeJson(
            document,
            body
        );

    if (error)
    {
        Serial0.print("[WebServerManager] invalid JSON: ");
        Serial0.println(error.c_str());

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "invalid JSON"
        );

        return false;
    }

    if (!document.is<JsonObject>())
    {
        serialLog("JSON object required");

        sendError(
            HTTP_BAD_REQUEST_CODE,
            "JSON object required"
        );

        return false;
    }

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
    Serial0.print("[WebServerManager] sendJson code=");
    Serial0.print(code);
    Serial0.print(" length=");
    Serial0.println(body.length());

    _server.send(
        code,
        "application/json; charset=utf-8",
        body
    );
}


// ============================================================
// SEND OK
// ============================================================

void WebServerManager::sendOk()
{
    sendJson(
        HTTP_OK_CODE,
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
    serialLogError(code, message);

    JsonDocument doc;

    doc["ok"] = false;

    doc["error"] =
        message != nullptr
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
// VALIDATE ALARM ID
// ============================================================

bool WebServerManager::isValidAlarmId(
    const String& id
) const
{
    if (id.isEmpty())
        return false;

    if (id.length() > 64)
        return false;

    for (
        size_t i = 0;
        i < id.length();
        ++i
    )
    {
        const char c =
            id[i];

        const bool valid =
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '_' ||
            c == '-';

        if (!valid)
            return false;
    }

    return true;
}


// ============================================================
// GET ALARM ID
// ============================================================

String WebServerManager::getAlarmIdFromRequest()
{
    String id =
        _server.arg("id");

    if (id.isEmpty())
    {
        id =
            alarmIdFromUri(
                _server.uri()
            );
    }

    return id;
}


// ============================================================
// GET BOOLEAN
// ============================================================

bool WebServerManager::getBoolean(
    JsonObjectConst object,
    const char* key,
    bool& value
) const
{
    if (!object[key])
        return false;

    JsonVariantConst variant =
        object[key];

    if (variant.is<bool>())
    {
        value =
            variant.as<bool>();

        return true;
    }

    if (variant.is<int>())
    {
        const int number =
            variant.as<int>();

        if (number == 0)
        {
            value = false;
            return true;
        }

        if (number == 1)
        {
            value = true;
            return true;
        }
    }

    return false;
}


// ============================================================
// GET UINT32
// ============================================================

bool WebServerManager::getUnsigned32(
    JsonObjectConst object,
    const char* key,
    uint32_t& value
) const
{
    if (!object[key])
        return false;

    JsonVariantConst variant =
        object[key];

    if (variant.is<uint32_t>())
    {
        value =
            variant.as<uint32_t>();

        return true;
    }

    if (variant.is<unsigned long>())
    {
        value =
            variant.as<unsigned long>();

        return true;
    }

    if (variant.is<unsigned int>())
    {
        value =
            variant.as<unsigned int>();

        return true;
    }

    if (variant.is<int>())
    {
        const int number =
            variant.as<int>();

        if (number < 0)
            return false;

        value =
            static_cast<uint32_t>(
                number
            );

        return true;
    }

    if (variant.is<long>())
    {
        const long number =
            variant.as<long>();

        if (number < 0)
            return false;

        value =
            static_cast<uint32_t>(
                number
            );

        return true;
    }

    return false;
}