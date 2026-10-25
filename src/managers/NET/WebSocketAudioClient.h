// #pragma once

// #include <Arduino.h>
// #include <WebSocketsClient.h>

// class WebSocketAudioClient {
// public:
//     static constexpr uint32_t SAMPLE_RATE = 16000;
//     static constexpr uint8_t CHANNELS = 1;
//     static constexpr uint8_t BITS_PER_SAMPLE = 16;

//     // 20 мс аудио: 16000 * 0.020 = 320 отсчётов.
//     static constexpr size_t SAMPLES_PER_CHUNK = 320;
//     static constexpr size_t PCM_CHUNK_SIZE =
//         SAMPLES_PER_CHUNK * sizeof(int16_t);

//     enum class Event : uint8_t {
//         CONNECTED,
//         DISCONNECTED,
//         TEXT_RECEIVED,
//         PCM_RECEIVED,
//         ERROR
//     };

//     using EventCallback = void (*)(
//         Event event,
//         const uint8_t* payload,
//         size_t length
//     );

//     WebSocketAudioClient();

//     bool begin(
//         const char* host,
//         uint16_t port,
//         const char* path = "/audio",
//         const char* deviceId = "smartclock-01"
//     );

//     void update();
//     void stop();

//     bool isConnected() const;

//     // Отправка PCM S16LE на сервер.
//     bool sendPcm(const int16_t* samples, size_t sampleCount);

//     // Отправка произвольного блока PCM-байтов.
//     bool sendPcmBytes(const uint8_t* data, size_t length);

//     // Управляющие сообщения в JSON.
//     bool sendText(const char* text);

//     // Регистрация обработчика событий.
//     void setEventCallback(EventCallback callback);

// private:
//     WebSocketsClient _webSocket;

//     EventCallback _callback;

//     bool _connected;
//     bool _started;

//     char _deviceId[48];

//     static WebSocketAudioClient* _instance;

//     static void onWebSocketEvent(
//         WStype_t type,
//         uint8_t* payload,
//         size_t length
//     );

//     void handleWebSocketEvent(
//         WStype_t type,
//         uint8_t* payload,
//         size_t length
//     );

//     void emitEvent(
//         Event event,
//         const uint8_t* payload = nullptr,
//         size_t length = 0
//     );
// };