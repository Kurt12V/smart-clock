#include <Arduino.h>
#include <WiFi.h>

#include "Config.h"
#include "Pins.h"
#include "Constants.h"
#include "Version.h"

#include "./managers/SettingsManager.h"
#include "./managers/WebServerManager.h"


// ============================================================
// GLOBAL OBJECTS
// ============================================================

SettingsManager settings;
WebServerManager webServer;


// ============================================================
// WIFI SETTINGS
// ============================================================

// УКАЖИ СВОЮ Wi-Fi СЕТЬ
static const char* WIFI_SSID =
    "tpl47";

static const char* WIFI_PASSWORD =
    "12713714";


// ============================================================
// WIFI CHECK INTERVAL
// ============================================================

static constexpr uint32_t WIFI_CHECK_INTERVAL =
    5000;


// ============================================================
// SETUP
// ============================================================

void setup()
{
    // --------------------------------------------------------
    // SERIAL
    // --------------------------------------------------------

    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println();
    Serial.println(
        "========================================"
    );
    Serial.println(
        "          ESP32 SMART CLOCK"
    );
    Serial.println(
        "             WEB CONTROL"
    );
    Serial.println(
        "========================================"
    );

    Serial.println();


    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    Serial.println(
        "[MAIN] Initializing SettingsManager..."
    );

    if (!settings.begin())
    {
        Serial.println(
            "[MAIN] ERROR: SettingsManager failed"
        );
    }
    else
    {
        Serial.println(
            "[MAIN] SettingsManager OK"
        );
    }


    // --------------------------------------------------------
    // PRINT SETTINGS
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[MAIN] Current settings"
    );

    Serial.println(
        "----------------------------------------"
    );

    Serial.print(
        "Volume: "
    );

    Serial.println(
        settings.volume()
    );


    Serial.print(
        "Max volume: "
    );

    Serial.println(
        settings.maxVolume()
    );


    Serial.print(
        "Audio: "
    );

    Serial.println(
        settings.audioEnabled()
            ? "ON"
            : "OFF"
    );


    Serial.print(
        "Microphone: "
    );

    Serial.println(
        settings.microphoneEnabled()
            ? "ON"
            : "OFF"
    );


    Serial.print(
        "Mic auto gain: "
    );

    Serial.println(
        settings.microphoneAutoGain()
            ? "ON"
            : "OFF"
    );


    Serial.print(
        "Mic gain: "
    );

    Serial.println(
        settings.microphoneGain()
    );


    Serial.print(
        "Mic sensitivity: "
    );

    Serial.println(
        settings.microphoneSensitivity()
    );


    Serial.print(
        "Noise reduction: "
    );

    Serial.println(
        settings.noiseReduction()
            ? "ON"
            : "OFF"
    );


    Serial.print(
        "Voice detection: "
    );

    Serial.println(
        settings.voiceDetection()
            ? "ON"
            : "OFF"
    );


    Serial.print(
        "Wake word: "
    );

    Serial.println(
        settings.wakeWordEnabled()
            ? "ON"
            : "OFF"
    );


    Serial.print(
        "Display brightness: "
    );

    Serial.println(
        settings.displayBrightness()
    );


    Serial.print(
        "Timezone: "
    );

    Serial.println(
        settings.timezone()
    );

    Serial.println(
        "----------------------------------------"
    );


    // --------------------------------------------------------
    // WEB SERVER
    // --------------------------------------------------------

    Serial.println();

    Serial.println(
        "[MAIN] Starting WebServer..."
    );


    if (
        webServer.begin(
            WIFI_SSID,
            WIFI_PASSWORD
        )
    )
    {
        Serial.println();

        Serial.println(
            "========================================"
        );

        Serial.println(
            "       WEB CONTROL STARTED"
        );

        Serial.println(
            "========================================"
        );

        Serial.print(
            "IP address: http://"
        );

        Serial.print(
            webServer.getIP()
        );

        Serial.println(
            "/"
        );

        Serial.println(
            "========================================"
        );

        Serial.println();
    }
    else
    {
        Serial.println();

        Serial.println(
            "========================================"
        );

        Serial.println(
            "       WEB SERVER ERROR"
        );

        Serial.println(
            "========================================"
        );

        Serial.println();
    }
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
    // WIFI MONITOR
    // --------------------------------------------------------

    static uint32_t lastWiFiCheck = 0;

    if (
        millis() - lastWiFiCheck >=
        WIFI_CHECK_INTERVAL
    )
    {
        lastWiFiCheck = millis();


        // ----------------------------------------------------
        // CONNECTED
        // ----------------------------------------------------

        if (
            WiFi.status() == WL_CONNECTED
        )
        {
            static bool wasConnected = false;

            if (!wasConnected)
            {
                wasConnected = true;

                Serial.println();

                Serial.println(
                    "[WIFI] Connected"
                );

                Serial.print(
                    "[WIFI] SSID: "
                );

                Serial.println(
                    WiFi.SSID()
                );

                Serial.print(
                    "[WIFI] IP: "
                );

                Serial.println(
                    WiFi.localIP()
                );

                Serial.print(
                    "[WIFI] RSSI: "
                );

                Serial.print(
                    WiFi.RSSI()
                );

                Serial.println(
                    " dBm"
                );

                Serial.println();
            }
        }


        // ----------------------------------------------------
        // DISCONNECTED
        // ----------------------------------------------------

        else
        {
            static bool wasConnected = true;

            if (wasConnected)
            {
                wasConnected = false;

                Serial.println();

                Serial.println(
                    "[WIFI] Disconnected"
                );

                Serial.println();
            }
        }
    }


    // --------------------------------------------------------
    // SMALL DELAY
    // --------------------------------------------------------

    delay(2);
}