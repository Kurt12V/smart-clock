
#pragma once

#include <Arduino.h>
#include <WebServer.h>

// ============================================================
// MANAGERS
// ============================================================

#include "./managers/Setting/SettingsManager.h"
#include "./managers/SPI/SPIManager.h"
#include "./managers/I2S/I2SManager.h"
#include "./managers/Sound/SoundManager.h"
#include "./managers/Sensors/SensorsManager.h"
#include "./managers/Sensors/MicrophoneManager.h"
#include "./managers/Display/BrightnessManager.h"
#include "./managers/Led/LightingManager.h"
#include "./managers/Alarm/AlarmManager.h"
#include "./managers/Alarm/AlarmController.h"
#include "./managers/WiFi/WiFiManager.h"

// ============================================================
// NETWORK MANAGERS
// ============================================================

#include "./managers/WEB/WebWiFiManager.h"
#include "./managers/NET/SmartClockMqttClient.h"
#include "./managers/NET/MqttCommandHandler.h"

// ============================================================
// AUDIO HARDWARE
// ============================================================

#include "./hardware/audio/Microphone.h"
#include "./hardware/audio/AudioRecorder.h"

#include "./managers/NET/WebSocketAudioClient.h"

// ============================================================
// SYSTEMS
// ============================================================

#include "./core/ClockSystem.h"
#include "./core/DisplaySystem.h"

// ============================================================
// APP
// ============================================================

class App
{
public:

    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    App();

    // ========================================================
    // LIFECYCLE
    // ========================================================

    bool begin();
    void update();

    // ========================================================
    // STATUS
    // ========================================================

    bool isInitialized() const
    {
        return _initialized;
    }

    bool isMqttConnected()
    {
        return _mqttClient.isConnected();
    }

private:

    // ========================================================
    // INITIALIZATION
    // ========================================================

    bool initSettings();
    bool initSPI();
    bool initI2S();
    bool initClock();
    bool initSensors();
    bool initLighting();
    bool initDisplay();
    bool initBrightness();
    bool initSound();
    bool initAlarm();
    bool initWiFi();
    bool initMqtt();
    bool initWebWiFi();
    bool initAudio();

    // ========================================================
    // VOICE AUDIO SESSION
    // ========================================================

    enum class AudioMode : uint8_t
    {
        WAITING,
        RECORDING,
        RECORDING_DRAINING,
        PROCESSING,
        PLAYING,
        DRAINING
    };

    static constexpr uint32_t AUDIO_SAMPLE_RATE = 16000;
    static constexpr uint32_t AUDIO_RECORDING_DURATION_MS = 5000;
    static constexpr uint32_t AUDIO_DIAGNOSTIC_INTERVAL_MS = 1000;
    static constexpr size_t PCM_COPY_BLOCK_BYTES = 1024;

    static App* _audioCallbackInstance;

    static void onAudioEvent(
        WebSocketAudioClient::Event event,
        const uint8_t* payload,
        size_t length
    );

    void handleAudioEvent(
        WebSocketAudioClient::Event event,
        const uint8_t* payload,
        size_t length
    );

    void updateAudio();
    void startAudioClient();
    void resetAudioSession();
    void startRecording();
    void stopRecordingCapture();
    void finishRecordingAfterDrain();
    void startPlayback();
    void finishPlayback();
    void transmitMicrophoneAudio();
    void updateAudioSession();
    void handleIncomingPCM(
        const uint8_t* payload,
        size_t length
    );

    // ========================================================
    // TELEMETRY
    // ========================================================

    void updateTelemetry();
    bool publishSensorTelemetry();

    static constexpr uint32_t TELEMETRY_INTERVAL_MS = 5000;

    // ========================================================
    // CORE MANAGERS
    // ========================================================

    SettingsManager _settings;
    SPIManager _spiManager;
    I2SManager _i2sManager;

    Microphone _microphone;
    AudioRecorder _audioRecorder;
    MicrophoneManager _microphoneManager;

    SoundManager _soundManager;
    SensorManager _sensorManager;

    ClockSystem _clockSystem;
    LightingManager _lighting;
    DisplaySystem _displaySystem;
    BrightnessManager _brightnessManager;

    // ========================================================
    // ALARMS
    // ========================================================

    AlarmManager _alarmManager;
    AlarmController _alarmController;

    // ========================================================
    // NETWORK
    // ========================================================

    WiFiManager _wifiManager;
    WebServer _webServer;
    WebWiFiManager _webWiFiManager;

    // ========================================================
    // MQTT
    // ========================================================

    SmartClockMqttClient _mqttClient;
    MqttCommandHandler _mqttCommandHandler;

    // ========================================================
    // AUDIO WEBSOCKET
    // ========================================================

    WebSocketAudioClient _audioWebSocket;

    // ========================================================
    // AUDIO STATE
    // ========================================================

    AudioMode _audioMode = AudioMode::WAITING;

    bool _audioWifiWasConnected = false;
    bool _audioInitialized = false;

    uint32_t _audioModeStartTime = 0;
    uint32_t _audioLastSentChunkId = 0;
    uint32_t _audioLastDiagnosticTime = 0;

    int16_t _pcmCopyBuffer[
        PCM_COPY_BLOCK_BYTES / sizeof(int16_t)
    ]{};

    // ========================================================
    // TELEMETRY STATE
    // ========================================================

    uint32_t _lastTelemetryPublish = 0;
    bool _mqttWasConnected = false;

    // ========================================================
    // STATE
    // ========================================================

    bool _initialized = false;
};
