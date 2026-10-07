#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "SettingsManager.h"

// ============================================================
// WEB SETTINGS MANAGER
// ============================================================
//
// HTTP API for SettingsManager.
//
// Routes:
//
// GET  /api/param?name=brightness
// POST /api/param
// GET  /api/params
// POST /api/reset
//
// ============================================================

class WebSettingsManager
{
public:

    explicit WebSettingsManager(
        SettingsManager& settings
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

    void handleGetParam(
        WebServer& server
    );

    void handleSetParam(
        WebServer& server
    );

    void handleGetAllParams(
        WebServer& server
    );

    void handleReset(
        WebServer& server
    );


    // ========================================================
    // RESPONSE HELPERS
    // ========================================================

    void sendError(
        WebServer& server,
        int statusCode,
        const char* message
    ) const;

    void sendParam(
        WebServer& server,
        SettingsManager::Param param,
        int statusCode = 200
    ) const;


    // ========================================================
    // VALIDATION
    // ========================================================

    bool resolveParam(
        const String& name,
        SettingsManager::Param& param
    ) const;


private:

    SettingsManager& _settings;
};