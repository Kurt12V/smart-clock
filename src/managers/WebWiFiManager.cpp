#include "WebWiFiManager.h"

#include <ESP.h>

// ============================================================
// CONSTRUCTOR
// ============================================================

WebWiFiManager::WebWiFiManager(
    WiFiManager& wifiManager
)
    : _wifiManager(wifiManager)
{
}

// ============================================================
// ROUTES
// ============================================================

void WebWiFiManager::setupRoutes(
    WebServer& server
)
{
    server.on(
        "/api/wifi",
        HTTP_GET,
        [this, &server]()
        {
            handleStatus(server);
        }
    );

    server.on(
        "/api/wifi/scan",
        HTTP_GET,
        [this, &server]()
        {
            handleScan(server);
        }
    );

    server.on(
        "/api/wifi/connect",
        HTTP_POST,
        [this, &server]()
        {
            handleConnect(server);
        }
    );

    server.on(
        "/api/wifi/forget",
        HTTP_POST,
        [this, &server]()
        {
            handleForget(server);
        }
    );

    server.on(
        "/api/wifi/setup",
        HTTP_POST,
        [this, &server]()
        {
            handleSetup(server);
        }
    );

    Serial0.println(
        "[WEB][WIFI] Routes registered"
    );
}

// ============================================================
// DYNAMIC REQUEST
// ============================================================

bool WebWiFiManager::handleDynamicRequest(
    WebServer& server
)
{
    const String uri =
        server.uri();

    if (
        uri == "/api/wifi" ||
        uri.startsWith("/api/wifi/")
    )
    {
        return true;
    }

    return false;
}

// ============================================================
// STATUS
// ============================================================

void WebWiFiManager::handleStatus(
    WebServer& server
)
{
    JsonDocument doc;

    bool connected =
        _wifiManager.isConnected();

    bool setup =
        _wifiManager.isSetupMode();

    doc["ok"] = true;

    doc["connected"] =
        connected;

    doc["setupMode"] =
        setup;

    doc["mode"] =
        setup
            ? "setup"
            : connected
                ? "connected"
                : "disconnected";

    doc["hostname"] =
        _wifiManager.getHostname();

    doc["ssid"] =
        _wifiManager.getSSID();

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

    serializeJson(
        doc,
        response
    );

    server.send(
        200,
        "application/json",
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
    int count =
        _wifiManager.scanNetworks();

    JsonDocument doc;

    doc["ok"] = true;

    JsonArray networks =
        doc["networks"].to<JsonArray>();

    for (
        int i = 0;
        i < count;
        ++i
    )
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

    serializeJson(
        doc,
        response
    );

    server.send(
        200,
        "application/json",
        response
    );
}

// ============================================================
// CONNECT
// ============================================================

void WebWiFiManager::handleConnect(
    WebServer& server
)
{
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

    DeserializationError error =
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

    Serial0.println();
    Serial0.println(
        "[WEB][WIFI] New credentials received"
    );

    Serial0.print(
        "[WEB][WIFI] SSID: "
    );

    Serial0.println(
        ssid
    );

    /*
     * Save credentials into NVS.
     *
     * We intentionally DO NOT connect immediately.
     *
     * First:
     *
     *   1. save credentials
     *   2. send HTTP response
     *   3. restart ESP32
     *
     * After reboot WiFiManager::begin()
     * loads the saved credentials and tries
     * to connect.
     */

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

    JsonDocument responseDoc;

    responseDoc["ok"] = true;

    responseDoc["saved"] = true;

    responseDoc["restarting"] = true;

    responseDoc["message"] =
        "WiFi credentials saved. ESP32 is restarting.";

    String response;

    serializeJson(
        responseDoc,
        response
    );

    server.send(
        200,
        "application/json",
        response
    );

    /*
     * Give WebServer a little time to flush
     * the HTTP response.
     */

    delay(500);

    Serial0.println(
        "[WEB][WIFI] Restarting ESP32..."
    );

    ESP.restart();
}

// ============================================================
// FORGET
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

    serializeJson(
        doc,
        response
    );

    server.send(
        200,
        "application/json",
        response
    );

    delay(500);

    ESP.restart();
}

// ============================================================
// SETUP
// ============================================================

void WebWiFiManager::handleSetup(
    WebServer& server
)
{
    Serial0.println(
        "[WEB][WIFI] Starting setup mode"
    );

    if (
        !_wifiManager.startSetupMode()
    )
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

    serializeJson(
        doc,
        response
    );

    server.send(
        200,
        "application/json",
        response
    );
}

// ============================================================
// ERROR
// ============================================================

void WebWiFiManager::sendError(
    WebServer& server,
    int code,
    const char* message
)
{
    JsonDocument doc;

    doc["ok"] = false;

    doc["error"] =
        message;

    String response;

    serializeJson(
        doc,
        response
    );

    server.send(
        code,
        "application/json",
        response
    );
}
bool WebWiFiManager::isSetupMode() const
{
    return _wifiManager.isSetupMode();
}
// ============================================================
// IP
// ============================================================

String WebWiFiManager::getIP() const
{
    return _wifiManager.getIP();
}


// ============================================================
// AP IP
// ============================================================

String WebWiFiManager::getAPIP() const
{
    return _wifiManager.getAPIP();
}