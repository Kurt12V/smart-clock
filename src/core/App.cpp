#include "App.h"

#include <Arduino.h>

// ============================================================
// AUDIO SERVER CONFIGURATION
// ============================================================

// IP-адрес компьютера с Python WebSocket-сервером.
static constexpr char AUDIO_SERVER_HOST[] = "192.168.1.100";

// Порт WebSocket-сервера.
static constexpr uint16_t AUDIO_SERVER_PORT = 8765;

// WebSocket endpoint.
static constexpr char AUDIO_SERVER_PATH[] = "/audio";

// Идентификатор устройства.
static constexpr char AUDIO_DEVICE_ID[] = "smartclock-01";


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
    , _brightnessManager()
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
    , _mqttClient()
    // , _audioWebSocket()
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
        "[APP] Step 1/14: Settings"
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
        "[APP] Step 2/14: SPI"
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
        "[APP] Step 3/14: I2S"
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
        "[APP] Step 4/14: Clock"
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
        "[APP] Step 5/14: Sensors"
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
        "[APP] Step 6/14: Lighting"
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
        "[APP] Step 7/14: Displays"
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
    // STEP 8: BRIGHTNESS
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 8/14: Brightness"
    );

    if (!initBrightness())
    {
        Serial0.println(
            "[APP][ERROR] Brightness initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Brightness initialized"
    );


    // --------------------------------------------------------
    // STEP 9: SOUND
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 9/14: Sound"
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
    // STEP 10: ALARMS
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 10/14: Alarms"
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
    // STEP 11: WI-FI
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 11/14: Wi-Fi"
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
    // STEP 12: MQTT
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 12/14: MQTT"
    );

    if (!initMqtt())
    {
        Serial0.println(
            "[APP][ERROR] MQTT client initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] MQTT client initialized"
    );


    // --------------------------------------------------------
    // STEP 13: WEB INTERFACE
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 13/14: Web interface"
    );

    if (!initWebWiFi())
    {
        Serial0.println(
            "[APP][ERROR] Web interface initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Web interface initialized"
    );


    // --------------------------------------------------------
    // STEP 14: AUDIO WEBSOCKET
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 14/14: Audio WebSocket"
    );

    // WebSocket пока отключён.
    // Сначала проверяем MQTT-связь с сервером.

    Serial0.println(
        "[APP] Audio WebSocket is currently disabled"
    );


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
    // BRIGHTNESS
    // --------------------------------------------------------

    _brightnessManager.update();


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
    // MQTT
    // --------------------------------------------------------

    // Обслуживание MQTT-клиента.
    // Метод нужно вызывать постоянно, без больших задержек.
    // Клиент обрабатывает входящие сообщения и соединение.

    _mqttClient.update();


    // --------------------------------------------------------
    // HTTP SERVER
    // --------------------------------------------------------

    _webServer.handleClient();


    // --------------------------------------------------------
    // AUDIO WEBSOCKET
    // --------------------------------------------------------

    // WebSocket будет подключён отдельно после MQTT.
    // _audioWebSocket.update();
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
// BRIGHTNESS
// ============================================================

bool App::initBrightness()
{
    Serial0.println(
        "[APP] Initializing BrightnessManager..."
    );

    _brightnessManager.begin(
        &_settings,
        &_sensorManager,
        &_clockSystem
    );

    if (!_brightnessManager.isInitialized())
    {
        Serial0.println(
            "[APP][ERROR] BrightnessManager initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] BrightnessManager initialized"
    );

    return true;
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
// MQTT
// ============================================================

bool App::initMqtt()
{
    Serial0.println(
        "[APP] Initializing MQTT client..."
    );

    // Адрес брокера, порт, логин, пароль и ID устройства
    // задаются в SmartClockMqttClient.h:
    //
    // namespace SmartClockMqttConfig
    //
    // begin() использует эти настройки.

    return _mqttClient.begin();
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


// ============================================================
// AUDIO WEBSOCKET INITIALIZATION
// ============================================================

// WebSocket пока не запускается.
// После проверки MQTT сюда можно добавить подключение
// WebSocket-клиента для передачи аудиоданных.