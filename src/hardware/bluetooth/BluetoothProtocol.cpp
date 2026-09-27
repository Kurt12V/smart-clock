#include "BluetoothProtocol.h"

// ============================================================
// PARSE REQUEST
// ============================================================

bool BluetoothProtocol::parse(
    const String& message,
    JsonDocument& document,
    Request& request
)
{
    // --------------------------------------------------------
    // RESET
    // --------------------------------------------------------

    request.version = 0;
    request.id = 0;
    request.command = "";
    request.data = JsonObject();

    document.clear();

    // --------------------------------------------------------
    // DEBUG
    // --------------------------------------------------------

    Serial0.println(
        "[BLE PROTOCOL] Parsing..."
    );

    Serial0.print(
        "[BLE PROTOCOL] Message: "
    );

    Serial0.println(message);

    // --------------------------------------------------------
    // EMPTY
    // --------------------------------------------------------

    if (message.length() == 0)
    {
        Serial0.println(
            "[BLE PROTOCOL] ERROR: Empty message"
        );

        return false;
    }

    // --------------------------------------------------------
    // JSON
    // --------------------------------------------------------

    DeserializationError error =
        deserializeJson(
            document,
            message
        );

    if (error)
    {
        Serial0.print(
            "[BLE PROTOCOL] ERROR: JSON parse failed: "
        );

        Serial0.println(
            error.c_str()
        );

        return false;
    }

    Serial0.println(
        "[BLE PROTOCOL] JSON parsed successfully"
    );

    // --------------------------------------------------------
    // ROOT
    // --------------------------------------------------------

    JsonVariant root =
        document.as<JsonVariant>();

    if (root.isNull())
    {
        Serial0.println(
            "[BLE PROTOCOL] ERROR: Root is null"
        );

        return false;
    }

    // ========================================================
    // VERSION
    // ========================================================

    if (!root["v"].is<int>())
    {
        Serial0.println(
            "[BLE PROTOCOL] ERROR: Missing v"
        );

        return false;
    }

    int version =
        root["v"].as<int>();

    Serial0.print(
        "[BLE PROTOCOL] v = "
    );

    Serial0.println(version);

    if (version != VERSION)
    {
        Serial0.print(
            "[BLE PROTOCOL] ERROR: Unsupported version: "
        );

        Serial0.println(version);

        return false;
    }

    request.version =
        static_cast<uint8_t>(version);

    // ========================================================
    // ID
    // ========================================================

    if (root["id"].isNull())
    {
        Serial0.println(
            "[BLE PROTOCOL] ERROR: Missing id"
        );

        return false;
    }

    /*
     * Не используем:
     *
     * root["id"].is<uint32_t>()
     *
     * потому что ArduinoJson может хранить
     * обычное JSON-число как int.
     */

    long id =
        root["id"].as<long>();

    if (id < 0)
    {
        Serial0.println(
            "[BLE PROTOCOL] ERROR: Invalid id"
        );

        return false;
    }

    request.id =
        static_cast<uint32_t>(id);

    Serial0.print(
        "[BLE PROTOCOL] id = "
    );

    Serial0.println(
        request.id
    );

    // ========================================================
    // COMMAND
    // ========================================================

    JsonVariant commandValue =
        root["cmd"];

    if (commandValue.isNull())
    {
        Serial0.println(
            "[BLE PROTOCOL] ERROR: Missing cmd"
        );

        return false;
    }

    const char* command =
        commandValue.as<const char*>();

    if (command == nullptr)
    {
        Serial0.println(
            "[BLE PROTOCOL] ERROR: cmd is not string"
        );

        return false;
    }

    if (command[0] == '\0')
    {
        Serial0.println(
            "[BLE PROTOCOL] ERROR: Empty cmd"
        );

        return false;
    }

    request.command =
        String(command);

    Serial0.print(
        "[BLE PROTOCOL] cmd = "
    );

    Serial0.println(
        request.command
    );

    // ========================================================
    // DATA
    // ========================================================

    JsonVariant dataValue =
        root["data"];

    if (dataValue.is<JsonObject>())
    {
        request.data =
            dataValue.as<JsonObject>();

        Serial0.println(
            "[BLE PROTOCOL] data = object"
        );
    }
    else
    {
        /*
         * Если data отсутствует —
         * создаём пустой объект.
         */

        request.data =
            document["data"].to<JsonObject>();

        Serial0.println(
            "[BLE PROTOCOL] data = empty object"
        );
    }

    // ========================================================
    // SUCCESS
    // ========================================================

    Serial0.println(
        "[BLE PROTOCOL] Request parsed OK"
    );

    return true;
}

// ============================================================
// SIMPLE RESPONSE
// ============================================================

String BluetoothProtocol::response(
    uint32_t id
)
{
    JsonDocument document;

    document["v"] = VERSION;
    document["id"] = id;
    document["ok"] = true;

    String result;

    serializeJson(
        document,
        result
    );

    return result;
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

    document["v"] = VERSION;
    document["id"] = id;
    document["ok"] = true;

    JsonObject responseData =
        document["data"].to<JsonObject>();

    for (JsonPairConst pair : data)
    {
        responseData[pair.key()] =
            pair.value();
    }

    String result;

    serializeJson(
        document,
        result
    );

    return result;
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

    document["v"] = VERSION;
    document["id"] = id;
    document["ok"] = false;

    JsonObject errorObject =
        document["error"].to<JsonObject>();

    errorObject["code"] =
        code != nullptr
            ? code
            : "UNKNOWN_ERROR";

    errorObject["message"] =
        message != nullptr
            ? message
            : "Unknown error";

    String result;

    serializeJson(
        document,
        result
    );

    return result;
}

// ============================================================
// PUBLISH
// ============================================================

String BluetoothProtocol::publish(
    const char* topic
)
{
    JsonDocument document;

    document["v"] = VERSION;
    document["type"] = "publish";
    document["topic"] =
        topic != nullptr
            ? topic
            : "";

    document["data"].to<JsonObject>();

    String result;

    serializeJson(
        document,
        result
    );

    return result;
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

    document["v"] = VERSION;
    document["type"] = "publish";
    document["topic"] =
        topic != nullptr
            ? topic
            : "";

    JsonObject publishData =
        document["data"].to<JsonObject>();

    for (JsonPairConst pair : data)
    {
        publishData[pair.key()] =
            pair.value();
    }

    String result;

    serializeJson(
        document,
        result
    );

    return result;
}
