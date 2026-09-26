#include "App.h"

#include "Pins.h"
#include "Config.h"
#include "Constants.h"
#include "Version.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

App::App()

    // --------------------------------------------------------
    // SoundManager
    // --------------------------------------------------------

    : _soundManager(
          _sdManager,
          _audioSettings,
          _i2sManager
      )

    // --------------------------------------------------------
    // ClockSystem
    // --------------------------------------------------------

    , _clockSystem(
          _clockSettings
      )

    // --------------------------------------------------------
    // DisplaySystem
    // --------------------------------------------------------

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
    Serial0.println("############################################");
    Serial0.println("#");
    Serial0.println("#        ESP32-S3 SMART CLOCK");
    Serial0.println("#");
    Serial0.println("#        APPLICATION STARTUP");
    Serial0.println("#");
    Serial0.println("############################################");
    Serial0.println();

    // ========================================================
    // HARDWARE
    // ========================================================

    if (!initSPI())
        Serial0.println("[APP] SPI initialization failed");

    if (!initI2S())
        Serial0.println("[APP] I2S initialization failed");

    if (!initSD())
        Serial0.println("[APP] SD initialization failed");

    // ========================================================
    // AUDIO
    // ========================================================

    if (!initSound())
        Serial0.println("[APP] Sound initialization failed");

    // ========================================================
    // SYSTEMS
    // ========================================================

    if (!initClock())
        Serial0.println("[APP] Clock initialization failed");

    if (!initSensors())
        Serial0.println("[APP] Sensors initialization failed");

    if (!initDisplay())
    {
        Serial0.println("[APP] Display initialization failed");

        _ready = false;
        return false;
    }

    if (!initInput())
        Serial0.println("[APP] Input initialization failed");

    // ========================================================
    // READY
    // ========================================================

    _ready = true;

    printReady();

    return true;
}

// ============================================================
// SPI
// ============================================================

bool App::initSPI()
{
    Serial0.println("[APP] Initializing SPI...");

    if (!_spiManager.begin())
    {
        Serial0.println("[APP] SPI ERROR");
        return false;
    }

    Serial0.println("[APP] SPI OK");

    return true;
}

// ============================================================
// I2S
// ============================================================

bool App::initI2S()
{
    Serial0.println("[APP] Initializing I2S...");

    if (!_i2sManager.begin())
    {
        Serial0.println("[APP] I2S ERROR");
        return false;
    }

    Serial0.println("[APP] I2S OK");

    return true;
}

// ============================================================
// SD
// ============================================================

bool App::initSD()
{
    Serial0.println("[APP] Initializing SD...");

    if (!_sdManager.begin(PIN_SD_CS))
    {
        Serial0.println("[APP] SD ERROR");
        return false;
    }

    Serial0.println("[APP] SD OK");

    return true;
}

// ============================================================
// SOUND
// ============================================================

bool App::initSound()
{
    Serial0.println("[APP] Initializing SoundManager...");

    if (!_soundManager.begin())
    {
        Serial0.println("[APP] SoundManager ERROR");
        return false;
    }

    Serial0.println("[APP] SoundManager OK");

    // --------------------------------------------------------
    // Startup sound
    // --------------------------------------------------------

    Serial0.print("[APP] Playing startup sound: ");
    Serial0.println(STARTUP_SOUND);

    if (!_soundManager.playWav(STARTUP_SOUND))
    {
        Serial0.println("[APP] Startup sound FAILED");

        return false;
    }

    Serial0.println("[APP] Startup sound PLAYING");

    return true;
}

// ============================================================
// CLOCK
// ============================================================

bool App::initClock()
{
    Serial0.println("[APP] Initializing ClockSystem...");

    if (!_clockSystem.begin())
    {
        Serial0.println("[APP] ClockSystem ERROR");
        return false;
    }

    Serial0.println("[APP] ClockSystem OK");

    return true;
}

// ============================================================
// SENSORS
// ============================================================

bool App::initSensors()
{
    Serial0.println("[APP] Initializing SensorManager...");

    if (!_sensorManager.begin())
    {
        Serial0.println("[APP] SensorManager ERROR");
        return false;
    }

    Serial0.println("[APP] SensorManager OK");

    return true;
}

// ============================================================
// DISPLAY
// ============================================================

bool App::initDisplay()
{
    Serial0.println("[APP] Initializing DisplaySystem...");

    if (!_displaySystem.begin())
    {
        Serial0.println("[APP] DisplaySystem ERROR");
        return false;
    }

    Serial0.println("[APP] DisplaySystem OK");

    return true;
}

// ============================================================
// INPUT
// ============================================================

bool App::initInput()
{
    Serial0.println("[APP] Initializing InputManager...");

    if (!_inputManager.begin())
    {
        Serial0.println("[APP] InputManager ERROR");
        return false;
    }

    Serial0.println("[APP] InputManager OK");

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void App::update()
{
    // --------------------------------------------------------
    // Audio must be updated continuously.
    // --------------------------------------------------------

    _soundManager.update();

    // --------------------------------------------------------
    // Clock
    // --------------------------------------------------------

    _clockSystem.update();

    // --------------------------------------------------------
    // Sensors
    // --------------------------------------------------------

    _sensorManager.update();

    // --------------------------------------------------------
    // Display
    // --------------------------------------------------------

    _displaySystem.update();

    // --------------------------------------------------------
    // Input
    // --------------------------------------------------------

    _inputManager.update();

    // --------------------------------------------------------
    // Small yield
    // --------------------------------------------------------

    delay(1);
}

// ============================================================
// READY
// ============================================================

bool App::isReady() const
{
    return _ready;
}

// ============================================================
// HEADER
// ============================================================

void App::printHeader()
{
    Serial0.println();
    Serial0.println("############################################");
    Serial0.println("# ESP32-S3 SMART CLOCK");
    Serial0.println("############################################");
    Serial0.println();
}

// ============================================================
// READY MESSAGE
// ============================================================

void App::printReady()
{
    Serial0.println();
    Serial0.println("============================================");
    Serial0.println("             SYSTEM READY");
    Serial0.println("============================================");

    if (_soundManager.isPlaying())
    {
        Serial0.println("[APP] Startup sound is playing");
    }
    else
    {
        Serial0.println("[APP] Startup sound finished");
    }

    Serial0.println();
}