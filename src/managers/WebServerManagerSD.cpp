#include "WebServerManager.h"

namespace
{
    constexpr size_t MAX_SD_FILES = 300;
}

// ============================================================
// SD
// ============================================================

void WebServerManager::handleSD()
{
    if (!_sd)
    {
        sendError(
            503,
            "SDManager unavailable"
        );

        return;
    }

    SDFileEntry entries[MAX_SD_FILES];

    const size_t count =
        _sd->listFiles(
            entries,
            MAX_SD_FILES
        );

    Serial0.printf(
        "[WEB][SD] count=%u\n",
        static_cast<unsigned>(
            count
        )
    );

    JsonDocument doc;

    JsonArray files =
        doc["files"].to<JsonArray>();

    for (
        size_t i = 0;
        i < count;
        ++i)
    {
        JsonObject item =
            files.add<JsonObject>();

        item["path"] =
            entries[i].path;

        item["size"] =
            entries[i].size;

        item["isDir"] =
            entries[i].isDir;

        if (i < 20)
        {
            Serial0.printf(
                "[WEB][SD][FILE] #%u path=%s size=%llu dir=%s\n",
                static_cast<unsigned>(i),
                entries[i].path.c_str(),
                static_cast<unsigned long long>(
                    entries[i].size
                ),
                entries[i].isDir
                    ? "true"
                    : "false"
            );
        }
    }

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