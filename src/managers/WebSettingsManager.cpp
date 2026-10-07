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
// GET /api/param?name=brightness
//
// ============================================================

void WebSettingsManager::handleGetParam(
    WebServer& server
)
{
    // --------------------------------------------------------
    // Check name
    // --------------------------------------------------------

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


    // --------------------------------------------------------
    // Resolve parameter
    // --------------------------------------------------------

    SettingsManager::Param param;

    if (!resolveParam(name, param))
    {
        sendError(
            server,
            404,
            "Unknown parameter"
        );

        return;
    }


    // --------------------------------------------------------
    // Send parameter
    // --------------------------------------------------------

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
//     "name": "brightness",
//     "value": 80
// }
//
// ============================================================

void WebSettingsManager::handleSetParam(
    WebServer& server
)
{
    // --------------------------------------------------------
    // Check body
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


    // --------------------------------------------------------
    // Parse JSON
    // --------------------------------------------------------

    JsonDocument doc;

    const DeserializationError error =
        deserializeJson(
            doc,
            server.arg("plain")
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
    // Get name
    // --------------------------------------------------------

    const char* name =
        doc["name"];

    if (name == nullptr || name[0] == '\0')
    {
        sendError(
            server,
            400,
            "Missing name"
        );

        return;
    }


    // --------------------------------------------------------
    // Resolve parameter
    // --------------------------------------------------------

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
    // Check value
    // --------------------------------------------------------

    if (!doc["value"].is<int>())
    {
        sendError(
            server,
            400,
            "Value must be integer"
        );

        return;
    }


    int value =
        doc["value"].as<int>();


    // --------------------------------------------------------
    // Get description
    // --------------------------------------------------------

    const SettingsManager::ParamDesc& desc =
        _settings.getDesc(param);


    // --------------------------------------------------------
    // Clamp
    // --------------------------------------------------------

    if (value < desc.minValue)
        value = desc.minValue;

    if (value > desc.maxValue)
        value = desc.maxValue;


    // --------------------------------------------------------
    // Set
    // --------------------------------------------------------
    //
    // SettingsManager::set() returns false both when:
    //
    // 1. the value is unchanged
    // 2. the parameter cannot be set
    //
    // Therefore false is NOT treated as HTTP 500.
    //
    // The current value is returned to the client.
    //
    // --------------------------------------------------------

    _settings.set(
        param,
        value
    );


    // --------------------------------------------------------
    // Return actual current value
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
    JsonDocument doc;

    JsonArray array =
        doc["params"].to<JsonArray>();


    // --------------------------------------------------------
    // Iterate through all valid parameters
    // --------------------------------------------------------
    //
    // IMPORTANT:
    //
    // The enum value is COUNT, not Count.
    //
    // --------------------------------------------------------

    constexpr uint8_t PARAM_COUNT =
        static_cast<uint8_t>(
            SettingsManager::Param::COUNT
        );


    for (uint8_t i = 0;
         i < PARAM_COUNT;
         ++i)
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


    // --------------------------------------------------------
    // Send response
    // --------------------------------------------------------

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
    // --------------------------------------------------------
    // IMPORTANT:
    //
    // SettingsManager::resetAll() returns void.
    //
    // It only changes values in RAM and marks them dirty.
    // SettingsManager::update() will save them after the
    // normal delayed-save interval.
    //
    // --------------------------------------------------------

    _settings.resetAll();


    // --------------------------------------------------------
    // Return all reset values
    // --------------------------------------------------------

    JsonDocument doc;

    doc["ok"] = true;


    JsonArray array =
        doc["params"].to<JsonArray>();


    constexpr uint8_t PARAM_COUNT =
        static_cast<uint8_t>(
            SettingsManager::Param::COUNT
        );


    for (uint8_t i = 0;
         i < PARAM_COUNT;
         ++i)
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
// RESOLVE PARAMETER
// ============================================================

bool WebSettingsManager::resolveParam(
    const String& name,
    SettingsManager::Param& param
) const
{
    param =
        _settings.paramFromName(
            name.c_str()
        );


    if (param ==
        SettingsManager::Param::COUNT)
    {
        return false;
    }


    return true;
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
    doc["error"] = message;


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