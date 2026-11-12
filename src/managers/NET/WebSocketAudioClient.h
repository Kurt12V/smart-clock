#pragma once

#include <Arduino.h>
#include <WebSocketsClient.h>

#include <stdint.h>
#include <stddef.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// ============================================================
// AUDIO CONFIGURATION
// ============================================================

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 16000
#endif

#ifndef BITS_PER_SAMPLE
#define BITS_PER_SAMPLE 16
#endif

// ============================================================
// WEBSOCKET AUDIO CLIENT
// ============================================================

class WebSocketAudioClient
{
public:

    enum class Event : uint8_t
    {
        CONNECTED,
        DISCONNECTED,
        TEXT_RECEIVED,
        PCM_RECEIVED,
        ERROR
    };

    using EventCallback = void (*)(
        Event event,
        const uint8_t* payload,
        size_t length
    );

    WebSocketAudioClient();

    ~WebSocketAudioClient();

    // --------------------------------------------------------
    // INITIALIZATION
    // --------------------------------------------------------

    bool begin(
        const char* host,
        uint16_t port,
        const char* path,
        const char* deviceId
    );

    // --------------------------------------------------------
    // UPDATE
    // --------------------------------------------------------

    // Вызывать регулярно из одного контекста.
    // Только этот метод обслуживает WebSocketsClient
    // и отправляет накопленные PCM-пакеты.
    void update();

    // --------------------------------------------------------
    // STOP
    // --------------------------------------------------------

    void stop();

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    bool isConnected() const;
    bool isStarted() const;

    // --------------------------------------------------------
    // SEND PCM
    // --------------------------------------------------------

    // Копирует PCM в очередь.
    // Не выполняет сетевую отправку непосредственно.
    bool sendPcm(
        const int16_t* samples,
        size_t sampleCount
    );

    bool sendPcmBytes(
        const uint8_t* data,
        size_t length
    );

    // --------------------------------------------------------
    // SEND TEXT
    // --------------------------------------------------------

    // Текстовые управляющие сообщения отправляются напрямую.
    // Вызывать из того же контекста, что и update().
    bool sendText(const char* text);

    // --------------------------------------------------------
    // CALLBACK
    // --------------------------------------------------------

    void setEventCallback(EventCallback callback);

    // --------------------------------------------------------
    // STATISTICS
    // --------------------------------------------------------

    uint32_t getQueuedPackets() const;
    uint32_t getDroppedPackets() const;
    uint32_t getSentPackets() const;
    uint32_t getSendErrors() const;

private:

    // ========================================================
    // QUEUE CONFIGURATION
    // ========================================================

    static constexpr size_t PCM_PACKET_SIZE = 1024;
    static constexpr uint8_t PCM_QUEUE_LENGTH = 8;
    static constexpr size_t DEVICE_ID_SIZE = 48;

    struct PcmPacket
    {
        uint16_t length;
        uint8_t data[PCM_PACKET_SIZE];
    };

    // ========================================================
    // STATIC INSTANCE
    // ========================================================

    static WebSocketAudioClient* _instance;

    // ========================================================
    // WEBSOCKET
    // ========================================================

    WebSocketsClient _webSocket;

    // ========================================================
    // QUEUE
    // ========================================================

    QueueHandle_t _pcmQueue;

    // ========================================================
    // STATE
    // ========================================================

    bool _started;
    bool _connected;
    bool _helloPending;

    // ========================================================
    // DEVICE
    // ========================================================

    char _deviceId[DEVICE_ID_SIZE];

    // ========================================================
    // CALLBACK
    // ========================================================

    EventCallback _callback;

    // ========================================================
    // STATISTICS
    // ========================================================

    volatile uint32_t _droppedPackets;
    volatile uint32_t _sentPackets;
    volatile uint32_t _sendErrors;

    // ========================================================
    // WEBSOCKET EVENTS
    // ========================================================

    static void onWebSocketEvent(
        WStype_t type,
        uint8_t* payload,
        size_t length
    );

    void handleWebSocketEvent(
        WStype_t type,
        uint8_t* payload,
        size_t length
    );

    void emitEvent(
        Event event,
        const uint8_t* payload = nullptr,
        size_t length = 0
    );

    // ========================================================
    // PCM QUEUE
    // ========================================================

    bool enqueuePcm(
        const uint8_t* data,
        size_t length
    );

    bool sendNextPcmPacket();

    void clearPcmQueue();

    // ========================================================
    // HANDSHAKE
    // ========================================================

    bool sendHello();
};