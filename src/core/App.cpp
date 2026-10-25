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
        Serial0.println("[APP] Already initialized");
        return true;
    }

    Serial0.begin(115200);
    delay(300);

    Serial0.println();
    Serial0.println("============================================");
    Serial0.println("          SMART CLOCK STARTUP");
    Serial0.println("============================================");

    // --------------------------------------------------------
    // 1. SETTINGS
    // --------------------------------------------------------

    Serial0.println("[APP] Step 1/12: Settings");

    if (!initSettings())
    {
        Serial0.println("[APP][ERROR] Settings initialization failed");
        return false;
    }

    Serial0.println("[APP] Settings initialized");

    // --------------------------------------------------------
    // 2. SPI
    // --------------------------------------------------------

    Serial0.println("[APP] Step 2/12: SPI");

    if (!initSPI())
    {
        Serial0.println("[APP][ERROR] SPI initialization failed");
        return false;
    }

    Serial0.println("[APP] SPI initialized");

    // --------------------------------------------------------
    // 3. I2S
    // --------------------------------------------------------

    Serial0.println("[APP] Step 3/12: I2S");

    if (!initI2S())
    {
        Serial0.println("[APP][ERROR] I2S initialization failed");
        return false;
    }

    Serial0.println("[APP] I2S initialized");

    // --------------------------------------------------------
    // 4. CLOCK
    // --------------------------------------------------------

    Serial0.println("[APP] Step 4/12: Clock");

    if (!initClock())
    {
        Serial0.println("[APP][ERROR] Clock initialization failed");
        return false;
    }

    Serial0.println("[APP] Clock initialized");

    // --------------------------------------------------------
    // 5. SENSORS
    // --------------------------------------------------------

    Serial0.println("[APP] Step 5/12: Sensors");

    if (!initSensors())
    {
        Serial0.println("[APP][ERROR] Sensors initialization failed");
        return false;
    }

    Serial0.println("[APP] Sensors initialized");

    // --------------------------------------------------------
    // 6. LIGHTING
    // --------------------------------------------------------

    Serial0.println("[APP] Step 6/12: Lighting");

    if (!initLighting())
    {
        Serial0.println("[APP][ERROR] Lighting initialization failed");
        return false;
    }

    Serial0.println("[APP] Lighting initialized");

    // --------------------------------------------------------
    // 7. DISPLAY
    // --------------------------------------------------------

    // Сначала инициализируем дисплеи.
    // SD-карта пока не монтируется.

    Serial0.println("[APP] Step 7/12: Displays");

    if (!initDisplay())
    {
        Serial0.println("[APP][ERROR] Display initialization failed");
        return false;
    }

    Serial0.println("[APP] Displays initialized");

    // --------------------------------------------------------
    // 8. SD CARD
    // --------------------------------------------------------

    // После инициализации дисплеев проверяем SD.
    // SPIManager должен продолжать использовать общую SPI-шину.
    // Перед обращением к SD линии CS дисплеев должны быть HIGH.

    Serial0.println("[APP] Step 8/12: SD card");

    if (!initSD())
    {
        Serial0.println("[APP][ERROR] SD initialization failed");
        return false;
    }

    Serial0.println("[APP] SD card initialized");

    // --------------------------------------------------------
    // 9. SOUND
    // --------------------------------------------------------

    // SoundManager зависит от SDManager и I2SManager.

    Serial0.println("[APP] Step 9/12: Sound");

    if (!initSound())
    {
        Serial0.println("[APP][ERROR] Sound initialization failed");
        return false;
    }

    Serial0.println("[APP] Sound initialized");

    // --------------------------------------------------------
    // 10. ALARMS
    // --------------------------------------------------------

    Serial0.println("[APP] Step 10/12: Alarms");

    if (!initAlarm())
    {
        Serial0.println("[APP][ERROR] Alarm initialization failed");
        return false;
    }

    Serial0.println("[APP] Alarm manager initialized");

    // --------------------------------------------------------
    // 11. WI-FI
    // --------------------------------------------------------

    Serial0.println("[APP] Step 11/12: Wi-Fi");

    if (!initWiFi())
    {
        Serial0.println("[APP][ERROR] Wi-Fi initialization failed");
        return false;
    }

    Serial0.println("[APP] Wi-Fi initialized");

    // --------------------------------------------------------
    // 12. WEB SERVER
    // --------------------------------------------------------

    Serial0.println("[APP] Step 12/12: Web server");

    if (!initWebWiFi())
    {
        Serial0.println("[APP][ERROR] Web server initialization failed");
        return false;
    }

    // --------------------------------------------------------
    // READY
    // --------------------------------------------------------

    _initialized = true;

    Serial0.println();
    Serial0.println("============================================");
    Serial0.println("[APP] SMART CLOCK READY");
    Serial0.println("============================================");
    Serial0.println();

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
    // SETTINGS
    // --------------------------------------------------------

    _settings.update();

    // --------------------------------------------------------
    // CLOCK
    // --------------------------------------------------------

    _clockSystem.update();

    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    _sensorManager.update();

    // --------------------------------------------------------
    // LIGHTING
    // --------------------------------------------------------

    _lighting.update();

    // --------------------------------------------------------
    // DISPLAY
    // --------------------------------------------------------

    _displaySystem.update();

    // --------------------------------------------------------
    // SOUND
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
    // HTTP SERVER
    // --------------------------------------------------------

    _webServer.handleClient();
}

// ============================================================
// SETTINGS
// ============================================================

bool App::initSettings()
{
    Serial0.println("[APP] Initializing settings...");

    return _settings.begin();
}

// ============================================================
// SPI
// ============================================================

bool App::initSPI()
{
    Serial0.println("[APP] Initializing SPI...");

    return _spiManager.begin();
}

// ============================================================
// I2S
// ============================================================

bool App::initI2S()
{
    Serial0.println("[APP] Initializing I2S...");

    return _i2sManager.begin();
}

// ============================================================
// CLOCK
// ============================================================

bool App::initClock()
{
    Serial0.println("[APP] Initializing clock...");

    return _clockSystem.begin();
}

// ============================================================
// SENSORS
// ============================================================

bool App::initSensors()
{
    Serial0.println("[APP] Initializing sensors...");

    return _sensorManager.begin();
}

// ============================================================
// LIGHTING
// ============================================================

bool App::initLighting()
{
    Serial0.println("[APP] Initializing lighting...");

    return _lighting.begin();
}

// ============================================================
// DISPLAY
// ============================================================

bool App::initDisplay()
{
    Serial0.println("[APP] Initializing displays...");

    return _displaySystem.begin();
}

// ============================================================
// SD
// ============================================================

bool App::initSD()
{
    Serial0.println("[APP] Initializing SD card...");

    if (!_sdManager.begin(PIN_SD_CS))
    {
        Serial0.println("[APP][ERROR] SD card mount failed");
        return false;
    }

    Serial0.println("[APP] SD card mounted successfully");

    return true;
}

// ============================================================
// SOUND
// ============================================================

bool App::initSound()
{
    Serial0.println("[APP] Initializing sound...");

    return _soundManager.begin();
}

// ============================================================
// ALARM
// ============================================================

bool App::initAlarm()
{
    Serial0.println("[APP] Initializing alarm manager...");

    return _alarmManager.begin();
}

// ============================================================
// WI-FI
// ============================================================

bool App::initWiFi()
{
    Serial0.println("[APP] Initializing Wi-Fi...");

    return _wifiManager.begin();
}

// ============================================================
// WEB SERVER
// ============================================================

bool App::initWebWiFi()
{
    Serial0.println("[APP] Configuring web server routes...");

    _webWiFiManager.setupRoutes(_webServer);

    Serial0.println("[APP] Starting HTTP server...");

    _webServer.begin();

    Serial0.println("[APP] HTTP server started on port 80");

    return true;
}