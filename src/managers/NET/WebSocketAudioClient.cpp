#include "WebSocketAudioClient.h"

#include <Arduino.h>

#include <cstring>
#include <cstdio>

// ============================================================
// CONFIGURATION
// ============================================================
namespace AudioConfig
{
    constexpr uint32_t SAMPLE_RATE_HZ = 16000;
}
namespace WebSocketAudioConfig
{
    constexpr uint32_t RECONNECT_INTERVAL = 5000;

    constexpr uint32_t HEARTBEAT_INTERVAL = 15000;
    constexpr uint32_t HEARTBEAT_TIMEOUT = 5000;

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
    : _pcmQueue(nullptr),
      _started(false),
      _connected(false),
      _helloPending(false),
      _callback(nullptr),
      _droppedPackets(0),
      _sentPackets(0),
      _sendErrors(0)
{
    memset(_deviceId, 0, sizeof(_deviceId));
}

// ============================================================
// DESTRUCTOR
// ============================================================

WebSocketAudioClient::~WebSocketAudioClient()
{
    stop();

    if (_pcmQueue != nullptr)
    {
        vQueueDelete(_pcmQueue);
        _pcmQueue = nullptr;
    }
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
    if (host == nullptr ||
        path == nullptr ||
        deviceId == nullptr ||
        host[0] == '\0' ||
        path[0] == '\0' ||
        deviceId[0] == '\0' ||
        port == 0)
    {
        Serial0.println(
            "[WS-AUDIO][ERROR] Invalid configuration"
        );

        return false;
    }

    if (strlen(deviceId) >= sizeof(_deviceId))
    {
        Serial0.println(
            "[WS-AUDIO][ERROR] Device ID is too long"
        );

        return false;
    }

    if (_instance != nullptr && _instance != this)
    {
        Serial0.println(
            "[WS-AUDIO][ERROR] Another client is active"
        );

        return false;
    }

    if (_started)
    {
        stop();
    }

    // --------------------------------------------------------
    // CREATE PCM QUEUE
    // --------------------------------------------------------

    if (_pcmQueue == nullptr)
    {
        _pcmQueue = xQueueCreate(
            PCM_QUEUE_LENGTH,
            sizeof(PcmPacket)
        );
    }

    if (_pcmQueue == nullptr)
    {
        Serial0.println(
            "[WS-AUDIO][ERROR] Cannot create PCM queue"
        );

        return false;
    }

    clearPcmQueue();

    // --------------------------------------------------------
    // RESET STATE
    // --------------------------------------------------------

    memset(_deviceId, 0, sizeof(_deviceId));

    strncpy(
        _deviceId,
        deviceId,
        sizeof(_deviceId) - 1
    );

    _connected = false;
    _helloPending = false;

    _droppedPackets = 0;
    _sentPackets = 0;
    _sendErrors = 0;

    _instance = this;

    // --------------------------------------------------------
    // START WEBSOCKET
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println(
        "[WS-AUDIO] Initializing WebSocket"
    );

    Serial0.printf(
        "[WS-AUDIO] Server: %s:%u%s\n",
        host,
        static_cast<unsigned int>(port),
        path
    );

    Serial0.printf(
        "[WS-AUDIO] Device: %s\n",
        _deviceId
    );

    Serial0.printf(
        "[WS-AUDIO] Audio: %u Hz, %u-bit, mono PCM\n",
        static_cast<unsigned int>(AudioConfig::SAMPLE_RATE_HZ),
        static_cast<unsigned int>(BITS_PER_SAMPLE)
    );

    Serial0.printf(
        "[WS-AUDIO] Queue: %u packets, %u bytes each\n",
        static_cast<unsigned int>(PCM_QUEUE_LENGTH),
        static_cast<unsigned int>(PCM_PACKET_SIZE)
    );

    _webSocket.begin(
        host,
        port,
        path
    );

    _webSocket.onEvent(
        WebSocketAudioClient::onWebSocketEvent
    );

    _webSocket.setReconnectInterval(
        WebSocketAudioConfig::RECONNECT_INTERVAL
    );

    _webSocket.enableHeartbeat(
        WebSocketAudioConfig::HEARTBEAT_INTERVAL,
        WebSocketAudioConfig::HEARTBEAT_TIMEOUT,
        WebSocketAudioConfig::HEARTBEAT_FAIL_LIMIT
    );

    _started = true;

    Serial0.println(
        "[WS-AUDIO] Client started"
    );

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

    // Важно:
    // loop() и все операции отправки выполняются
    // из одного контекста.

    _webSocket.loop();

    if (!_started || !_connected)
    {
        return;
    }

    // --------------------------------------------------------
    // SEND HANDSHAKE
    // --------------------------------------------------------

    if (_helloPending)
    {
        if (sendHello())
        {
            _helloPending = false;
        }

        return;
    }

    // --------------------------------------------------------
    // SEND ONE PCM PACKET
    // --------------------------------------------------------

    sendNextPcmPacket();
}

// ============================================================
// STOP
// ============================================================

void WebSocketAudioClient::stop()
{
    if (!_started)
    {
        return;
    }

    Serial0.println(
        "[WS-AUDIO] Stopping client"
    );

    _started = false;
    _connected = false;
    _helloPending = false;

    _webSocket.disconnect();

    clearPcmQueue();

    if (_instance == this)
    {
        _instance = nullptr;
    }

    Serial0.println(
        "[WS-AUDIO] Client stopped"
    );
}

// ============================================================
// STATE
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
    if (samples == nullptr || sampleCount == 0)
    {
        return false;
    }

    if (sampleCount > SIZE_MAX / sizeof(int16_t))
    {
        return false;
    }

    const size_t byteLength =
        sampleCount * sizeof(int16_t);

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
    if (!_started || !_connected)
    {
        return false;
    }

    if (data == nullptr || length == 0)
    {
        return false;
    }

    if ((length % sizeof(int16_t)) != 0)
    {
        Serial0.println(
            "[WS-AUDIO][ERROR] PCM length must be even"
        );

        return false;
    }

    return enqueuePcm(data, length);
}

// ============================================================
// ENQUEUE PCM
// ============================================================

bool WebSocketAudioClient::enqueuePcm(
    const uint8_t* data,
    size_t length
)
{
    if (_pcmQueue == nullptr ||
        data == nullptr ||
        length == 0)
    {
        return false;
    }

    size_t offset = 0;

    while (offset < length)
    {
        const size_t remaining = length - offset;

        size_t packetLength =
            remaining > PCM_PACKET_SIZE
                ? PCM_PACKET_SIZE
                : remaining;

        // PCM S16LE: длина должна быть кратна двум.
        packetLength -= packetLength % 2;

        if (packetLength == 0)
        {
            return false;
        }

        PcmPacket packet{};

        packet.length =
            static_cast<uint16_t>(packetLength);

        memcpy(
            packet.data,
            data + offset,
            packetLength
        );

        // Не ждём освобождения очереди.
        // Если очередь заполнена, отбрасываем пакет.
        if (xQueueSend(
                _pcmQueue,
                &packet,
                0
            ) != pdTRUE)
        {
            ++_droppedPackets;

            return false;
        }

        offset += packetLength;
    }

    return true;
}

// ============================================================
// SEND NEXT PCM PACKET
// ============================================================

bool WebSocketAudioClient::sendNextPcmPacket()
{
    if (!_started ||
        !_connected ||
        _pcmQueue == nullptr)
    {
        return false;
    }

    PcmPacket packet{};

    if (xQueueReceive(
            _pcmQueue,
            &packet,
            0
        ) != pdTRUE)
    {
        return false;
    }

    if (packet.length == 0 ||
        packet.length > PCM_PACKET_SIZE)
    {
        ++_sendErrors;
        return false;
    }

    const bool result = _webSocket.sendBIN(
        packet.data,
        packet.length
    );

    if (!result)
    {
        ++_sendErrors;

        Serial0.printf(
            "[WS-AUDIO][ERROR] PCM send failed. Errors: %lu\n",
            static_cast<unsigned long>(_sendErrors)
        );

        return false;
    }

    ++_sentPackets;

    return true;
}

// ============================================================
// CLEAR PCM QUEUE
// ============================================================

void WebSocketAudioClient::clearPcmQueue()
{
    if (_pcmQueue == nullptr)
    {
        return;
    }

    PcmPacket packet{};

    while (xQueueReceive(
        _pcmQueue,
        &packet,
        0
    ) == pdTRUE)
    {
        // Удаляем устаревшие пакеты.
    }
}

// ============================================================
// SEND TEXT
// ============================================================

bool WebSocketAudioClient::sendText(
    const char* text
)
{
    if (!_started ||
        !_connected ||
        text == nullptr ||
        text[0] == '\0')
    {
        return false;
    }

    const bool result = _webSocket.sendTXT(
        const_cast<char*>(text)
    );

    if (!result)
    {
        Serial0.println(
            "[WS-AUDIO][ERROR] Text transmission failed"
        );
    }

    return result;
}

// ============================================================
// CALLBACK REGISTRATION
// ============================================================

void WebSocketAudioClient::setEventCallback(
    EventCallback callback
)
{
    _callback = callback;
}

// ============================================================
// STATISTICS
// ============================================================

uint32_t WebSocketAudioClient::getQueuedPackets() const
{
    if (_pcmQueue == nullptr)
    {
        return 0;
    }

    return static_cast<uint32_t>(
        uxQueueMessagesWaiting(_pcmQueue)
    );
}

uint32_t WebSocketAudioClient::getDroppedPackets() const
{
    return _droppedPackets;
}

uint32_t WebSocketAudioClient::getSentPackets() const
{
    return _sentPackets;
}

uint32_t WebSocketAudioClient::getSendErrors() const
{
    return _sendErrors;
}

// ============================================================
// SEND HELLO
// ============================================================

bool WebSocketAudioClient::sendHello()
{
    if (!_started || !_connected)
    {
        return false;
    }

    char message[256]{};

    const int length = snprintf(
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
        static_cast<unsigned int>(AudioConfig::SAMPLE_RATE_HZ),
        static_cast<unsigned int>(BITS_PER_SAMPLE)
    );

    if (length <= 0 ||
        static_cast<size_t>(length) >= sizeof(message))
    {
        return false;
    }

    if (!_webSocket.sendTXT(message))
    {
        Serial0.println(
            "[WS-AUDIO][ERROR] Handshake transmission failed"
        );

        return false;
    }

    Serial0.println(
        "[WS-AUDIO] Handshake sent"
    );

    return true;
}

// ============================================================
// STATIC WEBSOCKET EVENT
// ============================================================

void WebSocketAudioClient::onWebSocketEvent(
    WStype_t type,
    uint8_t* payload,
    size_t length
)
{
    if (_instance == nullptr)
    {
        return;
    }

    _instance->handleWebSocketEvent(
        type,
        payload,
        length
    );
}

// ============================================================
// HANDLE WEBSOCKET EVENT
// ============================================================

void WebSocketAudioClient::handleWebSocketEvent(
    WStype_t type,
    uint8_t* payload,
    size_t length
)
{
    switch (type)
    {
        // ----------------------------------------------------
        // CONNECTED
        // ----------------------------------------------------

        case WStype_CONNECTED:
        {
            _connected = true;
            _helloPending = true;

            clearPcmQueue();

            Serial0.println(
                "[WS-AUDIO] Connected"
            );

            emitEvent(Event::CONNECTED);

            break;
        }

        // ----------------------------------------------------
        // DISCONNECTED
        // ----------------------------------------------------

        case WStype_DISCONNECTED:
        {
            _connected = false;
            _helloPending = false;

            clearPcmQueue();

            Serial0.println(
                "[WS-AUDIO] Disconnected"
            );

            emitEvent(Event::DISCONNECTED);

            break;
        }

        // ----------------------------------------------------
        // TEXT
        // ----------------------------------------------------

        case WStype_TEXT:
        {
            if (payload == nullptr || length == 0)
            {
                break;
            }

            emitEvent(
                Event::TEXT_RECEIVED,
                payload,
                length
            );

            break;
        }

        // ----------------------------------------------------
        // BINARY
        // ----------------------------------------------------

        case WStype_BIN:
        {
            if (payload == nullptr || length == 0)
            {
                break;
            }

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

        case WStype_ERROR:
        {
            Serial0.println(
                "[WS-AUDIO][ERROR] WebSocket transport error"
            );

            emitEvent(
                Event::ERROR,
                payload,
                length
            );

            break;
        }

        // ----------------------------------------------------
        // PING / PONG
        // ----------------------------------------------------

        case WStype_PING:
        case WStype_PONG:
        {
            // Не выводим сообщение на каждый heartbeat.
            break;
        }

        default:
        {
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
        return;
    }

    // ВАЖНО:
    // payload принадлежит WebSocket-библиотеке.
    // Использовать его после выхода из callback нельзя.
    // Если данные нужны позже, callback должен скопировать их.

    _callback(
        event,
        payload,
        length
    );
}