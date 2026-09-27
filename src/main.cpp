#include <Arduino.h>

// ============================================================
// PROJECT
// ============================================================

#include "Config.h"
#include "Pins.h"
#include "Constants.h"
#include "Version.h"

// ============================================================
// MANAGERS
// ============================================================

#include "./managers/SettingsManager.h"
#include "./managers/WebServerManager.h"

// ============================================================
// GLOBAL OBJECTS
// ============================================================

SettingsManager settings;
WebServerManager webServer;

// ============================================================
// SETUP
// ============================================================

void setup()
{
    // --------------------------------------------------------
    // SERIAL
    // --------------------------------------------------------

    Serial.begin(115200);

    delay(1500);

    Serial.println();
    Serial.println("========================================");
    Serial.println("          SMART CLOCK");
    Serial.println("        WEB CONTROL TEST");
    Serial.println("========================================");

    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    Serial.println();
    Serial.println("[MAIN] Initializing settings...");

    if (!settings.begin())
    {
        Serial.println(
            "[MAIN] Settings initialization FAILED"
        );
    }
    else
    {
        Serial.println(
            "[MAIN] Settings initialized"
        );
    }

    // --------------------------------------------------------
    // DISPLAY
    // --------------------------------------------------------

    Serial.println();

    Serial.print(
        "[MAIN] Display brightness: "
    );

    Serial.println(
        settings.displayBrightness()
    );

    // --------------------------------------------------------
    // MATRIX
    // --------------------------------------------------------

    Serial.print(
        "[MAIN] Matrix: "
    );

    Serial.println(
        settings.matrixEnabled()
            ? "ON"
            : "OFF"
    );

    Serial.print(
        "[MAIN] Matrix brightness: "
    );

    Serial.println(
        settings.matrixBrightness()
    );

    // --------------------------------------------------------
    // COB
    // --------------------------------------------------------

    Serial.print(
        "[MAIN] COB: "
    );

    Serial.println(
        settings.cobEnabled()
            ? "ON"
            : "OFF"
    );

    Serial.print(
        "[MAIN] COB brightness 1: "
    );

    Serial.println(
        settings.cobBrightness1()
    );

    Serial.print(
        "[MAIN] COB brightness 2: "
    );

    Serial.println(
        settings.cobBrightness2()
    );

    Serial.print(
        "[MAIN] COB brightness 3: "
    );

    Serial.println(
        settings.cobBrightness3()
    );

    Serial.print(
        "[MAIN] COB brightness 4: "
    );

    Serial.println(
        settings.cobBrightness4()
    );

    // --------------------------------------------------------
    // AUDIO
    // --------------------------------------------------------

    Serial.print(
        "[MAIN] Volume: "
    );

    Serial.println(
        settings.volume()
    );

    // --------------------------------------------------------
    // MICROPHONE
    // --------------------------------------------------------

    Serial.print(
        "[MAIN] Microphone: "
    );

    Serial.println(
        settings.microphoneEnabled()
            ? "ON"
            : "OFF"
    );

    // --------------------------------------------------------
    // CLOCK
    // --------------------------------------------------------

    Serial.print(
        "[MAIN] UTC offset: "
    );

    Serial.println(
        static_cast<int>(
            settings.utcOffset()
        )
    );

    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    Serial.println();

    Serial.print(
        "[MAIN] WiFi SSID: "
    );

    Serial.println(
        settings.wifiSSID()
    );

    // --------------------------------------------------------
    // WEB SERVER
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[MAIN] Starting web server..."
    );

    bool webStarted = webServer.begin(
        settings.wifiSSID(),
        settings.wifiPassword()
    );

    if (webStarted)
    {
        Serial.println();
        Serial.println("========================================");
        Serial.println("        WEB SERVER SUCCESS");
        Serial.println("========================================");

        Serial.print(
            "[MAIN] Open in browser: http://"
        );

        Serial.println(
            webServer.getIP()
        );
    }
    else
    {
        Serial.println();
        Serial.println("========================================");
        Serial.println("        WEB SERVER FAILED");
        Serial.println("========================================");
    }

    Serial.println();
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // WEB SERVER
    // --------------------------------------------------------

    webServer.update();

    // --------------------------------------------------------
    // WIFI STATUS
    // --------------------------------------------------------

    static uint32_t lastWiFiCheck = 0;

    if (
        millis() - lastWiFiCheck >= 5000
    )
    {
        lastWiFiCheck = millis();

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.print(
                "[MAIN] WiFi connected | IP: "
            );

            Serial.print(
                WiFi.localIP()
            );

            Serial.print(
                " | RSSI: "
            );

            Serial.println(
                WiFi.RSSI()
            );
        }
        else
        {
            Serial.print(
                "[MAIN] WiFi disconnected | status: "
            );

            Serial.println(
                WiFi.status()
            );
        }
    }

    delay(10);
}