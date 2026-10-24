#include <Arduino.h>
#include <Preferences.h>

#include "core/App.h"

static App app;

static void clearWiFiCredentials()
{
    Preferences preferences;

    if (!preferences.begin("wifi", false))
    {
        Serial0.println("[TEST] ERROR: Failed to open Wi-Fi preferences");
        return;
    }

    // Очищаем сохранённые SSID и пароль.
    preferences.putString("ssid", "");
    preferences.putString("pass", "");

    preferences.end();

    Serial0.println("[TEST] Wi-Fi SSID cleared");
    Serial0.println("[TEST] Wi-Fi password cleared");
}

void setup()
{
    Serial0.begin(115200);
    delay(300);

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("          SmartClock");
    Serial0.println("       Wi-Fi connection test");
    Serial0.println("========================================");

    // Очищаем Wi-Fi-данные до запуска приложения.
    clearWiFiCredentials();

    Serial0.println("[APP] Starting SmartClock...");

    app.begin();
}

void loop()
{
    app.update();
    delay(1);
}