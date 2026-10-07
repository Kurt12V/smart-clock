#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "SDManager.h"


class WebSDManager
{
public:

    explicit WebSDManager(
        SDManager& sd
    );

    void setupRoutes(
        WebServer& server
    );


private:

    // ========================================================
    // CONFIG
    // ========================================================

    static constexpr size_t DEFAULT_LIMIT = 10;
    static constexpr size_t MAX_LIMIT     = 10;
    static constexpr uint8_t MAX_DEPTH    = 8;


    // ========================================================
    // HANDLER
    // ========================================================

    void handleSD(
        WebServer& server
    );


    // ========================================================
    // RESPONSE
    // ========================================================

    void sendError(
        WebServer& server,
        int code,
        const char* message
    );


    // ========================================================
    // HELPERS
    // ========================================================

    size_t getQuerySize(
        WebServer& server,
        const char* name,
        size_t defaultValue
    ) const;


    // ========================================================
    // DATA
    // ========================================================

    SDManager& _sd;
};