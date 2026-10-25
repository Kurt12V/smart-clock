#include <Arduino.h>
#include "./core/App.h"

// ============================================================
// APPLICATION
// ============================================================

static App app;

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);
    delay(500);

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("           SMART CLOCK");
    Serial0.println("========================================");

    if (!app.begin())
    {
        Serial0.println("[MAIN] Application initialization FAILED");
        return;
    }

    Serial0.println("[MAIN] Application initialized successfully");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    app.update();
}