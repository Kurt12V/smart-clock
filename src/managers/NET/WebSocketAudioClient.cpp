
#include "WebSocketAudioClient.h"

#include <Arduino.h>
#include <cstring>
#include <cstdio>

// ============================================================
// CONFIGURATION
// ============================================================

namespace WebSocketAudioConfig {

    constexpr size_t PCM_CHUNK_SIZE = 1280;

    constexpr uint32_t RECONNECT_INTERVAL = 5000;

    constexpr uint32_t HEARTBEAT_INTERVAL = 15000;
    constexpr uint32_t HEARTBEAT_TIMEOUT = 3000;

    constexpr uint8_t HEARTBEAT_FAIL_LIMIT = 2;

}


// ============================================================
// STATIC INSTANCE
// ============================================================

WebSocketAudioClient*
WebSocketAudioClient::_instance = nullptr;


// ============================================================
// CONSTRUCTOR
// ============================================================

WebSocketAudioClient::WebSocketAudioClient()
    : _connected(false),
      _started(false),
      _callback(nullptr) {

    memset(
        _deviceId,
        0,
        sizeof(_deviceId)
    );
}


// ============================================================
// BEGIN
// ============================================================

bool WebSocketAudioClient::begin(
    const char* host,
    uint16_t port,
    const char* path,
    const char* deviceId
) {

    if (
        host == nullptr ||
        path == nullptr ||
        deviceId == nullptr
    ) {

        Serial0.println(
            "[WS-AUDIO] ERROR: Invalid configuration"
        );

        return false;
    }

    if (
        host[0] == '\0' ||
        path[0] == '\0' ||
        deviceId[0] == '\0'
    ) {

        Serial0.println(
            "[WS-AUDIO] ERROR: Empty configuration"
        );

        return false;
    }

    if (
        strlen(deviceId) >= sizeof(_deviceId)
    ) {

        Serial0.println(
            "[WS-AUDIO] ERROR: Device ID too long"
        );

        return false;
    }

    if (_started) {
        stop();
    }

    if (
        _instance != nullptr &&
        _instance != this
    ) {

        Serial0.println(
            "[WS-AUDIO] ERROR: Another client instance is active"
        );

        return false;
    }

    memset(
        _deviceId,
        0,
        sizeof(_deviceId)
    );

    memcpy(
        _deviceId,
        deviceId,
        strlen(deviceId)
    );

    _instance = this;

    Serial0.println();
    Serial0.println(
        "[WS-AUDIO] Initializing WebSocket"
    );

    Serial0.printf(
        "[WS-AUDIO] Host: %s\n",
        host
    );

    Serial0.printf(
        "[WS-AUDIO] Port: %u\n",
        port
    );

    Serial0.printf(
        "[WS-AUDIO] Path: %s\n",
        path
    );

    Serial0.printf(
        "[WS-AUDIO] Device: %s\n",
        _deviceId
    );

    Serial0.printf(
        "[WS-AUDIO] PCM: %u Hz, %u-bit, mono\n",
        static_cast<unsigned int>(SAMPLE_RATE),
        static_cast<unsigned int>(BITS_PER_SAMPLE)
    );

    Serial0.printf(
        "[WS-AUDIO] Chunk size: %u bytes\n",
        static_cast<unsigned int>(
            WebSocketAudioConfig::PCM_CHUNK_SIZE
        )
    );

    // WebSocket connection.
    _webSocket.begin(
        host,
        port,
        path
    );

    _webSocket.onEvent(
        WebSocketAudioClient::onWebSocketEvent
    );

    // Automatic reconnection.
    _webSocket.setReconnectInterval(
        WebSocketAudioConfig::RECONNECT_INTERVAL
    );

    // Heartbeat.
    _webSocket.enableHeartbeat(
        WebSocketAudioConfig::HEARTBEAT_INTERVAL,
        WebSocketAudioConfig::HEARTBEAT_TIMEOUT,
        WebSocketAudioConfig::HEARTBEAT_FAIL_LIMIT
    );

    _connected = false;
    _started = true;

    Serial0.println(
        "[WS-AUDIO] Client initialized"
    );

    return true;
}


// ============================================================
// UPDATE
// ============================================================

void WebSocketAudioClient::update() {

    if (!_started) {
        return;
    }

    _webSocket.loop();
}


// ============================================================
// STOP
// ============================================================

void WebSocketAudioClient::stop() {

    if (!_started) {
        return;
    }

    Serial0.println(
        "[WS-AUDIO] Stopping WebSocket"
    );

    _connected = false;
    _started = false;

    _webSocket.disconnect();

    if (_instance == this) {
        _instance = nullptr;
    }

    Serial0.println(
        "[WS-AUDIO] Client stopped"
    );
}


// ============================================================
// CONNECTION STATE
// ============================================================

bool WebSocketAudioClient::isConnected() const {

    return _started && _connected;
}


bool WebSocketAudioClient::isStarted() const {

    return _started;
}


// ============================================================
// SEND PCM SAMPLES
// ============================================================

bool WebSocketAudioClient::sendPcm(
    const int16_t* samples,
    size_t sampleCount
) {

    if (
        samples == nullptr ||
        sampleCount == 0
    ) {
        return false;
    }

    if (
        sampleCount > SIZE_MAX / sizeof(int16_t)
    ) {

        Serial0.println(
            "[WS-AUDIO] ERROR: PCM sample count overflow"
        );

        return false;
    }

    return sendPcmBytes(
        reinterpret_cast<const uint8_t*>(samples),
        sampleCount * sizeof(int16_t)
    );
}


// ============================================================
// SEND PCM BYTES
//
// 1280 bytes = 40 ms at 16 kHz / 16-bit / mono.
//
// Every chunk is a separate WebSocket BIN message.
// ============================================================

bool WebSocketAudioClient::sendPcmBytes(
    const uint8_t* data,
    size_t length
) {

    if (!_started || !_connected) {
        return false;
    }

    if (
        data == nullptr ||
        length == 0
    ) {
        return false;
    }

    if (length % sizeof(int16_t) != 0) {

        Serial0.println(
            "[WS-AUDIO] ERROR: Invalid PCM length"
        );

        return false;
    }

    constexpr size_t CHUNK_SIZE =
        WebSocketAudioConfig::PCM_CHUNK_SIZE;

    size_t offset = 0;

    const size_t totalChunks =
        (length + CHUNK_SIZE - 1) / CHUNK_SIZE;

    size_t chunkIndex = 0;

    while (offset < length) {

        if (!_started || !_connected) {

            Serial0.println(
                "[WS-AUDIO] ERROR: Connection lost during transmission"
            );

            return false;
        }

        const size_t remaining =
            length - offset;

        size_t chunkLength =
            remaining > CHUNK_SIZE
                ? CHUNK_SIZE
                : remaining;

        // PCM S16LE: even number of bytes.
        chunkLength -= chunkLength % 2;

        if (chunkLength == 0) {
            return false;
        }

        const bool result = _webSocket.sendBIN(
            const_cast<uint8_t*>(data + offset),
            chunkLength
        );

        if (!result) {

            Serial0.printf(
                "[WS-AUDIO] ERROR: Chunk %u/%u failed\n",
                static_cast<unsigned int>(chunkIndex + 1),
                static_cast<unsigned int>(totalChunks)
            );

            return false;
        }

        offset += chunkLength;
        chunkIndex++;
    }

    return true;
}


// ============================================================
// SEND TEXT
// ============================================================

bool WebSocketAudioClient::sendText(
    const char* text
) {

    if (
        !_started ||
        !_connected ||
        text == nullptr ||
        text[0] == '\0'
    ) {
        return false;
    }

    const bool result = _webSocket.sendTXT(
        const_cast<char*>(text)
    );

    if (!result) {

        Serial0.println(
            "[WS-AUDIO] ERROR: Text send failed"
        );
    }

    return result;
}


// ============================================================
// EVENT CALLBACK REGISTRATION
// ============================================================

void WebSocketAudioClient::setEventCallback(
    EventCallback callback
) {

    _callback = callback;
}


// ============================================================
// SEND HELLO
// ============================================================

bool WebSocketAudioClient::sendHello() {

    if (!_started || !_connected) {
        return false;
    }

    char message[256];

    const int messageLength = snprintf(
        message,
        sizeof(message),

        "{"
            "\"type\":\"audio_hello\","
            "\"device_id\":\"%s\","
            "\"sample_rate\":%u,"
            "\"channels\":1,"
            "\"bits_per_sample\":%u,"
            "\"format\":\"pcm_s16le\""
        "}",

        _deviceId,

        static_cast<unsigned int>(SAMPLE_RATE),

        static_cast<unsigned int>(BITS_PER_SAMPLE)
    );

    if (
        messageLength <= 0 ||
        static_cast<size_t>(messageLength) >= sizeof(message)
    ) {

        Serial0.println(
            "[WS-AUDIO] ERROR: Failed to build audio_hello"
        );

        return false;
    }

    const bool result = _webSocket.sendTXT(message);

    if (!result) {

        Serial0.println(
            "[WS-AUDIO] ERROR: audio_hello send failed"
        );

        return false;
    }

    Serial0.println(
        "[WS-AUDIO] Audio handshake sent"
    );

    return true;
}


// ============================================================
// STATIC WEBSOCKET CALLBACK
// ============================================================

void WebSocketAudioClient::onWebSocketEvent(
    WStype_t type,
    uint8_t* payload,
    size_t length
) {

    if (_instance == nullptr) {
        return;
    }

    _instance->handleWebSocketEvent(
        type,
        payload,
        length
    );
}


// ============================================================
// HANDLE WEBSOCKET EVENTS
// ============================================================

void WebSocketAudioClient::handleWebSocketEvent(
    WStype_t type,
    uint8_t* payload,
    size_t length
) {

    switch (type) {

        // ----------------------------------------------------
        // CONNECTED
        // ----------------------------------------------------

        case WStype_CONNECTED: {

            _connected = true;

            Serial0.println(
                "[WS-AUDIO] Connected to server"
            );

            sendHello();

            emitEvent(
                Event::CONNECTED
            );

            break;
        }


        // ----------------------------------------------------
        // DISCONNECTED
        // ----------------------------------------------------

        case WStype_DISCONNECTED: {

            _connected = false;

            Serial0.println(
                "[WS-AUDIO] Disconnected"
            );

            emitEvent(
                Event::DISCONNECTED
            );

            break;
        }


        // ----------------------------------------------------
        // TEXT RECEIVED
        // ----------------------------------------------------

        case WStype_TEXT: {

            if (
                payload == nullptr ||
                length == 0
            ) {
                break;
            }

            Serial0.printf(
                "[WS-AUDIO] Text received: %.*s\n",
                static_cast<int>(length),
                reinterpret_cast<const char*>(payload)
            );

            emitEvent(
                Event::TEXT_RECEIVED,
                payload,
                length
            );

            break;
        }


        // ----------------------------------------------------
        // BINARY RECEIVED
        // ----------------------------------------------------

        case WStype_BIN: {

            if (
                payload == nullptr ||
                length == 0
            ) {
                break;
            }

            // Не сохранять payload как указатель.
            // Получатель должен скопировать данные
            // в собственный буфер внутри callback.

            emitEvent(
                Event::PCM_RECEIVED,
                payload,
                length
            );

            break;
        }


        // ----------------------------------------------------
        // ERROR
        // ----------------------------------------------------

        case WStype_ERROR: {

            Serial0.println(
                "[WS-AUDIO] WebSocket error"
            );

            emitEvent(
                Event::ERROR,
                payload,
                length
            );

            break;
        }


        // ----------------------------------------------------
        // HEARTBEAT
        // ----------------------------------------------------

        case WStype_PING:
        case WStype_PONG: {

            break;
        }


        // ----------------------------------------------------
        // DEFAULT
        // ----------------------------------------------------

        default: {

            break;
        }
    }
}


// ============================================================
// EMIT EVENT
// ============================================================

void WebSocketAudioClient::emitEvent(
    Event event,
    const uint8_t* payload,
    size_t length
) {

    if (_callback == nullptr) {
        return;
    }

    _callback(
        event,
        payload,
        length
    );
}
