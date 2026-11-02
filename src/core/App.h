
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
// NETWORK
// ============================================================

#include "./managers/WEB/WebWiFiManager.h"
#include "./managers/NET/SmartClockMqttClient.h"
#include "./managers/NET/MqttCommandHandler.h"

// ============================================================
// SYSTEMS
// ============================================================

#include "./core/ClockSystem.h"
#include "./core/DisplaySystem.h"


class App {

public:

    App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;


    // ========================================================
    // LIFECYCLE
    // ========================================================

    bool begin();

    void update();

    bool isInitialized() const;


    // ========================================================
    // STATUS
    // ========================================================

    bool isMqttConnected();


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
    bool initMqttCommandHandler();

    bool initWebWiFi();


    // ========================================================
    // MANAGERS
    // ========================================================

    SettingsManager _settings;

    SPIManager _spiManager;
    I2SManager _i2sManager;

    SoundManager _soundManager;

    SensorManager _sensorsManager;

    ClockSystem _clockSystem;

    LightingManager _lighting;

    DisplaySystem _displaySystem;

    BrightnessManager _brightnessManager;

    AlarmManager _alarmManager;
    AlarmController _alarmController;

    WiFiManager _wifiManager;


    // ========================================================
    // NETWORK
    // ========================================================

    WebServer _webServer;

    WebWiFiManager _webWiFiManager;

    SmartClockMqttClient _mqttClient;

    MqttCommandHandler _mqttCommandHandler;


    // ========================================================
    // STATE
    // ========================================================

    bool _initialized;
};
