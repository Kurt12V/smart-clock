#include <Arduino.h>
#include <WiFi.h>

#include "Config.h"
#include "Pins.h"
#include "Constants.h"
#include "Version.h"

#include "Settings.h"
#include "./managers/WebServerManager.h"

// ============================================================
// GLOBAL SETTINGS
// ============================================================

Settings::Data settings;


// ============================================================
// WEB SERVER
// ============================================================

WebServerManager webServer;


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);

    delay(1000);

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("          SMART CLOCK");
    Serial0.println("        WEB CONTROL TEST");
    Serial0.println("========================================");


    // ========================================================
    // SETTINGS
    // ========================================================

    Serial0.println("[MAIN] Settings initialized");


    Serial0.print("[MAIN] WiFi SSID: ");
    Serial0.println(settings.wifi.ssid);

    Serial0.print("[MAIN] Volume: ");
    Serial0.println(settings.audio.volume);

    Serial0.print("[MAIN] Audio: ");
    Serial0.println(
        settings.audio.enabled
            ? "ON"
            : "OFF"
    );

    Serial0.print("[MAIN] UTC offset: ");
    Serial0.println(
        static_cast<int>(
            settings.clock.utcOffset
        )
    );


    // ========================================================
    // WIFI / WEB SERVER
    // ========================================================

    Serial0.println();
    Serial0.println("[MAIN] Starting web server...");

    bool webStarted =
        webServer.begin(
            settings.wifi.ssid.c_str(),
            settings.wifi.password.c_str()
        );


    if (webStarted)
    {
        Serial0.println();
        Serial0.println("========================================");
        Serial0.println("[MAIN] WEB SERVER READY");
        Serial0.print("[MAIN] IP: http://");
        Serial0.print(webServer.getIP());
        Serial0.println("/");
        Serial0.println("========================================");
    }
    else
    {
        Serial0.println();
        Serial0.println("========================================");
        Serial0.println("[MAIN] WEB SERVER FAILED");
        Serial0.println("========================================");
    }
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    // ========================================================
    // WEB SERVER
    // ========================================================

    webServer.update();


    // ========================================================
    // WIFI STATUS
    // ========================================================

    static uint32_t lastWiFiCheck = 0;

    if (millis() - lastWiFiCheck >= 5000)
    {
        lastWiFiCheck = millis();


        if (WiFi.status() == WL_CONNECTED)
        {
            static bool wasConnected = false;

            if (!wasConnected)
            {
                wasConnected = true;

                Serial0.println(
                    "[MAIN] WiFi connected"
                );

                Serial0.print(
                    "[MAIN] IP: "
                );

                Serial0.println(
                    WiFi.localIP()
                );
            }
        }
        else
        {
            Serial0.println(
                "[MAIN] WiFi disconnected"
            );
        }
    }


    delay(2);
}