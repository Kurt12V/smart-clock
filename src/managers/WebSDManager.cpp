#include "WebSDManager.h"

#include <memory>


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
// GET QUERY PARAMETER
// ============================================================

size_t WebSDManager::getQuerySize(
    WebServer& server,
    const char* name,
    size_t defaultValue
) const
{
    if (!server.hasArg(name))
    {
        return defaultValue;
    }

    const String value =
        server.arg(name);

    if (value.isEmpty())
    {
        return defaultValue;
    }

    char* end = nullptr;

    const unsigned long parsed =
        strtoul(
            value.c_str(),
            &end,
            10
        );

    if (end == value.c_str())
    {
        return defaultValue;
    }

    return static_cast<size_t>(parsed);
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
    // QUERY
    //
    // /api/sd
    // /api/sd?offset=10
    // /api/sd?offset=20&limit=10
    // ========================================================

    const size_t offset =
        getQuerySize(
            server,
            "offset",
            0
        );

    size_t limit =
        getQuerySize(
            server,
            "limit",
            DEFAULT_LIMIT
        );


    // ========================================================
    // LIMIT
    //
    // Никогда не разрешаем запросить больше MAX_LIMIT.
    // ========================================================

    if (limit == 0)
    {
        limit = DEFAULT_LIMIT;
    }

    if (limit > MAX_LIMIT)
    {
        limit = MAX_LIMIT;
    }


    // ========================================================
    // ALLOCATE ONLY ONE PAGE
    // ========================================================

    std::unique_ptr<SDFileEntry[]> entries(
        new SDFileEntry[limit]
    );


    // ========================================================
    // LIST FILES
    //
    // ВАЖНО:
    //
    // SDManager::listFiles() сейчас возвращает только первые
    // maxFiles элементов.
    //
    // Поэтому для offset нам нужно получить данные начиная
    // с offset.
    //
    // Здесь используем отдельный временный буфер:
    //
    // offset + limit
    //
    // Но это всё равно находится в HEAP, а не в stack.
    // ========================================================

    const size_t required =
        offset + limit;


    std::unique_ptr<SDFileEntry[]> allEntries;

    if (required > limit)
    {
        allEntries.reset(
            new SDFileEntry[required]
        );
    }


    SDFileEntry* source =
        allEntries
            ? allEntries.get()
            : entries.get();


    const size_t maxFiles =
        allEntries
            ? required
            : limit;


    const size_t totalRead =
        _sd.listFiles(
            source,
            maxFiles,
            MAX_DEPTH,
            "/"
        );


    // ========================================================
    // OFFSET OUT OF RANGE
    // ========================================================

    if (offset >= totalRead)
    {
        JsonDocument doc;

        JsonArray files =
            doc["files"].to<JsonArray>();

        doc["offset"] =
            offset;

        doc["limit"] =
            limit;

        doc["count"] =
            0;

        doc["hasMore"] =
            false;

        doc["total"] =
            totalRead;


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

        return;
    }


    // ========================================================
    // ACTUAL PAGE COUNT
    // ========================================================

    size_t pageCount =
        totalRead - offset;

    if (pageCount > limit)
    {
        pageCount = limit;
    }


    // ========================================================
    // JSON
    // ========================================================

    JsonDocument doc;

    JsonArray files =
        doc["files"].to<JsonArray>();


    // ========================================================
    // ADD CURRENT PAGE
    // ========================================================

    for (
        size_t i = 0;
        i < pageCount;
        ++i
    )
    {
        const SDFileEntry& entry =
            source[offset + i];


        JsonObject file =
            files.add<JsonObject>();


        file["path"] =
            entry.path;

        file["size"] =
            entry.size;

        file["isDir"] =
            entry.isDir;
    }


    // ========================================================
    // PAGINATION INFO
    // ========================================================

    doc["offset"] =
        offset;

    doc["limit"] =
        limit;

    doc["count"] =
        pageCount;

    doc["total"] =
        totalRead;

    doc["hasMore"] =
        (offset + pageCount < totalRead);


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