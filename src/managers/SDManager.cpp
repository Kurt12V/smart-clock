#include "WebSDManager.h"

#include <memory>


namespace
{
    constexpr size_t DEFAULT_LIMIT = 10;
    constexpr size_t MAX_LIMIT     = 10;
    constexpr uint8_t MAX_DEPTH    = 8;
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
// QUERY PARAMETER
// ============================================================

size_t WebSDManager::getQuerySize(
    WebServer& server,
    const char* name,
    size_t defaultValue
) const
{
    if (!server.hasArg(name))
        return defaultValue;


    const String value =
        server.arg(name);


    if (value.isEmpty())
        return defaultValue;


    char* end = nullptr;


    const unsigned long parsed =
        strtoul(
            value.c_str(),
            &end,
            10
        );


    if (end == value.c_str())
        return defaultValue;


    return static_cast<size_t>(
        parsed
    );
}


// ============================================================
// GET SD
// ============================================================

void WebSDManager::handleSD(
    WebServer& server
)
{
    // ========================================================
    // CHECK SD
    // ========================================================

    if (!_sd.isReady())
    {
        sendError(
            server,
            503,
            "SD card is not ready"
        );

        return;
    }


    // ========================================================
    // OFFSET
    // ========================================================

    const size_t offset =
        getQuerySize(
            server,
            "offset",
            0
        );


    // ========================================================
    // LIMIT
    // ========================================================

    size_t limit =
        getQuerySize(
            server,
            "limit",
            DEFAULT_LIMIT
        );


    if (limit == 0)
        limit = DEFAULT_LIMIT;


    if (limit > MAX_LIMIT)
        limit = MAX_LIMIT;


    // ========================================================
    // ALLOCATE ONLY CURRENT PAGE
    // ========================================================

    std::unique_ptr<SDFileEntry[]> entries(
        new SDFileEntry[limit]
    );


    // ========================================================
    // READ PAGE
    // ========================================================

    bool hasMore = false;


    const size_t count =
        _sd.listFilesPage(
            entries.get(),
            limit,
            offset,
            hasMore,
            MAX_DEPTH,
            "/"
        );


    // ========================================================
    // JSON
    // ========================================================

    JsonDocument doc;


    JsonArray files =
        doc["files"].to<JsonArray>();


    // ========================================================
    // FILES
    // ========================================================

    for (
        size_t i = 0;
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


    // ========================================================
    // PAGINATION
    // ========================================================

    doc["offset"] =
        offset;


    doc["limit"] =
        limit;


    doc["count"] =
        count;


    doc["hasMore"] =
        hasMore;


    // ========================================================
    // SERIALIZE
    // ========================================================

    String response;


    serializeJson(
        doc,
        response
    );


    // ========================================================
    // RESPONSE
    // ========================================================

    server.send(
        200,
        "application/json",
        response
    );
}


// ============================================================
// ERROR
// ============================================================

void WebSDManager::sendError(
    WebServer& server,
    int code,
    const char* message
)
{
    JsonDocument doc;


    doc["error"] =
        message;


    String response;


    serializeJson(
        doc,
        response
    );


    server.send(
        code,
        "application/json",
        response
    );
}