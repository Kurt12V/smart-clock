#include "App.h"

#include "Pins.h"
#include "Config.h"
#include "Constants.h"
#include "Version.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

App::App()

    : _ready(false),

      // --------------------------------------------------------
      // Sound
      // --------------------------------------------------------

      _soundManager(
          _sdManager,
          _audioSettings,
          _i2sManager
      ),

      // --------------------------------------------------------
      // Bluetooth
      // --------------------------------------------------------

      _bluetoothManager(),

      _bluetoothSubscriptions(),

      _bluetoothPublisher(
          _bluetoothManager,
          _bluetoothSubscriptions,
          _sensorManager,
          _clockSystem
      ),

      // --------------------------------------------------------
      // Clock
      // --------------------------------------------------------

      _clockSystem(
          _clockSettings
      ),

      // --------------------------------------------------------
      // Display
      // --------------------------------------------------------

      _displaySystem(
          _clockSystem,
          _sensorManager
      )
{
}

// ============================================================
// BEGIN
// ============================================================

bool App::begin()
{
    Serial0.println();
    Serial0.println("============================================");
    Serial0.println("        ESP32-S3 SMART CLOCK");
    Serial0.println("        APPLICATION START");
    Serial0.println("============================================");
    Serial0.println();

    // ========================================================
    // SPI
    // ========================================================

    if (!initSPI())
    {
        Serial0.println(
            "[APP] SPI initialization failed"
        );
    }

    // ========================================================
    // I2S
    // ========================================================

    if (!initI2S())
    {
        Serial0.println(
            "[APP] I2S initialization failed"
        );
    }

    // ========================================================
    // SD
    // ========================================================

    if (!initSD())
    {
        Serial0.println(
            "[APP] SD initialization failed"
        );
    }

    // ========================================================
    // SOUND
    // ========================================================

    if (!initSound())
    {
        Serial0.println(
            "[APP] Sound initialization failed"
        );
    }

    // ========================================================
    // CLOCK
    // ========================================================

    if (!initClock())
    {
        Serial0.println(
            "[APP] Clock initialization failed"
        );
    }

    // ========================================================
    // SENSORS
    // ========================================================

    if (!initSensors())
    {
        Serial0.println(
            "[APP] Sensor initialization failed"
        );
    }

    // ========================================================
    // DISPLAY
    // ========================================================

    if (!initDisplay())
    {
        Serial0.println(
            "[APP] Display initialization failed"
        );

        _ready = false;

        return false;
    }

    // ========================================================
    // INPUT
    // ========================================================

    if (!initInput())
    {
        Serial0.println(
            "[APP] Input initialization failed"
        );
    }

    // ========================================================
    // BLUETOOTH
    // ========================================================

    if (!initBluetooth())
    {
        Serial0.println(
            "[APP] Bluetooth initialization failed"
        );
    }

    // ========================================================
    // READY
    // ========================================================

    _ready = true;

    Serial0.println();
    Serial0.println("============================================");
    Serial0.println("             SMART CLOCK READY");
    Serial0.println("============================================");
    Serial0.println();

    Serial0.println(
        "[SYSTEM] BLE waiting for connection..."
    );

    return true;
}

// ============================================================
// SPI
// ============================================================

bool App::initSPI()
{
    Serial0.println(
        "[INIT] SPIManager..."
    );

    if (!_spiManager.begin())
    {
        Serial0.println(
            "[INIT] SPIManager FAILED"
        );

        return false;
    }

    Serial0.println(
        "[INIT] SPIManager OK"
    );

    return true;
}

// ============================================================
// I2S
// ============================================================

bool App::initI2S()
{
    Serial0.println(
        "[INIT] I2SManager..."
    );

    if (!_i2sManager.begin())
    {
        Serial0.println(
            "[INIT] I2SManager FAILED"
        );

        return false;
    }

    Serial0.println(
        "[INIT] I2SManager OK"
    );

    return true;
}

// ============================================================
// SD
// ============================================================

bool App::initSD()
{
    Serial0.println(
        "[INIT] SDManager..."
    );

    if (!_sdManager.begin(PIN_SD_CS))
    {
        Serial0.println(
            "[INIT] SDManager FAILED"
        );

        return false;
    }

    Serial0.println(
        "[INIT] SDManager OK"
    );

    return true;
}

// ============================================================
// SOUND
// ============================================================

bool App::initSound()
{
    Serial0.println(
        "[INIT] SoundManager..."
    );

    if (!_soundManager.begin())
    {
        Serial0.println(
            "[INIT] SoundManager FAILED"
        );

        return false;
    }

    Serial0.println(
        "[INIT] SoundManager OK"
    );

    // --------------------------------------------------------
    // Startup sound
    // --------------------------------------------------------

    Serial0.println(
        "[INIT] Starting startup sound..."
    );

    if (!_soundManager.playWav(
            STARTUP_SOUND
        ))
    {
        Serial0.println(
            "[INIT] Startup sound FAILED"
        );

        return false;
    }

    Serial0.println(
        "[INIT] Startup sound PLAYING"
    );

    return true;
}

// ============================================================
// CLOCK
// ============================================================

bool App::initClock()
{
    Serial0.println(
        "[INIT] ClockSystem..."
    );

    if (!_clockSystem.begin())
    {
        Serial0.println(
            "[INIT] ClockSystem FAILED"
        );

        return false;
    }

    Serial0.println(
        "[INIT] ClockSystem OK"
    );

    return true;
}

// ============================================================
// SENSORS
// ============================================================

bool App::initSensors()
{
    Serial0.println(
        "[INIT] SensorManager..."
    );

    if (!_sensorManager.begin())
    {
        Serial0.println(
            "[INIT] SensorManager FAILED"
        );

        return false;
    }

    Serial0.println(
        "[INIT] SensorManager OK"
    );

    return true;
}

// ============================================================
// DISPLAY
// ============================================================

bool App::initDisplay()
{
    Serial0.println(
        "[INIT] DisplaySystem..."
    );

    if (!_displaySystem.begin())
    {
        Serial0.println(
            "[INIT] DisplaySystem FAILED"
        );

        return false;
    }

    Serial0.println(
        "[INIT] DisplaySystem OK"
    );

    return true;
}

// ============================================================
// INPUT
// ============================================================

bool App::initInput()
{
    Serial0.println(
        "[INIT] InputManager..."
    );

    if (!_inputManager.begin())
    {
        Serial0.println(
            "[INIT] InputManager FAILED"
        );

        return false;
    }

    Serial0.println(
        "[INIT] InputManager OK"
    );

    return true;
}

// ============================================================
// BLUETOOTH
// ============================================================

bool App::initBluetooth()
{
    // ========================================================
    // BLUETOOTH MANAGER
    // ========================================================

    Serial0.println(
        "[INIT] BluetoothManager..."
    );

    if (!_bluetoothManager.begin())
    {
        Serial0.println(
            "[INIT] BluetoothManager FAILED"
        );

        return false;
    }

    Serial0.println(
        "[INIT] BluetoothManager OK"
    );

    // ========================================================
    // BLUETOOTH PUBLISHER
    // ========================================================

    Serial0.println(
        "[INIT] BluetoothPublisher..."
    );

    if (!_bluetoothPublisher.begin())
    {
        Serial0.println(
            "[INIT] BluetoothPublisher FAILED"
        );

        return false;
    }

    Serial0.println(
        "[INIT] BluetoothPublisher OK"
    );

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void App::update()
{
    // ========================================================
    // SOUND
    // ========================================================

    _soundManager.update();

    // ========================================================
    // CLOCK
    // ========================================================

    _clockSystem.update();

    // ========================================================
    // SENSORS
    // ========================================================

    _sensorManager.update();

    // ========================================================
    // DISPLAY
    // ========================================================

    _displaySystem.update();

    // ========================================================
    // INPUT
    // ========================================================

    _inputManager.update();

    // ========================================================
    // BLUETOOTH COMMANDS
    // ========================================================

    _bluetoothManager.update();

    // ========================================================
    // BLUETOOTH PUBLISHER
    // ========================================================

    _bluetoothPublisher.update();

    // ========================================================
    // CPU YIELD
    // ========================================================

    yield();
}

// ============================================================
// READY
// ============================================================

bool App::isReady() const
{
    return _ready;
}