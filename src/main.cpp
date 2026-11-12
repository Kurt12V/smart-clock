
#include <Arduino.h>
#include <LittleFS.h>
#include <cstring>

// ============================================================
// PROJECT HEADERS
// ============================================================

#include "./managers/I2S/I2SManager.h"
#include "./managers/Setting/SettingsManager.h"
#include "managers/WiFi/WiFiManager.h"

#include "./hardware/audio/Microphone.h"
#include "./managers/Sensors/MicrophoneManager.h"
#include "./hardware/audio/AudioRecorder.h"

#include "./managers/Sound/SoundManager.h"
#include "./managers/NET/WebSocketAudioClient.h"

// ============================================================
// CONFIGURATION
// ============================================================

namespace AudioConfig
{
    constexpr char SERVER_HOST[] = "192.168.1.93";
    constexpr uint16_t SERVER_PORT = 8765;
    constexpr char SERVER_PATH[] = "/audio";

    constexpr char DEVICE_ID[] = "smartclock-01";

    constexpr uint32_t SAMPLE_RATE = 16000;

    constexpr uint32_t RECORDING_DURATION_MS = 5000;

    // Максимальный размер одного входящего WebSocket PCM-пакета.
    // 1024 байта = 512 samples при PCM S16LE.
    constexpr size_t PCM_BUFFER_BYTES = 1024;
    constexpr size_t PCM_BUFFER_SAMPLES =
        PCM_BUFFER_BYTES / sizeof(int16_t);

    constexpr uint32_t DIAGNOSTIC_INTERVAL_MS = 1000;
}

// ============================================================
// OBJECTS
// ============================================================

SettingsManager settingsManager;

I2SManager i2sManager;

WiFiManager wifiManager;

Microphone microphone(i2sManager);

AudioRecorder audioRecorder(
    microphone,
    LittleFS
);

MicrophoneManager microphoneManager(
    microphone,
    audioRecorder
);

SoundManager soundManager(
    settingsManager,
    i2sManager
);

WebSocketAudioClient audioClient;

// ============================================================
// AUDIO STATE
// ============================================================

enum class AudioMode : uint8_t
{
    WAITING,

    RECORDING,

    RECORDING_DRAINING,

    PROCESSING,

    PLAYING,

    DRAINING
};

static AudioMode audioMode = AudioMode::WAITING;

// ============================================================
// STATE
// ============================================================

static bool wifiWasConnected = false;
static bool audioInitialized = false;

static uint32_t modeStartTime = 0;

static uint32_t lastSentChunkId = 0;
static uint32_t lastDiagnosticTime = 0;

static int16_t pcmBuffer[
    AudioConfig::PCM_BUFFER_SAMPLES
];

// ============================================================
// FORWARD DECLARATIONS
// ============================================================

static void startAudioClient();

static void startRecording();

static void stopRecordingCapture();

static void finishRecordingAfterDrain();

static void startPlayback();

static void finishPlayback();

static void updateAudioSession();

static void transmitMicrophoneAudio();

static void handleIncomingPCM(
    const uint8_t* payload,
    size_t length
);

static void resetAudioSession();

static void onAudioEvent(
    WebSocketAudioClient::Event event,
    const uint8_t* payload,
    size_t length
);

// ============================================================
// RESET AUDIO SESSION
// ============================================================

static void resetAudioSession()
{
    microphoneManager.stopListening();

    soundManager.stop();

    audioInitialized = false;

    audioMode = AudioMode::WAITING;

    modeStartTime = millis();

    lastSentChunkId =
        microphoneManager.getAudioChunkId();

    lastDiagnosticTime = 0;

    Serial0.println(
        "[Audio] Session reset"
    );
}

// ============================================================
// HANDLE INCOMING PCM
// ============================================================

static void handleIncomingPCM(
    const uint8_t* payload,
    size_t length
)
{
    if (!audioInitialized)
    {
        return;
    }

    if (
        audioMode != AudioMode::PROCESSING &&
        audioMode != AudioMode::PLAYING
    )
    {
        return;
    }

    if (
        payload == nullptr ||
        length == 0 ||
        (length % sizeof(int16_t)) != 0
    )
    {
        Serial0.println(
            "[Audio][WARN] Invalid PCM packet"
        );

        return;
    }

    const size_t sampleCount =
        length / sizeof(int16_t);

    if (
        sampleCount >
        AudioConfig::PCM_BUFFER_SAMPLES
    )
    {
        Serial0.printf(
            "[Audio][ERROR] PCM packet too large: %u bytes\n",
            static_cast<unsigned int>(length)
        );

        return;
    }

    // WebSocket payload не гарантирует выравнивание
    // по границе int16_t. Поэтому копируем его.
    memcpy(
        pcmBuffer,
        payload,
        length
    );

    // Только добавляем PCM в FIFO.
    // Воспроизведение запускает SoundManager после prebuffer.
    if (!soundManager.submitPCM(
        pcmBuffer,
        sampleCount
    ))
    {
        Serial0.printf(
            "[Audio][ERROR] PCM FIFO overflow. "
            "Queued: %u samples, incoming: %u samples\n",
            static_cast<unsigned int>(
                soundManager.queuedPCM()
            ),
            static_cast<unsigned int>(sampleCount)
        );

        return;
    }
}

// ============================================================
// WEBSOCKET EVENT CALLBACK
// ============================================================

static void onAudioEvent(
    WebSocketAudioClient::Event event,
    const uint8_t* payload,
    size_t length
)
{
    switch (event)
    {
        // ----------------------------------------------------
        // CONNECTED
        // ----------------------------------------------------

        case WebSocketAudioClient::Event::CONNECTED:
        {
            Serial0.println(
                "[Audio] WebSocket connected"
            );

            resetAudioSession();

            break;
        }

        // ----------------------------------------------------
        // DISCONNECTED
        // ----------------------------------------------------

        case WebSocketAudioClient::Event::DISCONNECTED:
        {
            Serial0.println(
                "[Audio] WebSocket disconnected"
            );

            resetAudioSession();

            break;
        }

        // ----------------------------------------------------
        // PCM RECEIVED
        // ----------------------------------------------------

        case WebSocketAudioClient::Event::PCM_RECEIVED:
        {
            handleIncomingPCM(
                payload,
                length
            );

            break;
        }

        // ----------------------------------------------------
        // TEXT RECEIVED
        // ----------------------------------------------------

        case WebSocketAudioClient::Event::TEXT_RECEIVED:
        {
            if (
                payload == nullptr ||
                length == 0
            )
            {
                break;
            }

            char message[256]{};

            const size_t copyLength =
                length < sizeof(message) - 1
                    ? length
                    : sizeof(message) - 1;

            memcpy(
                message,
                payload,
                copyLength
            );

            message[copyLength] = '\0';

            Serial0.printf(
                "[Audio] Server: %s\n",
                message
            );

            // -----------------------------------------------
            // PLAYBACK END
            // -----------------------------------------------

            if (
                strstr(
                    message,
                    "\"playback_end\""
                ) != nullptr
            )
            {
                if (
                    audioInitialized &&
                    (
                        audioMode == AudioMode::PROCESSING ||
                        audioMode == AudioMode::PLAYING
                    )
                )
                {
                    Serial0.println(
                        "[Audio] Server playback ended"
                    );

                    // Запрещаем поступление новых PCM-данных.
                    // SoundManager должен воспроизвести остаток FIFO.
                    soundManager.finishPCM();

                    // Не останавливаем динамик здесь.
                    // Ждём фактического завершения FIFO и I2S DMA.
                    audioMode = AudioMode::DRAINING;
                }
            }

            break;
        }

        // ----------------------------------------------------
        // ERROR
        // ----------------------------------------------------

        case WebSocketAudioClient::Event::ERROR:
        {
            Serial0.println(
                "[Audio][ERROR] WebSocket error"
            );

            break;
        }
    }
}

// ============================================================
// START WEBSOCKET CLIENT
// ============================================================

static void startAudioClient()
{
    if (audioClient.isStarted())
    {
        return;
    }

    Serial0.println(
        "[Audio] Starting WebSocket..."
    );

    audioClient.setEventCallback(
        onAudioEvent
    );

    if (!audioClient.begin(
        AudioConfig::SERVER_HOST,
        AudioConfig::SERVER_PORT,
        AudioConfig::SERVER_PATH,
        AudioConfig::DEVICE_ID
    ))
    {
        Serial0.println(
            "[Audio][ERROR] WebSocket start failed"
        );

        return;
    }

    Serial0.println(
        "[Audio] WebSocket started"
    );
}

// ============================================================
// START RECORDING
// ============================================================

static void startRecording()
{
    if (!audioClient.isConnected())
    {
        return;
    }

    // Предыдущая сессия должна быть завершена.
    soundManager.stop();

    audioInitialized = false;

    lastSentChunkId =
        microphoneManager.getAudioChunkId();

    if (!microphoneManager.startListening())
    {
        Serial0.println(
            "[Audio][ERROR] Microphone start failed"
        );

        return;
    }

    if (!audioClient.sendText(
        "{\"type\":\"audio_start\","
        "\"sample_rate\":16000,"
        "\"format\":\"s16le\","
        "\"channels\":1}"
    ))
    {
        Serial0.println(
            "[Audio][ERROR] audio_start transmission failed"
        );

        microphoneManager.stopListening();

        return;
    }

    audioMode = AudioMode::RECORDING;

    modeStartTime = millis();

    Serial0.println(
        "[Audio] RECORDING started"
    );
}

// ============================================================
// STOP RECORDING CAPTURE
// ============================================================

static void stopRecordingCapture()
{
    if (audioMode != AudioMode::RECORDING)
    {
        return;
    }

    microphoneManager.stopListening();

    // Сначала отправляем все оставшиеся блоки микрофона.
    // audio_end будет отправлен только после опустошения
    // исходящей очереди WebSocket.
    audioMode = AudioMode::RECORDING_DRAINING;

    modeStartTime = millis();

    Serial0.println(
        "[Audio] Recording capture stopped"
    );

    Serial0.println(
        "[Audio] Waiting for outgoing queue..."
    );
}

// ============================================================
// FINISH RECORDING AFTER WEBSOCKET QUEUE DRAIN
// ============================================================

static void finishRecordingAfterDrain()
{
    if (
        audioMode != AudioMode::RECORDING_DRAINING
    )
    {
        return;
    }

    if (!audioClient.isConnected())
    {
        Serial0.println(
            "[Audio][ERROR] WebSocket disconnected during recording drain"
        );

        resetAudioSession();

        return;
    }

    const uint32_t now = millis();

    const uint32_t currentChunkId =
        microphoneManager.getAudioChunkId();

    const uint32_t queuedPackets =
        audioClient.getQueuedPackets();

    const bool chunksMatch =
        currentChunkId == lastSentChunkId;

    const bool queueEmpty =
        queuedPackets == 0;

    if (
        now - lastDiagnosticTime >=
        AudioConfig::DIAGNOSTIC_INTERVAL_MS
    )
    {
        lastDiagnosticTime = now;

        Serial0.println(
            "[Audio][DRAIN] ------------------------"
        );

        Serial0.printf(
            "[Audio][DRAIN] Current chunk: %lu\n",
            static_cast<unsigned long>(currentChunkId)
        );

        Serial0.printf(
            "[Audio][DRAIN] Last sent chunk: %lu\n",
            static_cast<unsigned long>(lastSentChunkId)
        );

        Serial0.printf(
            "[Audio][DRAIN] Chunk IDs match: %s\n",
            chunksMatch ? "YES" : "NO"
        );

        Serial0.printf(
            "[Audio][DRAIN] Queued packets: %lu\n",
            static_cast<unsigned long>(queuedPackets)
        );

        Serial0.printf(
            "[Audio][DRAIN] Sent packets: %lu\n",
            static_cast<unsigned long>(
                audioClient.getSentPackets()
            )
        );

        Serial0.printf(
            "[Audio][DRAIN] Dropped packets: %lu\n",
            static_cast<unsigned long>(
                audioClient.getDroppedPackets()
            )
        );

        Serial0.printf(
            "[Audio][DRAIN] Send errors: %lu\n",
            static_cast<unsigned long>(
                audioClient.getSendErrors()
            )
        );
    }

    // Последний блок микрофона ещё не добавлен в очередь.
    if (!chunksMatch)
    {
        return;
    }

    // В очереди ещё есть данные.
    if (!queueEmpty)
    {
        return;
    }

    Serial0.println(
        "[Audio][DRAIN] Outgoing queue drained"
    );

    // Сервер может начинать обработку только после audio_end.
    if (!audioClient.sendText(
        "{\"type\":\"audio_end\"}"
    ))
    {
        Serial0.println(
            "[Audio][ERROR] Failed to send audio_end"
        );

        return;
    }

    Serial0.println(
        "[Audio] audio_end sent"
    );

    // Подготавливаем входящий PCM-поток.
    if (!soundManager.beginPCM(
        AudioConfig::SAMPLE_RATE,
        SoundManager::AudioStream::System
    ))
    {
        Serial0.println(
            "[Audio][ERROR] PCM initialization failed"
        );

        soundManager.stop();

        audioInitialized = false;
        audioMode = AudioMode::WAITING;

        return;
    }

    audioInitialized = true;

    audioMode = AudioMode::PROCESSING;

    modeStartTime = millis();

    Serial0.println(
        "[Audio] Waiting for server PCM..."
    );
}

// ============================================================
// START PLAYBACK STATE
// ============================================================

static void startPlayback()
{
    if (audioMode != AudioMode::PROCESSING)
    {
        return;
    }

    audioMode = AudioMode::PLAYING;

    Serial0.println(
        "[Audio] Prebuffer ready. PLAYING."
    );
}

// ============================================================
// FINISH PLAYBACK
// ============================================================

static void finishPlayback()
{
    if (!audioInitialized)
    {
        return;
    }

    Serial0.println(
        "[Audio] Playback completed"
    );

    // SoundManager уже подтвердил завершение FIFO и DMA.
    soundManager.stop();

    audioInitialized = false;

    if (!audioClient.sendText(
        "{\"type\":\"playback_finished\"}"
    ))
    {
        Serial0.println(
            "[Audio][WARN] playback_finished transmission failed"
        );
    }

    audioMode = AudioMode::WAITING;

    modeStartTime = millis();

    Serial0.println(
        "[Audio] Ready for next recording"
    );
}

// ============================================================
// MICROPHONE TRANSMISSION
// ============================================================

static void transmitMicrophoneAudio()
{
    if (
        audioMode != AudioMode::RECORDING &&
        audioMode != AudioMode::RECORDING_DRAINING
    )
    {
        return;
    }

    if (!audioClient.isConnected())
    {
        return;
    }

    if (!microphoneManager.hasNewAudio())
    {
        return;
    }

    const uint32_t chunkId =
        microphoneManager.getAudioChunkId();

    if (chunkId == lastSentChunkId)
    {
        return;
    }

    const int16_t* samples = nullptr;

    const size_t sampleCount =
        microphoneManager.getAudioChunk(samples);

    if (
        samples == nullptr ||
        sampleCount == 0
    )
    {
        return;
    }

    // sendPcm копирует блок в собственную очередь.
    // При неудаче lastSentChunkId не обновляем:
    // следующий проход loop() повторит попытку.
    if (audioClient.sendPcm(
        samples,
        sampleCount
    ))
    {
        lastSentChunkId = chunkId;
    }
}

// ============================================================
// UPDATE AUDIO SESSION
// ============================================================

static void updateAudioSession()
{
    if (!audioClient.isConnected())
    {
        return;
    }

    switch (audioMode)
    {
        // ----------------------------------------------------
        // WAITING
        // ----------------------------------------------------

        case AudioMode::WAITING:
        {
            startRecording();

            break;
        }

        // ----------------------------------------------------
        // RECORDING
        // ----------------------------------------------------

        case AudioMode::RECORDING:
        {
            if (
                millis() - modeStartTime >=
                AudioConfig::RECORDING_DURATION_MS
            )
            {
                stopRecordingCapture();
            }

            break;
        }

        // ----------------------------------------------------
        // RECORDING DRAINING
        // ----------------------------------------------------

        case AudioMode::RECORDING_DRAINING:
        {
            finishRecordingAfterDrain();

            break;
        }

        // ----------------------------------------------------
        // PROCESSING
        // ----------------------------------------------------

        case AudioMode::PROCESSING:
        {
            // SoundManager сам запускает I2S после prebuffer.
            // Здесь меняем только состояние аудиосессии.
            if (soundManager.isPCMPrebufferReady())
            {
                startPlayback();
            }

            break;
        }

        // ----------------------------------------------------
        // PLAYING
        // ----------------------------------------------------

        case AudioMode::PLAYING:
        {
            // Новые данные принимает WebSocket callback.
            // Воспроизведение обслуживается soundManager.update().
            break;
        }

        // ----------------------------------------------------
        // DRAINING
        // ----------------------------------------------------

        case AudioMode::DRAINING:
        {
            // Никаких таймеров.
            // Ждём фактического завершения FIFO и I2S DMA.
            if (soundManager.isPCMPlaybackFinished())
            {
                finishPlayback();
            }

            break;
        }
    }
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);

    delay(500);

    Serial0.println();

    Serial0.println(
        "========================================"
    );

    Serial0.println(
        "         SMARTCLOCK AUDIO"
    );

    Serial0.println(
        "========================================"
    );

    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    Serial0.println(
        "[Setup] SettingsManager..."
    );

    if (!settingsManager.begin())
    {
        Serial0.println(
            "[Setup][ERROR] Settings initialization failed"
        );

        return;
    }

    // --------------------------------------------------------
    // I2S
    // --------------------------------------------------------

    Serial0.println(
        "[Setup] I2SManager..."
    );

    if (!i2sManager.begin())
    {
        Serial0.println(
            "[Setup][ERROR] I2S initialization failed"
        );

        return;
    }

    // --------------------------------------------------------
    // SOUND
    // --------------------------------------------------------

    Serial0.println(
        "[Setup] SoundManager..."
    );

    if (!soundManager.begin())
    {
        Serial0.println(
            "[Setup][ERROR] SoundManager initialization failed"
        );

        return;
    }

    // --------------------------------------------------------
    // LITTLEFS
    // --------------------------------------------------------

    Serial0.println(
        "[Setup] LittleFS..."
    );

    if (!LittleFS.begin(false))
    {
        Serial0.println(
            "[Setup][WARN] LittleFS mount failed"
        );
    }

    // --------------------------------------------------------
    // MICROPHONE
    // --------------------------------------------------------

    Serial0.println(
        "[Setup] MicrophoneManager..."
    );

    if (!microphoneManager.begin())
    {
        Serial0.println(
            "[Setup][ERROR] Microphone initialization failed"
        );

        return;
    }

    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    Serial0.println(
        "[Setup] WiFiManager..."
    );

    if (!wifiManager.begin())
    {
        Serial0.println(
            "[Setup][ERROR] WiFi initialization failed"
        );

        return;
    }

    Serial0.println();

    Serial0.println(
        "[Setup] Initialization completed"
    );

    Serial0.println(
        "[Setup] Waiting for WiFi..."
    );
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // WIFI UPDATE
    // --------------------------------------------------------

    wifiManager.update();

    const bool wifiConnected =
        wifiManager.isConnected();

    // --------------------------------------------------------
    // WIFI DISCONNECTED
    // --------------------------------------------------------

    if (!wifiConnected)
    {
        if (wifiWasConnected)
        {
            Serial0.println(
                "[WiFi] Connection lost"
            );

            audioClient.stop();

            resetAudioSession();

            wifiWasConnected = false;
        }

        microphoneManager.update();

        soundManager.update();

        settingsManager.update();

        delay(1);

        return;
    }

    // --------------------------------------------------------
    // WIFI CONNECTED
    // --------------------------------------------------------

    if (!wifiWasConnected)
    {
        wifiWasConnected = true;

        Serial0.println(
            "[WiFi] Connected"
        );

        Serial0.print(
            "[WiFi] IP: "
        );

        Serial0.println(
            wifiManager.getIP()
        );

        startAudioClient();
    }

    // --------------------------------------------------------
    // WEBSOCKET
    // --------------------------------------------------------

    // Принимаем входящие сообщения и отправляем
    // не более одного исходящего PCM-пакета за update().
    audioClient.update();

    // --------------------------------------------------------
    // MICROPHONE TRANSMISSION
    // --------------------------------------------------------

    // Повторяем попытку отправки последнего блока,
    // если очередь WebSocket была заполнена.
    transmitMicrophoneAudio();

    // --------------------------------------------------------
    // MICROPHONE UPDATE
    // --------------------------------------------------------

    microphoneManager.update();

    // --------------------------------------------------------
    // AUDIO SESSION
    // --------------------------------------------------------

    updateAudioSession();

    // --------------------------------------------------------
    // SOUND UPDATE
    // --------------------------------------------------------

    // FIFO и I2S обслуживаются постоянно, независимо
    // от того, поступают ли сейчас новые WebSocket-пакеты.
    soundManager.update();

    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    settingsManager.update();

    delay(1);
}
