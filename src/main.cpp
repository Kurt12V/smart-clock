#include <Arduino.h>

#include "Config.h"
#include "Pins.h"
#include "./managers/Setting/SettingsManager.h"
#include "./managers/Led/LightingManager.h"
#include "./managers/Led/LedMatrixManager.h"

// ============================================================
// GLOBALS
// ============================================================

SettingsManager settings;
LightingManager lighting(settings);

// ============================================================
// HELPERS
// ============================================================

static void printSeparator(const char* title)
{
    Serial0.println();
    Serial0.println("========================================");
    Serial0.print  ("  ");
    Serial0.println(title);
    Serial0.println("========================================");
}

static void printMatrixSettings()
{
    Serial0.println("[Test] Current matrix settings from SettingsManager:");

    Serial0.printf(
        "  MATRIX_ENABLED    = %d\n",
        settings.get(SettingsManager::Param::MATRIX_ENABLED));

    Serial0.printf(
        "  MATRIX_BRIGHTNESS = %d\n",
        settings.get(SettingsManager::Param::MATRIX_BRIGHTNESS));

    Serial0.printf(
        "  MATRIX_EFFECT     = %d\n",
        settings.get(SettingsManager::Param::MATRIX_EFFECT));

    Serial0.printf(
        "  MATRIX_SPEED      = %d\n",
        settings.get(SettingsManager::Param::MATRIX_SPEED));
}

static void updateAll(uint32_t ms)
{
    const uint32_t start = millis();
    while (millis() - start < ms)
    {
        settings.update();
        lighting.update();
        delay(5);
    }
}

// ============================================================
// TEST 1: MATRIX ON/OFF
// ============================================================

static void testEnabled()
{
    printSeparator("TEST: MATRIX_ENABLED toggle");

    Serial0.println("[Test] set MATRIX_ENABLED = 1");
    settings.set(SettingsManager::Param::MATRIX_ENABLED, 1);
    updateAll(500);

    Serial0.println("[Test] set MATRIX_ENABLED = 0 (should clear)");
    settings.set(SettingsManager::Param::MATRIX_ENABLED, 0);
    updateAll(500);

    Serial0.println("[Test] set MATRIX_ENABLED = 1 (restore)");
    settings.set(SettingsManager::Param::MATRIX_ENABLED, 1);
    updateAll(500);
}

// ============================================================
// TEST 2: BRIGHTNESS SWEEP
// ============================================================

static void testBrightness()
{
    printSeparator("TEST: MATRIX_BRIGHTNESS sweep");

    // Включаем эффект, чтобы было видно яркость
    settings.set(SettingsManager::Param::MATRIX_EFFECT,
                 Config::MATRIX_EFFECT_DEFAULT);
    settings.set(SettingsManager::Param::MATRIX_SPEED,
                 Config::MATRIX_SPEED_DEFAULT);
    updateAll(300);

    static const uint8_t steps[] = { 10, 40, 80, 160, 255, 80, 30 };

    for (uint8_t i = 0; i < sizeof(steps); ++i)
    {
        Serial0.printf("[Test] brightness = %u\n", (unsigned)steps[i]);

        settings.set(
            SettingsManager::Param::MATRIX_BRIGHTNESS,
            steps[i]);

        updateAll(600);
    }
}

// ============================================================
// TEST 3: EFFECT SWEEP
// ============================================================

static void testEffects()
{
    printSeparator("TEST: MATRIX_EFFECT sweep");

    settings.set(SettingsManager::Param::MATRIX_BRIGHTNESS, 80);
    settings.set(SettingsManager::Param::MATRIX_SPEED, 128);

    const int minE = Config::MATRIX_EFFECT_MIN;
    const int maxE = Config::MATRIX_EFFECT_MAX;

    for (int e = minE; e <= maxE; ++e)
    {
        Serial0.printf("[Test] effect = %d\n", e);

        settings.set(
            SettingsManager::Param::MATRIX_EFFECT,
            e);

        updateAll(1200);
    }
}

// ============================================================
// TEST 4: SPEED SWEEP
// ============================================================

static void testSpeed()
{
    printSeparator("TEST: MATRIX_SPEED sweep");

    settings.set(SettingsManager::Param::MATRIX_BRIGHTNESS, 80);

    // Используем дефолтный эффект (обычно анимация), чтобы скорость была заметна
    settings.set(SettingsManager::Param::MATRIX_EFFECT,
                 Config::MATRIX_EFFECT_DEFAULT);
    updateAll(300);

    static const uint8_t speeds[] = { 0, 64, 128, 192, 255 };

    for (uint8_t i = 0; i < sizeof(speeds); ++i)
    {
        Serial0.printf("[Test] speed = %u\n", (unsigned)speeds[i]);

        settings.set(
            SettingsManager::Param::MATRIX_SPEED,
            speeds[i]);

        updateAll(1200);
    }
}

// ============================================================
// TEST 5: ALARM OVERRIDE
// ============================================================

static void testAlarmOverride()
{
    printSeparator("TEST: Alarm override");

    Serial0.println("[Test] beginAlarmOverride()");
    lighting.beginAlarmOverride();
    updateAll(200);

    Serial0.println("[Test] setAlarmMatrix() red 100");
    lighting.setAlarmMatrix(100, 255, 0, 0);
    updateAll(600);

    Serial0.println("[Test] setAlarmMatrix() green 100");
    lighting.setAlarmMatrix(100, 0, 255, 0);
    updateAll(600);

    Serial0.println("[Test] setAlarmMatrix() blue 255");
    lighting.setAlarmMatrix(255, 0, 0, 255);
    updateAll(600);

    Serial0.println("[Test] setAlarmMatrix() white 40");
    lighting.setAlarmMatrix(40, 255, 255, 255);
    updateAll(600);

    Serial0.println("[Test] endAlarmOverride()");
    lighting.endAlarmOverride();
    updateAll(800);
}

// ============================================================
// TEST 6: RESET TO DEFAULTS
// ============================================================

static void testResetDefaults()
{
    printSeparator("TEST: reset matrix params to defaults");

    settings.set(SettingsManager::Param::MATRIX_ENABLED,
                 Config::MATRIX_ENABLED_DEFAULT);

    settings.set(SettingsManager::Param::MATRIX_BRIGHTNESS,
                 Config::MATRIX_BRIGHTNESS_DEFAULT);

    settings.set(SettingsManager::Param::MATRIX_EFFECT,
                 Config::MATRIX_EFFECT_DEFAULT);

    settings.set(SettingsManager::Param::MATRIX_SPEED,
                 Config::MATRIX_SPEED_DEFAULT);

    updateAll(1000);
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);
    delay(300);

    printSeparator("Matrix test via LightingManager");

    Serial0.println("[Main] settings.begin()");
    if (!settings.begin())
    {
        Serial0.println("[Main] WARNING: settings.begin() failed, using defaults");
    }

    printMatrixSettings();

    Serial0.println("[Main] lighting.begin()");
    if (!lighting.begin())
    {
        Serial0.println("[Main] WARNING: lighting.begin() reported error (COB?)");
    }

    // Однократный прогон тестов
    testEnabled();
    testBrightness();
    testEffects();
    testSpeed();
    testAlarmOverride();
    testResetDefaults();

    printSeparator("All tests done");

    settings.flush();
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    settings.update();
    lighting.update();
    delay(5);
}