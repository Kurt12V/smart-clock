#include "App.h"

#include <Arduino.h>

// ============================================================
// CONSTRUCTOR
// ============================================================

App::App()
    : _settings()
    , _spiManager()
    , _i2sManager()
    , _soundManager(
        _settings,
        _i2sManager
    )
    , _sensorManager()
    , _clockSystem(
        _settings
    )
    , _lighting(
        _settings
    )
    , _displaySystem(
        _settings,
        _clockSystem,
        _sensorManager,
        _spiManager
    )
    , _alarmManager(
        _clockSystem
    )
    , _alarmController(
        _alarmManager
    )
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
        Serial0.println(
            "[APP] Already initialized"
        );

        return true;
    }

    Serial0.println();
    Serial0.println(
        "============================================"
    );
    Serial0.println(
        "          SMART CLOCK STARTUP"
    );
    Serial0.println(
        "============================================"
    );
    Serial0.println();

    // --------------------------------------------------------
    // STEP 1: SETTINGS
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 1/11: Settings"
    );

    if (!initSettings())
    {
        Serial0.println(
            "[APP][ERROR] Settings initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Settings initialized"
    );

    // --------------------------------------------------------
    // STEP 2: SPI
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 2/11: SPI"
    );

    if (!initSPI())
    {
        Serial0.println(
            "[APP][ERROR] SPI initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] SPI initialized"
    );

    // --------------------------------------------------------
    // STEP 3: I2S
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 3/11: I2S"
    );

    if (!initI2S())
    {
        Serial0.println(
            "[APP][ERROR] I2S initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] I2S initialized"
    );

    // --------------------------------------------------------
    // STEP 4: CLOCK
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 4/11: Clock"
    );

    if (!initClock())
    {
        Serial0.println(
            "[APP][ERROR] Clock initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Clock initialized"
    );

    // --------------------------------------------------------
    // STEP 5: SENSORS
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 5/11: Sensors"
    );

    if (!initSensors())
    {
        Serial0.println(
            "[APP][ERROR] Sensor initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Sensors initialized"
    );

    // --------------------------------------------------------
    // STEP 6: LIGHTING
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 6/11: Lighting"
    );

    if (!initLighting())
    {
        Serial0.println(
            "[APP][ERROR] Lighting initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Lighting initialized"
    );

    // --------------------------------------------------------
    // STEP 7: DISPLAYS
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 7/11: Displays"
    );

    if (!initDisplay())
    {
        Serial0.println(
            "[APP][ERROR] Display initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Displays initialized"
    );

    // --------------------------------------------------------
    // STEP 8: SOUND
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 8/11: Sound"
    );

    if (!initSound())
    {
        Serial0.println(
            "[APP][ERROR] Sound initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Sound initialized"
    );

    // --------------------------------------------------------
    // STEP 9: ALARMS
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 9/11: Alarms"
    );

    if (!initAlarm())
    {
        Serial0.println(
            "[APP][ERROR] Alarm initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Alarms initialized"
    );

    // --------------------------------------------------------
    // STEP 10: WI-FI
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 10/11: Wi-Fi"
    );

    if (!initWiFi())
    {
        Serial0.println(
            "[APP][ERROR] Wi-Fi initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Wi-Fi initialized"
    );

    // --------------------------------------------------------
    // STEP 11: WEB / WI-FI CONFIGURATION
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 11/11: Web interface"
    );

    if (!initWebWiFi())
    {
        Serial0.println(
            "[APP][ERROR] Web interface initialization failed"
        );

        return false;
    }

    // --------------------------------------------------------
    // INITIALIZATION COMPLETE
    // --------------------------------------------------------

    _initialized = true;

    Serial0.println();
    Serial0.println(
        "============================================"
    );
    Serial0.println(
        "[APP] Initialization complete"
    );
    Serial0.println(
        "[APP] SMART CLOCK READY"
    );
    Serial0.println(
        "============================================"
    );
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
    // DISPLAYS
    // --------------------------------------------------------

    _displaySystem.update();

    // --------------------------------------------------------
    // SOUND
    // --------------------------------------------------------

    _soundManager.update();

    // --------------------------------------------------------
    // ALARM MANAGER
    // --------------------------------------------------------

    _alarmManager.update();

    // --------------------------------------------------------
    // ALARM CONTROLLER
    // --------------------------------------------------------

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
    Serial0.println(
        "[APP] Initializing settings..."
    );

    return _settings.begin();
}

// ============================================================
// SPI
// ============================================================

bool App::initSPI()
{
    Serial0.println(
        "[APP] Initializing SPI..."
    );

    return _spiManager.begin();
}

// ============================================================
// I2S
// ============================================================

bool App::initI2S()
{
    Serial0.println(
        "[APP] Initializing I2S..."
    );

    return _i2sManager.begin();
}

// ============================================================
// CLOCK
// ============================================================

bool App::initClock()
{
    Serial0.println(
        "[APP] Initializing clock..."
    );

    return _clockSystem.begin();
}

// ============================================================
// SENSORS
// ============================================================

bool App::initSensors()
{
    Serial0.println(
        "[APP] Initializing sensors..."
    );

    return _sensorManager.begin();
}

// ============================================================
// LIGHTING
// ============================================================

bool App::initLighting()
{
    Serial0.println(
        "[APP] Initializing lighting..."
    );

    return _lighting.begin();
}

// ============================================================
// DISPLAY
// ============================================================

bool App::initDisplay()
{
    Serial0.println(
        "[APP] Initializing displays..."
    );

    return _displaySystem.begin();
}

// ============================================================
// SOUND
// ============================================================

bool App::initSound()
{
    Serial0.println(
        "[APP] Initializing sound..."
    );

    return _soundManager.begin();
}

// ============================================================
// ALARMS
// ============================================================

bool App::initAlarm()
{
    Serial0.println(
        "[APP] Initializing alarms..."
    );

    return _alarmManager.begin();
}

// ============================================================
// WI-FI
// ============================================================

bool App::initWiFi()
{
    Serial0.println(
        "[APP] Initializing Wi-Fi..."
    );

    return _wifiManager.begin();
}

// ============================================================
// WEB / WI-FI CONFIGURATION
// ============================================================

bool App::initWebWiFi()
{
    Serial0.println(
        "[APP] Configuring web interface..."
    );

    // --------------------------------------------------------
    // REGISTER WI-FI ROUTES
    // --------------------------------------------------------

    _webWiFiManager.setupRoutes(
        _webServer
    );

    // --------------------------------------------------------
    // START HTTP SERVER
    // --------------------------------------------------------

    _webServer.begin();

    Serial0.println(
        "[APP] HTTP server started"
    );

    Serial0.println(
        "[APP] HTTP port: 80"
    );

    return true;
}