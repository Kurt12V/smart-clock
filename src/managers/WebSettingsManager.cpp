#include "WebSettingsManager.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

WebSettingsManager::WebSettingsManager(
    SettingsManager& settings
)
    : _settings(settings)
{
}


// ============================================================
// ROUTES
// ============================================================

void WebSettingsManager::setupRoutes(
    WebServer& server
)
{
    // --------------------------------------------------------
    // GET /api/param?name=...
    // --------------------------------------------------------

    server.on(
        "/api/param",
        HTTP_GET,
        [&server, this]()
        {
            handleGetParam(server);
        }
    );


    // --------------------------------------------------------
    // POST /api/param
    // --------------------------------------------------------

    server.on(
        "/api/param",
        HTTP_POST,
        [&server, this]()
        {
            handleSetParam(server);
        }
    );


    // --------------------------------------------------------
    // GET /api/params
    // --------------------------------------------------------

    server.on(
        "/api/params",
        HTTP_GET,
        [&server, this]()
        {
            handleGetAllParams(server);
        }
    );


    // --------------------------------------------------------
    // POST /api/reset
    // --------------------------------------------------------

    server.on(
        "/api/reset",
        HTTP_POST,
        [&server, this]()
        {
            handleReset(server);
        }
    );
}


// ============================================================
// GET PARAM
// ============================================================
//
// GET /api/param?name=matrixEnabled
//
// Also accepts:
//
// GET /api/param?name=matrix_on
//
// ============================================================

void WebSettingsManager::handleGetParam(
    WebServer& server
)
{
    if (!server.hasArg("name"))
    {
        sendError(
            server,
            400,
            "Missing parameter name"
        );

        return;
    }


    const String name =
        server.arg("name");


    if (name.isEmpty())
    {
        sendError(
            server,
            400,
            "Parameter name is empty"
        );

        return;
    }


    SettingsManager::Param param;

    if (!resolveParam(
            name,
            param))
    {
        sendError(
            server,
            404,
            "Unknown parameter"
        );

        return;
    }


    sendParam(
        server,
        param
    );
}


// ============================================================
// SET PARAM
// ============================================================
//
// POST /api/param
//
// {
//     "name": "matrixEnabled",
//     "value": true
// }
//
// Numeric:
//
// {
//     "name": "matrixBrightness",
//     "value": 80
// }
//
// ============================================================

void WebSettingsManager::handleSetParam(
    WebServer& server
)
{
    // --------------------------------------------------------
    // Request body
    // --------------------------------------------------------

    if (!server.hasArg("plain"))
    {
        sendError(
            server,
            400,
            "Missing JSON body"
        );

        return;
    }


    const String body =
        server.arg("plain");


    if (body.isEmpty())
    {
        sendError(
            server,
            400,
            "Empty JSON body"
        );

        return;
    }


    // --------------------------------------------------------
    // Parse JSON
    // --------------------------------------------------------

    JsonDocument doc;

    const DeserializationError error =
        deserializeJson(
            doc,
            body
        );

    if (error)
    {
        sendError(
            server,
            400,
            "Invalid JSON"
        );

        return;
    }


    // --------------------------------------------------------
    // Parameter name
    // --------------------------------------------------------

    if (!doc["name"].is<const char*>())
    {
        sendError(
            server,
            400,
            "Missing name"
        );

        return;
    }


    const char* name =
        doc["name"].as<const char*>();


    if (
        name == nullptr ||
        name[0] == '\0'
    )
    {
        sendError(
            server,
            400,
            "Missing name"
        );

        return;
    }


    SettingsManager::Param param;

    if (!resolveParam(
            String(name),
            param))
    {
        sendError(
            server,
            404,
            "Unknown parameter"
        );

        return;
    }


    // --------------------------------------------------------
    // Parameter description
    // --------------------------------------------------------

    const SettingsManager::ParamDesc& desc =
        _settings.getDesc(param);


    // --------------------------------------------------------
    // Value
    // --------------------------------------------------------

    if (!doc.containsKey("value"))
    {
        sendError(
            server,
            400,
            "Missing value"
        );

        return;
    }


    int value = 0;

    if (!readValue(
            doc["value"],
            desc,
            value))
    {
        sendError(
            server,
            400,
            "Invalid parameter value"
        );

        return;
    }


    // --------------------------------------------------------
    // Clamp
    // --------------------------------------------------------

    if (value < desc.minValue)
    {
        value = desc.minValue;
    }

    if (value > desc.maxValue)
    {
        value = desc.maxValue;
    }


    // --------------------------------------------------------
    // Set
    // --------------------------------------------------------
    //
    // SettingsManager::set() returning false does not
    // necessarily mean an error.
    //
    // The value may simply already be equal to the current
    // value.
    //
    // Therefore the actual value is returned below.
    // --------------------------------------------------------

    _settings.set(
        param,
        value
    );


    // --------------------------------------------------------
    // Return actual state
    // --------------------------------------------------------

    sendParam(
        server,
        param
    );
}


// ============================================================
// GET ALL PARAMETERS
// ============================================================
//
// GET /api/params
//
// ============================================================

void WebSettingsManager::handleGetAllParams(
    WebServer& server
)
{
    sendAllParams(server);
}


// ============================================================
// RESET
// ============================================================
//
// POST /api/reset
//
// ============================================================

void WebSettingsManager::handleReset(
    WebServer& server
)
{
    _settings.resetAll();

    sendAllParams(server);
}


// ============================================================
// RESOLVE PARAMETER
// ============================================================
//
// The HTTP API can identify a parameter by:
//
// 1. ParamDesc::name
// 2. ParamDesc::key
// 3. SettingsManager::paramFromName()
//
// This allows the frontend and internal settings names
// to remain independent.
//
// ============================================================

bool WebSettingsManager::resolveParam(
    const String& name,
    SettingsManager::Param& param
) const
{
    // --------------------------------------------------------
    // First use SettingsManager's own resolver.
    // --------------------------------------------------------

    param =
        _settings.paramFromName(
            name.c_str()
        );

    if (
        param !=
        SettingsManager::Param::COUNT
    )
    {
        return true;
    }


    // --------------------------------------------------------
    // Fallback:
    // search ParamDesc::name and ParamDesc::key.
    // --------------------------------------------------------

    constexpr uint8_t PARAM_COUNT =
        static_cast<uint8_t>(
            SettingsManager::Param::COUNT
        );


    for (
        uint8_t i = 0;
        i < PARAM_COUNT;
        ++i
    )
    {
        const SettingsManager::Param candidate =
            static_cast<SettingsManager::Param>(i);


        const SettingsManager::ParamDesc& desc =
            _settings.getDesc(candidate);


        if (
            name.equals(
                desc.name
            )
        )
        {
            param = candidate;
            return true;
        }


        if (
            name.equals(
                desc.key
            )
        )
        {
            param = candidate;
            return true;
        }
    }


    param =
        SettingsManager::Param::COUNT;

    return false;
}


// ============================================================
// READ VALUE
// ============================================================
//
// Supports:
//
// bool
// int
// unsigned int
// float/double
//
// SettingsManager currently stores the value as integer,
// therefore everything is converted to int.
//
// Boolean values are converted:
//
// false -> 0
// true  -> 1
//
// ============================================================

bool WebSettingsManager::readValue(
    JsonVariantConst value,
    const SettingsManager::ParamDesc& desc,
    int& result
) const
{
    // --------------------------------------------------------
    // Boolean
    // --------------------------------------------------------

    if (value.is<bool>())
    {
        result =
            value.as<bool>()
                ? 1
                : 0;

        return true;
    }


    // --------------------------------------------------------
    // Signed integer
    // --------------------------------------------------------

    if (value.is<int>())
    {
        result =
            value.as<int>();

        return true;
    }


    // --------------------------------------------------------
    // Unsigned integer
    // --------------------------------------------------------

    if (value.is<unsigned int>())
    {
        const unsigned int raw =
            value.as<unsigned int>();


        if (
            raw >
            static_cast<unsigned int>(
                INT_MAX
            )
        )
        {
            result = INT_MAX;
        }
        else
        {
            result =
                static_cast<int>(raw);
        }

        return true;
    }


    // --------------------------------------------------------
    // Long
    // --------------------------------------------------------

    if (value.is<long>())
    {
        const long raw =
            value.as<long>();


        if (raw < INT_MIN)
        {
            result = INT_MIN;
        }
        else if (raw > INT_MAX)
        {
            result = INT_MAX;
        }
        else
        {
            result =
                static_cast<int>(raw);
        }

        return true;
    }


    // --------------------------------------------------------
    // Unsigned long
    // --------------------------------------------------------

    if (value.is<unsigned long>())
    {
        const unsigned long raw =
            value.as<unsigned long>();


        if (
            raw >
            static_cast<unsigned long>(
                INT_MAX
            )
        )
        {
            result = INT_MAX;
        }
        else
        {
            result =
                static_cast<int>(raw);
        }

        return true;
    }


    // --------------------------------------------------------
    // Floating point
    //
    // SettingsManager uses integer values, so accept a number
    // only if it represents an integer.
    // --------------------------------------------------------

    if (value.is<float>())
    {
        const float raw =
            value.as<float>();


        const int converted =
            static_cast<int>(raw);


        if (
            static_cast<float>(converted) != raw
        )
        {
            return false;
        }


        result = converted;

        return true;
    }


    if (value.is<double>())
    {
        const double raw =
            value.as<double>();


        const int converted =
            static_cast<int>(raw);


        if (
            static_cast<double>(converted) != raw
        )
        {
            return false;
        }


        result = converted;

        return true;
    }


    return false;
}


// ============================================================
// SEND PARAMETER
// ============================================================

void WebSettingsManager::sendParam(
    WebServer& server,
    SettingsManager::Param param,
    int statusCode
) const
{
    const SettingsManager::ParamDesc& desc =
        _settings.getDesc(param);


    JsonDocument doc;


    doc["ok"] = true;

    doc["name"] =
        desc.name;

    doc["key"] =
        desc.key;

    doc["value"] =
        _settings.get(param);

    doc["min"] =
        desc.minValue;

    doc["max"] =
        desc.maxValue;

    doc["default"] =
        desc.defaultValue;


    String response;

    serializeJson(
        doc,
        response
    );


    server.send(
        statusCode,
        "application/json",
        response
    );
}


// ============================================================
// SEND ALL PARAMETERS
// ============================================================

void WebSettingsManager::sendAllParams(
    WebServer& server
) const
{
    JsonDocument doc;

    doc["ok"] = true;


    JsonArray array =
        doc["params"].to<JsonArray>();


    constexpr uint8_t PARAM_COUNT =
        static_cast<uint8_t>(
            SettingsManager::Param::COUNT
        );


    for (
        uint8_t i = 0;
        i < PARAM_COUNT;
        ++i
    )
    {
        const SettingsManager::Param param =
            static_cast<SettingsManager::Param>(i);


        const SettingsManager::ParamDesc& desc =
            _settings.getDesc(param);


        JsonObject item =
            array.add<JsonObject>();


        item["name"] =
            desc.name;

        item["key"] =
            desc.key;

        item["value"] =
            _settings.get(param);

        item["min"] =
            desc.minValue;

        item["max"] =
            desc.maxValue;

        item["default"] =
            desc.defaultValue;
    }


    doc["count"] =
        PARAM_COUNT;


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


// ============================================================
// SEND ERROR
// ============================================================

void WebSettingsManager::sendError(
    WebServer& server,
    int statusCode,
    const char* message
) const
{
    JsonDocument doc;

    doc["ok"] = false;

    doc["error"] =
        message;


    String response;

    serializeJson(
        doc,
        response
    );


    server.send(
        statusCode,
        "application/json",
        response
    );
}