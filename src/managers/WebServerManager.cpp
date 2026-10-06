#include "WebServerManager.h"

// ============================================================
// CONSTANTS
// ============================================================

namespace
{
    constexpr uint32_t SERIAL_BAUDRATE = 115200;

    constexpr uint32_t WIFI_TIMEOUT_MS =
        20000UL;
}

// ============================================================
// CONSTRUCTOR
// ============================================================

WebServerManager::WebServerManager()
    : _server(80),
      _settings(nullptr),
      _sd(nullptr),
      _sound(nullptr),
      _alarmManager(nullptr),
      _alarmController(nullptr),
      _initialized(false)
{

    Serial0.println();
    Serial0.println(
        "============================================================"
    );
    Serial0.println(
        "[WEB] WebServerManager CONSTRUCTOR"
    );
    Serial0.println(
        "============================================================"
    );

    Serial0.println(
        "[WEB] HTTP port = 80"
    );
}

// ============================================================
// BEGIN
// ============================================================

bool WebServerManager::begin(
    SettingsManager& settings,
    SDManager& sd,
    SoundManager& sound,
    const char* ssid,
    const char* password
)
{
    const uint32_t startedAt =
        millis();

    Serial0.println();
    Serial0.println(
        "============================================================"
    );
    Serial0.println(
        "[WEB] WebServerManager BEGIN"
    );
    Serial0.println(
        "============================================================"
    );

    _initialized = false;

    _settings = &settings;
    _sd = &sd;
    _sound = &sound;

    Serial0.printf(
        "[WEB][PTR] SettingsManager=%p\n",
        static_cast<void*>(_settings)
    );

    Serial0.printf(
        "[WEB][PTR] SDManager=%p\n",
        static_cast<void*>(_sd)
    );

    Serial0.printf(
        "[WEB][PTR] SoundManager=%p\n",
        static_cast<void*>(_sound)
    );

    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    Serial0.println(
        "[WEB][STEP] Connecting to WiFi"
    );

    if (!ssid)
    {
        Serial0.println(
            "[WEB][ERROR] WiFi SSID is nullptr"
        );

        return false;
    }

    Serial0.printf(
        "[WEB][WIFI] SSID=%s\n",
        ssid
    );

    WiFi.mode(
        WIFI_STA
    );

    WiFi.begin(
        ssid,
        password
    );

    const uint32_t wifiStartedAt =
        millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - wifiStartedAt <
            WIFI_TIMEOUT_MS
    )
    {
        delay(250);

        Serial0.printf(
            "[WEB][WIFI] status=%d elapsed=%lu ms\n",
            static_cast<int>(
                WiFi.status()
            ),
            static_cast<unsigned long>(
                millis() - wifiStartedAt
            )
        );
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial0.printf(
            "[WEB][WIFI][ERROR] Connection failed status=%d\n",
            static_cast<int>(
                WiFi.status()
            )
        );

        return false;
    }

    Serial0.println(
        "[WEB][WIFI] Connected"
    );

    Serial0.printf(
        "[WEB][WIFI] IP=%s\n",
        WiFi.localIP().toString().c_str()
    );

    Serial0.printf(
        "[WEB][WIFI] Gateway=%s\n",
        WiFi.gatewayIP().toString().c_str()
    );

    Serial0.printf(
        "[WEB][WIFI] Netmask=%s\n",
        WiFi.subnetMask().toString().c_str()
    );

    Serial0.printf(
        "[WEB][WIFI] RSSI=%d dBm\n",
        WiFi.RSSI()
    );

    // --------------------------------------------------------
    // LITTLEFS
    // --------------------------------------------------------

    Serial0.println(
        "[WEB][STEP] Mounting LittleFS"
    );

    if (!LittleFS.begin(true))
    {
        Serial0.println(
            "[WEB][ERROR] LittleFS mount failed"
        );

        return false;
    }

    Serial0.println(
        "[WEB] LittleFS mounted"
    );

    // --------------------------------------------------------
    // ROUTES
    // --------------------------------------------------------

    setupRoutes();

    // --------------------------------------------------------
    // SERVER
    // --------------------------------------------------------

    Serial0.println(
        "[WEB][STEP] Starting WebServer"
    );

    _server.begin();

    _initialized = true;

    Serial0.println();
    Serial0.println(
        "============================================================"
    );
    Serial0.println(
        "[WEB] WebServerManager READY"
    );
    Serial0.println(
        "============================================================"
    );

    Serial0.printf(
        "[WEB] URL=http://%s\n",
        WiFi.localIP().toString().c_str()
    );

    Serial0.printf(
        "[WEB] begin() completed in %lu ms\n",
        static_cast<unsigned long>(
            millis() - startedAt
        )
    );

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void WebServerManager::update()
{
    if (!_initialized)
        return;

    _server.handleClient();
}

// ============================================================
// CONNECTION
// ============================================================

bool WebServerManager::isConnected() const
{
    return
        _initialized &&
        WiFi.status() == WL_CONNECTED;
}

// ============================================================
// GET IP
// ============================================================

String WebServerManager::getIP() const
{
    if (WiFi.status() != WL_CONNECTED)
        return String();

    return WiFi.localIP().toString();
}

// ============================================================
// SET ALARM MANAGER
// ============================================================

void WebServerManager::setAlarmManager(
    AlarmManager& alarmManager
)
{
    _alarmManager =
        &alarmManager;

    Serial0.printf(
        "[WEB][ALARM] AlarmManager attached: %p\n",
        static_cast<void*>(
            _alarmManager
        )
    );
}

// ============================================================
// SET ALARM CONTROLLER
// ============================================================

void WebServerManager::setAlarmController(
    AlarmController& alarmController
)
{
    _alarmController =
        &alarmController;

    Serial0.printf(
        "[WEB][ALARM] AlarmController attached: %p\n",
        static_cast<void*>(
            _alarmController
        )
    );
}