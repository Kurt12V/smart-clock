#include "App.h"

#include "Pins.h"
#include "Config.h"
#include "Constants.h"
#include "Version.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

App::App()

    : _soundManager(
          _sdManager,
          _audioSettings,
          _i2sManager
      )

    , _clockSystem(
          _clockSettings
      )

    , _displaySystem(
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

    // ========================================================
    // SPI
    // ========================================================

    Serial0.println("[APP] SPI...");

    if (!_spiManager.begin())
    {
        Serial0.println("[APP] SPI ERROR");
    }
    else
    {
        Serial0.println("[APP] SPI OK");
    }

    // ========================================================
    // I2S
    // ========================================================

    Serial0.println("[APP] I2S...");

    if (!_i2sManager.begin())
    {
        Serial0.println("[APP] I2S ERROR");
    }
    else
    {
        Serial0.println("[APP] I2S OK");
    }

    // ========================================================
    // SD
    // ========================================================

    Serial0.println("[APP] SD...");

    if (!_sdManager.begin(PIN_SD_CS))
    {
        Serial0.println("[APP] SD ERROR");
    }
    else
    {
        Serial0.println("[APP] SD OK");
    }

    // ========================================================
    // SOUND MANAGER
    // ========================================================

    Serial0.println("[APP] SoundManager...");

    if (!_soundManager.begin())
    {
        Serial0.println("[APP] SoundManager ERROR");
    }
    else
    {
        Serial0.println("[APP] SoundManager OK");
    }

    // ========================================================
    // CLOCK
    // ========================================================

    Serial0.println("[APP] ClockSystem...");

    if (!_clockSystem.begin())
    {
        Serial0.println("[APP] ClockSystem ERROR");
    }
    else
    {
        Serial0.println("[APP] ClockSystem OK");
    }

    // ========================================================
    // SENSORS
    // ========================================================

    Serial0.println("[APP] SensorManager...");

    if (!_sensorManager.begin())
    {
        Serial0.println("[APP] SensorManager ERROR");
    }
    else
    {
        Serial0.println("[APP] SensorManager OK");
    }

    // ========================================================
    // DISPLAY
    // ========================================================

    Serial0.println("[APP] DisplaySystem...");

    if (!_displaySystem.begin())
    {
        Serial0.println("[APP] DisplaySystem ERROR");

        _ready = false;

        return false;
    }

    Serial0.println("[APP] DisplaySystem OK");

    // ========================================================
    // INPUT
    // ========================================================

    Serial0.println("[APP] InputManager...");

    if (!_inputManager.begin())
    {
        Serial0.println("[APP] InputManager ERROR");
    }
    else
    {
        Serial0.println("[APP] InputManager OK");
    }

    // ========================================================
    // APPLICATION READY
    // ========================================================

    _ready = true;

    Serial0.println();
    Serial0.println("============================================");
    Serial0.println("             SYSTEM READY");
    Serial0.println("============================================");

    // ========================================================
    // STARTUP SOUND
    // ========================================================

    Serial0.println("[APP] Starting startup sound...");

    if (!_soundManager.isInitialized())
    {
        Serial0.println("[APP] SoundManager is not initialized");
    }
    else
    {
        if (_soundManager.playWav(STARTUP_SOUND))
        {
            Serial0.println("[APP] Startup sound started");
        }
        else
        {
            Serial0.println("[APP] Startup sound FAILED");
        }
    }

    Serial0.println("[APP] begin() finished");

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
    // CPU YIELD
    // ========================================================

    yield();
}

// ============================================================
// STATE
// ============================================================

bool App::isReady() const
{
    return _ready;
}

// ============================================================
// READY
// ============================================================

void App::printReady()
{
    Serial0.println();
    Serial0.println("============================================");
    Serial0.println("SYSTEM READY");
    Serial0.println("============================================");
}