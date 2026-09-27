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
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("          SMART CLOCK");
    Serial.println("        WEB CONTROL TEST");
    Serial.println("========================================");


    // ========================================================
    // SETTINGS
    // ========================================================

    Serial.println("[MAIN] Settings initialized");


    Serial.print("[MAIN] WiFi SSID: ");
    Serial.println(settings.wifi.ssid);

    Serial.print("[MAIN] Volume: ");
    Serial.println(settings.audio.volume);

    Serial.print("[MAIN] Audio: ");
    Serial.println(
        settings.audio.enabled
            ? "ON"
            : "OFF"
    );

    Serial.print("[MAIN] UTC offset: ");
    Serial.println(
        static_cast<int>(
            settings.clock.utcOffset
        )
    );


    // ========================================================
    // WIFI / WEB SERVER
    // ========================================================

    Serial.println();
    Serial.println("[MAIN] Starting web server...");

    bool webStarted =
        webServer.begin(
            settings.wifi.ssid.c_str(),
            settings.wifi.password.c_str()
        );


    if (webStarted)
    {
        Serial.println();
        Serial.println("========================================");
        Serial.println("[MAIN] WEB SERVER READY");
        Serial.print("[MAIN] IP: http://");
        Serial.print(webServer.getIP());
        Serial.println("/");
        Serial.println("========================================");
    }
    else
    {
        Serial.println();
        Serial.println("========================================");
        Serial.println("[MAIN] WEB SERVER FAILED");
        Serial.println("========================================");
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

                Serial.println(
                    "[MAIN] WiFi connected"
                );

                Serial.print(
                    "[MAIN] IP: "
                );

                Serial.println(
                    WiFi.localIP()
                );
            }
        }
        else
        {
            Serial.println(
                "[MAIN] WiFi disconnected"
            );
        }
    }


    delay(2);
}