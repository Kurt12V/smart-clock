// #include "BluetoothProtocol.h"

// bool BluetoothProtocol::parse(
//     const String& message,
//     JsonDocument& document,
//     Request& request
// )
// {
//     Serial.println("[BLE PROTOCOL] Parsing...");
//     Serial.print("[BLE PROTOCOL] Message: ");
//     Serial.println(message);

//     // ------------------------------------------------------------
//     // RESET
//     // ------------------------------------------------------------

//     document.clear();

//     request.version = 0;
//     request.id = 0;
//     request.command = "";
//     request.data = JsonObject();

//     // ------------------------------------------------------------
//     // JSON PARSE
//     // ------------------------------------------------------------

//     DeserializationError error =
//         deserializeJson(
//             document,
//             message
//         );

//     if (error)
//     {
//         Serial.print(
//             "[BLE PROTOCOL] JSON parse error: "
//         );
//         Serial.println(error.c_str());

//         return false;
//     }

//     Serial.println(
//         "[BLE PROTOCOL] JSON parsed successfully"
//     );

//     // ------------------------------------------------------------
//     // ROOT
//     // ------------------------------------------------------------

//     JsonObject root =
//         document.as<JsonObject>();

//     if (root.isNull())
//     {
//         Serial.println(
//             "[BLE PROTOCOL] Root is not object"
//         );

//         return false;
//     }

//     // ------------------------------------------------------------
//     // VERSION
//     // ------------------------------------------------------------

//     JsonVariant versionValue =
//         root["v"];

//     if (versionValue.isNull())
//     {
//         Serial.println(
//             "[BLE PROTOCOL] Missing version"
//         );

//         return false;
//     }

//     request.version =
//         versionValue.as<uint8_t>();

//     Serial.print(
//         "[BLE PROTOCOL] v = "
//     );
//     Serial.println(
//         request.version
//     );

//     if (request.version != VERSION)
//     {
//         Serial.print(
//             "[BLE PROTOCOL] Unsupported version: "
//         );
//         Serial.println(
//             request.version
//         );

//         return false;
//     }

//     // ------------------------------------------------------------
//     // ID
//     // ------------------------------------------------------------

//     JsonVariant idValue =
//         root["id"];

//     if (idValue.isNull())
//     {
//         Serial.println(
//             "[BLE PROTOCOL] Missing id"
//         );

//         return false;
//     }

//     request.id =
//         idValue.as<uint32_t>();

//     Serial.print(
//         "[BLE PROTOCOL] id = "
//     );
//     Serial.println(
//         request.id
//     );

//     // ------------------------------------------------------------
//     // COMMAND
//     // ------------------------------------------------------------

//     JsonVariant commandValue =
//         root["cmd"];

//     if (commandValue.isNull())
//     {
//         Serial.println(
//             "[BLE PROTOCOL] Missing cmd"
//         );

//         return false;
//     }

//     const char* command =
//         commandValue.as<const char*>();

//     if (
//         command == nullptr ||
//         command[0] == '\0'
//     )
//     {
//         Serial.println(
//             "[BLE PROTOCOL] Empty cmd"
//         );

//         return false;
//     }

//     request.command =
//         String(command);

//     Serial.print(
//         "[BLE PROTOCOL] cmd = "
//     );
//     Serial.println(
//         request.command
//     );

//     // ------------------------------------------------------------
//     // DATA
//     // ------------------------------------------------------------

//     JsonVariant dataValue =
//         root["data"];

//     if (
//         !dataValue.isNull() &&
//         dataValue.is<JsonObject>()
//     )
//     {
//         JsonObject data =
//             dataValue.as<JsonObject>();

//         request.data =
//             data;

//         Serial.println(
//             "[BLE PROTOCOL] data = object"
//         );

//         Serial.print(
//             "[BLE PROTOCOL] data JSON = "
//         );

//         serializeJson(
//             data,
//             Serial
//         );

//         Serial.println();
//     }
//     else
//     {
//         /*
//          * Commands such as hello/ping do not require data.
//          */
//         request.data =
//             JsonObject();

//         Serial.println(
//             "[BLE PROTOCOL] data = empty"
//         );
//     }

//     // ------------------------------------------------------------
//     // FINAL DEBUG
//     // ------------------------------------------------------------

//     Serial.println(
//         "[BLE PROTOCOL] Request parsed OK"
//     );

//     Serial.print(
//         "[BLE PROTOCOL] command = "
//     );
//     Serial.println(
//         request.command
//     );

//     if (!request.data.isNull())
//     {
//         Serial.print(
//             "[BLE PROTOCOL] request.data = "
//         );

//         serializeJson(
//             request.data,
//             Serial
//         );

//         Serial.println();
//     }

//     return true;
// }

// // ================================================================
// // RESPONSE
// // ================================================================

// String BluetoothProtocol::response(
//     uint32_t id
// )
// {
//     JsonDocument document;

//     document["v"] = VERSION;
//     document["id"] = id;
//     document["ok"] = true;

//     String result;

//     serializeJson(
//         document,
//         result
//     );

//     return result;
// }

// // ================================================================
// // RESPONSE WITH DATA
// // ================================================================

// String BluetoothProtocol::response(
//     uint32_t id,
//     JsonObjectConst data
// )
// {
//     JsonDocument document;

//     document["v"] = VERSION;
//     document["id"] = id;
//     document["ok"] = true;

//     if (!data.isNull())
//     {
//         JsonObject output =
//             document["data"]
//                 .to<JsonObject>();

//         for (
//             JsonPairConst pair :
//             data
//         )
//         {
//             output[pair.key()] =
//                 pair.value();
//         }
//     }

//     String result;

//     serializeJson(
//         document,
//         result
//     );

//     return result;
// }

// // ================================================================
// // ERROR
// // ================================================================

// String BluetoothProtocol::error(
//     uint32_t id,
//     const char* code,
//     const char* message
// )
// {
//     JsonDocument document;

//     document["v"] = VERSION;
//     document["id"] = id;
//     document["ok"] = false;

//     JsonObject errorObject =
//         document["error"]
//             .to<JsonObject>();

//     errorObject["code"] =
//         code != nullptr
//             ? code
//             : "UNKNOWN_ERROR";

//     errorObject["message"] =
//         message != nullptr
//             ? message
//             : "";

//     String result;

//     serializeJson(
//         document,
//         result
//     );

//     return result;
// }

// // ================================================================
// // PUBLISH WITHOUT DATA
// // ================================================================

// String BluetoothProtocol::publish(
//     const char* topic
// )
// {
//     JsonDocument document;

//     document["v"] = VERSION;
//     document["type"] = "publish";
//     document["topic"] =
//         topic != nullptr
//             ? topic
//             : "";

//     String result;

//     serializeJson(
//         document,
//         result
//     );

//     return result;
// }

// // ================================================================
// // PUBLISH WITH DATA
// // ================================================================

// String BluetoothProtocol::publish(
//     const char* topic,
//     JsonObjectConst data
// )
// {
//     JsonDocument document;

//     document["v"] = VERSION;
//     document["type"] = "publish";
//     document["topic"] =
//         topic != nullptr
//             ? topic
//             : "";

//     if (!data.isNull())
//     {
//         JsonObject output =
//             document["data"]
//                 .to<JsonObject>();

//         for (
//             JsonPairConst pair :
//             data
//         )
//         {
//             output[pair.key()] =
//                 pair.value();
//         }
//     }

//     String result;

//     serializeJson(
//         document,
//         result
//     );

//     return result;
// }
