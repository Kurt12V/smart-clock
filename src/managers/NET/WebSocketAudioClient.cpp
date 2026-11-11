
#include "WebSocketAudioClient.h"

#include <Arduino.h>
#include <cstring>
#include <cstdio>

// ============================================================
// CONFIGURATION
// ============================================================

namespace WebSocketAudioConfig
{
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
      _callback(nullptr)
{
    memset(_deviceId, 0, sizeof(_deviceId));
}

// ============================================================
// BEGIN
// ============================================================

bool WebSocketAudioClient::begin(
    const char* host,
    uint16_t port,
    const char* path,
    const char* deviceId
)
{
    Serial0.println();
    Serial0.println("[WS-AUDIO] ========================================");
    Serial0.println("[WS-AUDIO] INITIALIZATION");
    Serial0.println("[WS-AUDIO] ========================================");

    if (host == nullptr || path == nullptr || deviceId == nullptr)
    {
        Serial0.println("[WS-AUDIO][ERROR] Invalid configuration");
        return false;
    }

    if (host[0] == '\0' || path[0] == '\0' || deviceId[0] == '\0')
    {
        Serial0.println("[WS-AUDIO][ERROR] Empty configuration");
        return false;
    }

    if (strlen(deviceId) >= sizeof(_deviceId))
    {
        Serial0.println("[WS-AUDIO][ERROR] Device ID too long");
        return false;
    }

    if (_started)
    {
        Serial0.println("[WS-AUDIO] Previous client instance detected");
        stop();
    }

    if (_instance != nullptr && _instance != this)
    {
        Serial0.println("[WS-AUDIO][ERROR] Another client instance is active");
        return false;
    }

    memset(_deviceId, 0, sizeof(_deviceId));
    memcpy(_deviceId, deviceId, strlen(deviceId));

    _instance = this;

    Serial0.printf("[WS-AUDIO] Host: %s\n", host);
    Serial0.printf("[WS-AUDIO] Port: %u\n", port);
    Serial0.printf("[WS-AUDIO] Path: %s\n", path);
    Serial0.printf("[WS-AUDIO] Device: %s\n", _deviceId);

    Serial0.printf(
        "[WS-AUDIO] PCM format: %u Hz, %u-bit, mono\n",
        static_cast<unsigned int>(SAMPLE_RATE),
        static_cast<unsigned int>(BITS_PER_SAMPLE)
    );

    Serial0.printf(
        "[WS-AUDIO] PCM chunk size: %u bytes\n",
        static_cast<unsigned int>(WebSocketAudioConfig::PCM_CHUNK_SIZE)
    );

    Serial0.println("[WS-AUDIO] Creating WebSocket connection...");

    _webSocket.begin(host, port, path);

    _webSocket.onEvent(WebSocketAudioClient::onWebSocketEvent);

    _webSocket.setReconnectInterval(
        WebSocketAudioConfig::RECONNECT_INTERVAL
    );

    _webSocket.enableHeartbeat(
        WebSocketAudioConfig::HEARTBEAT_INTERVAL,
        WebSocketAudioConfig::HEARTBEAT_TIMEOUT,
        WebSocketAudioConfig::HEARTBEAT_FAIL_LIMIT
    );

    _connected = false;
    _started = true;

    Serial0.println("[WS-AUDIO] Reconnect enabled");
    Serial0.println("[WS-AUDIO] Heartbeat enabled");
    Serial0.println("[WS-AUDIO] Client initialized");
    Serial0.println("[WS-AUDIO] Waiting for connection...");

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void WebSocketAudioClient::update()
{
    if (!_started)
    {
        return;
    }

    _webSocket.loop();
}

// ============================================================
// STOP
// ============================================================

void WebSocketAudioClient::stop()
{
    if (!_started)
    {
        Serial0.println("[WS-AUDIO] Stop ignored: client not started");
        return;
    }

    Serial0.println("[WS-AUDIO] Stopping WebSocket client...");

    _connected = false;
    _started = false;

    _webSocket.disconnect();

    if (_instance == this)
    {
        _instance = nullptr;
    }

    Serial0.println("[WS-AUDIO] Client stopped");
}

// ============================================================
// CONNECTION STATE
// ============================================================

bool WebSocketAudioClient::isConnected() const
{
    return _started && _connected;
}

bool WebSocketAudioClient::isStarted() const
{
    return _started;
}

// ============================================================
// SEND PCM SAMPLES
// ============================================================

bool WebSocketAudioClient::sendPcm(
    const int16_t* samples,
    size_t sampleCount
)
{
    Serial0.println("[WS-AUDIO] ----------------------------------------");
    Serial0.println("[WS-AUDIO] PCM SEND REQUEST");

    if (samples == nullptr)
    {
        Serial0.println("[WS-AUDIO][ERROR] PCM samples pointer is null");
        return false;
    }

    if (sampleCount == 0)
    {
        Serial0.println("[WS-AUDIO][ERROR] PCM sample count is zero");
        return false;
    }

    if (sampleCount > SIZE_MAX / sizeof(int16_t))
    {
        Serial0.println("[WS-AUDIO][ERROR] PCM sample count overflow");
        return false;
    }

    const size_t byteLength = sampleCount * sizeof(int16_t);

    Serial0.printf("[WS-AUDIO] Samples: %u\n",
        static_cast<unsigned int>(sampleCount));

    Serial0.printf("[WS-AUDIO] Bytes: %u\n",
        static_cast<unsigned int>(byteLength));

    Serial0.printf("[WS-AUDIO] Connected: %s\n",
        _connected ? "YES" : "NO");

    Serial0.printf("[WS-AUDIO] Started: %s\n",
        _started ? "YES" : "NO");

    return sendPcmBytes(
        reinterpret_cast<const uint8_t*>(samples),
        byteLength
    );
}

// ============================================================
// SEND PCM BYTES
// ============================================================

bool WebSocketAudioClient::sendPcmBytes(
    const uint8_t* data,
    size_t length
)
{
    if (!_started)
    {
        Serial0.println("[WS-AUDIO][ERROR] PCM send rejected: client not started");
        return false;
    }

    if (!_connected)
    {
        Serial0.println("[WS-AUDIO][ERROR] PCM send rejected: WebSocket disconnected");
        return false;
    }

    if (data == nullptr)
    {
        Serial0.println("[WS-AUDIO][ERROR] PCM data pointer is null");
        return false;
    }

    if (length == 0)
    {
        Serial0.println("[WS-AUDIO][ERROR] PCM length is zero");
        return false;
    }

    if (length % sizeof(int16_t) != 0)
    {
        Serial0.printf(
            "[WS-AUDIO][ERROR] Invalid PCM length: %u bytes\n",
            static_cast<unsigned int>(length)
        );

        return false;
    }

    constexpr size_t CHUNK_SIZE =
        WebSocketAudioConfig::PCM_CHUNK_SIZE;

    const size_t totalChunks =
        (length + CHUNK_SIZE - 1) / CHUNK_SIZE;

    Serial0.printf(
        "[WS-AUDIO] Starting binary transmission: %u bytes, %u chunks\n",
        static_cast<unsigned int>(length),
        static_cast<unsigned int>(totalChunks)
    );

    size_t offset = 0;
    size_t chunkIndex = 0;

    while (offset < length)
    {
        if (!_started || !_connected)
        {
            Serial0.println("[WS-AUDIO][ERROR] Connection lost during PCM transmission");

            Serial0.printf(
                "[WS-AUDIO] Sent before failure: %u/%u bytes\n",
                static_cast<unsigned int>(offset),
                static_cast<unsigned int>(length)
            );

            return false;
        }

        const size_t remaining = length - offset;

        size_t chunkLength =
            remaining > CHUNK_SIZE ? CHUNK_SIZE : remaining;

        chunkLength -= chunkLength % 2;

        if (chunkLength == 0)
        {
            Serial0.println("[WS-AUDIO][ERROR] Invalid chunk length");
            return false;
        }

        Serial0.printf(
            "[WS-AUDIO] Sending chunk %u/%u | offset=%u | size=%u\n",
            static_cast<unsigned int>(chunkIndex + 1),
            static_cast<unsigned int>(totalChunks),
            static_cast<unsigned int>(offset),
            static_cast<unsigned int>(chunkLength)
        );

        const uint32_t sendStart = millis();

        const bool result = _webSocket.sendBIN(
            const_cast<uint8_t*>(data + offset),
            chunkLength
        );

        const uint32_t sendDuration = millis() - sendStart;

        if (!result)
        {
            Serial0.println("[WS-AUDIO][ERROR] sendBIN() returned FALSE");

            Serial0.printf(
                "[WS-AUDIO] Failed chunk: %u/%u\n",
                static_cast<unsigned int>(chunkIndex + 1),
                static_cast<unsigned int>(totalChunks)
            );

            Serial0.printf(
                "[WS-AUDIO] Offset: %u\n",
                static_cast<unsigned int>(offset)
            );

            Serial0.printf(
                "[WS-AUDIO] Chunk length: %u\n",
                static_cast<unsigned int>(chunkLength)
            );

            Serial0.printf(
                "[WS-AUDIO] sendBIN duration: %lu ms\n",
                static_cast<unsigned long>(sendDuration)
            );

            Serial0.printf(
                "[WS-AUDIO] Started: %s\n",
                _started ? "YES" : "NO"
            );

            Serial0.printf(
                "[WS-AUDIO] Connected: %s\n",
                _connected ? "YES" : "NO"
            );

            return false;
        }

        Serial0.printf(
            "[WS-AUDIO] Chunk sent successfully in %lu ms\n",
            static_cast<unsigned long>(sendDuration)
        );

        offset += chunkLength;
        chunkIndex++;
    }

    Serial0.printf(
        "[WS-AUDIO] PCM transmission completed: %u bytes\n",
        static_cast<unsigned int>(offset)
    );

    return true;
}

// ============================================================
// SEND TEXT
// ============================================================

bool WebSocketAudioClient::sendText(
    const char* text
)
{
    Serial0.println("[WS-AUDIO] ----------------------------------------");
    Serial0.println("[WS-AUDIO] TEXT SEND REQUEST");

    if (!_started)
    {
        Serial0.println("[WS-AUDIO][ERROR] Text send rejected: client not started");
        return false;
    }

    if (!_connected)
    {
        Serial0.println("[WS-AUDIO][ERROR] Text send rejected: disconnected");
        return false;
    }

    if (text == nullptr || text[0] == '\0')
    {
        Serial0.println("[WS-AUDIO][ERROR] Invalid text message");
        return false;
    }

    Serial0.printf("[WS-AUDIO] Message: %s\n", text);

    const uint32_t sendStart = millis();

    const bool result = _webSocket.sendTXT(
        const_cast<char*>(text)
    );

    const uint32_t sendDuration = millis() - sendStart;

    Serial0.printf(
        "[WS-AUDIO] sendTXT duration: %lu ms\n",
        static_cast<unsigned long>(sendDuration)
    );

    if (!result)
    {
        Serial0.println("[WS-AUDIO][ERROR] sendTXT() returned FALSE");
        return false;
    }

    Serial0.println("[WS-AUDIO] Text sent successfully");

    return true;
}

// ============================================================
// EVENT CALLBACK REGISTRATION
// ============================================================

void WebSocketAudioClient::setEventCallback(
    EventCallback callback
)
{
    _callback = callback;

    Serial0.printf(
        "[WS-AUDIO] Event callback: %s\n",
        _callback != nullptr ? "REGISTERED" : "NULL"
    );
}

// ============================================================
// SEND HELLO
// ============================================================

bool WebSocketAudioClient::sendHello()
{
    if (!_started || !_connected)
    {
        Serial0.println("[WS-AUDIO][ERROR] Cannot send hello: disconnected");
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

    if (messageLength <= 0 ||
        static_cast<size_t>(messageLength) >= sizeof(message))
    {
        Serial0.println("[WS-AUDIO][ERROR] Failed to build audio_hello");
        return false;
    }

    Serial0.printf(
        "[WS-AUDIO] Sending handshake: %s\n",
        message
    );

    const bool result = _webSocket.sendTXT(message);

    if (!result)
    {
        Serial0.println("[WS-AUDIO][ERROR] audio_hello send failed");
        return false;
    }

    Serial0.println("[WS-AUDIO] Audio handshake sent successfully");

    return true;
}

// ============================================================
// STATIC WEBSOCKET CALLBACK
// ============================================================

void WebSocketAudioClient::onWebSocketEvent(
    WStype_t type,
    uint8_t* payload,
    size_t length
)
{
    if (_instance == nullptr)
    {
        Serial0.println("[WS-AUDIO][ERROR] Event received without active instance");
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
)
{
    switch (type)
    {
        case WStype_CONNECTED:
        {
            _connected = true;

            Serial0.println();
            Serial0.println("[WS-AUDIO] ========================================");
            Serial0.println("[WS-AUDIO] CONNECTED");
            Serial0.println("[WS-AUDIO] ========================================");

            sendHello();

            emitEvent(Event::CONNECTED);

            break;
        }

        case WStype_DISCONNECTED:
        {
            _connected = false;

            Serial0.println();
            Serial0.println("[WS-AUDIO] ========================================");
            Serial0.println("[WS-AUDIO] DISCONNECTED");
            Serial0.println("[WS-AUDIO] ========================================");

            emitEvent(Event::DISCONNECTED);

            break;
        }

        case WStype_TEXT:
        {
            if (payload == nullptr || length == 0)
            {
                Serial0.println("[WS-AUDIO][WARN] Empty text frame received");
                break;
            }

            Serial0.printf(
                "[WS-AUDIO] TEXT RECEIVED (%u bytes): %.*s\n",
                static_cast<unsigned int>(length),
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

        case WStype_BIN:
        {
            if (payload == nullptr || length == 0)
            {
                Serial0.println("[WS-AUDIO][WARN] Empty binary frame received");
                break;
            }

            Serial0.printf(
                "[WS-AUDIO] BINARY RECEIVED: %u bytes\n",
                static_cast<unsigned int>(length)
            );

            emitEvent(
                Event::PCM_RECEIVED,
                payload,
                length
            );

            break;
        }

        case WStype_ERROR:
        {
            Serial0.println("[WS-AUDIO][ERROR] WebSocket transport error");

            if (payload != nullptr && length > 0)
            {
                Serial0.printf(
                    "[WS-AUDIO][ERROR] Payload (%u bytes): %.*s\n",
                    static_cast<unsigned int>(length),
                    static_cast<int>(length),
                    reinterpret_cast<const char*>(payload)
                );
            }

            emitEvent(
                Event::ERROR,
                payload,
                length
            );

            break;
        }

        case WStype_PING:
        {
            Serial0.println("[WS-AUDIO] PING received");
            break;
        }

        case WStype_PONG:
        {
            Serial0.println("[WS-AUDIO] PONG received");
            break;
        }

        default:
        {
            Serial0.printf(
                "[WS-AUDIO] Unknown WebSocket event: %d\n",
                static_cast<int>(type)
            );

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
)
{
    if (_callback == nullptr)
    {
        Serial0.println("[WS-AUDIO][WARN] Event callback is not registered");
        return;
    }

    _callback(
        event,
        payload,
        length
    );
}
