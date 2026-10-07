#include "WebSDManager.h"


namespace
{
    constexpr size_t MAX_FILES = 300;
}


// ============================================================
// CONSTRUCTOR
// ============================================================

WebSDManager::WebSDManager(
    SDManager& sd
)
    : _sd(sd)
{
}


// ============================================================
// ROUTES
// ============================================================

void WebSDManager::setupRoutes(
    WebServer& server
)
{
    server.on(
        "/api/sd",
        HTTP_GET,
        [&server, this]()
        {
            handleSD(server);
        }
    );
}


// ============================================================
// GET SD
// ============================================================

void WebSDManager::handleSD(
    WebServer& server
)
{
    SDFileEntry entries[MAX_FILES];

    uint16_t count = 0;

    if (!_sd.listFiles(
        entries,
        MAX_FILES,
        count
    ))
    {
        server.send(
            500,
            "application/json",
            "{\"error\":\"Failed to list SD files\"}"
        );

        return;
    }


    JsonDocument doc;

    JsonArray files =
        doc["files"].to<JsonArray>();


    for (
        uint16_t i = 0;
        i < count;
        ++i
    )
    {
        JsonObject file =
            files.add<JsonObject>();

        file["path"] =
            entries[i].path;

        file["size"] =
            entries[i].size;

        file["isDir"] =
            entries[i].isDir;
    }


    String response;

    serializeJson(
        doc,
        response
    );

    server.send(
        200,
        "application/json",
        response
    );
}