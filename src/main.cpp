#include <Arduino.h>

#include "./managers/Setting/SettingsManager.h"
#include "./managers/I2S/I2SManager.h"
#include "./managers/Sound/SoundManager.h"
#include "./managers/Led/LightingManager.h"
#include "./managers/Alarm/SunriseAlarm.h"

// ============================================================
// MANAGERS
// ============================================================

SettingsManager settings;
I2SManager i2sManager;

SoundManager soundManager(
    settings,
    i2sManager
);

LightingManager lightingManager(
    settings
);

SunriseAlarm sunriseAlarm(
    lightingManager,
    soundManager
);

// ============================================================
// TEST STATE
// ============================================================

uint32_t sunriseStartTime = 0;


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);
    delay(500);

    Serial0.println();
    Serial0.println("================================");
    Serial0.println("       SUNRISE ALARM TEST");
    Serial0.println("================================");

    // --------------------------------------------------------
    // 1. SETTINGS
    // --------------------------------------------------------

    Serial0.println("[TEST] Initializing settings...");

    if (!settings.begin())
    {
        Serial0.println("[TEST][ERROR] Settings failed");
        return;
    }

    // --------------------------------------------------------
    // 2. I2S
    // --------------------------------------------------------

    Serial0.println("[TEST] Initializing I2S...");

    if (!i2sManager.begin())
    {
        Serial0.println("[TEST][ERROR] I2S failed");
        return;
    }

    // --------------------------------------------------------
    // 3. LIGHTING
    // --------------------------------------------------------

    Serial0.println("[TEST] Initializing lighting...");

    if (!lightingManager.begin())
    {
        Serial0.println("[TEST][ERROR] Lighting failed");
        return;
    }

    // --------------------------------------------------------
    // 4. SOUND
    // --------------------------------------------------------

    Serial0.println("[TEST] Initializing sound...");

    if (!soundManager.begin())
    {
        Serial0.println("[TEST][ERROR] Sound failed");
        return;
    }

    // --------------------------------------------------------
    // 5. START SUNRISE ALARM
    // --------------------------------------------------------

    Serial0.println("[TEST] Starting sunrise alarm...");

    sunriseAlarm.start();

    sunriseStartTime = millis();

    Serial0.println("[TEST] Sunrise alarm started");
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    // Обслуживание аудиопотока обязательно
    soundManager.update();

    // Обновление освещения
    lightingManager.update();

    // Обновление сценария рассвета
    const uint32_t elapsedTime =
        millis() - sunriseStartTime;

    sunriseAlarm.update(elapsedTime);

    delay(1);
}