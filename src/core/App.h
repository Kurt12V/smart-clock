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
    bool initWebWiFi();
    bool initMqtt();

    // ========================================================
    // CORE MANAGERS
    // ========================================================

    SettingsManager _settings;
    SPIManager _spiManager;
    I2SManager _i2sManager;
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

    // ========================================================
    // STATE
    // ========================================================

    bool _initialized = false;
};
