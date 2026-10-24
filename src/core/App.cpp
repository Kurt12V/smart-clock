#include "App.h"
#include "Pins.h"

#include <Arduino.h>

// ============================================================
// CONSTRUCTOR
// ============================================================

App::App()
    : _settings()
    , _spiManager()
    , _i2sManager()
    , _sdManager()
    , _soundManager(
        _sdManager,
        _settings,
        _i2sManager
    )
    , _sensorManager()
    , _clockSystem(_settings)
    , _lighting(_settings)
    , _displaySystem(
        _settings,
        _clockSystem,
        _sensorManager,
        _spiManager
    )
    , _alarmManager(
        _sdManager,
        _clockSystem
    )
    , _alarmController(_alarmManager)
    , _wifiManager()
    , _webServer(80)
    , _webWiFiManager(
        _wifiManager,
        _settings,
        _clockSystem
    )
    , _initialized(false)
{
}

// ============================================================
// BEGIN
// ============================================================

bool App::begin()
{
    if (_initialized)
    {
        return true;
    }

    if (!initSettings()) return false;
    if (!initSPI()) return false;
    if (!initI2S()) return false;
    if (!initSD()) return false;
    if (!initSound()) return false;
    if (!initClock()) return false;
    if (!initSensors()) return false;
    if (!initLighting()) return false;
    if (!initDisplay()) return false;
    if (!initAlarm()) return false;

    // Wi-Fi: подключение или создание точки доступа.
    if (!initWiFi()) return false;

    // HTTP: регистрация маршрутов и запуск сервера.
    if (!initWebWiFi()) return false;

    _initialized = true;

    Serial0.println();
    Serial0.println("============================================");
    Serial0.println("[APP] Initialization complete");
    Serial0.println("============================================");

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void App::update()
{
    if (!_initialized)
    {
        return;
    }

    // --------------------------------------------------------
    // SETTINGS AND CLOCK
    // --------------------------------------------------------

    _settings.update();
    _clockSystem.update();

    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    _sensorManager.update();

    // --------------------------------------------------------
    // LIGHTING AND DISPLAY
    // --------------------------------------------------------

    _lighting.update();
    _displaySystem.update();

    // --------------------------------------------------------
    // AUDIO
    // --------------------------------------------------------

    _soundManager.update();

    // --------------------------------------------------------
    // ALARMS
    // --------------------------------------------------------

    _alarmManager.update();
    _alarmController.update();

    // --------------------------------------------------------
    // WI-FI
    // --------------------------------------------------------

    _wifiManager.update();

    // --------------------------------------------------------
    // WEB SERVER
    // --------------------------------------------------------

    _webServer.handleClient();
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
    return _sdManager.begin(PIN_SD_CS);
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
// ALARM
// ============================================================

bool App::initAlarm()
{
    return _alarmManager.begin();
}

// ============================================================
// WI-FI
// ============================================================

bool App::initWiFi()
{
    return _wifiManager.begin();
}

// ============================================================
// WEB WI-FI
// ============================================================

bool App::initWebWiFi()
{
    // Регистрируем маршруты управления Wi-Fi.
    _webWiFiManager.setupRoutes(_webServer);

    // Запускаем HTTP-сервер.
    _webServer.begin();

    Serial0.println("[APP] HTTP server started");
    Serial0.println("[APP] Port: 80");

    return true;
}