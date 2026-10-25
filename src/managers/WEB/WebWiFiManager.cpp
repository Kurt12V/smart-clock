#include "WebWiFiManager.h"

#include <ArduinoJson.h>
#include <ESP.h>
#include <LittleFS.h>
#include <RTClib.h>

#include "Config.h"

// ============================================================
// CONSTANTS
// ============================================================

namespace
{
    constexpr char WIFI_SETUP_PAGE[] = "/wifi-setup.html";

    // ========================================================
    // DATE VALIDATION
    // ========================================================

    bool isLeapYear(uint16_t year)
    {
        return
            (year % 4 == 0 && year % 100 != 0) ||
            (year % 400 == 0);
    }

    uint8_t daysInMonth(
        uint16_t year,
        uint8_t month
    )
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
        {
            return false;
        }

        if (month < 1 || month > 12)
        {
            return false;
        }

        if (
            day < 1 ||
            day > daysInMonth(year, month)
        )
        {
            return false;
        }

        if (hour > 23 || minute > 59 || second > 59)
        {
            return false;
        }

        return true;
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
// ROUTES
// ============================================================

void WebWiFiManager::setupRoutes(
    WebServer& server
)
{
    // ========================================================
    // MAIN PAGE: /
    // ========================================================

    server.on(
        "/",
        HTTP_GET,
        [&server]()
        {
            if (!LittleFS.exists(WIFI_SETUP_PAGE))
            {
                server.send(
                    500,
                    "text/plain; charset=utf-8",
                    "Wi-Fi setup page not found in LittleFS"
                );

                return;
            }

            File file = LittleFS.open(
                WIFI_SETUP_PAGE,
                "r"
            );

            if (!file || file.isDirectory())
            {
                if (file)
                {
                    file.close();
                }

                server.send(
                    500,
                    "text/plain; charset=utf-8",
                    "Failed to open Wi-Fi setup page"
                );

                return;
            }

            server.sendHeader(
                "Cache-Control",
                "no-cache, no-store, must-revalidate"
            );

            server.sendHeader(
                "Pragma",
                "no-cache"
            );

            server.sendHeader(
                "Expires",
                "-1"
            );

            server.streamFile(
                file,
                "text/html; charset=utf-8"
            );

            file.close();
        }
    );

    // ========================================================
    // DIRECT PAGE ACCESS: /wifi-setup.html
    // ========================================================

    server.on(
        "/wifi-setup.html",
        HTTP_GET,
        [&server]()
        {
            if (!LittleFS.exists(WIFI_SETUP_PAGE))
            {
                server.send(
                    404,
                    "text/plain; charset=utf-8",
                    "wifi-setup.html not found in LittleFS"
                );

                return;
            }

            File file = LittleFS.open(
                WIFI_SETUP_PAGE,
                "r"
            );

            if (!file || file.isDirectory())
            {
                if (file)
                {
                    file.close();
                }

                server.send(
                    500,
                    "text/plain; charset=utf-8",
                    "Failed to open wifi-setup.html"
                );

                return;
            }

            server.sendHeader(
                "Cache-Control",
                "no-cache, no-store, must-revalidate"
            );

            server.streamFile(
                file,
                "text/html; charset=utf-8"
            );

            file.close();
        }
    );

    // ========================================================
    // WIFI STATUS
    // GET /api/wifi
    // ========================================================

    server.on(
        "/api/wifi",
        HTTP_GET,
        [this, &server]()
        {
            handleStatus(server);
        }
    );

    // ========================================================
    // SCAN NETWORKS
    // GET /api/wifi/scan
    // ========================================================

    server.on(
        "/api/wifi/scan",
        HTTP_GET,
        [this, &server]()
        {
            handleScan(server);
        }
    );

    // ========================================================
    // CONNECT WIFI
    // POST /api/wifi/connect
    // ========================================================

    server.on(
        "/api/wifi/connect",
        HTTP_POST,
        [this, &server]()
        {
            handleConnect(server);
        }
    );

    // ========================================================
    // FORGET WIFI
    // POST /api/wifi/forget
    // ========================================================

    server.on(
        "/api/wifi/forget",
        HTTP_POST,
        [this, &server]()
        {
            handleForget(server);
        }
    );

    // ========================================================
    // START SETUP MODE
    // POST /api/wifi/setup
    // ========================================================

    server.on(
        "/api/wifi/setup",
        HTTP_POST,
        [this, &server]()
        {
            handleSetup(server);
        }
    );

    // ========================================================
    // NOT FOUND
    // ========================================================

    server.onNotFound(
        [&server]()
        {
            const String uri = server.uri();

            // Serve a clear diagnostic response for unknown routes.
            JsonDocument doc;

            doc["ok"] = false;
            doc["message"] = "Route not found";
            doc["uri"] = uri;

            String response;
            serializeJson(doc, response);

            server.send(
                404,
                "application/json; charset=utf-8",
                response
            );
        }
    );

    Serial0.println(
        "[WEB][WIFI] Routes registered"
    );

    Serial0.println(
        "[WEB][WIFI] Main page: /"
    );

    Serial0.println(
        "[WEB][WIFI] Setup page: /wifi-setup.html"
    );
}

// ============================================================
// DYNAMIC REQUEST
// ============================================================

bool WebWiFiManager::handleDynamicRequest(
    WebServer& server
)
{
    const String uri = server.uri();

    return
        uri == "/api/wifi" ||
        uri.startsWith("/api/wifi/");
}

// ============================================================
// STATUS
// ============================================================

void WebWiFiManager::handleStatus(
    WebServer& server
)
{
    JsonDocument doc;

    const bool connected =
        _wifiManager.isConnected();

    const bool setup =
        _wifiManager.isSetupMode();

    doc["ok"] = true;
    doc["connected"] = connected;
    doc["setupMode"] = setup;

    doc["mode"] =
        setup ? "setup" :
        connected ? "connected" :
        "disconnected";

    doc["hostname"] =
        _wifiManager.getHostname();

    doc["ssid"] =
        _wifiManager.getSSID();

    doc["utcOffset"] =
        _settings.get(Param::UTC_OFFSET);

    if (connected)
    {
        doc["ip"] =
            _wifiManager.getIP();

        doc["rssi"] =
            _wifiManager.getRSSI();

        doc["url"] =
            "http://smartclock.local";
    }
    else
    {
        doc["ip"] = "";
        doc["rssi"] = 0;
        doc["url"] = "";
    }

    if (setup)
    {
        doc["setup"]["ssid"] =
            _wifiManager.getSetupSSID();

        doc["setup"]["password"] =
            _wifiManager.getSetupPassword();

        doc["setup"]["ip"] =
            _wifiManager.getAPIP();
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
// SCAN
// ============================================================

void WebWiFiManager::handleScan(
    WebServer& server
)
{
    const int count =
        _wifiManager.scanNetworks();

    JsonDocument doc;

    doc["ok"] = true;

    JsonArray networks =
        doc["networks"].to<JsonArray>();

    for (int i = 0; i < count; ++i)
    {
        JsonObject network =
            networks.add<JsonObject>();

        network["ssid"] =
            _wifiManager.getScanSSID(i);

        network["rssi"] =
            _wifiManager.getScanRSSI(i);

        network["encrypted"] =
            _wifiManager.getScanEncrypted(i);
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
// CONNECT WIFI + UTC SETTINGS
// ============================================================

void WebWiFiManager::handleConnect(
    WebServer& server
)
{
    // --------------------------------------------------------
    // 1. Validate request body
    // --------------------------------------------------------

    if (!server.hasArg("plain"))
    {
        sendError(
            server,
            400,
            "Request body is required"
        );

        return;
    }

    JsonDocument doc;

    const DeserializationError error =
        deserializeJson(
            doc,
            server.arg("plain")
        );

    if (error)
    {
        sendError(
            server,
            400,
            "Invalid JSON"
        );

        return;
    }

    // --------------------------------------------------------
    // 2. Read Wi-Fi credentials
    // --------------------------------------------------------

    String ssid =
        doc["ssid"] | "";

    String password =
        doc["password"] | "";

    ssid.trim();

    if (ssid.isEmpty())
    {
        sendError(
            server,
            400,
            "SSID is required"
        );

        return;
    }

    // --------------------------------------------------------
    // 3. Validate required date/time fields
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

    const int offsetValue =
        doc["utcOffset"].as<int>();

    const int yearValue =
        doc["utcYear"].as<int>();

    const int monthValue =
        doc["utcMonth"].as<int>();

    const int dayValue =
        doc["utcDay"].as<int>();

    const int hourValue =
        doc["utcHour"].as<int>();

    const int minuteValue =
        doc["utcMinute"].as<int>();

    const int secondValue =
        doc["utcSecond"].as<int>();

    // --------------------------------------------------------
    // 4. Validate UTC offset
    // --------------------------------------------------------

    if (
        offsetValue < Config::UTC_OFFSET_MIN ||
        offsetValue > Config::UTC_OFFSET_MAX
    )
    {
        sendError(
            server,
            400,
            "UTC offset is out of range"
        );

        return;
    }

    // --------------------------------------------------------
    // 5. Validate numeric ranges before casting
    // --------------------------------------------------------

    if (
        yearValue < 2000 ||
        yearValue > 2099 ||
        monthValue < 1 ||
        monthValue > 12 ||
        dayValue < 1 ||
        dayValue > 31 ||
        hourValue < 0 ||
        hourValue > 23 ||
        minuteValue < 0 ||
        minuteValue > 59 ||
        secondValue < 0 ||
        secondValue > 59
    )
    {
        sendError(
            server,
            400,
            "Date or time value is out of range"
        );

        return;
    }

    const uint16_t year =
        static_cast<uint16_t>(yearValue);

    const uint8_t month =
        static_cast<uint8_t>(monthValue);

    const uint8_t day =
        static_cast<uint8_t>(dayValue);

    const uint8_t hour =
        static_cast<uint8_t>(hourValue);

    const uint8_t minute =
        static_cast<uint8_t>(minuteValue);

    const uint8_t second =
        static_cast<uint8_t>(secondValue);

    if (
        !isValidUTCDateTime(
            year,
            month,
            day,
            hour,
            minute,
            second
        )
    )
    {
        sendError(
            server,
            400,
            "Invalid UTC date or time"
        );

        return;
    }

    // --------------------------------------------------------
    // 6. Save Wi-Fi credentials
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println(
        "[WEB][WIFI] Initial setup received"
    );

    Serial0.print(
        "[WEB][WIFI] SSID: "
    );

    Serial0.println(ssid);

    if (
        !_wifiManager.connect(
            ssid,
            password
        )
    )
    {
        sendError(
            server,
            500,
            "Failed to save WiFi credentials"
        );

        return;
    }

    // --------------------------------------------------------
    // 7. Save UTC offset
    // --------------------------------------------------------

    const int previousOffset =
        _settings.get(Param::UTC_OFFSET);

    if (
        !_settings.set(
            Param::UTC_OFFSET,
            offsetValue
        )
    )
    {
        sendError(
            server,
            500,
            "Failed to update UTC offset"
        );

        return;
    }

    if (
        !_settings.save(
            Param::UTC_OFFSET
        )
    )
    {
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

    // --------------------------------------------------------
    // 8. Set RTC to UTC
    // --------------------------------------------------------

    if (
        !_clock.setUTCDateTime(
            year,
            month,
            day,
            hour,
            minute,
            second
        )
    )
    {
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
            "Failed to set UTC date/time on RTC"
        );

        return;
    }

    Serial0.println(
        "[WEB][WIFI] UTC offset saved"
    );

    Serial0.print(
        "[WEB][WIFI] UTC offset: "
    );

    Serial0.println(offsetValue);

    Serial0.println(
        "[WEB][WIFI] RTC UTC time updated"
    );

    // --------------------------------------------------------
    // 9. Send response before restarting
    // --------------------------------------------------------

    JsonDocument responseDoc;

    responseDoc["ok"] = true;
    responseDoc["saved"] = true;
    responseDoc["restarting"] = true;

    responseDoc["utcOffset"] =
        offsetValue;

    responseDoc["utcYear"] = year;
    responseDoc["utcMonth"] = month;
    responseDoc["utcDay"] = day;

    responseDoc["utcHour"] = hour;
    responseDoc["utcMinute"] = minute;
    responseDoc["utcSecond"] = second;

    responseDoc["message"] =
        "WiFi and UTC settings saved. Restarting.";

    String response;
    serializeJson(
        responseDoc,
        response
    );

    server.send(
        200,
        "application/json; charset=utf-8",
        response
    );

    delay(500);

    Serial0.println(
        "[WEB][WIFI] Restarting ESP32..."
    );

    ESP.restart();
}

// ============================================================
// FORGET WIFI
// ============================================================

void WebWiFiManager::handleForget(
    WebServer& server
)
{
    Serial0.println(
        "[WEB][WIFI] Forgetting WiFi credentials"
    );

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

void WebWiFiManager::handleSetup(
    WebServer& server
)
{
    Serial0.println(
        "[WEB][WIFI] Starting setup mode"
    );

    if (!_wifiManager.startSetupMode())
    {
        sendError(
            server,
            500,
            "Failed to start setup mode"
        );

        return;
    }

    JsonDocument doc;

    doc["ok"] = true;
    doc["setupMode"] = true;

    doc["ssid"] =
        _wifiManager.getSetupSSID();

    doc["password"] =
        _wifiManager.getSetupPassword();

    doc["ip"] =
        _wifiManager.getAPIP();

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