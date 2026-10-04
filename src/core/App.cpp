#include "App.h"

#include "./Config.h"
#include "./Pins.h"
#include "./Constants.h"
#include "./Version.h"


// ============================================================
// WIFI
// ============================================================

static constexpr const char* WIFI_SSID =
    "tpl45";

static constexpr const char* WIFI_PASSWORD =
    "12713714";


// ============================================================
// CONSTRUCTOR
// ============================================================

App::App()
    : _settings(),

      _spiManager(),

      _i2sManager(),

      _sdManager(),

      _soundManager(
          _sdManager,
          _settings,
          _i2sManager
      ),

      _sensorManager(),

      _clockSystem(
          _settings
      ),

      _lighting(
          _settings
      ),

      _displaySystem(
          _settings,
          _clockSystem,
          _sensorManager
      ),

      _webServer(),

      _initialized(false)
{
}


// ============================================================
// BEGIN
// ============================================================

bool App::begin()
{
    if (!initSettings())
        return false;

    if (!initSPI())
        return false;

    if (!initI2S())
        return false;

    if (!initSD())
        return false;

    if (!initSound())
        return false;

    if (!initClock())
        return false;

    if (!initSensors())
        return false;

    if (!initLighting())
        return false;

    if (!initDisplay())
        return false;

    if (!initWebServer())
        return false;

    _initialized = true;

    return true;
}


// ============================================================
// UPDATE
// ============================================================

void App::update()
{
    if (!_initialized)
        return;

    _settings.update();

    _clockSystem.update();

    _sensorManager.update();

    _lighting.update();

    _displaySystem.update();

    _soundManager.update();

    _webServer.update();
}


// ============================================================
// SETTINGS
// ============================================================

bool App::initSettings()
{
    return _settings.begin();
}


// ============================================================
// SPI
// ============================================================

bool App::initSPI()
{
    return _spiManager.begin();
}


// ============================================================
// I2S
// ============================================================

bool App::initI2S()
{
    return _i2sManager.begin();
}


// ============================================================
// SD
// ============================================================

bool App::initSD()
{
    return _sdManager.begin(
        PIN_SD_CS
    );
}


// ============================================================
// SOUND
// ============================================================

bool App::initSound()
{
    return _soundManager.begin();
}


// ============================================================
// CLOCK
// ============================================================

bool App::initClock()
{
    return _clockSystem.begin();
}


// ============================================================
// SENSORS
// ============================================================

bool App::initSensors()
{
    return _sensorManager.begin();
}


// ============================================================
// LIGHTING
// ============================================================

bool App::initLighting()
{
    return _lighting.begin();
}


// ============================================================
// DISPLAY
// ============================================================

bool App::initDisplay()
{
    return _displaySystem.begin();
}


// ============================================================
// WEB SERVER
// ============================================================

bool App::initWebServer()
{
    return _webServer.begin(
        _settings,
        _sdManager,
        _soundManager,
        WIFI_SSID,
        WIFI_PASSWORD
    );
}
