#include <Arduino.h>

#include "Config.h"
#include "Pins.h"
#include "Constants.h"
#include "Version.h"

#include "managers/SettingsManager.h"
#include "managers/WebServerManager.h"

// ============================================================
// WIFI CREDENTIALS (в коде)
// ============================================================

static constexpr const char* WIFI_SSID     = "tpl47";
static constexpr const char* WIFI_PASSWORD = "12713714";

// ============================================================
// GLOBALS
// ============================================================

SettingsManager  settings;
WebServerManager webServer;

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(Config::SERIAL_BAUD_RATE);
    delay(1000);

    Serial0.println();
    Serial0.println("=== SMARTCLOCK ===");

    settings.begin();

    if (!webServer.begin(settings, WIFI_SSID, WIFI_PASSWORD))
    {
        Serial0.println("[MAIN] WebServer FAILED");
    }
    else
    {
        Serial0.print("[MAIN] Web: http://");
        Serial0.println(webServer.getIP());
    }
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    webServer.update();
    delay(10);
}