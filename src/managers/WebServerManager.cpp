#include "WebServerManager.h"

#include "WebPageManager.h"
#include "WebSettingsManager.h"
#include "WebSDManager.h"
#include "WebAudioManager.h"
#include "WebAlarmManager.h"

namespace
{
    constexpr uint32_t WIFI_TIMEOUT_MS = 20000UL;
}


// ============================================================
// CONSTRUCTOR
// ============================================================

WebServerManager::WebServerManager()
    : _server(80)
    , _settings(nullptr)
    , _pageManager(nullptr)
    , _settingsManager(nullptr)
    , _sdManager(nullptr)
    , _audioManager(nullptr)
    , _alarmManager(nullptr)
    , _initialized(false)
{
}


// ============================================================
// BEGIN
// ============================================================

bool WebServerManager::begin(
    SettingsManager& settings,
    const char* ssid,
    const char* password
)
{
    _settings = &settings;

    // --------------------------------------------------------
    // WiFi
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println("============================================");
    Serial0.println("[WEB] WebServerManager");
    Serial0.println("============================================");

    Serial0.println("[WEB] WiFi");

    WiFi.mode(WIFI_STA);

    Serial0.print("[WEB] Connecting to WiFi: ");
    Serial0.println(ssid);

    WiFi.begin(
        ssid,
        password
    );

    const uint32_t startTime = millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < WIFI_TIMEOUT_MS
    )
    {
        delay(250);
        Serial0.print(".");
    }

    Serial0.println();

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial0.println(
            "[WEB][ERROR] WiFi connection FAILED"
        );

        Serial0.print(
            "[WEB][ERROR] WiFi status: "
        );

        Serial0.println(
            static_cast<int>(WiFi.status())
        );

        return false;
    }

    Serial0.println(
        "[WEB] WiFi connected"
    );

    Serial0.print(
        "[WEB] IP: "
    );

    Serial0.println(
        WiFi.localIP()
    );

    Serial0.print(
        "[WEB] RSSI: "
    );

    Serial0.print(
        WiFi.RSSI()
    );

    Serial0.println(
        " dBm"
    );


    // --------------------------------------------------------
    // LittleFS
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println("[WEB] LittleFS");

    if (!LittleFS.begin(true))
    {
        Serial0.println(
            "[WEB][ERROR] LittleFS mount FAILED"
        );

        return false;
    }

    Serial0.println(
        "[WEB] LittleFS mounted"
    );


    // --------------------------------------------------------
    // Routes
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println(
        "[WEB] Registering HTTP routes..."
    );

    setupRoutes();

    Serial0.println(
        "[WEB] HTTP routes registered"
    );


    // --------------------------------------------------------
    // Server
    // --------------------------------------------------------

    _server.begin();

    _initialized = true;

    Serial0.println();
    Serial0.println(
        "[WEB] HTTP server started"
    );

    Serial0.print(
        "[WEB] Server: http://"
    );

    Serial0.println(
        WiFi.localIP()
    );

    Serial0.println(
        "============================================"
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
// IP
// ============================================================

String WebServerManager::getIP() const
{
    if (WiFi.status() != WL_CONNECTED)
        return String();

    return WiFi.localIP().toString();
}


// ============================================================
// MODULE SETTERS
// ============================================================

void WebServerManager::setPageManager(
    WebPageManager& manager
)
{
    _pageManager = &manager;

    Serial0.println(
        "[WEB] PageManager attached"
    );
}


void WebServerManager::setSettingsManager(
    WebSettingsManager& manager
)
{
    _settingsManager = &manager;

    Serial0.println(
        "[WEB] SettingsManager attached"
    );
}


void WebServerManager::setSDManager(
    WebSDManager& manager
)
{
    _sdManager = &manager;

    Serial0.println(
        "[WEB] SDManager attached"
    );
}


void WebServerManager::setAudioManager(
    WebAudioManager& manager
)
{
    _audioManager = &manager;

    Serial0.println(
        "[WEB] AudioManager attached"
    );
}


void WebServerManager::setAlarmManager(
    WebAlarmManager& manager
)
{
    _alarmManager = &manager;

    Serial0.println(
        "[WEB] AlarmManager attached"
    );
}


// ============================================================
// ROUTES
// ============================================================

void WebServerManager::setupRoutes()
{
    // --------------------------------------------------------
    // ROOT
    // --------------------------------------------------------

    _server.on(
        "/",
        HTTP_GET,
        [this]()
        {
            handleRoot();
        }
    );


    // --------------------------------------------------------
    // PAGE
    // --------------------------------------------------------

    if (_pageManager)
    {
        _pageManager->setupRoutes(_server);

        Serial0.println(
            "[WEB] Page routes:      OK"
        );
    }
    else
    {
        Serial0.println(
            "[WEB][WARN] Page routes: NOT ATTACHED"
        );
    }


    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    if (_settingsManager)
    {
        _settingsManager->setupRoutes(_server);

        Serial0.println(
            "[WEB] Settings routes:  OK"
        );
    }
    else
    {
        Serial0.println(
            "[WEB][WARN] Settings routes: NOT ATTACHED"
        );
    }


    // --------------------------------------------------------
    // SD
    // --------------------------------------------------------

    if (_sdManager)
    {
        _sdManager->setupRoutes(_server);

        Serial0.println(
            "[WEB] SD routes:        OK"
        );
    }
    else
    {
        Serial0.println(
            "[WEB][WARN] SD routes: NOT ATTACHED"
        );
    }


    // --------------------------------------------------------
    // AUDIO
    // --------------------------------------------------------

    if (_audioManager)
    {
        _audioManager->setupRoutes(_server);

        Serial0.println(
            "[WEB] Audio routes:     OK"
        );
    }
    else
    {
        Serial0.println(
            "[WEB][WARN] Audio routes: NOT ATTACHED"
        );
    }


    // --------------------------------------------------------
    // ALARMS
    // --------------------------------------------------------

    if (_alarmManager)
    {
        _alarmManager->setupRoutes(_server);

        Serial0.println(
            "[WEB] Alarm routes:     OK"
        );
    }
    else
    {
        Serial0.println(
            "[WEB][WARN] Alarm routes: NOT ATTACHED"
        );
    }


    // --------------------------------------------------------
    // SINGLE GLOBAL NOT FOUND ROUTER
    // --------------------------------------------------------

    _server.onNotFound(
        [this]()
        {
            handleNotFound();
        }
    );

    Serial0.println(
        "[WEB] NotFound router:  OK"
    );
}


// ============================================================
// ROOT
// ============================================================

void WebServerManager::handleRoot()
{
    if (_pageManager)
    {
        _pageManager->handleRoot(_server);
        return;
    }


    if (!LittleFS.exists("/index.html"))
    {
        _server.send(
            404,
            "text/plain",
            "index.html not found"
        );

        return;
    }


    File file = LittleFS.open(
        "/index.html",
        "r"
    );

    if (!file)
    {
        _server.send(
            500,
            "text/plain",
            "Failed to open index.html"
        );

        return;
    }


    _server.streamFile(
        file,
        "text/html; charset=utf-8"
    );

    file.close();
}


// ============================================================
// NOT FOUND ROUTER
// ============================================================

void WebServerManager::handleNotFound()
{
    const String uri = _server.uri();


    // --------------------------------------------------------
    // Dynamic alarm API
    //
    // Important:
    // WebServer does not have a generic "{id}" route.
    // AlarmManager therefore receives dynamic alarm requests
    // through this single global router.
    // --------------------------------------------------------

    if (_alarmManager)
    {
        if (_alarmManager->handleDynamicRequest(_server))
        {
            return;
        }
    }


    // --------------------------------------------------------
    // Static files
    // --------------------------------------------------------

    if (_pageManager)
    {
        if (_pageManager->handleNotFound(_server))
        {
            return;
        }
    }


    // --------------------------------------------------------
    // Final 404
    // --------------------------------------------------------

    Serial0.print(
        "[WEB][404] "
    );

    switch (_server.method())
    {
        case HTTP_GET:
            Serial0.print("GET ");
            break;

        case HTTP_POST:
            Serial0.print("POST ");
            break;

        case HTTP_PUT:
            Serial0.print("PUT ");
            break;

        case HTTP_DELETE:
            Serial0.print("DELETE ");
            break;

        case HTTP_PATCH:
            Serial0.print("PATCH ");
            break;

        default:
            Serial0.print("REQUEST ");
            break;
    }

    Serial0.println(uri);


    _server.send(
        404,
        "application/json",
        "{\"ok\":false,\"error\":\"Not found\"}"
    );
}