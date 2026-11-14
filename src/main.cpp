#include <Arduino.h>

#include "managers/Setting/SettingsManager.h"
#include "managers/I2S/I2SManager.h"
#include "managers/Led/LightingManager.h"
#include "managers/Sound/SoundManager.h"

#include "managers/Alarm/SunriseAlarm.h"

// ============================================================
// MANAGERS
// ============================================================

SettingsManager settingsManager;

I2SManager i2sManager;

LightingManager lightingManager(
    settingsManager
);

SoundManager soundManager(
    settingsManager,
    i2sManager
);

// ============================================================
// SUNRISE ALARM
// ============================================================

SunriseAlarm sunriseAlarm(
    lightingManager,
    soundManager
);

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);

    delay(500);

    Serial0.println();
    Serial0.println("================================");
    Serial0.println("       SUNRISE ALARM TEST       ");
    Serial0.println("================================");

    // Settings.
    Serial0.println("[MAIN] Initializing settings");

    settingsManager.begin();

    // I2S.
    Serial0.println("[MAIN] Initializing I2S");

    i2sManager.begin();

    // Lighting.
    Serial0.println("[MAIN] Initializing lighting");

    lightingManager.begin();

    // Sound.
    Serial0.println("[MAIN] Initializing sound");

    soundManager.begin();

    // Sunrise.
    Serial0.println("[MAIN] Starting sunrise alarm");

    sunriseAlarm.start();

    Serial0.println("[MAIN] Setup completed");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    sunriseAlarm.update();

    delay(1);
}