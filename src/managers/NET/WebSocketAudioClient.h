
#pragma once

#include <Arduino.h>
#include <WebSocketsClient.h>
#include <stdint.h>
#include <stddef.h>

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

class WebSocketAudioClient {

public:

    // --------------------------------------------------------
    // EVENTS
    // --------------------------------------------------------

    enum class Event : uint8_t {

        CONNECTED,
        DISCONNECTED,

        TEXT_RECEIVED,
        PCM_RECEIVED,

        ERROR

    };


    // --------------------------------------------------------
    // CALLBACK
    // --------------------------------------------------------

    using EventCallback = void (*)(
        Event event,
        const uint8_t* payload,
        size_t length
    );


    // --------------------------------------------------------
    // CONSTRUCTOR
    // --------------------------------------------------------

    WebSocketAudioClient();


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

    void update();


    // --------------------------------------------------------
    // STOP
    // --------------------------------------------------------

    void stop();


    // --------------------------------------------------------
    // CONNECTION STATE
    // --------------------------------------------------------

    bool isConnected() const;

    bool isStarted() const;


    // --------------------------------------------------------
    // SEND PCM SAMPLES
    //
    // samples     - массив int16_t
    // sampleCount - количество отсчётов
    // --------------------------------------------------------

    bool sendPcm(
        const int16_t* samples,
        size_t sampleCount
    );


    // --------------------------------------------------------
    // SEND PCM BYTES
    //
    // Автоматическое разбиение на WebSocket BIN.
    // --------------------------------------------------------

    bool sendPcmBytes(
        const uint8_t* data,
        size_t length
    );


    // --------------------------------------------------------
    // SEND TEXT
    // --------------------------------------------------------

    bool sendText(
        const char* text
    );


    // --------------------------------------------------------
    // EVENT CALLBACK
    // --------------------------------------------------------

    void setEventCallback(
        EventCallback callback
    );


private:

    // --------------------------------------------------------
    // STATIC INSTANCE
    // --------------------------------------------------------

    static WebSocketAudioClient* _instance;


    // --------------------------------------------------------
    // WEBSOCKET
    // --------------------------------------------------------

    WebSocketsClient _webSocket;


    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    bool _connected;
    bool _started;


    // --------------------------------------------------------
    // DEVICE ID
    // --------------------------------------------------------

    static constexpr size_t DEVICE_ID_SIZE = 48;

    char _deviceId[DEVICE_ID_SIZE];


    // --------------------------------------------------------
    // CALLBACK
    // --------------------------------------------------------

    EventCallback _callback;


    // --------------------------------------------------------
    // STATIC WEBSOCKET CALLBACK
    // --------------------------------------------------------

    static void onWebSocketEvent(
        WStype_t type,
        uint8_t* payload,
        size_t length
    );


    // --------------------------------------------------------
    // HANDLE EVENTS
    // --------------------------------------------------------

    void handleWebSocketEvent(
        WStype_t type,
        uint8_t* payload,
        size_t length
    );


    // --------------------------------------------------------
    // EMIT EVENT
    // --------------------------------------------------------

    void emitEvent(
        Event event,
        const uint8_t* payload = nullptr,
        size_t length = 0
    );


    // --------------------------------------------------------
    // SEND HELLO
    // --------------------------------------------------------

    bool sendHello();

};
