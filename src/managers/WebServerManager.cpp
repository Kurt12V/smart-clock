
#include "WebServerManager.h"

#include <cstring>

// ============================================================
// CONSTRUCTOR
// ============================================================

WebServerManager::WebServerManager()
    : _server(80),
      _settings(nullptr),
      _sd(nullptr),
      _sound(nullptr),
      _initialized(false)
{
}


// ============================================================
// BEGIN
// ============================================================

bool WebServerManager::begin(
    SettingsManager& settings,
    SDManager&       sd,
    SoundManager&    sound,
    const char*      ssid,
    const char*      password
)
{
    _settings = &settings;
    _sd       = &sd;
    _sound    = &sound;

    // --------------------------------------------------------
    // WiFi
    // --------------------------------------------------------

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true, true);

    delay(300);

    WiFi.begin(
        ssid,
        password
    );

    const uint32_t startTime = millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 20000
    )
    {
        delay(500);
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial0.println("[WEB] WiFi connection failed");
        return false;
    }

    Serial0.print("[WEB] WiFi connected: ");
    Serial0.println(WiFi.localIP());

    // --------------------------------------------------------
    // LittleFS
    // --------------------------------------------------------

    if (!LittleFS.begin(true))
    {
        Serial0.println("[WEB] LittleFS mount failed");
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

    Serial0.print("[WEB] Server: http://");
    Serial0.println(WiFi.localIP());

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
// ROUTES
// ============================================================

void WebServerManager::setupRoutes()
{
    // --------------------------------------------------------
    // WEB UI
    // --------------------------------------------------------

    _server.on(
        "/",
        HTTP_GET,
        [this]()
        {
            handleRoot();
        }
    );

    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

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

    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    _server.on(
        "/api/sensors",
        HTTP_GET,
        [this]()
        {
            handleSensors();
        }
    );

    // --------------------------------------------------------
    // SD
    // --------------------------------------------------------

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
    // 404
    // --------------------------------------------------------

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

    File file = LittleFS.open(
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
        "text/html"
    );

    file.close();
}


// ============================================================
// 404
// ============================================================

void WebServerManager::handleNotFound()
{
    _server.send(
        404,
        "text/plain",
        "404 - Not Found"
    );
}


// ============================================================
// GET /api/param
// ============================================================
//
// /api/param?name=brightness
//
// Response:
//
// {
//     "ok": true,
//     "param": "brightness",
//     "value": 80
// }
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

    String name = _server.arg("name");

    if (name.isEmpty())
        name = _server.arg("param");

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

    if (param == SettingsManager::Param::COUNT)
    {
        sendError(
            400,
            "unknown param"
        );

        return;
    }

    JsonDocument doc;

    doc["ok"] = true;
    doc["param"] = _settings->paramName(param);
    doc["value"] = _settings->get(param);

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
//
// {
//     "name": "brightness",
//     "value": 80
// }
//
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

    const char* name = doc["name"];

    if (!name)
        name = doc["param"];

    if (!name)
    {
        sendError(
            400,
            "missing name"
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

    const SettingsManager::Param param =
        _settings->paramFromName(name);

    if (param == SettingsManager::Param::COUNT)
    {
        sendError(
            400,
            "unknown param"
        );

        return;
    }

    const uint8_t oldValue =
        _settings->get(param);

    const int requestedValue =
        doc["value"].as<int>();

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
        _settings->get(param);

    JsonDocument response;

    response["ok"] = true;
    response["param"] =
        _settings->paramName(param);

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
            static_cast<SettingsManager::Param>(i);

        const char* name =
            _settings->paramName(param);

        if (!name || name[0] == '\0')
            continue;

        doc[name] =
            _settings->get(param);
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
//
// Пока SensorsManager не передаётся в WebServerManager.
// Оставляем совместимый endpoint.
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

    if (!path)
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

    // --------------------------------------------------------
    // STREAM
    // --------------------------------------------------------

    const char* stream =
        doc["stream"] | "media";

    if (strcmp(stream, "alarm") == 0)
    {
        options.stream =
            SoundManager::AudioStream::Alarm;
    }
    else if (strcmp(stream, "system") == 0)
    {
        options.stream =
            SoundManager::AudioStream::System;
    }
    else
    {
        options.stream =
            SoundManager::AudioStream::Media;
    }

    // --------------------------------------------------------
    // VOLUME
    // --------------------------------------------------------

    int volume =
        doc["volume"] | 100;

    volume =
        constrain(
            volume,
            0,
            100
        );

    options.localPercent =
        static_cast<uint8_t>(volume);

    // --------------------------------------------------------
    // FADE
    // --------------------------------------------------------

    options.fadeInMs =
        doc["fade_in"] | 0;

    options.fadeOutMs =
        doc["fade_out"] | 0;

    // --------------------------------------------------------
    // CURVE
    // --------------------------------------------------------

    const char* curve =
        doc["curve"] | "linear";

    if (strcmp(curve, "exp") == 0)
    {
        options.curve =
            SoundManager::FadeCurve::Exponential;
    }
    else if (strcmp(curve, "log") == 0)
    {
        options.curve =
            SoundManager::FadeCurve::Logarithmic;
    }
    else
    {
        options.curve =
            SoundManager::FadeCurve::Linear;
    }

    // --------------------------------------------------------
    // PLAY
    // --------------------------------------------------------

    if (
        !_sound->play(
            path,
            options
        )
    )
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
    if (!_sound)
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
    if (!_sound)
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
    if (!_sound)
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
            fadeOut =
                doc["fade_out"] | 0;
        }
    }

    if (fadeOut > 0)
    {
        _sound->stop(fadeOut);
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
    if (!_sound)
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

    doc["position"] =
        _sound->getPositionMs();

    doc["duration"] =
        _sound->getDurationMs();

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
// PARSE JSON
// ============================================================

bool WebServerManager::parseJson(
    JsonDocument& doc
)
{
    if (!_server.hasArg("plain"))
    {
        sendError(
            400,
            "body missing"
        );

        return false;
    }

    const DeserializationError error =
        deserializeJson(
            doc,
            _server.arg("plain")
        );

    if (error)
    {
        sendError(
            400,
            "invalid JSON"
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
    return WiFi.status() == WL_CONNECTED;
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

