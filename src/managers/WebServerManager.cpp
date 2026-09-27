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
    Serial.println();
    Serial.println("[WEB] Starting Wi-Fi...");

    WiFi.mode(WIFI_STA);

    WiFi.begin(
        ssid,
        password
    );

    Serial.print("[WEB] Connecting");

    uint32_t startTime = millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 15000
    )
    {
        delay(300);

        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(
            "[WEB] Wi-Fi connection failed"
        );

        return false;
    }

    Serial.println(
        "[WEB] Wi-Fi connected"
    );

    Serial.print(
        "[WEB] IP: "
    );

    Serial.println(
        WiFi.localIP()
    );


    // ========================================================
    // LITTLEFS
    // ========================================================

    if (!LittleFS.begin(true))
    {
        Serial.println(
            "[WEB] LittleFS mount failed"
        );

        return false;
    }

    Serial.println(
        "[WEB] LittleFS mounted"
    );


    setupRoutes();


    _server.begin();

    Serial.println(
        "[WEB] HTTP server started"
    );

    _initialized = true;

    return true;
}


// ============================================================
// ROUTES
// ============================================================

void WebServerManager::setupRoutes()
{
    _server.on(
        "/",
        HTTP_GET,
        [this]()
        {
            handleRoot();
        }
    );


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


    File file =
        LittleFS.open(
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
// STATUS
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
    return WiFi.localIP().toString();
}