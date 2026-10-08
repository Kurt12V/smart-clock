#include "WebServerManager.h"

#include "WebPageManager.h"
#include "WebSettingsManager.h"
#include "WebSDManager.h"
#include "WebAudioManager.h"
#include "WebAlarmManager.h"
#include "WebWiFiManager.h"


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
    , _webWiFiManager(nullptr)
    , _initialized(false)
{
}


// ============================================================
// BEGIN
// ============================================================

bool WebServerManager::begin(
    SettingsManager& settings
)
{
    _settings = &settings;


    // --------------------------------------------------------
    // HEADER
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println(
        "============================================"
    );

    Serial0.println(
        "[WEB] WebServerManager"
    );

    Serial0.println(
        "============================================"
    );


    // --------------------------------------------------------
    // WIFI CHECK
    // --------------------------------------------------------

    Serial0.println(
        "[WEB] Checking WiFi..."
    );

    if (
        WiFi.status() != WL_CONNECTED
    )
    {
        Serial0.println(
            "[WEB][WARN] WiFi is not connected"
        );

        /*
         * Это не является фатальной ошибкой.
         *
         * WiFiManager может находиться в:
         *
         *   - Connecting
         *   - Setup AP
         *
         * WebServer всё равно запускается.
         *
         * В Setup Mode:
         *
         *   http://192.168.4.1
         *
         * После подключения к домашней сети:
         *
         *   http://smartclock.local
         */
    }
    else
    {
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
    }


    // --------------------------------------------------------
    // LittleFS
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println(
        "[WEB] LittleFS"
    );

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
    // CHECK SETUP PAGE
    // --------------------------------------------------------

    if (
        _webWiFiManager &&
        !LittleFS.exists(
            "/wifi-setup.html"
        )
    )
    {
        Serial0.println(
            "[WEB][WARN] /wifi-setup.html not found"
        );
    }
    else if (
        _webWiFiManager
    )
    {
        Serial0.println(
            "[WEB] WiFi setup page: OK"
        );
    }


    // --------------------------------------------------------
    // ROUTES
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
    // SERVER
    // --------------------------------------------------------

    _server.begin();

    _initialized = true;


    Serial0.println();
    Serial0.println(
        "[WEB] HTTP server started"
    );


    // --------------------------------------------------------
    // ADDRESS
    // --------------------------------------------------------

    if (
        WiFi.status() == WL_CONNECTED
    )
    {
        Serial0.print(
            "[WEB] Server: http://"
        );

        Serial0.println(
            WiFi.localIP()
        );

        Serial0.println(
            "[WEB] mDNS: http://smartclock.local"
        );
    }
    else
    {
        Serial0.println(
            "[WEB] Server waiting for WiFi..."
        );

        /*
         * Если WiFiManager находится в Setup Mode,
         * WebServer доступен через:
         *
         *   http://192.168.4.1
         */
    }


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
    {
        return;
    }

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
    if (!_webWiFiManager)
        return "";

    if (_webWiFiManager->isSetupMode())
        return _webWiFiManager->getAPIP();

    return _webWiFiManager->getIP();
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


void WebServerManager::setWiFiManager(
    WebWiFiManager& manager
)
{
    _webWiFiManager = &manager;

    Serial0.println(
        "[WEB] WiFiManager attached"
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
        _pageManager->setupRoutes(
            _server
        );

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
        _settingsManager->setupRoutes(
            _server
        );

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
        _sdManager->setupRoutes(
            _server
        );

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
        _audioManager->setupRoutes(
            _server
        );

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
        _alarmManager->setupRoutes(
            _server
        );

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
    // WIFI
    // --------------------------------------------------------

    if (_webWiFiManager)
    {
        _webWiFiManager->setupRoutes(
            _server
        );

        Serial0.println(
            "[WEB] WiFi routes:      OK"
        );
    }
    else
    {
        Serial0.println(
            "[WEB][WARN] WiFi routes: NOT ATTACHED"
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
    /*
     * ВАЖНО:
     *
     * Если SmartClock находится в Setup Mode,
     * корень:
     *
     *   http://192.168.4.1/
     *
     * должен показывать страницу настройки WiFi,
     * а не обычный SmartClock UI.
     */

    if (
        _webWiFiManager &&
        WiFi.getMode() == WIFI_AP
    )
    {
        if (serveWiFiSetupPage())
        {
            return;
        }

        /*
         * Если страница отсутствует,
         * продолжаем обычную обработку.
         */
    }


    // --------------------------------------------------------
    // NORMAL SMARTCLOCK PAGE
    // --------------------------------------------------------

    if (_pageManager)
    {
        _pageManager->handleRoot(
            _server
        );

        return;
    }


    serveFile(
        "/index.html",
        "text/html; charset=utf-8"
    );
}


// ============================================================
// WIFI SETUP PAGE
// ============================================================

bool WebServerManager::serveWiFiSetupPage()
{
    const char* path =
        "/wifi-setup.html";


    if (!LittleFS.exists(path))
    {
        Serial0.println(
            "[WEB][ERROR] WiFi setup page not found"
        );

        _server.send(
            404,
            "text/plain; charset=utf-8",
            "wifi-setup.html not found"
        );

        return false;
    }


    File file =
        LittleFS.open(
            path,
            "r"
        );


    if (!file)
    {
        Serial0.println(
            "[WEB][ERROR] Failed to open WiFi setup page"
        );

        _server.send(
            500,
            "text/plain; charset=utf-8",
            "Failed to open wifi-setup.html"
        );

        return false;
    }


    Serial0.println(
        "[WEB] Serving WiFi setup page"
    );


    _server.streamFile(
        file,
        "text/html; charset=utf-8"
    );


    file.close();

    return true;
}


// ============================================================
// STATIC FILE
// ============================================================

bool WebServerManager::serveFile(
    const char* path,
    const char* contentType
)
{
    if (!LittleFS.exists(path))
    {
        Serial0.print(
            "[WEB][ERROR] File not found: "
        );

        Serial0.println(path);

        _server.send(
            404,
            "text/plain; charset=utf-8",
            "File not found"
        );

        return false;
    }


    File file =
        LittleFS.open(
            path,
            "r"
        );


    if (!file)
    {
        Serial0.print(
            "[WEB][ERROR] Failed to open: "
        );

        Serial0.println(path);

        _server.send(
            500,
            "text/plain; charset=utf-8",
            "Failed to open file"
        );

        return false;
    }


    _server.streamFile(
        file,
        contentType
    );


    file.close();

    return true;
}


// ============================================================
// NOT FOUND ROUTER
// ============================================================

void WebServerManager::handleNotFound()
{
    const String uri =
        _server.uri();


    // --------------------------------------------------------
    // WIFI SETUP STATIC RESOURCES
    // --------------------------------------------------------
    //
    // В Setup Mode JS/CSS должны работать:
    //
    //   /css/wifi-setup.css
    //   /js/wifi-setup.js
    //
    // WebPageManager может обработать их самостоятельно.
    //
    // Здесь специально не перехватываем их.
    // --------------------------------------------------------


    // --------------------------------------------------------
    // DYNAMIC ALARM API
    //
    // Important:
    //
    // WebServer does not support generic:
    //
    //   /api/alarms/{id}
    //
    // routes.
    //
    // Therefore WebAlarmManager receives dynamic alarm
    // requests through this global router.
    // --------------------------------------------------------

    if (_alarmManager)
    {
        if (
            _alarmManager->handleDynamicRequest(
                _server
            )
        )
        {
            return;
        }
    }


    // --------------------------------------------------------
    // STATIC FILES
    // --------------------------------------------------------

    if (_pageManager)
    {
        if (
            _pageManager->handleNotFound(
                _server
            )
        )
        {
            return;
        }
    }


    // --------------------------------------------------------
    // FINAL 404
    // --------------------------------------------------------

    Serial0.print(
        "[WEB][404] "
    );


    switch (_server.method())
    {
        case HTTP_GET:
            Serial0.print(
                "GET "
            );
            break;

        case HTTP_POST:
            Serial0.print(
                "POST "
            );
            break;

        case HTTP_PUT:
            Serial0.print(
                "PUT "
            );
            break;

        case HTTP_DELETE:
            Serial0.print(
                "DELETE "
            );
            break;

        case HTTP_PATCH:
            Serial0.print(
                "PATCH "
            );
            break;

        default:
            Serial0.print(
                "REQUEST "
            );
            break;
    }


    Serial0.println(
        uri
    );


    _server.send(
        404,
        "application/json",
        "{\"ok\":false,\"error\":\"Not found\"}"
    );
}
// ============================================================
// SETUP MODE
// ============================================================

