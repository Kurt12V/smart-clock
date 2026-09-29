#include "WebServerManager.h"

#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>


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
    SDManager& sd,
    SoundManager& sound,
    const char* ssid,
    const char* password
)
{
    Serial.println();
    Serial.println(
        "========================================"
    );
    Serial.println(
        "          WEB SERVER STARTING"
    );
    Serial.println(
        "========================================"
    );


    // --------------------------------------------------------
    // MANAGERS
    // --------------------------------------------------------

    _settings = &settings;
    _sd       = &sd;
    _sound    = &sound;


    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    Serial.println(
        "[WEB] Starting WiFi..."
    );


    WiFi.mode(WIFI_STA);


    // Не стираем сохранённую конфигурацию WiFi.

    WiFi.disconnect(
        false,
        false
    );


    delay(50);


    if (
        !ssid ||
        !password
    )
    {
        Serial.println(
            "[WEB] ERROR: invalid WiFi credentials"
        );

        return false;
    }


    WiFi.begin(
        ssid,
        password
    );


    const uint32_t startTime =
        millis();


    // --------------------------------------------------------
    // WAIT FOR WIFI
    // --------------------------------------------------------

    while (
        WiFi.status() != WL_CONNECTED &&
        static_cast<uint32_t>(
            millis() - startTime
        ) < 20000UL
    )
    {
        delay(100);
        yield();
    }


    if (
        WiFi.status() != WL_CONNECTED
    )
    {
        Serial.println(
            "[WEB] WiFi connection FAILED"
        );

        Serial.print(
            "[WEB] Status: "
        );

        Serial.println(
            WiFi.status()
        );

        return false;
    }


    // --------------------------------------------------------
    // WIFI INFO
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[WEB] WiFi connected"
    );


    Serial.print(
        "[WEB] IP: "
    );

    Serial.println(
        WiFi.localIP()
    );


    Serial.print(
        "[WEB] RSSI: "
    );

    Serial.println(
        WiFi.RSSI()
    );


    // --------------------------------------------------------
    // LITTLEFS
    // --------------------------------------------------------

    Serial.println();

    Serial.println(
        "[WEB] Mounting LittleFS..."
    );


    if (
        !LittleFS.begin(true)
    )
    {
        Serial.println(
            "[WEB] LittleFS mount FAILED"
        );

        return false;
    }


    Serial.println(
        "[WEB] LittleFS mounted"
    );


    if (
        !LittleFS.exists(
            "/index.html"
        )
    )
    {
        Serial.println(
            "[WEB] WARNING: /index.html NOT FOUND"
        );
    }


    // --------------------------------------------------------
    // ROUTES
    // --------------------------------------------------------

    setupRoutes();


    // --------------------------------------------------------
    // START HTTP SERVER
    // --------------------------------------------------------

    _server.begin();


    _initialized = true;


    Serial.println();
    Serial.println(
        "[WEB] HTTP server started"
    );


    Serial.print(
        "[WEB] Open: http://"
    );

    Serial.println(
        WiFi.localIP()
    );


    Serial.println(
        "========================================"
    );

    Serial.println(
        "          WEB SERVER READY"
    );

    Serial.println(
        "========================================"
    );


    return true;
}


// ============================================================
// SETUP ROUTES
// ============================================================

void WebServerManager::setupRoutes()
{
    // --------------------------------------------------------
    // ROOT
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
    // GET PARAM
    // --------------------------------------------------------

    _server.on(
        "/api/param",
        HTTP_GET,
        [this]()
        {
            handleGetParam();
        }
    );


    // --------------------------------------------------------
    // SET PARAM
    // --------------------------------------------------------

    _server.on(
        "/api/param",
        HTTP_POST,
        [this]()
        {
            handleSetParam();
        }
    );


    // --------------------------------------------------------
    // GET ALL PARAMS
    // --------------------------------------------------------

    _server.on(
        "/api/params",
        HTTP_GET,
        [this]()
        {
            handleGetAllParams();
        }
    );


    // --------------------------------------------------------
    // RESET
    // --------------------------------------------------------

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
    // AUDIO PLAY
    // --------------------------------------------------------

    _server.on(
        "/api/audio/play",
        HTTP_POST,
        [this]()
        {
            handleAudioPlay();
        }
    );


    // --------------------------------------------------------
    // AUDIO PAUSE
    // --------------------------------------------------------

    _server.on(
        "/api/audio/pause",
        HTTP_POST,
        [this]()
        {
            handleAudioPause();
        }
    );


    // --------------------------------------------------------
    // AUDIO RESUME
    // --------------------------------------------------------

    _server.on(
        "/api/audio/resume",
        HTTP_POST,
        [this]()
        {
            handleAudioResume();
        }
    );


    // --------------------------------------------------------
    // AUDIO STOP
    // --------------------------------------------------------

    _server.on(
        "/api/audio/stop",
        HTTP_POST,
        [this]()
        {
            handleAudioStop();
        }
    );


    // --------------------------------------------------------
    // AUDIO STATUS
    // --------------------------------------------------------

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
    if (
        !LittleFS.exists(
            "/index.html"
        )
    )
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
            "Failed to open index.html"
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
        "404"
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


    if (
        name.length() == 0
    )
    {
        sendError(
            400,
            "missing name"
        );

        return;
    }


    Param param;


    if (
        !SettingsManager::paramFromName(
            name.c_str(),
            param
        )
    )
    {
        sendError(
            400,
            "unknown param"
        );

        return;
    }


    String body;

    body.reserve(64);


    body +=
        "{\"param\":\"";


    body +=
        SettingsManager::paramName(
            param
        );


    body +=
        "\",\"value\":";


    body +=
        String(
            _settings->get(param)
        );


    body +=
        "}";


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
//     "param": "brightness",
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


    if (
        !parseJson(doc)
    )
    {
        return;
    }


    const char* name =
        doc["param"];


    if (!name)
    {
        sendError(
            400,
            "missing param"
        );

        return;
    }


    JsonVariant value =
        doc["value"];


    if (
        value.isNull()
    )
    {
        sendError(
            400,
            "missing value"
        );

        return;
    }


    Param param;


    if (
        !SettingsManager::paramFromName(
            name,
            param
        )
    )
    {
        sendError(
            400,
            "unknown param"
        );

        return;
    }


    const int requestedValue =
        value.as<int>();


    // --------------------------------------------------------
    // SET RAM
    // --------------------------------------------------------

    const bool changed =
        _settings->set(
            param,
            requestedValue
        );


    // --------------------------------------------------------
    // ACTUAL VALUE
    // --------------------------------------------------------
    //
    // SettingsManager может сделать clamp.
    //
    // Например:
    //
    // 150 -> 100
    //
    // Поэтому возвращаем именно фактическое значение.
    //
    // --------------------------------------------------------

    const int actualValue =
        _settings->get(param);


    // --------------------------------------------------------
    // RESPONSE
    // --------------------------------------------------------

    String body;

    body.reserve(96);


    body +=
        "{\"ok\":true";


    body +=
        ",\"changed\":";


    body +=
        changed
        ? "true"
        : "false";


    body +=
        ",\"param\":\"";


    body +=
        SettingsManager::paramName(
            param
        );


    body +=
        "\",\"value\":";


    body +=
        String(actualValue);


    body +=
        "}";


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


    // --------------------------------------------------------
    // ADD ALL PARAMETERS
    // --------------------------------------------------------

    for (
        uint8_t i = 0;
        i < static_cast<uint8_t>(Param::COUNT);
        ++i
    )
    {
        const Param param =
            static_cast<Param>(i);


        doc[
            SettingsManager::paramName(
                param
            )
        ] =
            _settings->get(
                param
            );
    }


    String body;

    body.reserve(512);


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
// Сейчас WebServerManager не получает SensorsManager.
//
// Поэтому endpoint оставлен для совместимости с HTML/API,
// но реальные датчики здесь НЕ генерируются.
//
// ============================================================

void WebServerManager::handleSensors()
{
    JsonDocument doc;


    doc["temperature"] = 0;
    doc["humidity"]    = 0;
    doc["light"]       = 0;
    doc["distance"]    = 0;


    String body;

    body.reserve(128);


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
    if (!_sd)
    {
        sendError(
            503,
            "sd not initialized"
        );

        return;
    }


    if (
        !_sd->isReady()
    )
    {
        sendError(
            503,
            "SD not available"
        );

        return;
    }


    // --------------------------------------------------------
    // FILE LIST
    // --------------------------------------------------------

    static constexpr size_t MAX_FILES = 300;

    static constexpr uint8_t MAX_DEPTH = 2;


    static SDFileEntry entries[MAX_FILES];


    const size_t count =
        _sd->listFiles(
            entries,
            MAX_FILES,
            MAX_DEPTH,
            "/"
        );


    // --------------------------------------------------------
    // JSON
    // --------------------------------------------------------

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


    body.reserve(
        512 +
        count * 64
    );


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
//
// Current SoundManager API:
//
// PlayOptions:
//
//     stream
//     localPercent
//     fadeInMs
//     fadeOutMs
//     curve
//
// Старых:
//     volume
//     loop
//     fadeIn
//     fadeOut
//
// здесь больше НЕТ.
// ============================================================

void WebServerManager::handleAudioPlay()
{
    if (!_sound)
    {
        sendError(
            503,
            "audio unavailable"
        );

        return;
    }


    if (!_sd)
    {
        sendError(
            503,
            "sd not initialized"
        );

        return;
    }


    if (
        !_sd->isReady()
    )
    {
        sendError(
            503,
            "SD not available"
        );

        return;
    }


    JsonDocument doc;


    if (
        !parseJson(doc)
    )
    {
        return;
    }


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


    // --------------------------------------------------------
    // FILE CHECK
    // --------------------------------------------------------

    if (
        !_sd->card().exists(path)
    )
    {
        sendError(
            404,
            "file not found"
        );

        return;
    }


    // --------------------------------------------------------
    // OPTIONS
    // --------------------------------------------------------

    SoundManager::PlayOptions options;


    // --------------------------------------------------------
    // STREAM
    // --------------------------------------------------------

    if (
        !doc["stream"].isNull()
    )
    {
        const int stream =
            doc["stream"].as<int>();


        switch (stream)
        {
            case 1:

                options.stream =
                    SoundManager::AudioStream::Alarm;

                break;


            case 2:

                options.stream =
                    SoundManager::AudioStream::System;

                break;


            case 0:
            default:

                options.stream =
                    SoundManager::AudioStream::Media;

                break;
        }
    }


    // --------------------------------------------------------
    // LOCAL PERCENT
    // --------------------------------------------------------

    if (
        !doc["localPercent"].isNull()
    )
    {
        options.localPercent =
            static_cast<uint8_t>(
                constrain(
                    doc["localPercent"].as<int>(),
                    0,
                    100
                )
            );
    }


    // --------------------------------------------------------
    // FADE IN
    // --------------------------------------------------------

    if (
        !doc["fadeInMs"].isNull()
    )
    {
        options.fadeInMs =
            doc["fadeInMs"].as<uint32_t>();
    }


    // --------------------------------------------------------
    // FADE OUT
    // --------------------------------------------------------

    if (
        !doc["fadeOutMs"].isNull()
    )
    {
        options.fadeOutMs =
            doc["fadeOutMs"].as<uint32_t>();
    }


    // --------------------------------------------------------
    // FADE CURVE
    // --------------------------------------------------------

    if (
        !doc["curve"].isNull()
    )
    {
        const int curve =
            doc["curve"].as<int>();


        switch (curve)
        {
            case 1:

                options.curve =
                    SoundManager::FadeCurve::Exponential;

                break;


            case 2:

                options.curve =
                    SoundManager::FadeCurve::Logarithmic;

                break;


            case 0:
            default:

                options.curve =
                    SoundManager::FadeCurve::Linear;

                break;
        }
    }


    // --------------------------------------------------------
    // PLAY
    // --------------------------------------------------------

    const bool success =
        _sound->play(
            path,
            options
        );


    if (!success)
    {
        sendError(
            500,
            "play failed"
        );

        return;
    }


    // --------------------------------------------------------
    // RESPONSE
    // --------------------------------------------------------

    JsonDocument response;


    response["ok"] = true;

    response["state"] =
        _sound->getStateString();

    response["path"] =
        _sound->getCurrentPath();


    String body;

    body.reserve(160);


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
// POST /api/audio/pause
// ============================================================

void WebServerManager::handleAudioPause()
{
    if (!_sound)
    {
        sendError(
            503,
            "audio unavailable"
        );

        return;
    }


    if (
        !_sound->pause()
    )
    {
        sendError(
            409,
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
            "audio unavailable"
        );

        return;
    }


    if (
        !_sound->resume()
    )
    {
        sendError(
            409,
            "resume failed"
        );

        return;
    }


    sendOk();
}


// ============================================================
// POST /api/audio/stop
// ============================================================
//
// Можно передать:
//
// {
//     "fadeOutMs": 1000
// }
//
// или:
//
// {}
//
// ============================================================

void WebServerManager::handleAudioStop()
{
    if (!_sound)
    {
        sendError(
            503,
            "audio unavailable"
        );

        return;
    }


    uint32_t fadeOutMs = 0;


    // --------------------------------------------------------
    // JSON OPTIONAL
    // --------------------------------------------------------

    if (
        _server.hasArg("plain")
    )
    {
        JsonDocument doc;


        const String& body =
            _server.arg("plain");


        if (
            body.length() > 0
        )
        {
            const DeserializationError error =
                deserializeJson(
                    doc,
                    body
                );


            if (!error &&
                !doc["fadeOutMs"].isNull())
            {
                fadeOutMs =
                    doc["fadeOutMs"].as<uint32_t>();
            }
        }
    }


    // --------------------------------------------------------
    // STOP
    // --------------------------------------------------------

    if (fadeOutMs > 0)
    {
        _sound->stop(
            fadeOutMs
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
    if (!_sound)
    {
        sendError(
            503,
            "audio unavailable"
        );

        return;
    }


    JsonDocument doc;


    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    doc["ok"] =
        true;


    doc["state"] =
        _sound->getStateString();


    doc["playing"] =
        _sound->isPlaying();


    doc["paused"] =
        _sound->isPaused();


    doc["active"] =
        _sound->isActive();


    // --------------------------------------------------------
    // FILE
    // --------------------------------------------------------

    doc["path"] =
        _sound->getCurrentPath();


    // --------------------------------------------------------
    // POSITION
    // --------------------------------------------------------

    doc["positionMs"] =
        _sound->getPositionMs();


    doc["durationMs"] =
        _sound->getDurationMs();


    // --------------------------------------------------------
    // VOLUME
    // --------------------------------------------------------

    doc["localPercent"] =
        _sound->getLocalPercent();


    doc["effectiveVolume"] =
        _sound->getEffectiveVolume();


    // --------------------------------------------------------
    // STREAM
    // --------------------------------------------------------

    doc["stream"] =
        static_cast<uint8_t>(
            _sound->getCurrentStream()
        );


    String body;

    body.reserve(320);


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
    if (
        !_server.hasArg("plain")
    )
    {
        sendError(
            400,
            "missing body"
        );

        return false;
    }


    const String& body =
        _server.arg("plain");


    if (
        body.length() == 0
    )
    {
        sendError(
            400,
            "empty body"
        );

        return false;
    }


    const DeserializationError error =
        deserializeJson(
            doc,
            body
        );


    if (error)
    {
        sendError(
            400,
            "invalid json"
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
    _server.send(
        200,
        "application/json",
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
    String body;

    body.reserve(128);


    body +=
        "{\"ok\":false,\"error\":\"";


    if (message)
    {
        // ----------------------------------------------------
        // Минимальное экранирование кавычек.
        // ----------------------------------------------------

        for (
            const char* p = message;
            *p;
            ++p
        )
        {
            if (
                *p == '"' ||
                *p == '\\'
            )
            {
                body += '\\';
            }

            body += *p;
        }
    }


    body +=
        "\"}";


    _server.send(
        code,
        "application/json",
        body
    );
}


// ============================================================
// UPDATE
// ============================================================

void WebServerManager::update()
{
    if (!_initialized)
    {
        return;
    }


    // --------------------------------------------------------
    // HTTP
    // --------------------------------------------------------

    _server.handleClient();


    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------
    //
    // 700 ms debounce выполняется здесь.
    //
    // --------------------------------------------------------

    if (_settings)
    {
        _settings->update();
    }
}


// ============================================================
// IS CONNECTED
// ============================================================

bool WebServerManager::isConnected() const
{
    return (
        _initialized &&
        WiFi.status() == WL_CONNECTED
    );
}


// ============================================================
// GET IP
// ============================================================
//
// В .h возвращается String,
// поэтому здесь тоже String.
//
// ============================================================

String WebServerManager::getIP() const
{
    if (
        WiFi.status() != WL_CONNECTED
    )
    {
        return String();
    }


    return WiFi.localIP().toString();
}