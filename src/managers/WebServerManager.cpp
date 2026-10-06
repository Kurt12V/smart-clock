#include "WebServerManager.h"

#include <cstring>
#include <memory>
#include <new>

// ============================================================
// CONSTANTS
// ============================================================

namespace
{
    constexpr uint32_t SERIAL_BAUDRATE = 115200;

    constexpr int HTTP_OK                  = 200;
    constexpr int HTTP_BAD_REQUEST        = 400;
    constexpr int HTTP_NOT_FOUND          = 404;
    constexpr int HTTP_CONFLICT           = 409;
    constexpr int HTTP_INTERNAL_ERROR     = 500;
    constexpr int HTTP_SERVICE_UNAVAILABLE = 503;

    constexpr uint32_t WIFI_TIMEOUT_MS =
        20000UL;

    constexpr uint32_t MAX_SNOOZE_MS =
        24UL * 60UL * 60UL * 1000UL;

    constexpr size_t MAX_SD_FILES = 300;

    // ========================================================
    // SERIAL
    // ========================================================

    void log(const char* message)
    {
        Serial0.printf(
            "[WEB] %s\n",
            message ? message : ""
        );
    }

    void log(const String& message)
    {
        Serial0.printf(
            "[WEB] %s\n",
            message.c_str()
        );
    }

    void logError(const char* message)
    {
        Serial0.printf(
            "[WEB][ERROR] %s\n",
            message ? message : ""
        );
    }

    void logStep(const char* message)
    {
        Serial0.printf(
            "[WEB][STEP] %s\n",
            message ? message : ""
        );
    }

    void logRequest(WebServer& server)
    {
        Serial0.printf(
            "[WEB][REQUEST] %s %s\n",
            server.method() == HTTP_GET    ? "GET" :
            server.method() == HTTP_POST   ? "POST" :
            server.method() == HTTP_PUT    ? "PUT" :
            server.method() == HTTP_DELETE ? "DELETE" :
            "UNKNOWN",
            server.uri().c_str()
        );

        Serial0.printf(
            "[WEB][REQUEST] args=%d\n",
            server.args()
        );

        for (int i = 0; i < server.args(); ++i)
        {
            Serial0.printf(
                "[WEB][REQUEST][ARG] %s = %s\n",
                server.argName(i).c_str(),
                server.arg(i).c_str()
            );
        }
    }

    void logString(
        const char* name,
        const String& value
    )
    {
        Serial0.printf(
            "[WEB][VALUE] %s = %s\n",
            name,
            value.c_str()
        );
    }

    const char* methodName(HTTPMethod method)
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

    // ========================================================
    // URI HELPERS
    // ========================================================

    bool isAlarmPath(const String& uri)
    {
        constexpr const char* PREFIX =
            "/api/alarms/";

        if (!uri.startsWith(PREFIX))
            return false;

        const String rest =
            uri.substring(strlen(PREFIX));

        return !rest.isEmpty();
    }

    bool isAlarmEnabledPath(const String& uri)
    {
        return isAlarmPath(uri) &&
               uri.endsWith("/enabled");
    }

    String alarmIdFromUri(const String& uri)
    {
        constexpr const char* PREFIX =
            "/api/alarms/";

        String id =
            uri.substring(strlen(PREFIX));

        const int slash =
            id.indexOf('/');

        if (slash >= 0)
            id = id.substring(0, slash);

        id.trim();

        return id;
    }

    String alarmIdFromEnabledUri(const String& uri)
    {
        constexpr const char* PREFIX =
            "/api/alarms/";

        constexpr const char* SUFFIX =
            "/enabled";

        String id =
            uri.substring(strlen(PREFIX));

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
    Serial0.begin(
        SERIAL_BAUDRATE
    );

    Serial0.println();
    Serial0.println(
        "============================================================"
    );
    Serial0.println(
        "[WEB] WebServerManager CONSTRUCTOR"
    );
    Serial0.println(
        "============================================================"
    );

    Serial0.println(
        "[WEB] HTTP port = 80"
    );
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
    const uint32_t startedAt =
        millis();

    Serial0.println();
    Serial0.println(
        "============================================================"
    );
    Serial0.println(
        "[WEB] WebServerManager BEGIN"
    );
    Serial0.println(
        "============================================================"
    );

    _initialized = false;

    _settings = &settings;
    _sd = &sd;
    _sound = &sound;

    _alarmManager = nullptr;
    _alarmController = nullptr;

    Serial0.printf(
        "[WEB][PTR] SettingsManager=%p\n",
        static_cast<void*>(_settings)
    );

    Serial0.printf(
        "[WEB][PTR] SDManager=%p\n",
        static_cast<void*>(_sd)
    );

    Serial0.printf(
        "[WEB][PTR] SoundManager=%p\n",
        static_cast<void*>(_sound)
    );

    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    logStep(
        "Connecting to WiFi"
    );

    if (!ssid)
    {
        logError(
            "WiFi SSID is nullptr"
        );

        return false;
    }

    Serial0.printf(
        "[WEB][WIFI] SSID=%s\n",
        ssid
    );

    WiFi.mode(
        WIFI_STA
    );

    WiFi.begin(
        ssid,
        password
    );

    const uint32_t wifiStartedAt =
        millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - wifiStartedAt <
            WIFI_TIMEOUT_MS
    )
    {
        delay(250);

        Serial0.printf(
            "[WEB][WIFI] status=%d elapsed=%lu ms\n",
            static_cast<int>(
                WiFi.status()
            ),
            static_cast<unsigned long>(
                millis() - wifiStartedAt
            )
        );
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial0.printf(
            "[WEB][WIFI][ERROR] Connection failed status=%d\n",
            static_cast<int>(
                WiFi.status()
            )
        );

        return false;
    }

    Serial0.println(
        "[WEB][WIFI] Connected"
    );

    Serial0.printf(
        "[WEB][WIFI] IP=%s\n",
        WiFi.localIP().toString().c_str()
    );

    Serial0.printf(
        "[WEB][WIFI] Gateway=%s\n",
        WiFi.gatewayIP().toString().c_str()
    );

    Serial0.printf(
        "[WEB][WIFI] Netmask=%s\n",
        WiFi.subnetMask().toString().c_str()
    );

    Serial0.printf(
        "[WEB][WIFI] RSSI=%d dBm\n",
        WiFi.RSSI()
    );

    // --------------------------------------------------------
    // LITTLEFS
    // --------------------------------------------------------

    logStep(
        "Mounting LittleFS"
    );

    if (!LittleFS.begin(true))
    {
        logError(
            "LittleFS mount failed"
        );

        return false;
    }

    log(
        "LittleFS mounted"
    );

    // --------------------------------------------------------
    // ROUTES
    // --------------------------------------------------------

    setupRoutes();

    // --------------------------------------------------------
    // SERVER
    // --------------------------------------------------------

    logStep(
        "Starting WebServer"
    );

    _server.begin();

    _initialized = true;

    Serial0.println();
    Serial0.println(
        "============================================================"
    );
    Serial0.println(
        "[WEB] WebServerManager READY"
    );
    Serial0.println(
        "============================================================"
    );

    Serial0.printf(
        "[WEB] URL=http://%s\n",
        WiFi.localIP().toString().c_str()
    );

    Serial0.printf(
        "[WEB] begin() completed in %lu ms\n",
        static_cast<unsigned long>(
            millis() - startedAt
        )
    );

    return true;
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

String WebServerManager::getIP() const
{
    if (WiFi.status() != WL_CONNECTED)
        return String();

    return WiFi.localIP().toString();
}

// ============================================================
// SET ALARM MANAGERS
// ============================================================

void WebServerManager::setAlarmManager(
    AlarmManager& alarmManager
)
{
    _alarmManager =
        &alarmManager;

    Serial0.printf(
        "[WEB][ALARM] AlarmManager attached: %p\n",
        static_cast<void*>(
            _alarmManager
        )
    );
}

void WebServerManager::setAlarmController(
    AlarmController& alarmController
)
{
    _alarmController =
        &alarmController;

    Serial0.printf(
        "[WEB][ALARM] AlarmController attached: %p\n",
        static_cast<void*>(
            _alarmController
        )
    );
}

// ============================================================
// ROUTES
// ============================================================

void WebServerManager::setupRoutes()
{
    Serial0.println(
        "[WEB][ROUTES] Registering routes"
    );

    _server.on(
        "/",
        HTTP_GET,
        [this]()
        {
            handleRoot();
        }
    );

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

    _server.on(
        "/api/sensors",
        HTTP_GET,
        [this]()
        {
            handleSensors();
        }
    );

    _server.on(
        "/api/sd",
        HTTP_GET,
        [this]()
        {
            handleSD();
        }
    );

    // --------------------------------------------------------
    // AUDIO
    // --------------------------------------------------------

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

    // --------------------------------------------------------
    // ALARMS
    // --------------------------------------------------------

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

    // --------------------------------------------------------
    // LEGACY ALARM API
    // --------------------------------------------------------

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
    logRequest(_server);

    if (!LittleFS.exists(
            "/index.html"))
    {
        logError(
            "index.html not found"
        );

        sendError(
            HTTP_NOT_FOUND,
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
        logError(
            "Cannot open index.html"
        );

        sendError(
            HTTP_INTERNAL_ERROR,
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
// GET PARAM
// ============================================================

void WebServerManager::handleGetParam()
{
    logRequest(_server);

    if (!_settings)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "SettingsManager unavailable"
        );

        return;
    }

    String name;

    if (_server.hasArg("name"))
        name = _server.arg("name");
    else if (_server.hasArg("param"))
        name = _server.arg("param");

    name.trim();

    if (name.isEmpty())
    {
        sendError(
            HTTP_BAD_REQUEST,
            "Parameter name is required"
        );

        return;
    }

    const Param param =
        _settings->paramFromName(
            name.c_str()
        );

    if (param == Param::COUNT)
    {
        Serial0.printf(
            "[WEB][PARAM][ERROR] Unknown parameter=%s\n",
            name.c_str()
        );

        sendError(
            HTTP_NOT_FOUND,
            "Unknown parameter"
        );

        return;
    }

    const SettingsManager::ParamDesc& desc =
        _settings->getDesc(param);

    const int value =
        _settings->get(param);

    Serial0.printf(
        "[WEB][PARAM] %s value=%d range=%d..%d default=%d\n",
        name.c_str(),
        value,
        desc.minValue,
        desc.maxValue,
        desc.defaultValue
    );

    JsonDocument doc;

    doc["name"] = name;
    doc["value"] = value;
    doc["min"] = desc.minValue;
    doc["max"] = desc.maxValue;
    doc["default"] = desc.defaultValue;

    String output;
    serializeJson(
        doc,
        output
    );

    sendJson(
        HTTP_OK,
        output
    );
}

// ============================================================
// SET PARAM
// ============================================================

void WebServerManager::handleSetParam()
{
    logRequest(_server);

    if (!_settings)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "SettingsManager unavailable"
        );

        return;
    }

    JsonDocument doc;

    if (!parseJson(doc))
        return;

    JsonObjectConst object =
        doc.as<JsonObjectConst>();

    String name;

    if (object["name"].is<const char*>())
        name =
            object["name"].as<const char*>();
    else if (
        object["param"].is<const char*>())
        name =
            object["param"].as<const char*>();

    name.trim();

    if (name.isEmpty())
    {
        sendError(
            HTTP_BAD_REQUEST,
            "Parameter name is required"
        );

        return;
    }

    if (!object["value"].is<int>() &&
        !object["value"].is<long>() &&
        !object["value"].is<unsigned>() &&
        !object["value"].is<unsigned long>())
    {
        sendError(
            HTTP_BAD_REQUEST,
            "Parameter value must be numeric"
        );

        return;
    }

    const Param param =
        _settings->paramFromName(
            name.c_str()
        );

    if (param == Param::COUNT)
    {
        sendError(
            HTTP_NOT_FOUND,
            "Unknown parameter"
        );

        return;
    }

    const SettingsManager::ParamDesc& desc =
        _settings->getDesc(param);

    const int oldValue =
        _settings->get(param);

    int requestedValue =
        object["value"].as<int>();

    const int value =
        constrain(
            requestedValue,
            desc.minValue,
            desc.maxValue
        );

    Serial0.printf(
        "[WEB][PARAM] name=%s old=%d requested=%d value=%d range=%d..%d\n",
        name.c_str(),
        oldValue,
        requestedValue,
        value,
        desc.minValue,
        desc.maxValue
    );

    if (value != requestedValue)
    {
        Serial0.printf(
            "[WEB][PARAM] CLAMP %d -> %d\n",
            requestedValue,
            value
        );
    }

    if (!_settings->set(
            param,
            value))
    {
        sendError(
            HTTP_INTERNAL_ERROR,
            "Failed to set parameter"
        );

        return;
    }

    const int newValue =
        _settings->get(param);

    JsonDocument response;

    response["name"] = name;
    response["old"] = oldValue;
    response["requested"] =
        requestedValue;
    response["value"] =
        newValue;
    response["changed"] =
        newValue != oldValue;
    response["min"] =
        desc.minValue;
    response["max"] =
        desc.maxValue;
    response["default"] =
        desc.defaultValue;

    String output;

    serializeJson(
        response,
        output
    );

    sendJson(
        HTTP_OK,
        output
    );
}

// ============================================================
// GET ALL PARAMS
// ============================================================

void WebServerManager::handleGetAllParams()
{
    logRequest(_server);

    if (!_settings)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "SettingsManager unavailable"
        );

        return;
    }

    JsonDocument doc;

    JsonArray array =
        doc["params"].to<JsonArray>();

    for (
        uint8_t i = 0;
        i < static_cast<uint8_t>(
                Param::COUNT);
        ++i)
    {
        const Param param =
            static_cast<Param>(i);

        const SettingsManager::ParamDesc& desc =
            _settings->getDesc(param);

        JsonObject item =
            array.add<JsonObject>();

        item["name"] =
            desc.name;

        item["key"] =
            desc.key;

        item["value"] =
            _settings->get(param);

        item["min"] =
            desc.minValue;

        item["max"] =
            desc.maxValue;

        item["default"] =
            desc.defaultValue;
    }

    String output;

    serializeJson(
        doc,
        output
    );

    sendJson(
        HTTP_OK,
        output
    );
}

// ============================================================
// RESET
// ============================================================

void WebServerManager::handleReset()
{
    logRequest(_server);

    if (!_settings)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "SettingsManager unavailable"
        );

        return;
    }

    Serial0.println(
        "[WEB][RESET] resetAll()"
    );

    _settings->resetAll();

    Serial0.println(
        "[WEB][RESET] completed"
    );

    sendOk();
}

// ============================================================
// SENSORS
// ============================================================

void WebServerManager::handleSensors()
{
    logRequest(_server);

    JsonDocument doc;

    doc["temperature"] = nullptr;
    doc["humidity"] = nullptr;
    doc["lux"] = nullptr;
    doc["distance"] = nullptr;
    doc["co2"] = nullptr;

    String output;

    serializeJson(
        doc,
        output
    );

    sendJson(
        HTTP_OK,
        output
    );
}

// ============================================================
// SD
// ============================================================

void WebServerManager::handleSD()
{
    logRequest(_server);

    if (!_sd)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "SDManager unavailable"
        );

        return;
    }

    std::unique_ptr<SDFileEntry[]> entries(
        new (std::nothrow)
        SDFileEntry[MAX_SD_FILES]
    );

    if (!entries)
    {
        sendError(
            HTTP_INTERNAL_ERROR,
            "Memory allocation failed"
        );

        return;
    }

    const size_t count =
        _sd->listFiles(
            entries.get(),
            MAX_SD_FILES
        );

    Serial0.printf(
        "[WEB][SD] count=%u\n",
        static_cast<unsigned>(count)
    );

    JsonDocument doc;

    JsonArray files =
        doc["files"].to<JsonArray>();

    for (
        size_t i = 0;
        i < count;
        ++i)
    {
        JsonObject item =
            files.add<JsonObject>();

        item["path"] =
            entries[i].path;

        item["size"] =
            entries[i].size;

        item["isDir"] =
            entries[i].isDir;

        if (i < 20)
        {
            Serial0.printf(
                "[WEB][SD][FILE] #%u path=%s size=%llu dir=%s\n",
                static_cast<unsigned>(i),
                entries[i].path.c_str(),
                static_cast<unsigned long long>(
                    entries[i].size
                ),
                entries[i].isDir
                    ? "true"
                    : "false"
            );
        }
    }

    String output;

    serializeJson(
        doc,
        output
    );

    sendJson(
        HTTP_OK,
        output
    );
}

// ============================================================
// AUDIO PLAY
// ============================================================

void WebServerManager::handleAudioPlay()
{
    logRequest(_server);

    if (!_sound)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "SoundManager unavailable"
        );

        return;
    }

    if (!_sd)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "SDManager unavailable"
        );

        return;
    }

    JsonDocument doc;

    if (!parseJson(doc))
        return;

    JsonObjectConst object =
        doc.as<JsonObjectConst>();

    if (!object["path"].is<const char*>())
    {
        sendError(
            HTTP_BAD_REQUEST,
            "path is required"
        );

        return;
    }

    const String path =
        object["path"].as<String>();

    SoundManager::PlayOptions options;

    options.stream =
        SoundManager::AudioStream::Media;

    if (object["stream"].is<const char*>())
    {
        const String stream =
            object["stream"].as<String>();

        if (stream.equalsIgnoreCase("alarm"))
        {
            options.stream =
                SoundManager::AudioStream::Alarm;
        }
        else if (
            stream.equalsIgnoreCase("system"))
        {
            options.stream =
                SoundManager::AudioStream::System;
        }
    }

    if (object["localPercent"].is<uint32_t>())
        options.localPercent =
            object["localPercent"].as<uint32_t>();

    if (object["fadeInMs"].is<uint32_t>())
        options.fadeInMs =
            object["fadeInMs"].as<uint32_t>();

    if (object["fadeOutMs"].is<uint32_t>())
        options.fadeOutMs =
            object["fadeOutMs"].as<uint32_t>();

    if (object["curve"].is<const char*>())
    {
        const String curve =
            object["curve"].as<String>();

        if (curve.equalsIgnoreCase(
                "exponential"))
        {
            options.curve =
                SoundManager::FadeCurve::Exponential;
        }
        else if (
            curve.equalsIgnoreCase(
                "logarithmic"))
        {
            options.curve =
                SoundManager::FadeCurve::Logarithmic;
        }
        else
        {
            options.curve =
                SoundManager::FadeCurve::Linear;
        }
    }

    Serial0.printf(
        "[WEB][AUDIO] path=%s local=%u fadeIn=%lu fadeOut=%lu\n",
        path.c_str(),
        static_cast<unsigned>(
            options.localPercent
        ),
        static_cast<unsigned long>(
            options.fadeInMs
        ),
        static_cast<unsigned long>(
            options.fadeOutMs
        )
    );

    if (!_sd->fileExists(path))
    {
        Serial0.printf(
            "[WEB][AUDIO][ERROR] File not found=%s\n",
            path.c_str()
        );

        sendError(
            HTTP_NOT_FOUND,
            "Audio file not found"
        );

        return;
    }

    if (!_sound->play(
            path.c_str(),
            options))
    {
        sendError(
            HTTP_INTERNAL_ERROR,
            "Audio playback failed"
        );

        return;
    }

    JsonDocument response;

    response["state"] =
        _sound->getStateString();

    response["path"] =
        _sound->getCurrentPath();

    response["stream"] =
        static_cast<uint8_t>(
            _sound->getCurrentStream()
        );

    String output;

    serializeJson(
        response,
        output
    );

    sendJson(
        HTTP_OK,
        output
    );
}

// ============================================================
// AUDIO PAUSE
// ============================================================

void WebServerManager::handleAudioPause()
{
    logRequest(_server);

    if (!_sound)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "SoundManager unavailable"
        );

        return;
    }

    _sound->pause();

    sendOk();
}

// ============================================================
// AUDIO RESUME
// ============================================================

void WebServerManager::handleAudioResume()
{
    logRequest(_server);

    if (!_sound)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "SoundManager unavailable"
        );

        return;
    }

    _sound->resume();

    sendOk();
}

// ============================================================
// AUDIO STOP
// ============================================================

void WebServerManager::handleAudioStop()
{
    logRequest(_server);

    if (!_sound)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "SoundManager unavailable"
        );

        return;
    }

    uint32_t fadeOutMs = 0;

    if (_server.hasArg("fadeOutMs"))
    {
        fadeOutMs =
            _server.arg(
                "fadeOutMs"
            ).toInt();
    }

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
    logRequest(_server);

    if (!_sound)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "SoundManager unavailable"
        );

        return;
    }

    const uint32_t position =
        _sound->getPositionMs();

    const uint32_t duration =
        _sound->getDurationMs();

    JsonDocument doc;

    doc["state"] =
        _sound->getStateString();

    doc["path"] =
        _sound->getCurrentPath();

    doc["stream"] =
        static_cast<uint8_t>(
            _sound->getCurrentStream()
        );

    doc["positionMs"] =
        position;

    doc["durationMs"] =
        duration;

    doc["position"] =
        duration > 0
            ? static_cast<float>(position) /
              static_cast<float>(duration)
            : 0.0f;

    Serial0.printf(
        "[WEB][AUDIO][STATUS] state=%s path=%s position=%lu/%lu\n",
        _sound->getStateString(),
        _sound->getCurrentPath(),
        static_cast<unsigned long>(
            position
        ),
        static_cast<unsigned long>(
            duration
        )
    );

    String output;

    serializeJson(
        doc,
        output
    );

    sendJson(
        HTTP_OK,
        output
    );
}

// ============================================================
// GET ALARMS
// ============================================================

void WebServerManager::handleGetAlarms()
{
    logRequest(_server);

    if (!_alarmManager)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
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
    logRequest(_server);

    if (!_alarmManager)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "AlarmManager unavailable"
        );

        return;
    }

    const String id =
        getAlarmIdFromRequest();

    if (!isValidAlarmId(id))
    {
        sendError(
            HTTP_BAD_REQUEST,
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
            HTTP_NOT_FOUND,
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
    logRequest(_server);

    if (!_alarmManager)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
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

    if (alarm.id.isEmpty())
    {
        alarm.id =
            String("alarm_") +
            String(millis());
    }

    if (!isValidAlarmId(
            alarm.id))
    {
        sendError(
            HTTP_BAD_REQUEST,
            "Invalid alarm id"
        );

        return;
    }

    Alarm existing;

    if (_alarmManager->get(
            alarm.id,
            existing))
    {
        sendError(
            HTTP_CONFLICT,
            "Alarm already exists"
        );

        return;
    }

    if (!_alarmManager->create(
            alarm))
    {
        sendError(
            HTTP_INTERNAL_ERROR,
            "Failed to create alarm"
        );

        return;
    }

    sendAlarm(
        alarm
    );
}

// ============================================================
// UPDATE ALARM
// ============================================================

void WebServerManager::handleUpdateAlarm()
{
    logRequest(_server);

    if (!_alarmManager)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "AlarmManager unavailable"
        );

        return;
    }

    const String id =
        getAlarmIdFromRequest();

    if (!isValidAlarmId(id))
    {
        sendError(
            HTTP_BAD_REQUEST,
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
            HTTP_NOT_FOUND,
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

    updated.id = id;

    if (!_alarmManager->update(
            updated))
    {
        sendError(
            HTTP_INTERNAL_ERROR,
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
    logRequest(_server);

    if (!_alarmManager)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "AlarmManager unavailable"
        );

        return;
    }

    const String id =
        getAlarmIdFromRequest();

    if (!isValidAlarmId(id))
    {
        sendError(
            HTTP_BAD_REQUEST,
            "Invalid alarm id"
        );

        return;
    }

    if (!_alarmManager->remove(id))
    {
        sendError(
            HTTP_NOT_FOUND,
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
    logRequest(_server);

    if (!_alarmManager)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "AlarmManager unavailable"
        );

        return;
    }

    String id =
        getAlarmIdFromRequest();

    if (id.isEmpty())
    {
        id =
            alarmIdFromEnabledUri(
                _server.uri()
            );
    }

    if (!_alarmManager->setEnabled(
            id,
            true))
    {
        sendError(
            HTTP_NOT_FOUND,
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
    logRequest(_server);

    if (!_alarmManager)
    {
        sendError(
            HTTP_SERVICE_UNAVAILABLE,
            "AlarmManager unavailable"
        );

        return;
    }

    const String id =
        getAlarmIdFromRequest();

    if (!_alarmManager->setEnabled(
            id,
            false))
    {
        sendError(
            HTTP_NOT_FOUND,
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
    logRequest(_server);

    JsonDocument doc;

    bool active = false;

    String alarmId;

    uint8_t phase = 0;

    uint32_t elapsedMs = 0;

    if (_alarmController)
    {
        active =
            _alarmController->isActive();

        if (active)
        {
            alarmId =
                _alarmController->alarmId();

            phase =
                _alarmController->phaseIndex();
        }

        if (_alarmManager)
        {
            elapsedMs =
                _alarmManager->elapsedMs();
        }
    }
    else if (_alarmManager)
    {
        active =
            _alarmManager->isRunning();

        if (active)
        {
            const Alarm* alarm =
                _alarmManager->currentAlarm();

            if (alarm)
                alarmId = alarm->id;

            phase =
                _alarmManager->currentPhaseIndex();

            elapsedMs =
                _alarmManager->elapsedMs();
        }
    }

    doc["active"] =
        active;

    doc["alarmId"] =
        alarmId;

    doc["phase"] =
        phase;

    doc["elapsedMs"] =
        elapsedMs;

    String output;

    serializeJson(
        doc,
        output
    );

    sendJson(
        HTTP_OK,
        output
    );
}

// ============================================================
// DISMISS
// ============================================================

void WebServerManager::handleAlarmDismiss()
{
    logRequest(_server);

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
            HTTP_SERVICE_UNAVAILABLE,
            "Alarm system unavailable"
        );

        return;
    }

    Serial0.printf(
        "[WEB][ALARM][DISMISS] result=%s\n",
        result ? "true" : "false"
    );

    if (!result)
    {
        sendError(
            HTTP_CONFLICT,
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
    logRequest(_server);

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
                HTTP_BAD_REQUEST,
                "durationMs is required"
            );

            return;
        }
    }

    if (durationMs == 0)
    {
        sendError(
            HTTP_BAD_REQUEST,
            "durationMs is required"
        );

        return;
    }

    if (durationMs > MAX_SNOOZE_MS)
    {
        sendError(
            HTTP_BAD_REQUEST,
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
            HTTP_SERVICE_UNAVAILABLE,
            "Alarm system unavailable"
        );

        return;
    }

    if (!result)
    {
        sendError(
            HTTP_CONFLICT,
            "Alarm snooze failed"
        );

        return;
    }

    sendOk();
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
                HTTP_NOT_FOUND,
                "Route not found"
            );

            return;
        }

        if (!_alarmManager)
        {
            sendError(
                HTTP_SERVICE_UNAVAILABLE,
                "AlarmManager unavailable"
            );

            return;
        }

        const String id =
            alarmIdFromEnabledUri(uri);

        if (!isValidAlarmId(id))
        {
            sendError(
                HTTP_BAD_REQUEST,
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
                HTTP_BAD_REQUEST,
                "enabled is required"
            );

            return;
        }

        if (!_alarmManager->setEnabled(
                id,
                enabled))
        {
            sendError(
                HTTP_NOT_FOUND,
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
            HTTP_OK,
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
                HTTP_BAD_REQUEST,
                "Invalid alarm id"
            );

            return;
        }

        if (method == HTTP_GET)
        {
            if (!_alarmManager)
            {
                sendError(
                    HTTP_SERVICE_UNAVAILABLE,
                    "AlarmManager unavailable"
                );

                return;
            }

            Alarm alarm;

            if (!_alarmManager->loadFromSD(
                    id,
                    alarm))
            {
                sendError(
                    HTTP_NOT_FOUND,
                    "Alarm not found"
                );

                return;
            }

            sendAlarm(
                alarm
            );

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
        HTTP_NOT_FOUND,
        "Request handler not found"
    );
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
        logError(
            "AlarmManager unavailable"
        );

        return false;
    }

    if (!_alarmManager->deserialize(
            doc,
            alarm))
    {
        logError(
            "Alarm deserialize failed"
        );

        sendError(
            HTTP_BAD_REQUEST,
            "Invalid alarm JSON"
        );

        return false;
    }

    Serial0.printf(
        "[WEB][ALARM] id=%s name=%s enabled=%s time=%02u:%02u:%02u phases=%u repeatMask=0x%02X\n",
        alarm.id.c_str(),
        alarm.name.c_str(),
        alarm.enabled
            ? "true"
            : "false",
        alarm.time.hour,
        alarm.time.minute,
        alarm.time.second,
        alarm.phaseCount,
        alarm.repeatMask
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
            HTTP_SERVICE_UNAVAILABLE,
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
        HTTP_OK,
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
            HTTP_SERVICE_UNAVAILABLE,
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
        Serial0.println(
            "[WEB][ALARM][LIST] loadAll() failed"
        );

        sendError(
            HTTP_INTERNAL_ERROR,
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

        JsonDocument alarmDoc;

        _alarmManager->serialize(
            alarms[i],
            alarmDoc
        );

        item.set(
            alarmDoc.as<JsonObjectConst>()
        );
    }

    String output;

    serializeJson(
        doc,
        output
    );

    sendJson(
        HTTP_OK,
        output
    );
}

// ============================================================
// PARSE JSON
// ============================================================

bool WebServerManager::parseJson(
    JsonDocument& document
)
{
    if (!_server.hasArg(
            "plain"))
    {
        logError(
            "JSON body is missing"
        );

        sendError(
            HTTP_BAD_REQUEST,
            "JSON body is required"
        );

        return false;
    }

    const String body =
        _server.arg("plain");

    Serial0.printf(
        "[WEB][JSON] length=%u\n",
        static_cast<unsigned>(
            body.length()
        )
    );

    const DeserializationError error =
        deserializeJson(
            document,
            body
        );

    if (error)
    {
        Serial0.printf(
            "[WEB][JSON][ERROR] %s\n",
            error.c_str()
        );

        sendError(
            HTTP_BAD_REQUEST,
            "Invalid JSON"
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
    Serial0.printf(
        "[WEB][RESPONSE] HTTP %d length=%u\n",
        code,
        static_cast<unsigned>(
            body.length()
        )
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
    JsonDocument doc;

    doc["ok"] = true;

    String output;

    serializeJson(
        doc,
        output
    );

    sendJson(
        HTTP_OK,
        output
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
    Serial0.printf(
        "[WEB][ERROR] HTTP %d: %s\n",
        code,
        message ? message : ""
    );

    JsonDocument doc;

    doc["ok"] = false;
    doc["error"] =
        message ? message : "";

    String output;

    serializeJson(
        doc,
        output
    );

    sendJson(
        code,
        output
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
        ++i)
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
        {
            Serial0.printf(
                "[WEB][ALARM][ERROR] Invalid ID char '%c'\n",
                c
            );

            return false;
        }
    }

    return true;
}

// ============================================================
// GET ALARM ID
// ============================================================

String WebServerManager::getAlarmIdFromRequest()
{
    String id;

    if (_server.hasArg("id"))
        id =
            _server.arg("id");

    if (id.isEmpty() &&
        _server.hasArg("alarmId"))
    {
        id =
            _server.arg(
                "alarmId"
            );
    }

    if (id.isEmpty())
    {
        const String uri =
            _server.uri();

        if (isAlarmPath(uri))
        {
            id =
                alarmIdFromUri(uri);
        }
    }

    id.trim();

    Serial0.printf(
        "[WEB][ALARM] resolved id='%s'\n",
        id.c_str()
    );

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
    if (!object[key].is<bool>())
    {
        Serial0.printf(
            "[WEB][JSON][ERROR] Invalid boolean: %s\n",
            key
        );

        return false;
    }

    value =
        object[key].as<bool>();

    return true;
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
    if (!object[key].is<uint32_t>())
    {
        Serial0.printf(
            "[WEB][JSON][ERROR] Invalid uint32: %s\n",
            key
        );

        return false;
    }

    value =
        object[key].as<uint32_t>();

    return true;
}