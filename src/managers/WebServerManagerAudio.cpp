#include "WebServerManager.h"

// ============================================================
// AUDIO PLAY
// ============================================================

void WebServerManager::handleAudioPlay()
{
    if (!_sound)
    {
        sendError(
            503,
            "SoundManager unavailable"
        );

        return;
    }

    if (!_sd)
    {
        sendError(
            503,
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
            400,
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
    {
        options.localPercent =
            object["localPercent"].as<uint32_t>();
    }

    if (object["fadeInMs"].is<uint32_t>())
    {
        options.fadeInMs =
            object["fadeInMs"].as<uint32_t>();
    }

    if (object["fadeOutMs"].is<uint32_t>())
    {
        options.fadeOutMs =
            object["fadeOutMs"].as<uint32_t>();
    }

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
        sendError(
            404,
            "Audio file not found"
        );

        return;
    }

    if (!_sound->play(
            path.c_str(),
            options))
    {
        sendError(
            500,
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
        200,
        output
    );
}

// ============================================================
// AUDIO PAUSE
// ============================================================

void WebServerManager::handleAudioPause()
{
    if (!_sound)
    {
        sendError(
            503,
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
    if (!_sound)
    {
        sendError(
            503,
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
    if (!_sound)
    {
        sendError(
            503,
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
        _sound->stop(
            fadeOutMs
        );
    else
        _sound->stop();

    sendOk();
}

// ============================================================
// AUDIO STATUS
// ============================================================

void WebServerManager::handleAudioStatus()
{
    if (!_sound)
    {
        sendError(
            503,
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
            ? static_cast<float>(
                position
              ) /
              static_cast<float>(
                duration
              )
            : 0.0f;

    String output;

    serializeJson(
        doc,
        output
    );

    sendJson(
        200,
        output
    );
}