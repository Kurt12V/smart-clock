#include "WebServerManager.h"

WebServerManager::WebServerManager()
    : _server(80),
      _initialized(false)
{
}

// ============================================================
// BEGIN
// ============================================================

bool WebServerManager::begin(
    const char* ssid,
    const char* password
)
{
    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("          WEB SERVER START");
    Serial0.println("========================================");

    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    Serial0.println("[WEB] Setting WiFi mode: STA");

    WiFi.mode(WIFI_STA);

    // Полностью очищаем старое состояние подключения
    WiFi.disconnect(true, true);
    delay(500);

    Serial0.print("[WEB] SSID: ");
    Serial0.println(ssid);

    Serial0.println("[WEB] Starting WiFi connection...");

    WiFi.begin(
        ssid,
        password
    );

    uint32_t startTime = millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 20000
    )
    {
        delay(500);

        Serial0.print("[WEB] WiFi status: ");
        Serial0.println(WiFi.status());
    }

    // --------------------------------------------------------
    // CHECK WIFI
    // --------------------------------------------------------

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial0.println();
        Serial0.println("[WEB] ====================================");
        Serial0.println("[WEB] WIFI CONNECTION FAILED");
        Serial0.println("[WEB] ====================================");

        Serial0.print("[WEB] Status code: ");
        Serial0.println(WiFi.status());

        Serial0.print("[WEB] SSID: ");
        Serial0.println(WiFi.SSID());

        Serial0.print("[WEB] RSSI: ");
        Serial0.println(WiFi.RSSI());

        return false;
    }

    Serial0.println();
    Serial0.println("[WEB] WiFi connected!");

    Serial0.print("[WEB] IP address: ");
    Serial0.println(WiFi.localIP());

    Serial0.print("[WEB] Gateway: ");
    Serial0.println(WiFi.gatewayIP());

    Serial0.print("[WEB] Subnet: ");
    Serial0.println(WiFi.subnetMask());

    Serial0.print("[WEB] RSSI: ");
    Serial0.println(WiFi.RSSI());

    // --------------------------------------------------------
    // LITTLEFS
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println("[WEB] Mounting LittleFS...");

    if (!LittleFS.begin(true))
    {
        Serial0.println(
            "[WEB] LittleFS mount FAILED"
        );

        return false;
    }

    Serial0.println(
        "[WEB] LittleFS mounted"
    );

    // Проверяем index.html
    if (LittleFS.exists("/index.html"))
    {
        Serial0.println(
            "[WEB] /index.html found"
        );
    }
    else
    {
        Serial0.println(
            "[WEB] WARNING: /index.html NOT FOUND"
        );
    }

    // --------------------------------------------------------
    // ROUTES
    // --------------------------------------------------------

    setupRoutes();

    // --------------------------------------------------------
    // HTTP SERVER
    // --------------------------------------------------------

    _server.begin();

    Serial0.println();
    Serial0.println("[WEB] HTTP server started");

    Serial0.print("[WEB] Open in browser: http://");
    Serial0.println(WiFi.localIP());

    _initialized = true;

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("          WEB SERVER READY");
    Serial0.println("========================================");

    return true;
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
    // NOT FOUND
    // --------------------------------------------------------

    _server.onNotFound(
        [this]()
        {
            handleNotFound();
        }
    );
}

// ============================================================
// ROOT
// ============================================================

void WebServerManager::handleRoot()
{
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
        "text/html"
    );

    file.close();
}

// ============================================================
// NOT FOUND
// ============================================================

void WebServerManager::handleNotFound()
{
    Serial0.print("[WEB] 404: ");
    Serial0.println(_server.uri());

    _server.send(
        404,
        "text/plain",
        "404 - Not Found"
    );
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
    return WiFi.status() == WL_CONNECTED;
}

// ============================================================
// IP
// ============================================================

String WebServerManager::getIP() const
{
    if (WiFi.status() != WL_CONNECTED)
        return String("0.0.0.0");

    return WiFi.localIP().toString();
}