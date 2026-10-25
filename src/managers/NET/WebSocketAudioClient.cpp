// #include "WebSocketAudioClient.h"

// #include <cstring>

// // ============================================================
// // STATIC INSTANCE
// // ============================================================

// WebSocketAudioClient* WebSocketAudioClient::_instance = nullptr;


// // ============================================================
// // CONSTRUCTOR
// // ============================================================

// WebSocketAudioClient::WebSocketAudioClient()
//     : _callback(nullptr),
//       _connected(false),
//       _started(false) {

//     memset(_deviceId, 0, sizeof(_deviceId));
// }


// // ============================================================
// // BEGIN
// // ============================================================

// bool WebSocketAudioClient::begin(
//     const char* host,
//     uint16_t port,
//     const char* path,
//     const char* deviceId
// ) {
//     if (host == nullptr || path == nullptr || deviceId == nullptr) {
//         Serial0.println("[WS-AUDIO] ERROR: Invalid configuration");
//         return false;
//     }

//     if (strlen(deviceId) >= sizeof(_deviceId)) {
//         Serial0.println("[WS-AUDIO] ERROR: Device ID is too long");
//         return false;
//     }

//     strncpy(_deviceId, deviceId, sizeof(_deviceId) - 1);

//     _instance = this;

//     Serial0.println();
//     Serial0.println("[WS-AUDIO] Initializing WebSocket");
//     Serial0.printf("[WS-AUDIO] Host: %s\n", host);
//     Serial0.printf("[WS-AUDIO] Port: %u\n", port);
//     Serial0.printf("[WS-AUDIO] Path: %s\n", path);
//     Serial0.printf("[WS-AUDIO] Device: %s\n", _deviceId);
//     Serial0.printf(
//         "[WS-AUDIO] PCM: %lu Hz, %u-bit, mono\n",
//         static_cast<unsigned long>(SAMPLE_RATE),
//         BITS_PER_SAMPLE
//     );

//     // Обычный незашифрованный WebSocket (ws://).
//     _webSocket.begin(host, port, path);

//     _webSocket.onEvent(
//         WebSocketAudioClient::onWebSocketEvent
//     );

//     // Автоматическое повторное подключение.
//     _webSocket.setReconnectInterval(5000);

//     // Heartbeat для обнаружения потерянного соединения.
//     _webSocket.enableHeartbeat(15000, 3000, 2);

//     _started = true;

//     Serial0.println("[WS-AUDIO] Client initialized");

//     return true;
// }


// // ============================================================
// // UPDATE
// // Вызывать регулярно из App::update().
// // ============================================================

// void WebSocketAudioClient::update() {
//     if (!_started) {
//         return;
//     }

//     _webSocket.loop();
// }


// // ============================================================
// // STOP
// // ============================================================

// void WebSocketAudioClient::stop() {
//     if (!_started) {
//         return;
//     }

//     Serial0.println("[WS-AUDIO] Stopping WebSocket");

//     _webSocket.disconnect();

//     _connected = false;
//     _started = false;

//     if (_instance == this) {
//         _instance = nullptr;
//     }
// }


// // ============================================================
// // CONNECTION STATE
// // ============================================================

// bool WebSocketAudioClient::isConnected() const {
//     return _connected;
// }


// // ============================================================
// // SEND PCM SAMPLES
// // samples: массив int16_t
// // sampleCount: число отсчётов, а не число байтов
// // ============================================================

// bool WebSocketAudioClient::sendPcm(
//     const int16_t* samples,
//     size_t sampleCount
// ) {
//     if (samples == nullptr || sampleCount == 0) {
//         return false;
//     }

//     return sendPcmBytes(
//         reinterpret_cast<const uint8_t*>(samples),
//         sampleCount * sizeof(int16_t)
//     );
// }


// // ============================================================
// // SEND PCM BYTES
// // Отправляет бинарное WebSocket-сообщение.
// // ============================================================

// bool WebSocketAudioClient::sendPcmBytes(
//     const uint8_t* data,
//     size_t length
// ) {
//     if (!_started || !_connected) {
//         return false;
//     }

//     if (data == nullptr || length == 0) {
//         return false;
//     }

//     // Ограничиваем размер одного сообщения.
//     // При необходимости лимит можно увеличить.
//     constexpr size_t MAX_PCM_MESSAGE_SIZE = 4096;

//     if (length > MAX_PCM_MESSAGE_SIZE) {
//         Serial0.printf(
//             "[WS-AUDIO] ERROR: PCM message too large: %u bytes\n",
//             static_cast<unsigned int>(length)
//         );

//         return false;
//     }

//     const bool result = _webSocket.sendBIN(
//         const_cast<uint8_t*>(data),
//         length
//     );

//     if (!result) {
//         Serial0.println("[WS-AUDIO] ERROR: PCM send failed");
//         return false;
//     }

//     return true;
// }


// // ============================================================
// // SEND TEXT
// // ============================================================

// bool WebSocketAudioClient::sendText(const char* text) {
//     if (!_started || !_connected || text == nullptr) {
//         return false;
//     }

//     const bool result = _webSocket.sendTXT(
//         const_cast<char*>(text)
//     );

//     if (!result) {
//         Serial0.println("[WS-AUDIO] ERROR: Text send failed");
//     }

//     return result;
// }


// // ============================================================
// // EVENT CALLBACK REGISTRATION
// // ============================================================

// void WebSocketAudioClient::setEventCallback(
//     EventCallback callback
// ) {
//     _callback = callback;
// }


// // ============================================================
// // STATIC WEBSOCKET CALLBACK
// // ============================================================

// void WebSocketAudioClient::onWebSocketEvent(
//     WStype_t type,
//     uint8_t* payload,
//     size_t length
// ) {
//     if (_instance == nullptr) {
//         return;
//     }

//     _instance->handleWebSocketEvent(
//         type,
//         payload,
//         length
//     );
// }


// // ============================================================
// // HANDLE WEBSOCKET EVENTS
// // ============================================================

// void WebSocketAudioClient::handleWebSocketEvent(
//     WStype_t type,
//     uint8_t* payload,
//     size_t length
// ) {
//     switch (type) {

//         case WStype_CONNECTED: {
//             _connected = true;

//             Serial0.println("[WS-AUDIO] Connected to server");

//             char message[128];

//             snprintf(
//                 message,
//                 sizeof(message),
//                 "{\"type\":\"audio_hello\","
//                 "\"device_id\":\"%s\","
//                 "\"sample_rate\":16000,"
//                 "\"channels\":1,"
//                 "\"format\":\"pcm_s16le\"}",
//                 _deviceId
//             );

//             _webSocket.sendTXT(message);

//             emitEvent(Event::CONNECTED);

//             break;
//         }

//         case WStype_DISCONNECTED: {
//             _connected = false;

//             Serial0.println("[WS-AUDIO] Disconnected");

//             emitEvent(Event::DISCONNECTED);

//             break;
//         }

//         case WStype_TEXT: {
//             Serial0.printf(
//                 "[WS-AUDIO] Text received: %.*s\n",
//                 static_cast<int>(length),
//                 reinterpret_cast<const char*>(payload)
//             );

//             emitEvent(
//                 Event::TEXT_RECEIVED,
//                 payload,
//                 length
//             );

//             break;
//         }

//         case WStype_BIN: {
//             // Не выводим каждый аудиофрагмент в Serial0:
//             // это может создавать лишнюю нагрузку.
//             emitEvent(
//                 Event::PCM_RECEIVED,
//                 payload,
//                 length
//             );

//             break;
//         }

//         case WStype_ERROR: {
//             Serial0.println("[WS-AUDIO] WebSocket error");

//             emitEvent(
//                 Event::ERROR,
//                 payload,
//                 length
//             );

//             break;
//         }

//         case WStype_PING:
//         case WStype_PONG:
//             // Heartbeat обрабатывается библиотекой.
//             break;

//         default:
//             break;
//     }
// }


// // ============================================================
// // EMIT EVENT
// // ============================================================

// void WebSocketAudioClient::emitEvent(
//     Event event,
//     const uint8_t* payload,
//     size_t length
// ) {
//     if (_callback != nullptr) {
//         _callback(event, payload, length);
//     }
// }