#include "BluetoothProtocol.h"

bool BluetoothProtocol::parse(
    const String& message,
    JsonDocument& document,
    Request& request
)
{
    DeserializationError error =
        deserializeJson(document, message);

    if (error)
        return false;

    JsonObject root = document.as<JsonObject>();

    if (root.isNull())
        return false;

    request.version =
        root["v"] | 0;

    if (request.version != VERSION)
        return false;

    request.id =
        root["id"] | 0;

    const char* command =
        root["cmd"] | nullptr;

    if (command == nullptr)
        return false;

    request.command = command;

    JsonObject data =
        root["data"].as<JsonObject>();

    if (data.isNull())
    {
        data = document["data"].to<JsonObject>();
    }

    request.data = data;

    return true;
}

String BluetoothProtocol::response(uint32_t id)
{
    JsonDocument document;

    document["v"] = VERSION;
    document["id"] = id;
    document["ok"] = true;

    String output;
    serializeJson(document, output);

    return output;
}

String BluetoothProtocol::response(
    uint32_t id,
    JsonObjectConst data
)
{
    JsonDocument document;

    document["v"] = VERSION;
    document["id"] = id;
    document["ok"] = true;

    JsonObject responseData =
        document["data"].to<JsonObject>();

    for (JsonPairConst item : data)
    {
        responseData[item.key()] = item.value();
    }

    String output;
    serializeJson(document, output);

    return output;
}

String BluetoothProtocol::error(
    uint32_t id,
    const char* code,
    const char* message
)
{
    JsonDocument document;

    document["v"] = VERSION;
    document["id"] = id;
    document["ok"] = false;

    JsonObject error =
        document["error"].to<JsonObject>();

    error["code"] = code;
    error["message"] = message;

    String output;
    serializeJson(document, output);

    return output;
}

String BluetoothProtocol::publish(
    const char* topic
)
{
    JsonDocument document;

    document["v"] = VERSION;
    document["type"] = "publish";
    document["topic"] = topic;

    String output;
    serializeJson(document, output);

    return output;
}

String BluetoothProtocol::publish(
    const char* topic,
    JsonObjectConst data
)
{
    JsonDocument document;

    document["v"] = VERSION;
    document["type"] = "publish";
    document["topic"] = topic;

    JsonObject publishData =
        document["data"].to<JsonObject>();

    for (JsonPairConst item : data)
    {
        publishData[item.key()] = item.value();
    }

    String output;
    serializeJson(document, output);

    return output;
}
