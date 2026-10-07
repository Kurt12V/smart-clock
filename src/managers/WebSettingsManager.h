#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "SettingsManager.h"

// ============================================================
// WEB SETTINGS MANAGER
// ============================================================
//
// HTTP API:
//
// GET  /api/param?name=<name>
// POST /api/param
// GET  /api/params
// POST /api/reset
//
// POST body:
//
// {
//     "name": "matrixEnabled",
//     "value": true
// }
//
// The web API accepts both:
//
//     ParamDesc::name
//     ParamDesc::key
//
// This keeps the HTTP layer independent from internal
// SettingsManager/NVS naming.
//

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
    // PARAMETER RESOLUTION
    // ========================================================

    bool resolveParam(
        const String& name,
        SettingsManager::Param& param
    ) const;


    // ========================================================
    // VALUE HANDLING
    // ========================================================

    bool readValue(
        JsonVariantConst value,
        const SettingsManager::ParamDesc& desc,
        int& result
    ) const;


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

    void sendAllParams(
        WebServer& server
    ) const;


private:

    SettingsManager& _settings;
};