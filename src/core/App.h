#pragma once

#include <Arduino.h>

#include "Settings.h"

#include "./core/ClockSystem.h"
#include "./core/DisplaySystem.h"

#include "./managers/SensorsManager.h"
#include "./managers/InputManager.h"
#include "./managers/SPIManager.h"
#include "./managers/I2SManager.h"
#include "./managers/SDManager.h"
#include "./managers/SoundManager.h"

#include "./managers/BluetoothManager.h"
#include "./managers/BluetoothSubscriptionManager.h"
#include "./hardware/bluetooth/BluetoothPublisher.h"

// ============================================================
// APP
// ============================================================

class App
{
public:

    App();

    bool begin();
    void update();

    bool isReady() const;

private:

    // ========================================================
    // INITIALIZATION
    // ========================================================

    bool initSPI();
    bool initI2S();
    bool initSD();
    bool initSound();

    bool initClock();
    bool initSensors();

    bool initDisplay();
    bool initInput();

    bool initBluetooth();

    // ========================================================
    // STATE
    // ========================================================

    bool _ready;

    // ========================================================
    // SETTINGS
    // ========================================================

    Settings::Clock _clockSettings;
    Settings::Audio _audioSettings;

    // ========================================================
    // MANAGERS
    // ========================================================

    SPIManager _spiManager;
    I2SManager _i2sManager;
    SDManager _sdManager;

    SoundManager _soundManager;

    InputManager _inputManager;
    SensorManager _sensorManager;

    // ========================================================
    // BLUETOOTH
    // ========================================================

    BluetoothManager _bluetoothManager;

    BluetoothSubscriptionManager _bluetoothSubscriptions;

    BluetoothPublisher _bluetoothPublisher;

    // ========================================================
    // CORE SYSTEMS
    // ========================================================

    ClockSystem _clockSystem;

    DisplaySystem _displaySystem;

    // ========================================================
    // STARTUP SOUND
    // ========================================================

    static constexpr const char* STARTUP_SOUND =
        "/audio/system/START_SYSTEM.wav";
};