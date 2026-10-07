#include "WebRequestUtils.h"


namespace WebRequestUtils
{

// ============================================================
// PARSE JSON
// ============================================================

bool parseJson(
    WebServer& server,
    JsonDocument& document
)
{
    if (!server.hasArg("plain"))
    {
        sendError(
            server,
            400,
            "Missing JSON body"
        );

        return false;
    }


    const DeserializationError error =
        deserializeJson(
            document,
            server.arg("plain")
        );


    if (error)
    {
        sendError(
            server,
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

void sendJson(
    WebServer& server,
    int code,
    const String& body
)
{
    server.send(
        code,
        "application/json",
        body
    );
}


// ============================================================
// OK
// ============================================================

void sendOk(
    WebServer& server
)
{
    sendJson(
        server,
        200,
        "{\"ok\":true}"
    );
}


// ============================================================
// ERROR
// ============================================================

void sendError(
    WebServer& server,
    int code,
    const char* message
)
{
    JsonDocument doc;

    doc["error"] =
        message ? message : "Unknown error";


    String body;

    serializeJson(
        doc,
        body
    );


    sendJson(
        server,
        code,
        body
    );
}


// ============================================================
// BOOLEAN
// ============================================================

bool getBoolean(
    JsonObjectConst object,
    const char* key,
    bool& value
)
{
    if (!object[key].is<bool>() &&
        !object[key].is<int>() &&
        !object[key].is<const char*>())
    {
        return false;
    }


    if (object[key].is<bool>())
    {
        value =
            object[key].as<bool>();

        return true;
    }


    if (object[key].is<int>())
    {
        value =
            object[key].as<int>() != 0;

        return true;
    }


    const char* text =
        object[key].as<const char*>();


    if (!text)
        return false;


    String valueString =
        text;

    valueString.toLowerCase();


    if (
        valueString == "true" ||
        valueString == "1" ||
        valueString == "on" ||
        valueString == "yes"
    )
    {
        value = true;
        return true;
    }


    if (
        valueString == "false" ||
        valueString == "0" ||
        valueString == "off" ||
        valueString == "no"
    )
    {
        value = false;
        return true;
    }


    return false;
}


// ============================================================
// UINT32
// ============================================================

bool getUnsigned32(
    JsonObjectConst object,
    const char* key,
    uint32_t& value
)
{
    if (object[key].is<uint32_t>())
    {
        value =
            object[key].as<uint32_t>();

        return true;
    }


    if (object[key].is<int>())
    {
        const int number =
            object[key].as<int>();

        if (number < 0)
            return false;

        value =
            static_cast<uint32_t>(
                number
            );

        return true;
    }


    if (object[key].is<const char*>())
    {
        const char* text =
            object[key].as<const char*>();

        if (!text || !text[0])
            return false;


        char* end = nullptr;

        const unsigned long parsed =
            strtoul(
                text,
                &end,
                10
            );


        if (
            end == text ||
            *end != '\0'
        )
        {
            return false;
        }


        value =
            static_cast<uint32_t>(
                parsed
            );

        return true;
    }


    return false;
}


// ============================================================
// UUID V4
// ============================================================

bool isValidUuidV4(
    const String& id
)
{
    if (id.length() != 36)
        return false;


    for (
        int i = 0;
        i < 36;
        ++i
    )
    {
        if (
            i == 8 ||
            i == 13 ||
            i == 18 ||
            i == 23
        )
        {
            if (id[i] != '-')
                return false;

            continue;
        }


        const char c =
            id[i];


        const bool hex =
            (c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'f') ||
            (c >= 'A' && c <= 'F');


        if (!hex)
            return false;
    }


    if (id[14] != '4')
        return false;


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
// PATH PARAMETER
// ============================================================

String getPathParameter(
    const String& uri,
    const String& prefix
)
{
    if (!uri.startsWith(prefix))
        return String();


    String result =
        uri.substring(
            prefix.length()
        );


    const int slash =
        result.indexOf('/');


    if (slash >= 0)
    {
        result =
            result.substring(
                0,
                slash
            );
    }


    return result;
}

}