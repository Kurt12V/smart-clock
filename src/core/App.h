#pragma once

#include <Arduino.h>

#include "managers/SettingsManager.h"
#include "managers/SPIManager.h"
#include "managers/I2SManager.h"
#include "managers/SDManager.h"
#include "managers/SoundManager.h"
#include "managers/InputManager.h"
#include "managers/SensorsManager.h"
#include "managers/WebServerManager.h"

#include "core/ClockSystem.h"
#include "core/DisplaySystem.h"
// #include "core/BluetoothSystem.h"

class App
{
public:

    App();

    bool begin();

    void update();

    bool isReady() const;

private:

    // ========================================================
    // INITIALIZATION STEPS
    // ========================================================

    bool initSettings();
    bool initSPI();
    bool initI2S();
    bool initSD();
    bool initSound();
    bool initClock();
    bool initSensors();
    bool initDisplay();
    bool initInput();
    // bool initBluetooth();
    bool initWebServer();

    // ========================================================
    // STATE
    // ========================================================

    bool _ready;

    // ========================================================
    // SETTINGS
    //
    // ВАЖНО: должен идти ПЕРВЫМ — от него зависят
    // ClockSystem, DisplaySystem, SoundManager, WebServer.
    // ========================================================

    SettingsManager _settings;

    // ========================================================
    // MANAGERS
    // ========================================================

    SPIManager _spiManager;
    I2SManager _i2sManager;
    SDManager  _sdManager;

    SoundManager _soundManager;

    InputManager  _inputManager;
    SensorManager _sensorManager;

    // ========================================================
    // CORE SYSTEMS
    //
    // Порядок объявления = порядок конструирования.
    // _clockSystem ДО _displaySystem, потому что
    // DisplaySystem принимает его по ссылке.
    // ========================================================

    ClockSystem _clockSystem;

    DisplaySystem _displaySystem;

    // ========================================================
    // WEB SERVER
    // ========================================================

    WebServerManager _webServer;

    // ========================================================
    // BLUETOOTH
    // ========================================================

    // BluetoothSystem _bluetoothSystem;
};