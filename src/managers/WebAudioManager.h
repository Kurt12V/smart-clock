#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "SoundManager.h"
#include "SDManager.h"


class WebAudioManager
{
public:

    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    WebAudioManager(
        SoundManager& sound,
        SDManager& sd
    );


    // ========================================================
    // ROUTES
    // ========================================================

    void setupRoutes(
        WebServer& server
    );


private:

    // ========================================================
    // HANDLERS
    // ========================================================

    void handlePlay(
        WebServer& server
    );

    void handlePause(
        WebServer& server
    );

    void handleResume(
        WebServer& server
    );

    void handleStop(
        WebServer& server
    );

    void handleStatus(
        WebServer& server
    );


    // ========================================================
    // PARSERS
    // ========================================================

    SoundManager::AudioStream parseStream(
        const String& value
    ) const;

    SoundManager::FadeCurve parseCurve(
        const String& value
    ) const;


    // ========================================================
    // STRING HELPERS
    // ========================================================

    const char* streamToString(
        SoundManager::AudioStream stream
    ) const;

    const char* curveToString(
        SoundManager::FadeCurve curve
    ) const;


    // ========================================================
    // RESPONSE
    // ========================================================

    void sendError(
        WebServer& server,
        int code,
        const char* message
    ) const;


private:

    SoundManager& _sound;
    SDManager&    _sd;
};