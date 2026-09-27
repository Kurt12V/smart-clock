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
    // RESET REQUEST
    // --------------------------------------------------------

    request.version = 0;
    request.id = 0;
    request.command = "";
    request.data = JsonObject();

    document.clear();

    Serial.println();
    Serial.println("[BLE PROTOCOL] Parsing request:");
    Serial.println(message);

    // --------------------------------------------------------
    // EMPTY MESSAGE
    // --------------------------------------------------------

    if (message.isEmpty())
    {
        Serial.println(
            "[BLE PROTOCOL] Empty message"
        );

        return false;
    }

    // --------------------------------------------------------
    // DESERIALIZE JSON
    // --------------------------------------------------------

    DeserializationError error =
        deserializeJson(
            document,
            message
        );

    if (error)
    {
        Serial.print(
            "[BLE PROTOCOL] JSON error: "
        );

        Serial.println(
            error.c_str()
        );

        return false;
    }

    // --------------------------------------------------------
    // ROOT OBJECT
    // --------------------------------------------------------

    if (!document.is<JsonObject>())
    {
        Serial.println(
            "[BLE PROTOCOL] Root is not object"
        );

        return false;
    }

    JsonObject root =
        document.as<JsonObject>();

    // --------------------------------------------------------
    // DEBUG JSON
    // --------------------------------------------------------

    Serial.println(
        "[BLE PROTOCOL] JSON parsed"
    );

    serializeJson(
        root,
        Serial
    );

    Serial.println();

    // ========================================================
    // VERSION
    // ========================================================

    if (!root["v"].is<uint8_t>())
    {
        Serial.println(
            "[BLE PROTOCOL] Missing or invalid version"
        );

        return false;
    }

    uint8_t version =
        root["v"].as<uint8_t>();

    if (version != VERSION)
    {
        Serial.print(
            "[BLE PROTOCOL] Unsupported version: "
        );

        Serial.println(
            version
        );

        return false;
    }

    request.version = version;

    // ========================================================
    // ID
    // ========================================================

    if (!root["id"].is<uint32_t>())
    {
        Serial.println(
            "[BLE PROTOCOL] Missing or invalid id"
        );

        return false;
    }

    request.id =
        root["id"].as<uint32_t>();

    Serial.print(
        "[BLE PROTOCOL] ID: "
    );

    Serial.println(
        request.id
    );

    // ========================================================
    // COMMAND
    // ========================================================

    /*
     * ВАЖНО:
     *
     * Не используем:
     *
     * root["cmd"].is<const char*>()
     *
     * потому что в некоторых версиях ArduinoJson
     * такая проверка может вести себя не так,
     * как ожидается.
     *
     * Вместо этого просто получаем строку.
     */

    const char* command =
        root["cmd"] | nullptr;

    if (command == nullptr)
    {
        Serial.println(
            "[BLE PROTOCOL] Missing cmd"
        );

        return false;
    }

    if (command[0] == '\0')
    {
        Serial.println(
            "[BLE PROTOCOL] Empty cmd"
        );

        return false;
    }

    request.command =
        String(command);

    Serial.print(
        "[BLE PROTOCOL] Command: "
    );

    Serial.println(
        request.command
    );

    // ========================================================
    // DATA
    // ========================================================

    if (root["data"].is<JsonObject>())
    {
        request.data =
            root["data"].as<JsonObject>();

        Serial.println(
            "[BLE PROTOCOL] Data object found"
        );
    }
    else
    {
        /*
         * data необязателен.
         *
         * Создаём пустой объект внутри
         * document, чтобы обработчики могли
         * безопасно обращаться к request.data.
         */

        request.data =
            document["data"].to<JsonObject>();

        Serial.println(
            "[BLE PROTOCOL] Data object created"
        );
    }

    // ========================================================
    // SUCCESS
    // ========================================================

    Serial.println(
        "[BLE PROTOCOL] Request OK"
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

    for (
        JsonPairConst pair : data
    )
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
// ERROR RESPONSE
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
// PUBLISH WITHOUT DATA
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

    for (
        JsonPairConst pair : data
    )
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
