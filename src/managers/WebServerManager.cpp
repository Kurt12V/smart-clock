#include "WebServerManager.h"

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
    Serial.println();
    Serial.println("========================================");
    Serial.println("          WEB SERVER START");
    Serial.println("========================================");

    _settings = &settings;
    _sd       = &sd;
    _sound    = &sound;

    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    Serial.println("[WEB] WiFi mode: STA");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true, true);
    delay(300);

    Serial.print  ("[WEB] SSID: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    uint32_t startTime = millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 20000
    )
    {
        delay(500);

        Serial.print("[WEB] WiFi status: ");
        Serial.println(WiFi.status());
    }

    // --------------------------------------------------------
    // CHECK WIFI
    // --------------------------------------------------------

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println();
        Serial.println("[WEB] ================================");
        Serial.println("[WEB] WIFI CONNECTION FAILED");
        Serial.println("[WEB] ================================");

        Serial.print  ("[WEB] Status: ");
        Serial.println(WiFi.status());

        Serial.print  ("[WEB] SSID:   ");
        Serial.println(WiFi.SSID());

        Serial.print  ("[WEB] RSSI:   ");
        Serial.println(WiFi.RSSI());

        return false;
    }

    Serial.println();
    Serial.println("[WEB] WiFi connected");

    Serial.print  ("[WEB] IP:      ");
    Serial.println(WiFi.localIP());

    Serial.print  ("[WEB] Gateway: ");
    Serial.println(WiFi.gatewayIP());

    Serial.print  ("[WEB] Subnet:  ");
    Serial.println(WiFi.subnetMask());

    Serial.print  ("[WEB] RSSI:    ");
    Serial.println(WiFi.RSSI());

    // --------------------------------------------------------
    // LITTLEFS
    // --------------------------------------------------------

    Serial.println();
    Serial.println("[WEB] Mounting LittleFS...");

    if (!LittleFS.begin(true))
    {
        Serial.println("[WEB] LittleFS mount FAILED");
        return false;
    }

    Serial.println("[WEB] LittleFS mounted");

    if (LittleFS.exists("/index.html"))
        Serial.println("[WEB] /index.html found");
    else
        Serial.println("[WEB] WARNING: /index.html NOT FOUND");

    // --------------------------------------------------------
    // ROUTES + SERVER
    // --------------------------------------------------------

    setupRoutes();

    _server.begin();

    Serial.println();
    Serial.println("[WEB] HTTP server started");

    Serial.print  ("[WEB] Open: http://");
    Serial.println(WiFi.localIP());

    _initialized = true;

    Serial.println("========================================");
    Serial.println("          WEB SERVER READY");
    Serial.println("========================================");

    return true;
}

// ============================================================
// ROUTES
// ============================================================

void WebServerManager::setupRoutes()
{
    // --------------------------------------------------------
    // ROOT
    // --------------------------------------------------------

    _server.on(
        "/",
        HTTP_GET,
        [this]() { handleRoot(); }
    );

    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    _server.on(
        "/api/param",
        HTTP_GET,
        [this]() { handleGetParam(); }
    );

    _server.on(
        "/api/param",
        HTTP_POST,
        [this]() { handleSetParam(); }
    );

    _server.on(
        "/api/params",
        HTTP_GET,
        [this]() { handleGetAllParams(); }
    );

    _server.on(
        "/api/reset",
        HTTP_POST,
        [this]() { handleReset(); }
    );

    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    _server.on(
        "/api/sensors",
        HTTP_GET,
        [this]() { handleSensors(); }
    );

    // --------------------------------------------------------
    // SD CARD
    // --------------------------------------------------------

    _server.on(
        "/api/sd",
        HTTP_GET,
        [this]() { handleSD(); }
    );

    // --------------------------------------------------------
    // AUDIO
    // --------------------------------------------------------

    _server.on(
        "/api/audio/play",
        HTTP_POST,
        [this]() { handleAudioPlay(); }
    );

    _server.on(
        "/api/audio/pause",
        HTTP_POST,
        [this]() { handleAudioPause(); }
    );

    _server.on(
        "/api/audio/resume",
        HTTP_POST,
        [this]() { handleAudioResume(); }
    );

    _server.on(
        "/api/audio/stop",
        HTTP_POST,
        [this]() { handleAudioStop(); }
    );

    _server.on(
        "/api/audio/status",
        HTTP_GET,
        [this]() { handleAudioStatus(); }
    );

    // --------------------------------------------------------
    // 404
    // --------------------------------------------------------

    _server.onNotFound(
        [this]() { handleNotFound(); }
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

    File file = LittleFS.open("/index.html", "r");

    if (!file)
    {
        _server.send(
            500,
            "text/plain",
            "Failed to open index.html"
        );
        return;
    }

    _server.streamFile(file, "text/html");
    file.close();
}

// ============================================================
// 404
// ============================================================

void WebServerManager::handleNotFound()
{
    Serial.print("[WEB] 404: ");
    Serial.println(_server.uri());

    _server.send(
        404,
        "text/plain",
        "404 - Not Found"
    );
}

// ============================================================
// GET /api/param?name=brightness
//   -> {"param":"brightness","value":80}
// ============================================================

void WebServerManager::handleGetParam()
{
    if (!_settings)
    {
        sendError(500, "settings not initialized");
        return;
    }

    String name = _server.arg("name");

    if (name.length() == 0)
    {
        sendError(400, "missing name");
        return;
    }

    Param p;

    if (!SettingsManager::paramFromName(name.c_str(), p))
    {
        sendError(400, "unknown param");
        return;
    }

    String body;
    body.reserve(64);

    body  = "{\"param\":\"";
    body += SettingsManager::paramName(p);
    body += "\",\"value\":";
    body += _settings->get(p);
    body += "}";

    sendJson(200, body);
}

// ============================================================
// POST /api/param
//   body: {"param":"brightness","value":80}
//   -> {"ok":true}
// ============================================================

void WebServerManager::handleSetParam()
{
    if (!_settings)
    {
        sendError(500, "settings not initialized");
        return;
    }

    JsonDocument doc;

    if (!parseJson(doc))
        return;

    const char* name = doc["param"];

    if (!name)
    {
        sendError(400, "missing param");
        return;
    }

    if (!doc["value"].is<int>())
    {
        sendError(400, "missing value");
        return;
    }

    Param p;

    if (!SettingsManager::paramFromName(name, p))
    {
        sendError(400, "unknown param");
        return;
    }

    int oldValue = _settings->get(p);
    int newValue = doc["value"].as<int>();

    // set() сам clamp-ит по Config
    _settings->set(p, newValue);

    int clamped = _settings->get(p);

    // Если значение не изменилось — не пишем флеш
    if (clamped == oldValue)
    {
        sendOk();
        return;
    }

    _settings->save(p);

    Serial.print("[WEB] ");
    Serial.print(name);
    Serial.print(": ");
    Serial.print(oldValue);
    Serial.print(" -> ");
    Serial.println(clamped);

    sendOk();
}

// ============================================================
// GET /api/params
//   -> {"brightness":80,"mx_on":1,...,"utc":3}
// ============================================================

void WebServerManager::handleGetAllParams()
{
    if (!_settings)
    {
        sendError(500, "settings not initialized");
        return;
    }

    JsonDocument doc;

    for (uint8_t i = 0;
         i < static_cast<uint8_t>(Param::COUNT);
         ++i)
    {
        Param p = static_cast<Param>(i);

        doc[SettingsManager::paramName(p)] =
            _settings->get(p);
    }

    String body;
    serializeJson(doc, body);

    sendJson(200, body);
}

// ============================================================
// POST /api/reset
// ============================================================

void WebServerManager::handleReset()
{
    if (!_settings)
    {
        sendError(500, "settings not initialized");
        return;
    }

    _settings->resetAll();

    Serial.println("[WEB] settings reset");

    sendOk();
}

// ============================================================
// GET /api/sensors (placeholder)
// ============================================================

void WebServerManager::handleSensors()
{
    JsonDocument doc;

    doc["temperature"] = 0;
    doc["humidity"]    = 0;
    doc["light"]       = 0;
    doc["distance"]    = 0;

    String body;
    serializeJson(doc, body);

    sendJson(200, body);
}

// ============================================================
// GET /api/sd
//   -> { "files": [ {path,size,type}, ... ] }
// ============================================================

void WebServerManager::handleSD()
{
    if (!_sd || !_sd->isReady())
    {
        sendError(503, "SD not available");
        return;
    }

    // --------------------------------------------------------
    // Список файлов (макс. 300, глубина 2)
    // --------------------------------------------------------

    static constexpr size_t  MAX_FILES = 300;
    static constexpr uint8_t MAX_DEPTH = 2;

    static SDFileEntry entries[MAX_FILES];

    size_t count = _sd->listFiles(
        entries,
        MAX_FILES,
        MAX_DEPTH,
        "/"
    );

    // --------------------------------------------------------
    // JSON
    // --------------------------------------------------------

    JsonDocument doc;

    JsonArray files = doc["files"].to<JsonArray>();

    for (size_t i = 0; i < count; ++i)
    {
        JsonObject item = files.add<JsonObject>();

        item["path"] = entries[i].path;
        item["size"] = entries[i].size;
        item["type"] = entries[i].isDir ? "dir" : "file";
    }

    String body;
    serializeJson(doc, body);

    sendJson(200, body);
}

// ============================================================
// POST /api/audio/play
//
// body:
// {
//     "path":     "/music/song.wav",
//     "stream":   "media" | "alarm" | "system",
//     "volume":   0..100,          (local percent, optional)
//     "fade_in":  0..N ms,         (optional)
//     "fade_out": 0..N ms,         (optional)
//     "curve":    "linear" | "exp" | "log"   (optional)
// }
// ============================================================

void WebServerManager::handleAudioPlay()
{
    if (!_sound || !_sound->isInitialized())
    {
        sendError(503, "sound not ready");
        return;
    }

    JsonDocument doc;

    if (!parseJson(doc))
        return;

    const char* path = doc["path"];

    if (!path)
    {
        sendError(400, "missing path");
        return;
    }

    // --------------------------------------------------------
    // Проверка файла
    // --------------------------------------------------------

    if (!_sd || !_sd->isReady() || !_sd->card().exists(path))
    {
        sendError(404, "file not found");
        return;
    }

    // --------------------------------------------------------
    // Собираем PlayOptions
    // --------------------------------------------------------

    SoundManager::PlayOptions opts;

    // --- stream ---
    const char* streamStr = doc["stream"] | "media";

    if (strcmp(streamStr, "alarm") == 0)
        opts.stream = SoundManager::AudioStream::Alarm;
    else if (strcmp(streamStr, "system") == 0)
        opts.stream = SoundManager::AudioStream::System;
    else
        opts.stream = SoundManager::AudioStream::Media;

    // --- local percent ---
    opts.localPercent = static_cast<uint8_t>(
        constrain(doc["volume"] | 100, 0, 100)
    );

    // --- fades ---
    opts.fadeInMs  = doc["fade_in"]  | 0;
    opts.fadeOutMs = doc["fade_out"] | 0;

    // --- curve ---
    const char* curveStr = doc["curve"] | "linear";

    if (strcmp(curveStr, "exp") == 0)
        opts.curve = SoundManager::FadeCurve::Exponential;
    else if (strcmp(curveStr, "log") == 0)
        opts.curve = SoundManager::FadeCurve::Logarithmic;
    else
        opts.curve = SoundManager::FadeCurve::Linear;

    // --------------------------------------------------------
    // Play
    // --------------------------------------------------------

    if (!_sound->play(path, opts))
    {
        sendError(500, "play failed");
        return;
    }

    Serial.print("[WEB] audio play: ");
    Serial.println(path);

    sendOk();
}

// ============================================================
// POST /api/audio/pause
// ============================================================

void WebServerManager::handleAudioPause()
{
    if (!_sound)
    {
        sendError(503, "sound not ready");
        return;
    }

    if (!_sound->isPlaying())
    {
        sendError(409, "not playing");
        return;
    }

    if (!_sound->pause())
    {
        sendError(500, "pause failed");
        return;
    }

    Serial.println("[WEB] audio pause");

    sendOk();
}

// ============================================================
// POST /api/audio/resume
// ============================================================

void WebServerManager::handleAudioResume()
{
    if (!_sound)
    {
        sendError(503, "sound not ready");
        return;
    }

    if (!_sound->isPaused())
    {
        sendError(409, "not paused");
        return;
    }

    if (!_sound->resume())
    {
        sendError(500, "resume failed");
        return;
    }

    Serial.println("[WEB] audio resume");

    sendOk();
}

// ============================================================
// POST /api/audio/stop
//
// body (optional):
// {
//     "fade_out": 800   // ms плавного затухания
// }
// ============================================================

void WebServerManager::handleAudioStop()
{
    if (!_sound)
    {
        sendError(503, "sound not ready");
        return;
    }

    uint32_t fadeOut = 0;

    if (_server.hasArg("plain"))
    {
        JsonDocument doc;

        DeserializationError err =
            deserializeJson(doc, _server.arg("plain"));

        if (!err)
            fadeOut = doc["fade_out"] | 0;
    }

    if (fadeOut > 0)
        _sound->stop(fadeOut);
    else
        _sound->stop();

    Serial.print("[WEB] audio stop");
    if (fadeOut > 0)
    {
        Serial.print(" (fade ");
        Serial.print(fadeOut);
        Serial.print(" ms)");
    }
    Serial.println();

    sendOk();
}

// ============================================================
// GET /api/audio/status
//
// -> {
//      state:    "playing" | "paused" | "stopped" | "fading_out",
//      path:     "/music/song.wav",
//      stream:   0 | 1 | 2,
//      position: 12000,
//      duration: 180000,
//      volume:   72
//    }
// ============================================================

void WebServerManager::handleAudioStatus()
{
    if (!_sound)
    {
        sendError(503, "sound not ready");
        return;
    }

    JsonDocument doc;

    doc["state"]    = _sound->getStateString();
    doc["path"]     = _sound->getCurrentPath();
    doc["stream"]   = static_cast<uint8_t>(
                          _sound->getCurrentStream()
                      );
    doc["position"] = _sound->getPositionMs();
    doc["duration"] = _sound->getDurationMs();
    doc["volume"]   = _sound->getEffectiveVolume();

    String body;
    serializeJson(doc, body);

    sendJson(200, body);
}

// ============================================================
// HELPERS
// ============================================================

bool WebServerManager::parseJson(JsonDocument& doc)
{
    if (!_server.hasArg("plain"))
    {
        sendError(400, "body missing");
        return false;
    }

    DeserializationError err =
        deserializeJson(doc, _server.arg("plain"));

    if (err)
    {
        Serial.print("[WEB] JSON error: ");
        Serial.println(err.c_str());

        sendError(400, "invalid JSON");
        return false;
    }

    return true;
}

void WebServerManager::sendJson(
    int code,
    const String& body
)
{
    _server.send(code, "application/json", body);
}

void WebServerManager::sendOk()
{
    sendJson(200, "{\"ok\":true}");
}

void WebServerManager::sendError(
    int code,
    const char* message
)
{
    String body;
    body.reserve(64);

    body  = "{\"ok\":false,\"error\":\"";
    body += message;
    body += "\"}";

    sendJson(code, body);
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
// STATE
// ============================================================

bool WebServerManager::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

String WebServerManager::getIP() const
{
    if (WiFi.status() != WL_CONNECTED)
        return String("0.0.0.0");

    return WiFi.localIP().toString();
}