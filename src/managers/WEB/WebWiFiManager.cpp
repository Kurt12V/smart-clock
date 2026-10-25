#include "WebWiFiManager.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <RTClib.h>

#include "Config.h"

// ============================================================
// CONSTANTS AND HELPERS
// ============================================================

namespace
{
    constexpr char WIFI_SETUP_PAGE[] = "/wifi-setup.html";
    constexpr char WIFI_SETUP_CSS[]  = "/css/wifi-setup.css";
    constexpr char WIFI_SETUP_JS[]   = "/js/wifi-setup.js";

    // --------------------------------------------------------
    // Date validation
    // --------------------------------------------------------

    bool isLeapYear(uint16_t year)
    {
        return
            (year % 4 == 0 && year % 100 != 0) ||
            (year % 400 == 0);
    }

    uint8_t daysInMonth(uint16_t year, uint8_t month)
    {
        switch (month)
        {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                return 31;

            case 4:
            case 6:
            case 9:
            case 11:
                return 30;

            case 2:
                return isLeapYear(year) ? 29 : 28;

            default:
                return 0;
        }
    }

    bool isValidUTCDateTime(
        uint16_t year,
        uint8_t month,
        uint8_t day,
        uint8_t hour,
        uint8_t minute,
        uint8_t second
    )
    {
        if (year < 2000 || year > 2099)
            return false;

        if (month < 1 || month > 12)
            return false;

        if (day < 1 || day > daysInMonth(year, month))
            return false;

        if (hour > 23 || minute > 59 || second > 59)
            return false;

        return true;
    }

    // --------------------------------------------------------
    // MIME types
    // --------------------------------------------------------

    const char* getContentType(const String& path)
    {
        if (path.endsWith(".html"))
            return "text/html; charset=utf-8";

        if (path.endsWith(".css"))
            return "text/css; charset=utf-8";

        if (path.endsWith(".js"))
            return "application/javascript; charset=utf-8";

        if (path.endsWith(".json"))
            return "application/json; charset=utf-8";

        if (path.endsWith(".svg"))
            return "image/svg+xml";

        if (path.endsWith(".png"))
            return "image/png";

        if (path.endsWith(".jpg") || path.endsWith(".jpeg"))
            return "image/jpeg";

        if (path.endsWith(".ico"))
            return "image/x-icon";

        if (path.endsWith(".woff2"))
            return "font/woff2";

        if (path.endsWith(".woff"))
            return "font/woff";

        if (path.endsWith(".ttf"))
            return "font/ttf";

        if (path.endsWith(".webp"))
            return "image/webp";

        if (path.endsWith(".txt"))
            return "text/plain; charset=utf-8";

        return "application/octet-stream";
    }

    // --------------------------------------------------------
    // Allow only known static files
    // --------------------------------------------------------

    bool isAllowedStaticFile(const String& path)
    {
        return
            path == WIFI_SETUP_PAGE ||
            path == WIFI_SETUP_CSS ||
            path == WIFI_SETUP_JS;
    }

    // --------------------------------------------------------
    // Serve a static file from LittleFS
    // --------------------------------------------------------

    bool serveStaticFile(
        WebServer& server,
        const String& path
    )
    {
        if (!isAllowedStaticFile(path))
            return false;

        if (!LittleFS.exists(path))
            return false;

        File file = LittleFS.open(path, "r");

        if (!file || file.isDirectory())
        {
            if (file)
                file.close();

            return false;
        }

        server.sendHeader(
            "X-Content-Type-Options",
            "nosniff"
        );

        if (path.endsWith(".html"))
        {
            server.sendHeader(
                "Cache-Control",
                "no-cache, no-store, must-revalidate"
            );

            server.sendHeader("Pragma", "no-cache");
            server.sendHeader("Expires", "-1");
        }
        else
        {
            server.sendHeader(
                "Cache-Control",
                "no-cache"
            );
        }

        server.streamFile(file, getContentType(path));
        file.close();

        return true;
    }

    // --------------------------------------------------------
    // Send JSON error for unknown routes
    // --------------------------------------------------------

    void sendRouteNotFound(WebServer& server)
    {
        JsonDocument doc;

        doc["ok"] = false;
        doc["message"] = "Route not found";
        doc["uri"] = server.uri();

        String response;
        serializeJson(doc, response);

        server.send(
            404,
            "application/json; charset=utf-8",
            response
        );
    }
}

// ============================================================
// CONSTRUCTOR
// ============================================================

WebWiFiManager::WebWiFiManager(
    WiFiManager& wifiManager,
    SettingsManager& settings,
    ClockSystem& clock
)
    : _wifiManager(wifiManager)
    , _settings(settings)
    , _clock(clock)
{
}

// ============================================================
// ROUTE REGISTRATION
// ============================================================

void WebWiFiManager::setupRoutes(WebServer& server)
{
    // --------------------------------------------------------
    // Main page
    // --------------------------------------------------------

    server.on("/", HTTP_GET, [&server]()
    {
        if (!serveStaticFile(server, WIFI_SETUP_PAGE))
        {
            server.send(
                500,
                "text/plain; charset=utf-8",
                "Cannot load /wifi-setup.html from LittleFS"
            );
        }
    });

    // --------------------------------------------------------
    // Direct HTML access
    // --------------------------------------------------------

    server.on("/wifi-setup.html", HTTP_GET, [&server]()
    {
        if (!serveStaticFile(server, WIFI_SETUP_PAGE))
        {
            server.send(
                404,
                "text/plain; charset=utf-8",
                "wifi-setup.html not found in LittleFS"
            );
        }
    });

    // --------------------------------------------------------
    // Wi-Fi status
    // --------------------------------------------------------

    server.on("/api/wifi", HTTP_GET, [this, &server]()
    {
        handleStatus(server);
    });

    // --------------------------------------------------------
    // Scan networks
    // --------------------------------------------------------

    server.on("/api/wifi/scan", HTTP_GET, [this, &server]()
    {
        handleScan(server);
    });

    // --------------------------------------------------------
    // Connect Wi-Fi
    // --------------------------------------------------------

    server.on("/api/wifi/connect", HTTP_POST, [this, &server]()
    {
        handleConnect(server);
    });

    // --------------------------------------------------------
    // Forget Wi-Fi
    // --------------------------------------------------------

    server.on("/api/wifi/forget", HTTP_POST, [this, &server]()
    {
        handleForget(server);
    });

    // --------------------------------------------------------
    // Start setup mode
    // --------------------------------------------------------

    server.on("/api/wifi/setup", HTTP_POST, [this, &server]()
    {
        handleSetup(server);
    });

    // --------------------------------------------------------
    // Static assets and unknown routes
    // --------------------------------------------------------

    server.onNotFound([&server]()
    {
        if (server.method() == HTTP_GET)
        {
            if (serveStaticFile(server, server.uri()))
                return;
        }

        sendRouteNotFound(server);
    });

    Serial0.println("[WEB][WIFI] Routes registered");
    Serial0.println("[WEB][WIFI] Page: /");
    Serial0.println("[WEB][WIFI] CSS: /css/wifi-setup.css");
    Serial0.println("[WEB][WIFI] JS: /js/wifi-setup.js");
}

// ============================================================
// DYNAMIC REQUEST CHECK
// ============================================================

bool WebWiFiManager::handleDynamicRequest(WebServer& server)
{
    const String uri = server.uri();

    return
        uri == "/api/wifi" ||
        uri.startsWith("/api/wifi/");
}

// ============================================================
// WIFI STATUS
// ============================================================

void WebWiFiManager::handleStatus(WebServer& server)
{
    JsonDocument doc;

    const bool connected = _wifiManager.isConnected();
    const bool setup = _wifiManager.isSetupMode();

    doc["ok"] = true;
    doc["connected"] = connected;
    doc["setupMode"] = setup;

    doc["mode"] =
        setup ? "setup" :
        connected ? "connected" :
        "disconnected";

    doc["hostname"] = _wifiManager.getHostname();
    doc["ssid"] = _wifiManager.getSSID();
    doc["utcOffset"] = _settings.get(Param::UTC_OFFSET);

    if (connected)
    {
        doc["ip"] = _wifiManager.getIP();
        doc["rssi"] = _wifiManager.getRSSI();

        // No mDNS dependency. The IP is the actual address.
        doc["url"] = "http://" + _wifiManager.getIP();
    }
    else
    {
        doc["ip"] = "";
        doc["rssi"] = 0;
        doc["url"] = "";
    }

    if (setup)
    {
        doc["setup"]["ssid"] = _wifiManager.getSetupSSID();
        doc["setup"]["password"] = _wifiManager.getSetupPassword();
        doc["setup"]["ip"] = _wifiManager.getAPIP();
    }

    String response;
    serializeJson(doc, response);

    server.send(
        200,
        "application/json; charset=utf-8",
        response
    );
}

// ============================================================
// SCAN WIFI NETWORKS
// ============================================================

void WebWiFiManager::handleScan(WebServer& server)
{
    const int count = _wifiManager.scanNetworks();

    JsonDocument doc;
    doc["ok"] = true;

    JsonArray networks = doc["networks"].to<JsonArray>();

    for (int i = 0; i < count; ++i)
    {
        JsonObject network = networks.add<JsonObject>();

        network["ssid"] = _wifiManager.getScanSSID(i);
        network["rssi"] = _wifiManager.getScanRSSI(i);
        network["encrypted"] = _wifiManager.getScanEncrypted(i);
    }

    String response;
    serializeJson(doc, response);

    server.send(
        200,
        "application/json; charset=utf-8",
        response
    );
}

// ============================================================
// CONNECT WIFI + SET INITIAL TIME
// ============================================================

void WebWiFiManager::handleConnect(WebServer& server)
{
    // --------------------------------------------------------
    // Validate request body
    // --------------------------------------------------------

    if (!server.hasArg("plain"))
    {
        sendError(server, 400, "Request body is required");
        return;
    }

    JsonDocument doc;

    const DeserializationError error =
        deserializeJson(doc, server.arg("plain"));

    if (error)
    {
        sendError(server, 400, "Invalid JSON");
        return;
    }

    // --------------------------------------------------------
    // Read Wi-Fi credentials
    // --------------------------------------------------------

    String ssid = doc["ssid"] | "";
    String password = doc["password"] | "";

    ssid.trim();

    if (ssid.isEmpty())
    {
        sendError(server, 400, "SSID is required");
        return;
    }

    // --------------------------------------------------------
    // Validate required UTC fields
    // --------------------------------------------------------

    if (
        !doc["utcOffset"].is<int>() ||
        !doc["utcYear"].is<int>() ||
        !doc["utcMonth"].is<int>() ||
        !doc["utcDay"].is<int>() ||
        !doc["utcHour"].is<int>() ||
        !doc["utcMinute"].is<int>() ||
        !doc["utcSecond"].is<int>()
    )
    {
        sendError(
            server,
            400,
            "UTC offset, date and time are required"
        );

        return;
    }

    const int offsetValue = doc["utcOffset"].as<int>();
    const int yearValue = doc["utcYear"].as<int>();
    const int monthValue = doc["utcMonth"].as<int>();
    const int dayValue = doc["utcDay"].as<int>();
    const int hourValue = doc["utcHour"].as<int>();
    const int minuteValue = doc["utcMinute"].as<int>();
    const int secondValue = doc["utcSecond"].as<int>();

    // --------------------------------------------------------
    // Validate UTC offset
    // --------------------------------------------------------

    if (
        offsetValue < Config::UTC_OFFSET_MIN ||
        offsetValue > Config::UTC_OFFSET_MAX
    )
    {
        sendError(server, 400, "UTC offset is out of range");
        return;
    }

    // --------------------------------------------------------
    // Validate date/time ranges
    // --------------------------------------------------------

    if (
        yearValue < 2000 || yearValue > 2099 ||
        monthValue < 1 || monthValue > 12 ||
        dayValue < 1 || dayValue > 31 ||
        hourValue < 0 || hourValue > 23 ||
        minuteValue < 0 || minuteValue > 59 ||
        secondValue < 0 || secondValue > 59
    )
    {
        sendError(
            server,
            400,
            "Date or time value is out of range"
        );

        return;
    }

    const uint16_t year = static_cast<uint16_t>(yearValue);
    const uint8_t month = static_cast<uint8_t>(monthValue);
    const uint8_t day = static_cast<uint8_t>(dayValue);
    const uint8_t hour = static_cast<uint8_t>(hourValue);
    const uint8_t minute = static_cast<uint8_t>(minuteValue);
    const uint8_t second = static_cast<uint8_t>(secondValue);

    if (!isValidUTCDateTime(year, month, day, hour, minute, second))
    {
        sendError(server, 400, "Invalid UTC date or time");
        return;
    }

    // --------------------------------------------------------
    // Save Wi-Fi credentials
    // --------------------------------------------------------

    Serial0.println("[WEB][WIFI] Initial setup received");
    Serial0.print("[WEB][WIFI] SSID: ");
    Serial0.println(ssid);

    if (!_wifiManager.connect(ssid, password))
    {
        sendError(
            server,
            500,
            "Failed to save WiFi credentials"
        );

        return;
    }

    // --------------------------------------------------------
    // Save UTC offset only when it has changed
    // --------------------------------------------------------

    const int previousOffset =
        _settings.get(Param::UTC_OFFSET);

    if (offsetValue != previousOffset)
    {
        if (!_settings.set(Param::UTC_OFFSET, offsetValue))
        {
            sendError(
                server,
                500,
                "Failed to update UTC offset"
            );

            return;
        }

        if (!_settings.save(Param::UTC_OFFSET))
        {
            // Attempt to restore the previous value.
            _settings.set(
                Param::UTC_OFFSET,
                previousOffset
            );

            _settings.save(
                Param::UTC_OFFSET
            );

            sendError(
                server,
                500,
                "Failed to save UTC offset"
            );

            return;
        }

        Serial0.println("[WEB][WIFI] UTC offset changed and saved");
    }
    else
    {
        Serial0.println("[WEB][WIFI] UTC offset unchanged");
    }

    Serial0.print("[WEB][WIFI] UTC offset: ");
    Serial0.println(offsetValue);

    // --------------------------------------------------------
    // Set RTC to UTC
    // --------------------------------------------------------

    if (!_clock.setUTCDateTime(
            year,
            month,
            day,
            hour,
            minute,
            second
        ))
    {
        sendError(
            server,
            500,
            "Failed to set UTC date/time on RTC"
        );

        return;
    }

    Serial0.println("[WEB][WIFI] RTC UTC time updated");

    // --------------------------------------------------------
    // Send response before restart
    // --------------------------------------------------------

    JsonDocument responseDoc;

    responseDoc["ok"] = true;
    responseDoc["saved"] = true;
    responseDoc["restarting"] = true;

    responseDoc["utcOffset"] = offsetValue;
    responseDoc["utcYear"] = year;
    responseDoc["utcMonth"] = month;
    responseDoc["utcDay"] = day;
    responseDoc["utcHour"] = hour;
    responseDoc["utcMinute"] = minute;
    responseDoc["utcSecond"] = second;

    responseDoc["message"] =
        "WiFi and UTC settings saved. Restarting.";

    String response;
    serializeJson(responseDoc, response);

    server.send(
        200,
        "application/json; charset=utf-8",
        response
    );

    delay(500);

    Serial0.println("[WEB][WIFI] Restarting ESP32...");
    ESP.restart();
}

// ============================================================
// FORGET WIFI
// ============================================================

void WebWiFiManager::handleForget(WebServer& server)
{
    Serial0.println("[WEB][WIFI] Forgetting WiFi credentials");

    _wifiManager.clearCredentials();

    JsonDocument doc;

    doc["ok"] = true;
    doc["restarting"] = true;
    doc["message"] =
        "WiFi credentials cleared. ESP32 is restarting.";

    String response;
    serializeJson(doc, response);

    server.send(
        200,
        "application/json; charset=utf-8",
        response
    );

    delay(500);
    ESP.restart();
}

// ============================================================
// START SETUP MODE
// ============================================================

void WebWiFiManager::handleSetup(WebServer& server)
{
    Serial0.println("[WEB][WIFI] Starting setup mode");

    if (!_wifiManager.startSetupMode())
    {
        sendError(server, 500, "Failed to start setup mode");
        return;
    }

    JsonDocument doc;

    doc["ok"] = true;
    doc["setupMode"] = true;
    doc["ssid"] = _wifiManager.getSetupSSID();
    doc["password"] = _wifiManager.getSetupPassword();
    doc["ip"] = _wifiManager.getAPIP();

    String response;
    serializeJson(doc, response);

    server.send(
        200,
        "application/json; charset=utf-8",
        response
    );
}

// ============================================================
// ERROR RESPONSE
// ============================================================

void WebWiFiManager::sendError(
    WebServer& server,
    int code,
    const char* message
)
{
    JsonDocument doc;

    doc["ok"] = false;
    doc["message"] = message;

    String response;
    serializeJson(doc, response);

    server.send(
        code,
        "application/json; charset=utf-8",
        response
    );
}

// ============================================================
// SETUP MODE
// ============================================================

bool WebWiFiManager::isSetupMode() const
{
    return _wifiManager.isSetupMode();
}

// ============================================================
// GET IP
// ============================================================

String WebWiFiManager::getIP() const
{
    return _wifiManager.getIP();
}

// ============================================================
// GET AP IP
// ============================================================

String WebWiFiManager::getAPIP() const
{
    return _wifiManager.getAPIP();
}