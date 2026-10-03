#include "App.h"

#include <Arduino.h>

#include "./Config.h"
#include "./Pins.h"
#include "./Constants.h"
#include "./Version.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

App::App()

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    : _ready(false)


    // --------------------------------------------------------
    // BASIC MANAGERS
    // --------------------------------------------------------

    , _settings()

    , _spiManager()

    , _i2sManager()

    , _sdManager()

    , _soundManager(
        _sdManager,
        _settings,
        _i2sManager
    )

    , _inputManager()

    , _sensorManager()


    // --------------------------------------------------------
    // CLOCK
    // --------------------------------------------------------

    , _clockSystem(
        _settings
    )


    // --------------------------------------------------------
    // COB LEDS
    // --------------------------------------------------------

    , _cob1(
        PIN_COB1,
        1,
        Config::COB_PWM_FREQUENCY,
        Config::COB_PWM_RESOLUTION
    )

    , _cob2(
        PIN_COB2,
        2,
        Config::COB_PWM_FREQUENCY,
        Config::COB_PWM_RESOLUTION
    )

    , _cob3(
        PIN_COB3,
        3,
        Config::COB_PWM_FREQUENCY,
        Config::COB_PWM_RESOLUTION
    )

    , _cob4(
        PIN_COB4,
        4,
        Config::COB_PWM_FREQUENCY,
        Config::COB_PWM_RESOLUTION
    )


    // --------------------------------------------------------
    // COB MANAGER
    // --------------------------------------------------------

    , _cobManager(
        _cob1,
        _cob2,
        _cob3,
        _cob4,
        _settings
    )


    // --------------------------------------------------------
    // LED MATRIX
    // --------------------------------------------------------

    , _matrixManager(
        PIN_LED_MATRIX,
        _settings
    )


    // --------------------------------------------------------
    // LIGHT SYSTEM
    // --------------------------------------------------------

    , _lightSystem(
        _cobManager,
        _matrixManager
    )


    // --------------------------------------------------------
    // ALARM MANAGER
    // --------------------------------------------------------

    , _alarmManager(
        _clockSystem,
        _sdManager
    )


    // --------------------------------------------------------
    // DISPLAY
    // --------------------------------------------------------

    , _displaySystem(
        _settings,
        _clockSystem,
        _sensorManager
    )


    // --------------------------------------------------------
    // WEB SERVER
    // --------------------------------------------------------

    , _webServer()
{
}


// ============================================================
// BEGIN
// ============================================================

bool App::begin()
{
    Serial0.println();
    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("        SMART CLOCK STARTUP");
    Serial0.println("========================================");
    Serial0.println();


    // ========================================================
    // SETTINGS
    // ========================================================

    if (!initSettings())
    {
        Serial0.println("[APP] Settings initialization failed");
        return false;
    }


    // ========================================================
    // SPI
    // ========================================================

    if (!initSPI())
    {
        Serial0.println("[APP] SPI initialization failed");
        return false;
    }


    // ========================================================
    // I2S
    // ========================================================

    if (!initI2S())
    {
        Serial0.println("[APP] I2S initialization failed");
        return false;
    }


    // ========================================================
    // SD
    // ========================================================

    if (!initSD())
    {
        Serial0.println("[APP] SD initialization failed");
        return false;
    }


    // ========================================================
    // SOUND
    // ========================================================

    if (!initSound())
    {
        Serial0.println("[APP] Sound initialization failed");
        return false;
    }


    // ========================================================
    // CLOCK
    // ========================================================

    if (!initClock())
    {
        Serial0.println("[APP] Clock initialization failed");
        return false;
    }


    // ========================================================
    // ALARM
    // ========================================================

    if (!initAlarm())
    {
        Serial0.println("[APP] Alarm initialization failed");
        return false;
    }


    // ========================================================
    // SENSORS
    // ========================================================

    if (!initSensors())
    {
        Serial0.println("[APP] Sensor initialization failed");
        return false;
    }


    // ========================================================
    // LIGHT
    // ========================================================

    if (!initLight())
    {
        Serial0.println("[APP] Light initialization failed");
        return false;
    }


    // ========================================================
    // DISPLAY
    // ========================================================

    if (!initDisplay())
    {
        Serial0.println("[APP] Display initialization failed");
        return false;
    }


    // ========================================================
    // INPUT
    // ========================================================

    if (!initInput())
    {
        Serial0.println("[APP] Input initialization failed");
        return false;
    }


    // ========================================================
    // WEB SERVER
    // ========================================================

    if (!initWebServer())
    {
        Serial0.println("[APP] Web server initialization failed");
        return false;
    }


    // ========================================================
    // READY
    // ========================================================

    _ready = true;

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("        SMART CLOCK READY");
    Serial0.println("========================================");
    Serial0.println();

    return true;
}


// ============================================================
// UPDATE
// ============================================================

void App::update()
{
    if (!_ready)
        return;


    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    _settings.update();


    // --------------------------------------------------------
    // INPUT
    // --------------------------------------------------------

    _inputManager.update();


    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    _sensorManager.update();


    // --------------------------------------------------------
    // CLOCK
    // --------------------------------------------------------

    _clockSystem.update();


    // --------------------------------------------------------
    // ALARMS
    // --------------------------------------------------------

    _alarmManager.update();


    // --------------------------------------------------------
    // LIGHT
    // --------------------------------------------------------

    _lightSystem.update();


    // --------------------------------------------------------
    // DISPLAY
    // --------------------------------------------------------

    _displaySystem.update();


    // --------------------------------------------------------
    // SOUND
    // --------------------------------------------------------

    _soundManager.update();


    // --------------------------------------------------------
    // WEB
    // --------------------------------------------------------

    _webServer.update();
}


// ============================================================
// READY
// ============================================================

bool App::isReady() const
{
    return _ready;
}


// ============================================================
// SETTINGS
// ============================================================

bool App::initSettings()
{
    Serial0.println("[APP] Initializing settings...");

    // Если SettingsManager::begin() возвращает bool,
    // используем его.

    if (!_settings.begin())
    {
        Serial0.println("[APP] Settings failed");
        return false;
    }

    Serial0.println("[APP] Settings OK");

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
        Serial0.println("[APP] SPI failed");
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
        Serial0.println("[APP] I2S failed");
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
        Serial0.println("[APP] SD failed");
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
    Serial0.println("[APP] Initializing sound...");

    if (!_soundManager.begin())
    {
        Serial0.println("[APP] Sound failed");
        return false;
    }

    Serial0.println("[APP] Sound OK");

    return true;
}


// ============================================================
// CLOCK
// ============================================================

bool App::initClock()
{
    Serial0.println("[APP] Initializing clock...");

    if (!_clockSystem.begin())
    {
        Serial0.println("[APP] Clock failed");
        return false;
    }

    Serial0.println("[APP] Clock OK");

    return true;
}


// ============================================================
// ALARM
// ============================================================

bool App::initAlarm()
{
    Serial.println();
    Serial.println(
        "[APP] Initializing alarm manager..."
    );

    if (!_alarmManager.begin())
    {
        Serial.println(
            "[APP] Alarm manager failed"
        );

        return false;
    }

    Serial.println(
        "[APP] Alarm manager OK"
    );

    return true;
}


// ============================================================
// SENSORS
// ============================================================

bool App::initSensors()
{
    Serial0.println("[APP] Initializing sensors...");

    if (!_sensorManager.begin())
    {
        Serial0.println("[APP] Sensors failed");
        return false;
    }

    Serial0.println("[APP] Sensors OK");

    return true;
}


// ============================================================
// LIGHT
// ============================================================

bool App::initLight()
{
    Serial0.println("[APP] Initializing lighting...");

    if (!_lightSystem.begin())
    {
        Serial0.println("[APP] Lighting failed");
        return false;
    }

    Serial0.println("[APP] Lighting OK");

    return true;
}


// ============================================================
// DISPLAY
// ============================================================

bool App::initDisplay()
{
    Serial0.println("[APP] Initializing display...");

    if (!_displaySystem.begin())
    {
        Serial0.println("[APP] Display failed");
        return false;
    }

    Serial0.println("[APP] Display OK");

    return true;
}


// ============================================================
// INPUT
// ============================================================

bool App::initInput()
{
    Serial0.println("[APP] Initializing input...");

    if (!_inputManager.begin())
    {
        Serial0.println("[APP] Input failed");
        return false;
    }

    Serial0.println("[APP] Input OK");

    return true;
}


// ============================================================
// WEB SERVER

bool App::initWebServer()
{
    Serial0.println(
        "[APP] Initializing web server..."
    );


    if (
        !_webServer.begin(
            _settings,
            _sdManager,
            _soundManager,
            _clockSystem,
            _alarmManager,
            Config::WIFI_SSID,
            Config::WIFI_PASSWORD
        )
    )
    {
        Serial0.println(
            "[APP] Web server failed"
        );

        return false;
    }


    Serial0.println(
        "[APP] Web server OK"
    );


    return true;
}


