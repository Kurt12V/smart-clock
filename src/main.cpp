
#include <Arduino.h>
#include "./core/App.h"

// ============================================================
// GLOBAL APPLICATION
// ============================================================

App app;

bool appInitialized = false;

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);
    delay(500);

    Serial0.println();
    Serial0.println("================================");
    Serial0.println("       SMARTCLOCK START");
    Serial0.println("================================");

    Serial0.println("[MAIN] Starting App...");

    if (!app.begin())
    {
        Serial0.println("[MAIN] ERROR: App initialization FAILED!");
        return;
    }

    appInitialized = true;

    Serial0.println("[MAIN] App initialized successfully");
    Serial0.println("[MAIN] Entering main loop...");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    if (!appInitialized)
    {
        delay(1000);
        return;
    }

    static uint32_t lastDebugTime = 0;

    // Проверяем работу основного цикла раз в секунду.
    app.update();

    const uint32_t now = millis();

    if (now - lastDebugTime >= 1000)
    {
        lastDebugTime = now;

        Serial0.println("[MAIN] LOOP RUNNING");
    }
}
