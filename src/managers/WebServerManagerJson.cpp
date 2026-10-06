#include "WebServerManager.h"

// ============================================================
// PARSE JSON
// ============================================================

bool WebServerManager::parseJson(
    JsonDocument& document
)
{
    if (!_server.hasArg(
            "plain"))
    {
        Serial0.println(
            "[WEB][JSON][ERROR] JSON body is missing"
        );

        sendError(
            400,
            "JSON body is required"
        );

        return false;
    }

    const String body =
        _server.arg("plain");

    Serial0.printf(
        "[WEB][JSON] length=%u\n",
        static_cast<unsigned>(
            body.length()
        )
    );

    const DeserializationError error =
        deserializeJson(
            document,
            body
        );

    if (error)
    {
        Serial0.printf(
            "[WEB][JSON][ERROR] %s\n",
            error.c_str()
        );

        sendError(
            400,
            "Invalid JSON"
        );

        return false;
    }

    return true;
}

// ============================================================
// SEND JSON
// ============================================================

void WebServerManager::sendJson(
    int code,
    const String& body
)
{
    Serial0.printf(
        "[WEB][RESPONSE] HTTP %d length=%u\n",
        code,
        static_cast<unsigned>(
            body.length()
        )
    );

    _server.send(
        code,
        "application/json",
        body
    );
}

// ============================================================
// SEND OK
// ============================================================

void WebServerManager::sendOk()
{
    JsonDocument doc;

    doc["ok"] =
        true;

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

// ============================================================
// SEND ERROR
// ============================================================

void WebServerManager::sendError(
    int code,
    const char* message
)
{
    Serial0.printf(
        "[WEB][ERROR] HTTP %d: %s\n",
        code,
        message ? message : ""
    );

    JsonDocument doc;

    doc["ok"] =
        false;

    doc["error"] =
        message ? message : "";

    String output;

    serializeJson(
        doc,
        output
    );

    sendJson(
        code,
        output
    );
}

// ============================================================
// VALIDATE ALARM ID
// ============================================================

bool WebServerManager::isValidAlarmId(
    const String& id
) const
{
    /*
     * Alarm IDs are UUID v4.
     *
     * Expected:
     *
     * xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
     */

    if (id.length() != 36)
        return false;

    for (
        size_t i = 0;
        i < id.length();
        ++i)
    {
        const char c =
            id[i];

        if (
            i == 8 ||
            i == 13 ||
            i == 18 ||
            i == 23
        )
        {
            if (c != '-')
                return false;

            continue;
        }

        const bool hex =
            (c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'f') ||
            (c >= 'A' && c <= 'F');

        if (!hex)
            return false;
    }

    // UUID version 4
    if (
        id[14] != '4'
    )
    {
        return false;
    }

    // UUID variant 10xx
    const char variant =
        id[19];

    if (
        variant != '8' &&
        variant != '9' &&
        variant != 'a' &&
        variant != 'A' &&
        variant != 'b' &&
        variant != 'B'
    )
    {
        return false;
    }

    return true;
}

// ============================================================
// GET ALARM ID
// ============================================================

String WebServerManager::getAlarmIdFromRequest()
{
    String id;

    if (_server.hasArg("id"))
    {
        id =
            _server.arg("id");
    }

    if (
        id.isEmpty() &&
        _server.hasArg("alarmId")
    )
    {
        id =
            _server.arg(
                "alarmId"
            );
    }

    if (id.isEmpty())
    {
        const String uri =
            _server.uri();

        constexpr const char* PREFIX =
            "/api/alarms/";

        if (uri.startsWith(PREFIX))
        {
            id =
                uri.substring(
                    strlen(PREFIX)
                );

            const int slash =
                id.indexOf('/');

            if (slash >= 0)
            {
                id =
                    id.substring(
                        0,
                        slash
                    );
            }
        }
    }

    id.trim();

    Serial0.printf(
        "[WEB][ALARM] resolved id='%s'\n",
        id.c_str()
    );

    return id;
}

// ============================================================
// GET BOOLEAN
// ============================================================

bool WebServerManager::getBoolean(
    JsonObjectConst object,
    const char* key,
    bool& value
) const
{
    if (!object[key].is<bool>())
    {
        Serial0.printf(
            "[WEB][JSON][ERROR] Invalid boolean: %s\n",
            key
        );

        return false;
    }

    value =
        object[key].as<bool>();

    return true;
}

// ============================================================
// GET UINT32
// ============================================================

bool WebServerManager::getUnsigned32(
    JsonObjectConst object,
    const char* key,
    uint32_t& value
) const
{
    if (!object[key].is<uint32_t>())
    {
        Serial0.printf(
            "[WEB][JSON][ERROR] Invalid uint32: %s\n",
            key
        );

        return false;
    }

    value =
        object[key].as<uint32_t>();

    return true;
}