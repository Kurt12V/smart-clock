#include "WebAudioManager.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

WebAudioManager::WebAudioManager(
    SoundManager& sound,
    SDManager& sd
)
    : _sound(sound)
    , _sd(sd)
{
}


// ============================================================
// ROUTES
// ============================================================

void WebAudioManager::setupRoutes(
    WebServer& server
)
{
    // --------------------------------------------------------
    // PLAY
    // --------------------------------------------------------

    server.on(
        "/api/audio/play",
        HTTP_POST,
        [&server, this]()
        {
            handlePlay(server);
        }
    );


    // --------------------------------------------------------
    // PAUSE
    // --------------------------------------------------------

    server.on(
        "/api/audio/pause",
        HTTP_POST,
        [&server, this]()
        {
            handlePause(server);
        }
    );


    // --------------------------------------------------------
    // RESUME
    // --------------------------------------------------------

    server.on(
        "/api/audio/resume",
        HTTP_POST,
        [&server, this]()
        {
            handleResume(server);
        }
    );


    // --------------------------------------------------------
    // STOP
    // --------------------------------------------------------

    server.on(
        "/api/audio/stop",
        HTTP_POST,
        [&server, this]()
        {
            handleStop(server);
        }
    );


    // --------------------------------------------------------
    // STATUS
    // --------------------------------------------------------

    server.on(
        "/api/audio/status",
        HTTP_GET,
        [&server, this]()
        {
            handleStatus(server);
        }
    );
}


// ============================================================
// PLAY
// ============================================================

void WebAudioManager::handlePlay(
    WebServer& server
)
{
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


    // ========================================================
    // PATH
    // ========================================================

    const char* path =
        doc["path"] | "";


    if (!path || !path[0])
    {
        sendError(
            server,
            400,
            "Missing path"
        );

        return;
    }


    // ========================================================
    // FILE EXISTS
    // ========================================================

    if (!_sd.fileExists(path))
    {
        sendError(
            server,
            404,
            "File not found"
        );

        return;
    }


    // ========================================================
    // OPTIONS
    // ========================================================

    SoundManager::PlayOptions options;


    // --------------------------------------------------------
    // Stream
    // --------------------------------------------------------

    const String stream =
        doc["stream"] | "media";


    options.stream =
        parseStream(stream);


    // --------------------------------------------------------
    // Local volume
    // --------------------------------------------------------

    if (doc["localPercent"].is<int>())
    {
        const int value =
            doc["localPercent"].as<int>();


        options.localPercent =
            static_cast<uint8_t>(
                constrain(
                    value,
                    0,
                    100
                )
            );
    }


    // --------------------------------------------------------
    // Fade in
    // --------------------------------------------------------

    if (doc["fadeInMs"].is<uint32_t>())
    {
        options.fadeInMs =
            doc["fadeInMs"].as<uint32_t>();
    }


    // --------------------------------------------------------
    // Fade out
    // --------------------------------------------------------

    if (doc["fadeOutMs"].is<uint32_t>())
    {
        options.fadeOutMs =
            doc["fadeOutMs"].as<uint32_t>();
    }


    // --------------------------------------------------------
    // Curve
    // --------------------------------------------------------

    const String curve =
        doc["curve"] | "linear";


    options.curve =
        parseCurve(curve);


    // ========================================================
    // PLAY
    // ========================================================

    if (!_sound.play(
        path,
        options
    ))
    {
        sendError(
            server,
            500,
            "Failed to play audio"
        );

        return;
    }


    // ========================================================
    // RESPONSE
    // ========================================================

    JsonDocument response;


    response["ok"] =
        true;

    response["state"] =
        _sound.getStateString();

    response["path"] =
        _sound.getCurrentPath();

    response["stream"] =
        static_cast<uint8_t>(
            _sound.getCurrentStream()
        );

    response["streamName"] =
        streamToString(
            _sound.getCurrentStream()
        );

    response["localPercent"] =
        _sound.getLocalPercent();

    response["effectiveVolume"] =
        _sound.getEffectiveVolume();


    String body;


    serializeJson(
        response,
        body
    );


    server.send(
        200,
        "application/json",
        body
    );
}


// ============================================================
// PAUSE
// ============================================================

void WebAudioManager::handlePause(
    WebServer& server
)
{
    if (!_sound.isPlaying())
    {
        sendError(
            server,
            409,
            "Audio is not playing"
        );

        return;
    }


    if (!_sound.pause())
    {
        sendError(
            server,
            500,
            "Failed to pause"
        );

        return;
    }


    server.send(
        200,
        "application/json",
        "{\"ok\":true,\"state\":\"paused\"}"
    );
}


// ============================================================
// RESUME
// ============================================================

void WebAudioManager::handleResume(
    WebServer& server
)
{
    if (!_sound.isPaused())
    {
        sendError(
            server,
            409,
            "Audio is not paused"
        );

        return;
    }


    if (!_sound.resume())
    {
        sendError(
            server,
            500,
            "Failed to resume"
        );

        return;
    }


    server.send(
        200,
        "application/json",
        "{\"ok\":true,\"state\":\"playing\"}"
    );
}


// ============================================================
// STOP
// ============================================================

void WebAudioManager::handleStop(
    WebServer& server
)
{
    uint32_t fadeOutMs = 0;


    // ========================================================
    // JSON BODY
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
            doc["fadeOutMs"].is<uint32_t>())
        {
            fadeOutMs =
                doc["fadeOutMs"].as<uint32_t>();
        }
    }


    // ========================================================
    // QUERY FALLBACK
    // ========================================================

    if (
        fadeOutMs == 0 &&
        server.hasArg("fadeOutMs")
    )
    {
        fadeOutMs =
            static_cast<uint32_t>(
                server.arg(
                    "fadeOutMs"
                ).toInt()
            );
    }


    // ========================================================
    // STOP
    // ========================================================

    if (fadeOutMs > 0)
    {
        _sound.stop(
            fadeOutMs
        );
    }
    else
    {
        _sound.stop();
    }


    // ========================================================
    // RESPONSE
    // ========================================================

    JsonDocument response;


    response["ok"] =
        true;

    response["state"] =
        _sound.getStateString();

    response["fadeOutMs"] =
        fadeOutMs;


    String body;


    serializeJson(
        response,
        body
    );


    server.send(
        200,
        "application/json",
        body
    );
}


// ============================================================
// STATUS
// ============================================================

void WebAudioManager::handleStatus(
    WebServer& server
)
{
    JsonDocument doc;


    const SoundManager::AudioStream stream =
        _sound.getCurrentStream();


    const uint32_t positionMs =
        _sound.getPositionMs();


    const uint32_t durationMs =
        _sound.getDurationMs();


    // ========================================================
    // BASIC STATE
    // ========================================================

    doc["state"] =
        _sound.getStateString();

    doc["active"] =
        _sound.isActive();

    doc["playing"] =
        _sound.isPlaying();

    doc["paused"] =
        _sound.isPaused();


    // ========================================================
    // FILE
    // ========================================================

    doc["path"] =
        _sound.getCurrentPath();


    // ========================================================
    // STREAM
    // ========================================================

    doc["stream"] =
        static_cast<uint8_t>(
            stream
        );

    doc["streamName"] =
        streamToString(
            stream
        );


    // ========================================================
    // POSITION
    // ========================================================

    doc["positionMs"] =
        positionMs;

    doc["durationMs"] =
        durationMs;


    if (durationMs > 0)
    {
        doc["position"] =
            static_cast<float>(
                positionMs
            ) /
            static_cast<float>(
                durationMs
            );
    }
    else
    {
        doc["position"] =
            0.0f;
    }


    // ========================================================
    // VOLUME
    // ========================================================

    doc["localPercent"] =
        _sound.getLocalPercent();

    doc["streamVolume"] =
        _sound.getStreamVolume(stream);

    doc["effectiveVolume"] =
        _sound.getEffectiveVolume();


    // ========================================================
    // RESPONSE
    // ========================================================

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
// PARSE STREAM
// ============================================================

SoundManager::AudioStream
WebAudioManager::parseStream(
    const String& value
) const
{
    if (value.equalsIgnoreCase("alarm"))
    {
        return SoundManager::AudioStream::Alarm;
    }


    if (value.equalsIgnoreCase("system"))
    {
        return SoundManager::AudioStream::System;
    }


    return SoundManager::AudioStream::Media;
}


// ============================================================
// PARSE CURVE
// ============================================================

SoundManager::FadeCurve
WebAudioManager::parseCurve(
    const String& value
) const
{
    if (value.equalsIgnoreCase("exponential"))
    {
        return SoundManager::FadeCurve::Exponential;
    }


    if (value.equalsIgnoreCase("logarithmic"))
    {
        return SoundManager::FadeCurve::Logarithmic;
    }


    return SoundManager::FadeCurve::Linear;
}


// ============================================================
// STREAM TO STRING
// ============================================================

const char*
WebAudioManager::streamToString(
    SoundManager::AudioStream stream
) const
{
    switch (stream)
    {
        case SoundManager::AudioStream::Alarm:
            return "alarm";

        case SoundManager::AudioStream::System:
            return "system";

        case SoundManager::AudioStream::Media:
        default:
            return "media";
    }
}


// ============================================================
// CURVE TO STRING
// ============================================================

const char*
WebAudioManager::curveToString(
    SoundManager::FadeCurve curve
) const
{
    switch (curve)
    {
        case SoundManager::FadeCurve::Exponential:
            return "exponential";

        case SoundManager::FadeCurve::Logarithmic:
            return "logarithmic";

        case SoundManager::FadeCurve::Linear:
        default:
            return "linear";
    }
}


// ============================================================
// ERROR RESPONSE
// ============================================================

void WebAudioManager::sendError(
    WebServer& server,
    int code,
    const char* message
) const
{
    JsonDocument doc;


    doc["ok"] =
        false;

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