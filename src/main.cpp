
#include <Arduino.h>

#include "./managers/Setting/SettingsManager.h"
#include "./managers/Led/LightingManager.h"

// ============================================================
// MANAGERS
// ============================================================

SettingsManager settings;
LightingManager lighting(settings);

// ============================================================
// TEST TIMING
// ============================================================

constexpr uint32_t START_DELAY_MS = 10000;
constexpr uint32_t FADE_DURATION_MS = 60000;
constexpr uint32_t FADE_UPDATE_INTERVAL_MS = 50;

// ============================================================
// TEST STATE
// ============================================================

uint32_t testStartTime = 0;
uint32_t fadeStartTime = 0;
uint32_t lastFadeUpdate = 0;

bool lightingEnabled = false;
bool fadeCompleted = false;

// ============================================================
// SETUP
// ============================================================

void setup() {

    Serial0.begin(115200);
    delay(500);

    Serial0.println();
    Serial0.println("================================");
    Serial0.println("       LIGHTING TEST");
    Serial0.println("================================");

    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    Serial0.println("[Main] Initializing settings...");

    if (!settings.begin()) {
        Serial0.println("[Main] Settings initialization FAILED!");

        while (true) {
            delay(1000);
        }
    }

    Serial0.println("[Main] Settings initialized");

    // --------------------------------------------------------
    // FORCE LIGHTING OFF
    // --------------------------------------------------------

    Serial0.println("[Main] Forcing lighting OFF");

    settings.set(
        SettingsManager::Param::MATRIX_ENABLED,
        0
    );

    settings.set(
        SettingsManager::Param::COB_ENABLED,
        0
    );

    settings.set(
        SettingsManager::Param::MATRIX_BRIGHTNESS,
        0
    );

    settings.set(
        SettingsManager::Param::COB_BRIGHTNESS,
        0
    );

    settings.flush();

    // --------------------------------------------------------
    // LIGHTING
    // --------------------------------------------------------

    Serial0.println("[Main] Initializing lighting...");

    if (!lighting.begin()) {
        Serial0.println("[Main] Lighting initialization FAILED!");

        while (true) {
            delay(1000);
        }
    }

    Serial0.println("[Main] Lighting initialized");

    // --------------------------------------------------------
    // START TIMER
    // --------------------------------------------------------

    testStartTime = millis();

    Serial0.println("[Main] Lighting test started");
    Serial0.println("[Main] Waiting 10 seconds...");
}

// ============================================================
// LOOP
// ============================================================

void loop() {

    const uint32_t now = millis();

    settings.update();

    // --------------------------------------------------------
    // STEP 1: WAIT 10 SECONDS
    // --------------------------------------------------------

    if (!lightingEnabled) {

        if (now - testStartTime >= START_DELAY_MS) {

            Serial0.println("[Main] Enabling matrix and COB");

            settings.set(
                SettingsManager::Param::MATRIX_ENABLED,
                1
            );

            settings.set(
                SettingsManager::Param::COB_ENABLED,
                1
            );

            settings.set(
                SettingsManager::Param::MATRIX_BRIGHTNESS,
                0
            );

            settings.set(
                SettingsManager::Param::COB_BRIGHTNESS,
                0
            );

            lightingEnabled = true;

            fadeStartTime = now;
            lastFadeUpdate = now;
        }
    }

    // --------------------------------------------------------
    // STEP 2: GRADUAL BRIGHTNESS INCREASE
    // --------------------------------------------------------

    if (lightingEnabled && !fadeCompleted) {

        if (now - lastFadeUpdate >= FADE_UPDATE_INTERVAL_MS) {

            lastFadeUpdate = now;

            const uint32_t elapsed = now - fadeStartTime;

            uint8_t brightness = 0;

            if (elapsed >= FADE_DURATION_MS) {

                brightness = 100;
                fadeCompleted = true;

            } else {

                brightness = static_cast<uint8_t>(
                    (elapsed * 100UL) / FADE_DURATION_MS
                );
            }

            settings.set(
                SettingsManager::Param::MATRIX_BRIGHTNESS,
                brightness
            );

            settings.set(
                SettingsManager::Param::COB_BRIGHTNESS,
                brightness
            );

            Serial0.print("[Main] Brightness: ");
            Serial0.print(brightness);
            Serial0.println("%");

            if (fadeCompleted) {
                Serial0.println("[Main] Fade completed");
            }
        }
    }

    // --------------------------------------------------------
    // UPDATE LIGHTING
    // --------------------------------------------------------

    lighting.update();

    delay(1);
}
