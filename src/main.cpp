
#include <Arduino.h>

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

#include <Arduino.h>
#include <LittleFS.h>
#include <cstring>



// ============================================================
// CONFIGURATION
// ============================================================

static constexpr char AUDIO_SERVER_HOST[] = "192.168.1.93";
static constexpr uint16_t AUDIO_SERVER_PORT = 8765;
static constexpr char AUDIO_SERVER_PATH[] = "/audio";

static constexpr char DEVICE_ID[] = "smartclock-01";

static constexpr uint32_t AUDIO_SAMPLE_RATE = 16000;

// Only recording has a fixed duration.
static constexpr uint32_t RECORDING_DURATION_MS = 5000;

// Wait for the PCM queue to drain after playback_end.
static constexpr uint32_t PLAYBACK_DRAIN_MS = 500;

// Maximum incoming PCM packet: 2048 bytes.
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
    PROCESSING,
    PLAYING,
    DRAINING
};

static AudioMode audioMode = AudioMode::WAITING;

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
void finishRecording();
void startPlayback();
void finishPlayback();

void updateAudioSession();
void transmitMicrophoneAudio();

void onAudioEvent(
    WebSocketAudioClient::Event event,
    const uint8_t* payload,
    size_t length
);

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

            audioMode = AudioMode::WAITING;

            audioInitialized = false;
            serverPlaybackEnded = false;

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

            microphoneManager.stopListening();

            soundManager.stop();

            audioInitialized = false;
            serverPlaybackEnded = false;

            audioMode = AudioMode::WAITING;

            break;
        }

        // ----------------------------------------------------
        // PCM RECEIVED
        // ----------------------------------------------------

        case WebSocketAudioClient::Event::PCM_RECEIVED:
        {
            if (!audioInitialized)
            {
                break;
            }

            if (
                audioMode != AudioMode::PROCESSING &&
                audioMode != AudioMode::PLAYING
            )
            {
                break;
            }

            if (
                payload == nullptr ||
                length == 0 ||
                length % sizeof(int16_t) != 0
            )
            {
                break;
            }

            const size_t sampleCount =
                length / sizeof(int16_t);

            if (sampleCount > PCM_BUFFER_SAMPLES)
            {
                Serial0.println(
                    "[Audio] PCM packet too large"
                );

                break;
            }

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
                    "[Audio] PCM queue overflow"
                );

                break;
            }

            lastPcmReceivedTime = millis();

            if (audioMode == AudioMode::PROCESSING)
            {
                startPlayback();
            }

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

            // WebSocket payload is not guaranteed
            // to be null-terminated.
            char message[256];

            const size_t copyLength =
                (length < sizeof(message) - 1)
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

            // Python confirms that all PCM packets
            // have been sent.
            if (strstr(message, "\"playback_end\"") != nullptr)
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

                    // If no PCM was received, there is
                    // nothing to play.
                    if (audioMode == AudioMode::PROCESSING)
                    {
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

    audioClient.sendText(
        "{\"type\":\"audio_start\","
        "\"sample_rate\":16000,"
        "\"format\":\"s16le\","
        "\"channels\":1}"
    );

    Serial0.println(
        "[Audio] RECORDING started (5 seconds)"
    );
}

// ============================================================
// FINISH RECORDING
// ============================================================

void finishRecording()
{
    microphoneManager.stopListening();

    audioClient.sendText(
        "{\"type\":\"audio_end\"}"
    );

    if (!soundManager.beginPCM(
        AUDIO_SAMPLE_RATE,
        SoundManager::AudioStream::System
    ))
    {
        Serial0.println(
            "[Audio] PCM initialization failed"
        );

        audioMode = AudioMode::WAITING;

        return;
    }

    audioInitialized = true;

    serverPlaybackEnded = false;

    audioMode = AudioMode::PROCESSING;

    modeStartTime = millis();

    Serial0.println(
        "[Audio] RECORDING finished"
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
    if (audioMode != AudioMode::RECORDING)
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
                finishRecording();
            }

            break;
        }

        // ----------------------------------------------------
        // PROCESSING
        // ----------------------------------------------------

        case AudioMode::PROCESSING:
        {
            // No timeout.
            // Wait for the first PCM packet or playback_end.

            break;
        }

        // ----------------------------------------------------
        // PLAYING
        // ----------------------------------------------------

        case AudioMode::PLAYING:
        {
            // Wait for playback_end from Python.

            break;
        }

        // ----------------------------------------------------
        // DRAINING
        // ----------------------------------------------------

        case AudioMode::DRAINING:
        {
            // Wait until the final PCM packet has had
            // enough time to leave the SoundManager queue.

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

    if (!wifiConnected)
    {
        if (wifiWasConnected)
        {
            Serial0.println(
                "[WiFi] Connection lost"
            );

            audioClient.stop();

            microphoneManager.stopListening();

            soundManager.stop();

            audioInitialized = false;
            serverPlaybackEnded = false;

            audioMode = AudioMode::WAITING;

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
    // MICROPHONE
    // --------------------------------------------------------

    microphoneManager.update();

    // --------------------------------------------------------
    // WEBSOCKET
    // --------------------------------------------------------

    audioClient.update();

    // --------------------------------------------------------
    // SEND MICROPHONE PCM
    // --------------------------------------------------------

    transmitMicrophoneAudio();

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
