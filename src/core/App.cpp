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

    Serial0.println();
    Serial0.println("============================================");
    Serial0.println("          SMART CLOCK STARTUP");
    Serial0.println("============================================");

    // --------------------------------------------------------
    // STEP 1: SETTINGS
    // --------------------------------------------------------

    Serial0.println("[APP] Step 1/12: Settings");

    if (!initSettings())
    {
        Serial0.println("[APP][ERROR] Settings initialization failed");
        return false;
    }

    Serial0.println("[APP] Settings initialized");

    // --------------------------------------------------------
    // STEP 2: SPI
    // --------------------------------------------------------

    Serial0.println("[APP] Step 2/12: SPI");

    if (!initSPI())
    {
        Serial0.println("[APP][ERROR] SPI initialization failed");
        return false;
    }

    Serial0.println("[APP] SPI initialized");

    // --------------------------------------------------------
    // STEP 3: I2S
    // --------------------------------------------------------

    Serial0.println("[APP] Step 3/12: I2S");

    if (!initI2S())
    {
        Serial0.println("[APP][ERROR] I2S initialization failed");
        return false;
    }

    Serial0.println("[APP] I2S initialized");

    // --------------------------------------------------------
    // STEP 4: CLOCK
    // --------------------------------------------------------

    Serial0.println("[APP] Step 4/12: Clock");

    if (!initClock())
    {
        Serial0.println("[APP][ERROR] Clock initialization failed");
        return false;
    }

    Serial0.println("[APP] Clock initialized");

    // --------------------------------------------------------
    // STEP 5: SENSORS
    // --------------------------------------------------------

    Serial0.println("[APP] Step 5/12: Sensors");

    if (!initSensors())
    {
        Serial0.println("[APP][ERROR] Sensor initialization failed");
        return false;
    }

    Serial0.println("[APP] Sensors initialized");

    // --------------------------------------------------------
    // STEP 6: LIGHTING
    // --------------------------------------------------------

    Serial0.println("[APP] Step 6/12: Lighting");

    if (!initLighting())
    {
        Serial0.println("[APP][ERROR] Lighting initialization failed");
        return false;
    }

    Serial0.println("[APP] Lighting initialized");

    // --------------------------------------------------------
    // STEP 7: DISPLAYS
    // --------------------------------------------------------

    Serial0.println("[APP] Step 7/12: Displays");

    if (!initDisplay())
    {
        Serial0.println("[APP][ERROR] Display initialization failed");
        return false;
    }

    Serial0.println("[APP] Displays initialized");

    // --------------------------------------------------------
    // STEP 8: SD CARD
    // --------------------------------------------------------

    Serial0.println("[APP] Step 8/12: SD card");

    if (!initSD())
    {
        Serial0.println("[APP][ERROR] SD card initialization failed");
        return false;
    }

    Serial0.println("[APP] SD card initialized");

    // --------------------------------------------------------
    // STEP 9: SOUND
    // --------------------------------------------------------

    Serial0.println("[APP] Step 9/12: Sound");

    if (!initSound())
    {
        Serial0.println("[APP][ERROR] Sound initialization failed");
        return false;
    }

    Serial0.println("[APP] Sound initialized");

    // --------------------------------------------------------
    // STEP 10: ALARMS
    // --------------------------------------------------------

    Serial0.println("[APP] Step 10/12: Alarms");

    if (!initAlarm())
    {
        Serial0.println("[APP][ERROR] Alarm initialization failed");
        return false;
    }

    Serial0.println("[APP] Alarms initialized");

    // --------------------------------------------------------
    // STEP 11: WI-FI
    // --------------------------------------------------------

    Serial0.println("[APP] Step 11/12: Wi-Fi");

    if (!initWiFi())
    {
        Serial0.println("[APP][ERROR] Wi-Fi initialization failed");
        return false;
    }

    Serial0.println("[APP] Wi-Fi initialized");

    // --------------------------------------------------------
    // STEP 12: WEB SERVER
    // --------------------------------------------------------

    Serial0.println("[APP] Step 12/12: Web server");

    if (!initWebWiFi())
    {
        Serial0.println("[APP][ERROR] Web server initialization failed");
        return false;
    }

    // --------------------------------------------------------
    // COMPLETE
    // --------------------------------------------------------

    _initialized = true;

    Serial0.println();
    Serial0.println("============================================");
    Serial0.println("[APP] Initialization complete");
    Serial0.println("[APP] SMART CLOCK READY");
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
// SD CARD
// ============================================================

bool App::initSD()
{
    Serial0.println("[APP] Initializing SD card...");

    // --------------------------------------------------------
    // DESELECT ALL SPI DEVICES
    // --------------------------------------------------------
    // Все CS должны быть HIGH перед обращением к SD-карте.
    // Это диагностическая мера для общей SPI-шины.

    const int csPins[] =
    {
        PIN_TFT_CS1,
        PIN_TFT_CS2,
        PIN_TFT_CS3,
        PIN_TFT_CS4,
        PIN_SD_CS
    };

    const size_t csCount = sizeof(csPins) / sizeof(csPins[0]);

    for (size_t i = 0; i < csCount; ++i)
    {
        pinMode(csPins[i], OUTPUT);
        digitalWrite(csPins[i], HIGH);
    }

    delay(10);

    // --------------------------------------------------------
    // VERIFY CHIP SELECT LEVELS
    // --------------------------------------------------------

    bool csStateValid = true;

    for (size_t i = 0; i < csCount; ++i)
    {
        const int pin = csPins[i];
        const int level = digitalRead(pin);

        Serial0.printf(
            "[APP][SPI] CS GPIO %d = %d\n",
            pin,
            level
        );

        if (level != HIGH)
        {
            csStateValid = false;

            Serial0.printf(
                "[APP][SPI][ERROR] CS GPIO %d is not HIGH\n",
                pin
            );
        }
    }

    if (!csStateValid)
    {
        Serial0.println(
            "[APP][SD][ERROR] Invalid chip select state"
        );

        return false;
    }

    // --------------------------------------------------------
    // INITIALIZE SD CARD
    // --------------------------------------------------------

    Serial0.println("[APP][SD] All CS pins are HIGH");
    Serial0.println("[APP][SD] Calling SDManager::begin()...");

    const bool result = _sdManager.begin(PIN_SD_CS);

    if (!result)
    {
        Serial0.println();
        Serial0.println("============================================");
        Serial0.println("[APP][SD][ERROR] SD initialization failed");
        Serial0.println("[APP][SD][ERROR] CS pin: GPIO 35");
        Serial0.println("[APP][SD][ERROR] SPI bus may be busy");
        Serial0.println("============================================");

        return false;
    }

    Serial0.println("[APP][SD] SD card mounted successfully");

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
// ALARMS
// ============================================================

bool App::initAlarm()
{
    Serial0.println("[APP] Initializing alarms...");
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
    Serial0.println("[APP] Configuring web server...");

    // Регистрируем маршруты управления Wi-Fi.
    _webWiFiManager.setupRoutes(_webServer);

    // Запускаем HTTP-сервер.
    _webServer.begin();

    Serial0.println("[APP] HTTP server started");
    Serial0.println("[APP] Port: 80");

    return true;
}