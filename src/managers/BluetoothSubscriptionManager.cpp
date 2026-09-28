// #include "BluetoothSubscriptionManager.h"

// #include <string.h>

// // ============================================================
// // CONSTRUCTOR
// // ============================================================

// BluetoothSubscriptionManager::BluetoothSubscriptionManager()
//     : _count(0)
// {
//     clear();
// }

// // ============================================================
// // FIND TOPIC
// // ============================================================

// int BluetoothSubscriptionManager::findTopic(
//     const char* topic
// ) const
// {
//     if (topic == nullptr)
//         return -1;

//     for (uint8_t i = 0; i < MAX_SUBSCRIPTIONS; i++)
//     {
//         if (!_subscriptions[i].active)
//             continue;

//         if (strcmp(
//                 _subscriptions[i].topic,
//                 topic
//             ) == 0)
//         {
//             return i;
//         }
//     }

//     return -1;
// }

// // ============================================================
// // SUBSCRIBE
// // ============================================================

// bool BluetoothSubscriptionManager::subscribe(
//     const char* topic
// )
// {
//     if (topic == nullptr || topic[0] == '\0')
//         return false;

//     // Already subscribed
//     if (findTopic(topic) >= 0)
//         return true;

//     // Find free slot
//     for (uint8_t i = 0; i < MAX_SUBSCRIPTIONS; i++)
//     {
//         if (_subscriptions[i].active)
//             continue;

//         _subscriptions[i].active = true;

//         strncpy(
//             _subscriptions[i].topic,
//             topic,
//             sizeof(_subscriptions[i].topic) - 1
//         );

//         _subscriptions[i]
//             .topic[sizeof(_subscriptions[i].topic) - 1] = '\0';

//         _count++;

//         Serial.printf(
//             "[BLE SUB] Subscribed: %s\n",
//             _subscriptions[i].topic
//         );

//         return true;
//     }

//     Serial.println(
//         "[BLE SUB] ERROR: subscription limit reached"
//     );

//     return false;
// }

// // ============================================================
// // UNSUBSCRIBE
// // ============================================================

// bool BluetoothSubscriptionManager::unsubscribe(
//     const char* topic
// )
// {
//     const int index = findTopic(topic);

//     if (index < 0)
//         return false;

//     _subscriptions[index].active = false;
//     _subscriptions[index].topic[0] = '\0';

//     if (_count > 0)
//         _count--;

//     Serial.printf(
//         "[BLE SUB] Unsubscribed: %s\n",
//         topic
//     );

//     return true;
// }

// // ============================================================
// // IS SUBSCRIBED
// // ============================================================

// bool BluetoothSubscriptionManager::isSubscribed(
//     const char* topic
// ) const
// {
//     return findTopic(topic) >= 0;
// }

// // ============================================================
// // CLEAR
// // ============================================================

// void BluetoothSubscriptionManager::clear()
// {
//     for (uint8_t i = 0; i < MAX_SUBSCRIPTIONS; i++)
//     {
//         _subscriptions[i].active = false;
//         _subscriptions[i].topic[0] = '\0';
//     }

//     _count = 0;
// }

// // ============================================================
// // COUNT
// // ============================================================

// uint8_t BluetoothSubscriptionManager::count() const
// {
//     return _count;
// }

// // ============================================================
// // GET TOPIC
// // ============================================================

// bool BluetoothSubscriptionManager::getTopic(
//     uint8_t index,
//     char* buffer,
//     size_t bufferSize
// ) const
// {
//     if (buffer == nullptr || bufferSize == 0)
//         return false;

//     uint8_t current = 0;

//     for (uint8_t i = 0; i < MAX_SUBSCRIPTIONS; i++)
//     {
//         if (!_subscriptions[i].active)
//             continue;

//         if (current == index)
//         {
//             strncpy(
//                 buffer,
//                 _subscriptions[i].topic,
//                 bufferSize - 1
//             );

//             buffer[bufferSize - 1] = '\0';

//             return true;
//         }

//         current++;
//     }

//     buffer[0] = '\0';

//     return false;
// }
