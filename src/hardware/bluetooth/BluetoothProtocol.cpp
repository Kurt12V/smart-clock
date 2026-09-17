#include "BluetoothProtocol.h"

// ============================================================
// PARSE
// ============================================================

bool BluetoothProtocol::parse(
    const String& message,
    JsonDocument& document,
    Request& request
)
{
    request =
        Request{};

    document.clear();

    if (message.isEmpty())
    {
        return false;
    }

    // --------------------------------------------------------
    // DESERIALIZE
    // --------------------------------------------------------

    DeserializationError error =
        deserializeJson(
            document,
            message
        );

    if (error)
    {
        Serial0.print(
            "[BLE PROTOCOL] JSON error: "
        );

        Serial0.println(
            error.c_str()
        );

        return false;
    }

    // --------------------------------------------------------
    // ROOT
    // --------------------------------------------------------

    JsonObject root =
        document.as<JsonObject>();

    if (root.isNull())
    {
        return false;
    }

    // --------------------------------------------------------
    // VERSION
    // --------------------------------------------------------

    request.version =
        root["v"] | 0;

    if (request.version != VERSION)
    {
        Serial0.println(
            "[BLE PROTOCOL] Invalid version"
        );

        return false;
    }

    // --------------------------------------------------------
    // ID
    // --------------------------------------------------------

    if (!root["id"].is<uint32_t>() &&
        !root["id"].is<int>())
    {
        Serial0.println(
            "[BLE PROTOCOL] Missing id"
        );

        return false;
    }

    request.id =
        root["id"].as<uint32_t>();

    // --------------------------------------------------------
    // COMMAND
    // --------------------------------------------------------

    const char* command =
        root["cmd"] | nullptr;

    if (command == nullptr)
    {
        Serial0.println(
            "[BLE PROTOCOL] Missing cmd"
        );

        return false;
    }

    if (command[0] == '\0')
    {
        Serial0.println(
            "[BLE PROTOCOL] Empty cmd"
        );

        return false;
    }

    request.command =
        command;

    // --------------------------------------------------------
    // DATA
    // --------------------------------------------------------

    JsonObject data =
        root["data"].as<JsonObject>();

    if (data.isNull())
    {
        Serial0.println(
            "[BLE PROTOCOL] Missing data object"
        );

        return false;
    }

    request.data =
        data;

    return true;
}

// ============================================================
// RESPONSE
// ============================================================

String BluetoothProtocol::response(
    uint32_t id
)
{
    JsonDocument document;

    document["v"] =
        VERSION;

    document["id"] =
        id;

    document["ok"] =
        true;

    String output;

    serializeJson(
        document,
        output
    );

    return output;
}

// ============================================================
// RESPONSE WITH DATA
// ============================================================

String BluetoothProtocol::response(
    uint32_t id,
    JsonObjectConst data
)
{
    JsonDocument document;

    document["v"] =
        VERSION;

    document["id"] =
        id;

    document["ok"] =
        true;

    JsonObject responseData =
        document["data"].to<JsonObject>();

    for (JsonPairConst item : data)
    {
        responseData[item.key()] =
            item.value();
    }

    String output;

    serializeJson(
        document,
        output
    );

    return output;
}

// ============================================================
// ERROR
// ============================================================

String BluetoothProtocol::error(
    uint32_t id,
    const char* code,
    const char* message
)
{
    JsonDocument document;

    document["v"] =
        VERSION;

    document["id"] =
        id;

    document["ok"] =
        false;

    JsonObject error =
        document["error"].to<JsonObject>();

    error["code"] =
        code;

    error["message"] =
        message;

    String output;

    serializeJson(
        document,
        output
    );

    return output;
}

// ============================================================
// PUBLISH
// ============================================================

String BluetoothProtocol::publish(
    const char* topic
)
{
    JsonDocument document;

    document["v"] =
        VERSION;

    document["type"] =
        "publish";

    document["topic"] =
        topic;

    String output;

    serializeJson(
        document,
        output
    );

    return output;
}

// ============================================================
// PUBLISH WITH DATA
// ============================================================

String BluetoothProtocol::publish(
    const char* topic,
    JsonObjectConst data
)
{
    JsonDocument document;

    document["v"] =
        VERSION;

    document["type"] =
        "publish";

    document["topic"] =
        topic;

    JsonObject publishData =
        document["data"].to<JsonObject>();

    for (JsonPairConst item : data)
    {
        publishData[item.key()] =
            item.value();
    }

    String output;

    serializeJson(
        document,
        output
    );

    return output;
}