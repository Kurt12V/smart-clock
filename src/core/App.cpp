#include "App.h"

#include "Config.h"
#include "Pins.h"
#include "Constants.h"
#include "Version.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

App::App()
    : _ready(false),

      // ========================================================
      // SETTINGS
      // ========================================================

      _clockSettings(),
      _audioSettings(),

      // ========================================================
      // MANAGERS
      // ========================================================

      _spiManager(),
      _i2sManager(),
      _sdManager(),

      _soundManager(
          _sdManager,
          _audioSettings,
          _i2sManager
      ),

      _inputManager(),
      _sensorManager(),

      // ========================================================
      // CORE SYSTEMS
      // ========================================================

      _clockSystem(
          _clockSettings
      ),

      _displaySystem(
          _clockSystem,
          _sensorManager
      ),

      // ========================================================
      // BLUETOOTH
      // ========================================================

      _bluetoothSystem(
          _sensorManager,
          _clockSystem
      )
{
}

// ============================================================
// BEGIN
// ============================================================

bool App::begin()
{
    Serial.println();
    Serial.println("========================================");
    Serial.println("        SMART CLOCK STARTING");
    Serial.println("========================================");

    _ready = false;

    // ========================================================
    // SPI
    // ========================================================

    if (!initSPI())
    {
        Serial.println(
            "[APP] SPI initialization failed"
        );

        return false;
    }

    // ========================================================
    // I2S
    // ========================================================

    if (!initI2S())
    {
        Serial.println(
            "[APP] I2S initialization failed"
        );

        return false;
    }

    // ========================================================
    // SD
    // ========================================================

    if (!initSD())
    {
        Serial.println(
            "[APP] SD initialization failed"
        );

        return false;
    }

    // ========================================================
    // SOUND
    // ========================================================

    if (!initSound())
    {
        Serial.println(
            "[APP] Sound initialization failed"
        );

        return false;
    }

    // ========================================================
    // CLOCK
    // ========================================================

    if (!initClock())
    {
        Serial.println(
            "[APP] Clock initialization failed"
        );

        return false;
    }

    // ========================================================
    // SENSORS
    // ========================================================

    if (!initSensors())
    {
        Serial.println(
            "[APP] Sensors initialization failed"
        );

        return false;
    }

    // ========================================================
    // DISPLAY
    // ========================================================

    if (!initDisplay())
    {
        Serial.println(
            "[APP] Display initialization failed"
        );

        return false;
    }

    // ========================================================
    // INPUT
    // ========================================================

    if (!initInput())
    {
        Serial.println(
            "[APP] Input initialization failed"
        );

        return false;
    }

    // ========================================================
    // BLUETOOTH
    // ========================================================

    if (!initBluetooth())
    {
        Serial.println(
            "[APP] Bluetooth initialization failed"
        );

        return false;
    }

    // ========================================================
    // STARTUP SOUND
    // ========================================================

    Serial.println(
        "[APP] Playing startup sound..."
    );

    if (!_soundManager.playWav(
        STARTUP_SOUND
    ))
    {
        Serial.println(
            "[APP] WARNING: startup sound failed"
        );
    }

    // ========================================================
    // READY
    // ========================================================

    _ready = true;

    Serial.println();
    Serial.println("========================================");
    Serial.println("        SMART CLOCK READY");
    Serial.println("========================================");

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void App::update()
{
    if (!_ready)
        return;

    // ========================================================
    // INPUT
    // ========================================================

    _inputManager.update();

    // ========================================================
    // SENSORS
    // ========================================================

    _sensorManager.update();

    // ========================================================
    // CLOCK
    // ========================================================

    _clockSystem.update();

    // ========================================================
    // DISPLAY
    // ========================================================

    _displaySystem.update();

    // ========================================================
    // SOUND
    // ========================================================

    _soundManager.update();

    // ========================================================
    // BLUETOOTH
    //
    // BluetoothSystem internally handles:
    //
    // BLE transport
    // command queue
    // protocol parsing
    // command handling
    // subscriptions
    // publisher
    // notifications
    //
    // IMPORTANT:
    // Do NOT call BluetoothManager or BluetoothPublisher
    // directly here.
    // ========================================================

    _bluetoothSystem.update();
}

// ============================================================
// READY
// ============================================================

bool App::isReady() const
{
    return _ready;
}

// ============================================================
// INIT SPI
// ============================================================

bool App::initSPI()
{
    Serial.println(
        "[APP] Initializing SPI..."
    );

    if (!_spiManager.begin())
    {
        Serial.println(
            "[APP] SPI failed"
        );

        return false;
    }

    Serial.println(
        "[APP] SPI OK"
    );

    return true;
}

// ============================================================
// INIT I2S
// ============================================================

bool App::initI2S()
{
    Serial.println(
        "[APP] Initializing I2S..."
    );

    if (!_i2sManager.begin())
    {
        Serial.println(
            "[APP] I2S failed"
        );

        return false;
    }

    Serial.println(
        "[APP] I2S OK"
    );

    return true;
}

// ============================================================
// INIT SD
// ============================================================

bool App::initSD()
{
    Serial.println(
        "[APP] Initializing SD..."
    );

    /*
     * SDManager::begin() requires the CS pin.
     *
     * The SD CS pin must be defined in Pins.h.
     */

    if (!_sdManager.begin(PIN_SD_CS))
    {
        Serial.println(
            "[APP] SD failed"
        );

        return false;
    }

    Serial.println(
        "[APP] SD OK"
    );

    return true;
}

// ============================================================
// INIT SOUND
// ============================================================

bool App::initSound()
{
    Serial.println(
        "[APP] Initializing sound..."
    );

    if (!_soundManager.begin())
    {
        Serial.println(
            "[APP] Sound failed"
        );

        return false;
    }

    Serial.println(
        "[APP] Sound OK"
    );

    return true;
}

// ============================================================
// INIT CLOCK
// ============================================================

bool App::initClock()
{
    Serial.println(
        "[APP] Initializing clock..."
    );

    if (!_clockSystem.begin())
    {
        Serial.println(
            "[APP] Clock failed"
        );

        return false;
    }

    Serial.println(
        "[APP] Clock OK"
    );

    return true;
}

// ============================================================
// INIT SENSORS
// ============================================================

bool App::initSensors()
{
    Serial.println(
        "[APP] Initializing sensors..."
    );

    if (!_sensorManager.begin())
    {
        Serial.println(
            "[APP] Sensors failed"
        );

        return false;
    }

    Serial.println(
        "[APP] Sensors OK"
    );

    return true;
}

// ============================================================
// INIT DISPLAY
// ============================================================

bool App::initDisplay()
{
    Serial.println(
        "[APP] Initializing display..."
    );

    if (!_displaySystem.begin())
    {
        Serial.println(
            "[APP] Display failed"
        );

        return false;
    }

    Serial.println(
        "[APP] Display OK"
    );

    return true;
}

// ============================================================
// INIT INPUT
// ============================================================

bool App::initInput()
{
    Serial.println(
        "[APP] Initializing input..."
    );

    if (!_inputManager.begin())
    {
        Serial.println(
            "[APP] Input failed"
        );

        return false;
    }

    Serial.println(
        "[APP] Input OK"
    );

    return true;
}

// ============================================================
// INIT BLUETOOTH
// ============================================================

bool App::initBluetooth()
{
    Serial.println();
    Serial.println(
        "========================================"
    );
    Serial.println(
        "[APP] Initializing Bluetooth..."
    );
    Serial.println(
        "========================================"
    );

    /*
     * BluetoothSystem owns:
     *
     * BluetoothManager
     * BluetoothSubscriptionManager
     * BluetoothPublisher
     *
     * It also processes commands:
     *
     * Android
     *    ↓
     * NimBLE RX
     *    ↓
     * BluetoothManager queue
     *    ↓
     * BluetoothSystem::processCommand()
     *    ↓
     * BluetoothProtocol
     *    ↓
     * BluetoothCommands
     *    ↓
     * response
     */

    if (!_bluetoothSystem.begin())
    {
        Serial.println(
            "[APP] Bluetooth failed"
        );

        return false;
    }

    Serial.println(
        "[APP] Bluetooth OK"
    );

    Serial.println(
        "[APP] BLE device name: SmartClock"
    );

    Serial.println(
        "========================================"
    );

    return true;
}
