
#include "App.h"

#include <Arduino.h>
#include <LittleFS.h>

#include <algorithm>
#include <cstring>
#include <ctime>
#include <cstdio>

// ============================================================
// AUDIO SERVER CONFIGURATION
// ============================================================

// IP-адрес компьютера с Python WebSocket-сервером.
static constexpr char AUDIO_SERVER_HOST[] = "192.168.1.93";

// Порт WebSocket-сервера.
static constexpr uint16_t AUDIO_SERVER_PORT = 8765;

// WebSocket endpoint.
static constexpr char AUDIO_SERVER_PATH[] = "/audio";

// Идентификатор устройства.
static constexpr char AUDIO_DEVICE_ID[] = "smartclock-01";

App* App::_audioCallbackInstance = nullptr;


// ============================================================
// CONSTRUCTOR
// ============================================================

App::App()
    : _settings()
    , _spiManager()
    , _i2sManager()
    , _microphone(_i2sManager)
    , _audioRecorder(_microphone, LittleFS)
    , _microphoneManager(_microphone, _audioRecorder)
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
    , _mqttCommandHandler()
    , _audioWebSocket()
    , _lastTelemetryPublish(0)
    , _mqttWasConnected(false)
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

    // if (!initAudio())
    // {
    //     Serial0.println(
    //         "[APP][ERROR] Audio WebSocket initialization failed"
    //     );
    //
    //     return false;
    // }

    Serial0.println(
        "[APP] Audio WebSocket initialized"
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

    _mqttClient.update();


    // --------------------------------------------------------
    // TELEMETRY
    // --------------------------------------------------------

    updateTelemetry();


    // --------------------------------------------------------
    // HTTP SERVER
    // --------------------------------------------------------

    _webServer.handleClient();


    // --------------------------------------------------------
    // AUDIO WEBSOCKET
    // --------------------------------------------------------

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

    if (!_settings.begin())
    {
        Serial0.println(
            "[APP][ERROR] Settings initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Resetting startup lighting settings..."
    );


    // --------------------------------------------------------
    // LED MATRIX
    // --------------------------------------------------------

    _settings.set(
        SettingsManager::Param::MATRIX_ENABLED,
        0
    );

    _settings.set(
        SettingsManager::Param::MATRIX_EFFECT,
        0
    );


    // --------------------------------------------------------
    // COB LIGHTING
    // --------------------------------------------------------

    _settings.set(
        SettingsManager::Param::COB_ENABLED,
        0
    );

    _settings.set(
        SettingsManager::Param::COB_EFFECT,
        0
    );


    // --------------------------------------------------------
    // SAVE SETTINGS TO NVS
    // --------------------------------------------------------

    _settings.save(
        SettingsManager::Param::MATRIX_ENABLED
    );

    _settings.save(
        SettingsManager::Param::MATRIX_EFFECT
    );

    _settings.save(
        SettingsManager::Param::COB_ENABLED
    );

    _settings.save(
        SettingsManager::Param::COB_EFFECT
    );


    // --------------------------------------------------------
    // LOG
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Matrix disabled"
    );

    Serial0.println(
        "[APP] Matrix effect reset"
    );

    Serial0.println(
        "[APP] COB disabled"
    );

    Serial0.println(
        "[APP] COB effect reset"
    );

    Serial0.println(
        "[APP] Startup lighting settings saved"
    );

    return true;
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
    Serial0.println();
    Serial0.println("[APP] Initializing MQTT...");

    // --------------------------------------------------------
    // 1. MQTT CLIENT
    // --------------------------------------------------------

    if (!_mqttClient.begin())
    {
        Serial0.println(
            "[APP] ERROR: MQTT client initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] MQTT client initialized"
    );


    // --------------------------------------------------------
    // 2. MQTT COMMAND HANDLER
    // --------------------------------------------------------

    if (!_mqttCommandHandler.begin(
            _mqttClient,
            _settings,
            _clockSystem,
            _wifiManager,
            _alarmManager
        ))
    {
        Serial0.println(
            "[APP] ERROR: MQTT command handler initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] MQTT command handler initialized"
    );


    // --------------------------------------------------------
    // 3. COMPLETE
    // --------------------------------------------------------

    Serial0.println(
        "[APP] MQTT initialization completed"
    );

    return true;
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
// UPDATE TELEMETRY
// ============================================================

void App::updateTelemetry()
{
    const bool connected = _mqttClient.isConnected();

    if (!connected)
    {
        _mqttWasConnected = false;
        return;
    }

    const uint32_t now = millis();


    // --------------------------------------------------------
    // FIRST CONNECTION / RECONNECTION
    // --------------------------------------------------------

    if (!_mqttWasConnected)
    {
        _mqttWasConnected = true;
        _lastTelemetryPublish = now;

        Serial0.println(
            "[APP][MQTT] Подключение обнаружено: отправка телеметрии"
        );

        publishSensorTelemetry();

        return;
    }


    // --------------------------------------------------------
    // PERIODIC TELEMETRY
    // --------------------------------------------------------

    if (now - _lastTelemetryPublish >= TELEMETRY_INTERVAL_MS)
    {
        _lastTelemetryPublish = now;

        publishSensorTelemetry();
    }
}


// ============================================================
// PUBLISH SENSOR TELEMETRY
// ============================================================

bool App::publishSensorTelemetry()
{
    // --------------------------------------------------------
    // CHECK MQTT CONNECTION
    // --------------------------------------------------------

    if (!_mqttClient.isConnected())
    {
        Serial0.println(
            "[APP][MQTT] Нет подключения к MQTT"
        );

        return false;
    }


    // --------------------------------------------------------
    // GET SENSOR DATA
    // --------------------------------------------------------

    const SensorData& data = _sensorManager.getData();

    const bool hasTemperature = data.temperatureValid;
    const bool hasHumidity = data.humidityValid;
    const bool hasIlluminance = data.lightValid;

    if (!hasTemperature &&
        !hasHumidity &&
        !hasIlluminance)
    {
        Serial0.println(
            "[APP][MQTT] Нет валидных показаний датчиков"
        );

        return false;
    }


    // --------------------------------------------------------
    // GET LOCAL DATETIME
    // --------------------------------------------------------

    const time_t localTime = _clockSystem.getLocalTime();

    struct tm timeInfo {};

    if (gmtime_r(&localTime, &timeInfo) == nullptr)
    {
        Serial0.println(
            "[APP][MQTT] Ошибка получения локального времени"
        );

        return false;
    }


    // --------------------------------------------------------
    // FORMAT DATETIME
    // --------------------------------------------------------

    char datetime[24] = {};

    const int datetimeLength = snprintf(
        datetime,
        sizeof(datetime),
        "%04d-%02d-%02dT%02d:%02d:%02d",
        timeInfo.tm_year + 1900,
        timeInfo.tm_mon + 1,
        timeInfo.tm_mday,
        timeInfo.tm_hour,
        timeInfo.tm_min,
        timeInfo.tm_sec
    );

    if (datetimeLength < 0 ||
        static_cast<size_t>(datetimeLength) >= sizeof(datetime))
    {
        Serial0.println(
            "[APP][MQTT] Ошибка форматирования даты и времени"
        );

        return false;
    }


    // --------------------------------------------------------
    // CREATE JSON DOCUMENT
    // --------------------------------------------------------

    JsonDocument document;

    document["device_id"] = _mqttClient.getDeviceId();

    JsonObject sensors =
        document["sensors"].to<JsonObject>();


    // --------------------------------------------------------
    // TEMPERATURE
    // --------------------------------------------------------

    if (hasTemperature)
    {
        sensors["temperature"] = data.temperature;
    }


    // --------------------------------------------------------
    // HUMIDITY
    // --------------------------------------------------------

    if (hasHumidity)
    {
        sensors["humidity"] = data.humidity;
    }


    // --------------------------------------------------------
    // ILLUMINANCE
    // --------------------------------------------------------

    if (hasIlluminance)
    {
        sensors["illuminance"] = data.lightLux;
    }


    // --------------------------------------------------------
    // PUBLISH TELEMETRY
    // --------------------------------------------------------

    const bool published = _mqttClient.publishTelemetry(
        document,
        datetime
    );

    if (!published)
    {
        Serial0.println(
            "[APP][MQTT] Ошибка публикации телеметрии"
        );

        return false;
    }


    // --------------------------------------------------------
    // SUCCESS
    // --------------------------------------------------------

    Serial0.printf(
        "[APP][MQTT] Телеметрия опубликована: %s\n",
        datetime
    );

    return true;
}
