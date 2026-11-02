
#include "App.h"

#include <Arduino.h>


// ============================================================
// CONSTRUCTOR
// ============================================================

App::App()
    : _settings(),

      _spiManager(),

      _i2sManager(),

      _soundManager(
          _settings,
          _i2sManager
      ),

      _sensorsManager(),

      _clockSystem(
          _settings
      ),

      _lighting(
          _settings
      ),

      _displaySystem(
          _settings,
          _clockSystem,
          _sensorsManager,
          _spiManager
      ),

      _brightnessManager(),

      _alarmManager(
          _clockSystem
      ),

      _alarmController(
          _alarmManager
      ),

      _wifiManager(),

      _webServer(80),

      _webWiFiManager(
          _wifiManager,
          _settings,
          _clockSystem
      ),

      _mqttClient(),

      _mqttCommandHandler(),

      _initialized(false) {
}


// ============================================================
// BEGIN
// ============================================================

bool App::begin() {

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("          SMARTCLOCK STARTUP");
    Serial0.println("========================================");


    // --------------------------------------------------------
    // 1. SETTINGS
    // --------------------------------------------------------

    Serial0.println("[APP] 1/15 Settings");

    if (!initSettings()) {
        Serial0.println("[APP] Settings initialization failed");
        return false;
    }


    // --------------------------------------------------------
    // 2. SPI
    // --------------------------------------------------------

    Serial0.println("[APP] 2/15 SPI");

    if (!initSPI()) {
        Serial0.println("[APP] SPI initialization failed");
        return false;
    }


    // --------------------------------------------------------
    // 3. I2S
    // --------------------------------------------------------

    Serial0.println("[APP] 3/15 I2S");

    if (!initI2S()) {
        Serial0.println("[APP] I2S initialization failed");
        return false;
    }


    // --------------------------------------------------------
    // 4. CLOCK
    // --------------------------------------------------------

    Serial0.println("[APP] 4/15 Clock");

    if (!initClock()) {
        Serial0.println("[APP] Clock initialization failed");
        return false;
    }


    // --------------------------------------------------------
    // 5. SENSORS
    // --------------------------------------------------------

    Serial0.println("[APP] 5/15 Sensors");

    if (!initSensors()) {
        Serial0.println("[APP] Sensors initialization failed");
        return false;
    }


    // --------------------------------------------------------
    // 6. LIGHTING
    // --------------------------------------------------------

    Serial0.println("[APP] 6/15 Lighting");

    if (!initLighting()) {
        Serial0.println("[APP] Lighting initialization failed");
        return false;
    }


    // --------------------------------------------------------
    // 7. DISPLAY
    // --------------------------------------------------------

    Serial0.println("[APP] 7/15 Display");

    if (!initDisplay()) {
        Serial0.println("[APP] Display initialization failed");
        return false;
    }


    // --------------------------------------------------------
    // 8. BRIGHTNESS
    // --------------------------------------------------------

    Serial0.println("[APP] 8/15 Brightness");

    if (!initBrightness()) {
        Serial0.println("[APP] Brightness initialization failed");
        return false;
    }


    // --------------------------------------------------------
    // 9. SOUND
    // --------------------------------------------------------

    Serial0.println("[APP] 9/15 Sound");

    if (!initSound()) {
        Serial0.println("[APP] Sound initialization failed");
        return false;
    }


    // --------------------------------------------------------
    // 10. ALARM
    // --------------------------------------------------------

    Serial0.println("[APP] 10/15 Alarm");

    if (!initAlarm()) {
        Serial0.println("[APP] Alarm initialization failed");
        return false;
    }


    // --------------------------------------------------------
    // 11. WIFI
    // --------------------------------------------------------

    Serial0.println("[APP] 11/15 WiFi");

    if (!initWiFi()) {
        Serial0.println("[APP] WiFi initialization failed");
        return false;
    }


    // --------------------------------------------------------
    // 12. MQTT
    // --------------------------------------------------------

    Serial0.println("[APP] 12/15 MQTT");

    if (!initMqtt()) {
        Serial0.println("[APP] MQTT initialization failed");
        return false;
    }


    // --------------------------------------------------------
    // 13. MQTT COMMAND HANDLER
    // --------------------------------------------------------

    Serial0.println("[APP] 13/15 MQTT Command Handler");

    if (!initMqttCommandHandler()) {
        Serial0.println("[APP] MQTT Command Handler initialization failed");
        return false;
    }


    // --------------------------------------------------------
    // 14. WEB INTERFACE
    // --------------------------------------------------------

    Serial0.println("[APP] 14/15 Web Interface");

    if (!initWebWiFi()) {
        Serial0.println("[APP] Web interface initialization failed");
        return false;
    }


    // --------------------------------------------------------
    // 15. FINISH
    // --------------------------------------------------------

    _initialized = true;

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("       SMARTCLOCK INITIALIZED");
    Serial0.println("========================================");

    Serial0.printf(
        "[APP] MQTT: %s\n",
        _mqttClient.isConnected()
            ? "CONNECTED"
            : "WAITING FOR CONNECTION"
    );

    Serial0.println("[APP] MQTT command handler: ACTIVE");

    return true;
}


// ============================================================
// UPDATE
// ============================================================

void App::update() {

    if (!_initialized) {
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

    _sensorsManager.update();


    // --------------------------------------------------------
    // BRIGHTNESS
    // --------------------------------------------------------

    _brightnessManager.update();


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
    // ALARM
    // --------------------------------------------------------

    _alarmManager.update();

    _alarmController.update();


    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    _wifiManager.update();


    // --------------------------------------------------------
    // MQTT
    // --------------------------------------------------------

    _mqttClient.update();


    // --------------------------------------------------------
    // WEB SERVER
    // --------------------------------------------------------

    _webServer.handleClient();
}


// ============================================================
// INITIALIZATION METHODS
// ============================================================

bool App::initSettings() {

    return _settings.begin();
}


bool App::initSPI() {

    return _spiManager.begin();
}


bool App::initI2S() {

    return _i2sManager.begin();
}


bool App::initClock() {

    return _clockSystem.begin();
}


bool App::initSensors() {

    return _sensorsManager.begin();
}


bool App::initLighting() {

    return _lighting.begin();
}


bool App::initDisplay() {

    return _displaySystem.begin();
}


bool App::initBrightness() {

    _brightnessManager.begin(
        &_settings,
        &_sensorsManager,
        &_clockSystem
    );

    Serial0.println("[APP] Brightness manager initialized");

    return true;
}


bool App::initSound() {

    return _soundManager.begin();
}


bool App::initAlarm() {

    if (!_alarmManager.begin()) {
        Serial0.println("[APP] AlarmManager initialization failed");
        return false;
    }

    _alarmController.begin();

    return true;
}


bool App::initWiFi() {

    return _wifiManager.begin();
}


// ============================================================
// MQTT
// ============================================================

bool App::initMqtt() {

    if (!_mqttClient.begin()) {
        Serial0.println("[APP] MQTT client begin failed");
        return false;
    }

    Serial0.println("[APP] MQTT client initialized");

    return true;
}


// ============================================================
// MQTT COMMAND HANDLER
// ============================================================

bool App::initMqttCommandHandler() {

    if (!_mqttCommandHandler.begin(
            _mqttClient,
            _settings,
            _clockSystem,
            _wifiManager
        )) {

        Serial0.println(
            "[APP] MQTT command handler begin failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] MQTT command handler initialized"
    );

    return true;
}


// ============================================================
// WEB INTERFACE
// ============================================================

bool App::initWebWiFi() {

    _webWiFiManager.setupRoutes(_webServer);

    _webServer.begin();

    Serial0.println("[APP] Web server started");

    return true;
}


// ============================================================
// STATUS
// ============================================================

bool App::isInitialized() const {

    return _initialized;
}


bool App::isMqttConnected() {

    return _mqttClient.isConnected();
}
