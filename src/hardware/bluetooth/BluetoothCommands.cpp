// #include "BluetoothCommands.h"

// #include <Arduino.h>

// #include "BluetoothTopics.h"

// // ================================================================
// // HANDLE
// // ================================================================

// bool BluetoothCommands_handle(
//     const BluetoothProtocol::Request& request,
//     BluetoothSubscriptionManager& subscriptions,
//     String& response
// )
// {
//     Serial.println(
//         "[BT COMMANDS] ------------------------------"
//     );

//     Serial.print(
//         "[BT COMMANDS] id = "
//     );
//     Serial.println(
//         request.id
//     );

//     Serial.print(
//         "[BT COMMANDS] command = "
//     );
//     Serial.println(
//         request.command
//     );

//     // ------------------------------------------------------------
//     // DEBUG DATA
//     // ------------------------------------------------------------

//     if (request.data.isNull())
//     {
//         Serial.println(
//             "[BT COMMANDS] data = NULL"
//         );
//     }
//     else
//     {
//         Serial.print(
//             "[BT COMMANDS] data = "
//         );

//         serializeJson(
//             request.data,
//             Serial
//         );

//         Serial.println();
//     }

//     // ============================================================
//     // HELLO
//     // ============================================================

//     if (
//         request.command ==
//         "hello"
//     )
//     {
//         Serial.println(
//             "[BT COMMANDS] HELLO"
//         );

//         response =
//             BluetoothProtocol::response(
//                 request.id
//             );

//         return true;
//     }

//     // ============================================================
//     // PING
//     // ============================================================

//     if (
//         request.command ==
//         "ping"
//     )
//     {
//         Serial.println(
//             "[BT COMMANDS] PING"
//         );

//         response =
//             BluetoothProtocol::response(
//                 request.id
//             );

//         return true;
//     }

//     // ============================================================
//     // SUBSCRIBE
//     // ============================================================

//     if (
//         request.command ==
//         "subscribe"
//     )
//     {
//         Serial.println(
//             "[BT COMMANDS] SUBSCRIBE"
//         );

//         // --------------------------------------------------------
//         // DATA CHECK
//         // --------------------------------------------------------

//         if (request.data.isNull())
//         {
//             Serial.println(
//                 "[BT COMMANDS] "
//                 "SUBSCRIBE data is NULL"
//             );

//             response =
//                 BluetoothProtocol::error(
//                     request.id,
//                     "INVALID_DATA",
//                     "Subscribe data is required"
//                 );

//             return true;
//         }

//         // --------------------------------------------------------
//         // TOPIC
//         // --------------------------------------------------------

//         JsonVariant topicValue =
//             request.data["topic"];

//         Serial.print(
//             "[BT COMMANDS] topicValue = "
//         );

//         if (topicValue.isNull())
//         {
//             Serial.println(
//                 "NULL"
//             );
//         }
//         else
//         {
//             serializeJson(
//                 topicValue,
//                 Serial
//             );

//             Serial.println();
//         }

//         if (topicValue.isNull())
//         {
//             Serial.println(
//                 "[BT COMMANDS] "
//                 "Topic is missing"
//             );

//             response =
//                 BluetoothProtocol::error(
//                     request.id,
//                     "INVALID_TOPIC",
//                     "Topic is required"
//                 );

//             return true;
//         }

//         const char* topic =
//             topicValue.as<const char*>();

//         if (
//             topic == nullptr ||
//             topic[0] == '\0'
//         )
//         {
//             Serial.println(
//                 "[BT COMMANDS] "
//                 "Topic is empty"
//             );

//             response =
//                 BluetoothProtocol::error(
//                     request.id,
//                     "INVALID_TOPIC",
//                     "Topic is required"
//                 );

//             return true;
//         }

//         Serial.print(
//             "[BT COMMANDS] "
//             "Subscribe topic = "
//         );

//         Serial.println(
//             topic
//         );

//         // --------------------------------------------------------
//         // SUBSCRIBE
//         // --------------------------------------------------------

//         if (
//             !subscriptions.subscribe(
//                 topic
//             )
//         )
//         {
//             Serial.print(
//                 "[BT COMMANDS] "
//                 "Failed to subscribe: "
//             );

//             Serial.println(
//                 topic
//             );

//             response =
//                 BluetoothProtocol::error(
//                     request.id,
//                     "SUBSCRIBE_FAILED",
//                     "Failed to subscribe to topic"
//                 );

//             return true;
//         }

//         Serial.print(
//             "[BT COMMANDS] "
//             "Subscribed successfully: "
//         );

//         Serial.println(
//             topic
//         );

//         response =
//             BluetoothProtocol::response(
//                 request.id
//             );

//         return true;
//     }

//     // ============================================================
//     // UNSUBSCRIBE
//     // ============================================================

//     if (
//         request.command ==
//         "unsubscribe"
//     )
//     {
//         Serial.println(
//             "[BT COMMANDS] UNSUBSCRIBE"
//         );

//         if (request.data.isNull())
//         {
//             response =
//                 BluetoothProtocol::error(
//                     request.id,
//                     "INVALID_DATA",
//                     "Unsubscribe data is required"
//                 );

//             return true;
//         }

//         JsonVariant topicValue =
//             request.data["topic"];

//         if (topicValue.isNull())
//         {
//             response =
//                 BluetoothProtocol::error(
//                     request.id,
//                     "INVALID_TOPIC",
//                     "Topic is required"
//                 );

//             return true;
//         }

//         const char* topic =
//             topicValue.as<const char*>();

//         if (
//             topic == nullptr ||
//             topic[0] == '\0'
//         )
//         {
//             response =
//                 BluetoothProtocol::error(
//                     request.id,
//                     "INVALID_TOPIC",
//                     "Topic is required"
//                 );

//             return true;
//         }

//         Serial.print(
//             "[BT COMMANDS] "
//             "Unsubscribe topic = "
//         );

//         Serial.println(
//             topic
//         );

//         if (
//             !subscriptions.unsubscribe(
//                 topic
//             )
//         )
//         {
//             Serial.print(
//                 "[BT COMMANDS] "
//                 "Topic was not subscribed: "
//             );

//             Serial.println(
//                 topic
//             );
//         }

//         response =
//             BluetoothProtocol::response(
//                 request.id
//             );

//         return true;
//     }

//     // ============================================================
//     // UNSUBSCRIBE ALL
//     // ============================================================

//     if (
//         request.command ==
//         "unsubscribe_all"
//     )
//     {
//         Serial.println(
//             "[BT COMMANDS] UNSUBSCRIBE ALL"
//         );

//         subscriptions.clear();

//         response =
//             BluetoothProtocol::response(
//                 request.id
//             );

//         return true;
//     }

//     // ============================================================
//     // GET INFO
//     // ============================================================

//     if (
//         request.command ==
//         "get_info"
//     )
//     {
//         Serial.println(
//             "[BT COMMANDS] GET INFO"
//         );

//         JsonDocument data;

//         data["device"] =
//             "SmartClock";

//         data["protocol"] =
//             BluetoothProtocol::VERSION;

//         data["name"] =
//             "ESP32-S3 Smart Clock";

//         JsonObjectConst dataConst =
//             data.as<JsonObjectConst>();

//         response =
//             BluetoothProtocol::response(
//                 request.id,
//                 dataConst
//             );

//         return true;
//     }

//     // ============================================================
//     // GET STATUS
//     // ============================================================

//     if (
//         request.command ==
//         "get_status"
//     )
//     {
//         Serial.println(
//             "[BT COMMANDS] GET STATUS"
//         );

//         JsonDocument data;

//         data["connected"] =
//             true;

//         data["subscriptions"] =
//             subscriptions.count();

//         JsonObjectConst dataConst =
//             data.as<JsonObjectConst>();

//         response =
//             BluetoothProtocol::response(
//                 request.id,
//                 dataConst
//             );

//         return true;
//     }

//     // ============================================================
//     // GET SENSORS
//     // ============================================================

//     if (
//         request.command ==
//         "get_sensors"
//     )
//     {
//         Serial.println(
//             "[BT COMMANDS] GET SENSORS"
//         );

//         /*
//          * Пока команда только подтверждается.
//          *
//          * Реальные значения датчиков публикуются
//          * BluetoothPublisher через topic=sensors.
//          */

//         response =
//             BluetoothProtocol::response(
//                 request.id
//             );

//         return true;
//     }

//     // ============================================================
//     // GET TIME
//     // ============================================================

//     if (
//         request.command ==
//         "get_time"
//     )
//     {
//         Serial.println(
//             "[BT COMMANDS] GET TIME"
//         );

//         /*
//          * Пока только ACK.
//          * Реальное содержимое clock будет публиковаться
//          * через BluetoothPublisher.
//          */

//         response =
//             BluetoothProtocol::response(
//                 request.id
//             );

//         return true;
//     }

//     // ============================================================
//     // NOT IMPLEMENTED
//     // ============================================================

//     const char* knownCommands[] =
//     {
//         "set_time",
//         "set_datetime",
//         "set_timezone",

//         "alarm_list",
//         "alarm_create",
//         "alarm_set",
//         "alarm_enable",
//         "alarm_delete",
//         "alarm_stop",

//         "light_get",
//         "matrix_set",
//         "cob_set",
//         "cob_set_all",

//         "sound_get",
//         "sound_volume",
//         "sound_mute",
//         "sound_play",

//         "timer_start",
//         "timer_pause",
//         "timer_resume",
//         "timer_stop",

//         "stopwatch_start",
//         "stopwatch_stop",
//         "stopwatch_reset",

//         "display_brightness",
//         "clock_format"
//     };

//     for (
//         const char* known :
//         knownCommands
//     )
//     {
//         if (
//             request.command ==
//             known
//         )
//         {
//             Serial.print(
//                 "[BT COMMANDS] "
//                 "NOT IMPLEMENTED: "
//             );

//             Serial.println(
//                 request.command
//             );

//             response =
//                 BluetoothProtocol::error(
//                     request.id,
//                     "NOT_IMPLEMENTED",
//                     "Command is not implemented yet"
//                 );

//             return true;
//         }
//     }

//     // ============================================================
//     // UNKNOWN COMMAND
//     // ============================================================

//     Serial.print(
//         "[BT COMMANDS] "
//         "UNKNOWN COMMAND: "
//     );

//     Serial.println(
//         request.command
//     );

//     response =
//         BluetoothProtocol::error(
//             request.id,
//             "UNKNOWN_COMMAND",
//             "Unknown command"
//         );

//     return true;
// }
