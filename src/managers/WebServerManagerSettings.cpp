#include "WebServerManager.h"

// ============================================================
// GET PARAM
// ============================================================

void WebServerManager::handleGetParam()
{
    if (!_settings)
    {
        sendError(
            503,
            "SettingsManager unavailable"
        );

        return;
    }

    String name;

    if (_server.hasArg("name"))
        name =
            _server.arg("name");
    else if (_server.hasArg("param"))
        name =
            _server.arg("param");

    name.trim();

    if (name.isEmpty())
    {
        sendError(
            400,
            "Parameter name is required"
        );

        return;
    }

    const Param param =
        _settings->paramFromName(
            name.c_str()
        );

    if (param == Param::COUNT)
    {
        sendError(
            404,
            "Unknown parameter"
        );

        return;
    }

    const SettingsManager::ParamDesc& desc =
        _settings->getDesc(param);

    const int value =
        _settings->get(param);

    Serial0.printf(
        "[WEB][PARAM] %s value=%d range=%d..%d default=%d\n",
        name.c_str(),
        value,
        desc.minValue,
        desc.maxValue,
        desc.defaultValue
    );

    JsonDocument doc;

    doc["name"] =
        name;

    doc["value"] =
        value;

    doc["min"] =
        desc.minValue;

    doc["max"] =
        desc.maxValue;

    doc["default"] =
        desc.defaultValue;

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
// SET PARAM
// ============================================================

void WebServerManager::handleSetParam()
{
    if (!_settings)
    {
        sendError(
            503,
            "SettingsManager unavailable"
        );

        return;
    }

    JsonDocument doc;

    if (!parseJson(doc))
        return;

    JsonObjectConst object =
        doc.as<JsonObjectConst>();

    String name;

    if (object["name"].is<const char*>())
    {
        name =
            object["name"].as<const char*>();
    }
    else if (
        object["param"].is<const char*>())
    {
        name =
            object["param"].as<const char*>();
    }

    name.trim();

    if (name.isEmpty())
    {
        sendError(
            400,
            "Parameter name is required"
        );

        return;
    }

    if (!object["value"].is<int>() &&
        !object["value"].is<long>() &&
        !object["value"].is<unsigned>() &&
        !object["value"].is<unsigned long>())
    {
        sendError(
            400,
            "Parameter value must be numeric"
        );

        return;
    }

    const Param param =
        _settings->paramFromName(
            name.c_str()
        );

    if (param == Param::COUNT)
    {
        sendError(
            404,
            "Unknown parameter"
        );

        return;
    }

    const SettingsManager::ParamDesc& desc =
        _settings->getDesc(param);

    const int oldValue =
        _settings->get(param);

    const int requestedValue =
        object["value"].as<int>();

    const int value =
        constrain(
            requestedValue,
            desc.minValue,
            desc.maxValue
        );

    Serial0.printf(
        "[WEB][PARAM] name=%s old=%d requested=%d value=%d range=%d..%d\n",
        name.c_str(),
        oldValue,
        requestedValue,
        value,
        desc.minValue,
        desc.maxValue
    );

    if (!_settings->set(
            param,
            value))
    {
        sendError(
            500,
            "Failed to set parameter"
        );

        return;
    }

    const int newValue =
        _settings->get(param);

    JsonDocument response;

    response["name"] =
        name;

    response["old"] =
        oldValue;

    response["requested"] =
        requestedValue;

    response["value"] =
        newValue;

    response["changed"] =
        newValue != oldValue;

    response["min"] =
        desc.minValue;

    response["max"] =
        desc.maxValue;

    response["default"] =
        desc.defaultValue;

    String output;

    serializeJson(
        response,
        output
    );

    sendJson(
        200,
        output
    );
}

// ============================================================
// GET ALL PARAMS
// ============================================================

void WebServerManager::handleGetAllParams()
{
    if (!_settings)
    {
        sendError(
            503,
            "SettingsManager unavailable"
        );

        return;
    }

    JsonDocument doc;

    JsonArray array =
        doc["params"].to<JsonArray>();

    for (
        uint8_t i = 0;
        i < static_cast<uint8_t>(
                Param::COUNT);
        ++i)
    {
        const Param param =
            static_cast<Param>(i);

        const SettingsManager::ParamDesc& desc =
            _settings->getDesc(param);

        JsonObject item =
            array.add<JsonObject>();

        item["name"] =
            desc.name;

        item["key"] =
            desc.key;

        item["value"] =
            _settings->get(param);

        item["min"] =
            desc.minValue;

        item["max"] =
            desc.maxValue;

        item["default"] =
            desc.defaultValue;
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

// ============================================================
// RESET
// ============================================================

void WebServerManager::handleReset()
{
    if (!_settings)
    {
        sendError(
            503,
            "SettingsManager unavailable"
        );

        return;
    }

    Serial0.println(
        "[WEB][RESET] resetAll()"
    );

    _settings->resetAll();

    Serial0.println(
        "[WEB][RESET] completed"
    );

    sendOk();
}