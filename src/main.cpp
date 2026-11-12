
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

static constexpr char AUDIO_SERVER_HOST[] = "192.168.1.93";
static constexpr uint16_t AUDIO_SERVER_PORT = 8765;
static constexpr char AUDIO_SERVER_PATH[] = "/audio";

static constexpr char DEVICE_ID[] = "smartclock-01";

static constexpr uint32_t AUDIO_SAMPLE_RATE = 16000;

static constexpr uint32_t RECORDING_DURATION_MS = 5000;

// Задержка после playback_end.
// Пока SoundManager не предоставляет публичный метод
// проверки опустошения очереди, используется временной интервал.
static constexpr uint32_t PLAYBACK_DRAIN_MS = 500;

// Максимальный размер входящего PCM-пакета.
static constexpr size_t PCM_BUFFER_SAMPLES = 1024;

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

    // Микрофон уже остановлен.
    // Ожидаем отправки последнего PCM-пакета
    // и опустошения очереди WebSocket.
    RECORDING_DRAINING,

    PROCESSING,

    PLAYING,

    DRAINING
};

static AudioMode audioMode = AudioMode::WAITING;

// ============================================================
// STATE VARIABLES
// ============================================================

static uint32_t modeStartTime = 0;

static uint32_t lastSentChunkId = 0;

static uint32_t lastPcmReceivedTime = 0;

static bool audioInitialized = false;

static bool wifiWasConnected = false;

static bool serverPlaybackEnded = false;

// Incoming PCM buffer.
static int16_t pcmBuffer[PCM_BUFFER_SAMPLES];

// ============================================================
// FORWARD DECLARATIONS
// ============================================================

void startAudioClient();

void startRecording();

void stopRecordingCapture();

void finishRecordingAfterDrain();

void startPlayback();

void finishPlayback();

void updateAudioSession();

void transmitMicrophoneAudio();

void handleIncomingPCM(
    const uint8_t* payload,
    size_t length
);

void resetAudioSession();

void onAudioEvent(
    WebSocketAudioClient::Event event,
    const uint8_t* payload,
    size_t length
);

// ============================================================
// AUDIO SESSION RESET
// ============================================================

void resetAudioSession()
{
    microphoneManager.stopListening();

    soundManager.stop();

    audioInitialized = false;

    serverPlaybackEnded = false;

    audioMode = AudioMode::WAITING;

    modeStartTime = millis();

    lastPcmReceivedTime = 0;
}

// ============================================================
// WEBSOCKET EVENTS
// ============================================================

void onAudioEvent(
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

            lastSentChunkId =
                microphoneManager.getAudioChunkId();

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
            if (payload == nullptr || length == 0)
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
                    audioMode == AudioMode::PLAYING ||
                    audioMode == AudioMode::PROCESSING
                )
                {
                    serverPlaybackEnded = true;

                    Serial0.println(
                        "[Audio] Server playback ended"
                    );

                    if (audioMode == AudioMode::PROCESSING)
                    {
                        // Сервер не прислал ни одного PCM-пакета.
                        finishPlayback();
                    }
                    else
                    {
                        audioMode = AudioMode::DRAINING;
                    }
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
                "[Audio] WebSocket error"
            );

            break;
        }
    }
}

// ============================================================
// HANDLE INCOMING PCM
// ============================================================

void handleIncomingPCM(
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
        length % sizeof(int16_t) != 0
    )
    {
        return;
    }

    const size_t sampleCount =
        length / sizeof(int16_t);

    if (sampleCount > PCM_BUFFER_SAMPLES)
    {
        Serial0.println(
            "[Audio] PCM packet too large"
        );

        return;
    }

    // WebSocket payload не гарантированно выровнен
    // по границе int16_t, поэтому сначала копируем.
    memcpy(
        pcmBuffer,
        payload,
        length
    );

    if (!soundManager.submitPCM(
        pcmBuffer,
        sampleCount
    ))
    {
        Serial0.println(
            "[Audio] SoundManager PCM queue overflow"
        );

        return;
    }

    lastPcmReceivedTime = millis();

    if (audioMode == AudioMode::PROCESSING)
    {
        startPlayback();
    }
}

// ============================================================
// START WEBSOCKET
// ============================================================

void startAudioClient()
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
        AUDIO_SERVER_HOST,
        AUDIO_SERVER_PORT,
        AUDIO_SERVER_PATH,
        DEVICE_ID
    ))
    {
        Serial0.println(
            "[Audio] WebSocket start failed"
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

void startRecording()
{
    if (!audioClient.isConnected())
    {
        return;
    }

    soundManager.stop();

    audioInitialized = false;

    serverPlaybackEnded = false;

    // Запоминаем последний обработанный блок.
    lastSentChunkId =
        microphoneManager.getAudioChunkId();

    if (!microphoneManager.startListening())
    {
        Serial0.println(
            "[Audio] Microphone start failed"
        );

        return;
    }

    audioMode = AudioMode::RECORDING;

    modeStartTime = millis();

    if (!audioClient.sendText(
        "{\"type\":\"audio_start\","
        "\"sample_rate\":16000,"
        "\"format\":\"s16le\","
        "\"channels\":1}"
    ))
    {
        Serial0.println(
            "[Audio] audio_start transmission failed"
        );

        microphoneManager.stopListening();

        audioMode = AudioMode::WAITING;

        return;
    }

    Serial0.println(
        "[Audio] RECORDING started (5 seconds)"
    );
}

// ============================================================
// STOP RECORDING CAPTURE
// ============================================================

void stopRecordingCapture()
{
    if (audioMode != AudioMode::RECORDING)
    {
        return;
    }

    // Останавливаем получение новых блоков микрофона.
    microphoneManager.stopListening();

    // Не отправляем audio_end здесь.
    // Сначала необходимо отправить последний блок
    // и дождаться опустошения WebSocket-очереди.

    audioMode = AudioMode::RECORDING_DRAINING;

    modeStartTime = millis();

    Serial0.println(
        "[Audio] Recording capture stopped"
    );

    Serial0.println(
        "[Audio] Waiting for outgoing PCM queue..."
    );
}

// ============================================================
// FINISH RECORDING AFTER QUEUE DRAIN
// ============================================================

void finishRecordingAfterDrain()
{
    if (audioMode != AudioMode::RECORDING_DRAINING)
    {
        return;
    }

    if (!audioClient.isConnected())
    {
        resetAudioSession();
        return;
    }

    // Проверяем, что последний захваченный блок
    // действительно был поставлен в очередь.
    const uint32_t currentChunkId =
        microphoneManager.getAudioChunkId();

    if (currentChunkId != lastSentChunkId)
    {
        return;
    }

    // Очередь ещё содержит PCM-пакеты.
    if (audioClient.getQueuedPackets() > 0)
    {
        return;
    }

    // Все пакеты отправлены.
    if (!audioClient.sendText(
        "{\"type\":\"audio_end\"}"
    ))
    {
        Serial0.println(
            "[Audio] audio_end transmission failed"
        );

        return;
    }

    Serial0.println(
        "[Audio] Outgoing PCM queue drained"
    );

    Serial0.println(
        "[Audio] audio_end sent"
    );

    // --------------------------------------------------------
    // INITIALIZE SPEAKER PCM
    // --------------------------------------------------------

    if (!soundManager.beginPCM(
        AUDIO_SAMPLE_RATE,
        SoundManager::AudioStream::System
    ))
    {
        Serial0.println(
            "[Audio] PCM initialization failed"
        );

        audioMode = AudioMode::WAITING;

        audioInitialized = false;

        return;
    }

    audioInitialized = true;

    serverPlaybackEnded = false;

    audioMode = AudioMode::PROCESSING;

    modeStartTime = millis();

    Serial0.println(
        "[Audio] Recording finished"
    );

    Serial0.println(
        "[Audio] Waiting for server response..."
    );
}

// ============================================================
// START PLAYBACK
// ============================================================

void startPlayback()
{
    if (audioMode != AudioMode::PROCESSING)
    {
        return;
    }

    audioMode = AudioMode::PLAYING;

    lastPcmReceivedTime = millis();

    Serial0.println(
        "[Audio] PLAYING started"
    );
}

// ============================================================
// FINISH PLAYBACK
// ============================================================

void finishPlayback()
{
    soundManager.stop();

    audioInitialized = false;

    serverPlaybackEnded = false;

    audioClient.sendText(
        "{\"type\":\"playback_finished\"}"
    );

    audioMode = AudioMode::WAITING;

    modeStartTime = millis();

    Serial0.println(
        "[Audio] PLAYBACK finished"
    );

    Serial0.println(
        "[Audio] Starting next cycle..."
    );
}

// ============================================================
// MICROPHONE TRANSMISSION
// ============================================================

void transmitMicrophoneAudio()
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

    // Важно:
    // sendPcm копирует данные в очередь.
    // Поэтому после успешного вызова буфер микрофона
    // можно использовать для следующего блока.

    if (audioClient.sendPcm(
        samples,
        sampleCount
    ))
    {
        lastSentChunkId = chunkId;
    }
}

// ============================================================
// AUDIO SESSION UPDATE
// ============================================================

void updateAudioSession()
{
    if (!audioClient.isConnected())
    {
        return;
    }

    const uint32_t now = millis();

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
                now - modeStartTime >=
                RECORDING_DURATION_MS
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
            // Ожидаем первый PCM-пакет или playback_end.
            break;
        }

        // ----------------------------------------------------
        // PLAYING
        // ----------------------------------------------------

        case AudioMode::PLAYING:
        {
            // Ожидаем playback_end от Python.
            break;
        }

        // ----------------------------------------------------
        // DRAINING
        // ----------------------------------------------------

        case AudioMode::DRAINING:
        {
            if (
                now - lastPcmReceivedTime >=
                PLAYBACK_DRAIN_MS
            )
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
        "       SMARTCLOCK AUDIO TEST"
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
            "[Setup] Settings initialization failed"
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
            "[Setup] I2S initialization failed"
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
            "[Setup] SoundManager initialization failed"
        );

        return;
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
            "[Setup] Microphone initialization failed"
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

        // Для WebSocket-теста запись WAV не обязательна.
        // Продолжаем работу без файловой системы.
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
            "[Setup] WiFi initialization failed"
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
    // WIFI
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
    // WEBSOCKET UPDATE
    // --------------------------------------------------------

    // Сначала обслуживаем WebSocket:
    // принимаем события и отправляем следующий пакет очереди.

    audioClient.update();

    // --------------------------------------------------------
    // TRANSMIT PREVIOUS MICROPHONE BLOCK
    // --------------------------------------------------------

    // Перед обновлением микрофона отправляем предыдущий блок.
    // Это даёт возможность повторить попытку, если очередь
    // WebSocket временно была заполнена.

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
    // SOUND OUTPUT
    // --------------------------------------------------------

    soundManager.update();

    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    settingsManager.update();

    delay(1);
}
