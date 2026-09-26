#pragma once

#include <Arduino.h>

struct Settings
{
    // =========================================================
    // SYSTEM
    // =========================================================

    String timeZone = "Europe/Kyiv";

    bool use24Hour = true;

    // =========================================================
    // DISPLAY
    // =========================================================

    uint8_t brightness = 80;

    bool autoBrightness = true;

    // =========================================================
    // AUDIO
    // =========================================================

    uint8_t volume = 70;

    bool soundEnabled = true;

    bool voiceEnabled = true;

    // =========================================================
    // AI
    // =========================================================

    bool aiEnabled = true;

    bool llmEnabled = true;

    bool ttsEnabled = true;

    bool mlEnabled = true;

    String llmModel = "default";

    String ttsModel = "default";

    // =========================================================
    // SENSORS
    // =========================================================

    bool sensorsEnabled = true;

    bool environmentSensorsEnabled = true;

    bool motionSensorsEnabled = true;

    // =========================================================
    // STORAGE
    // =========================================================

    bool loggingEnabled = true;

    bool sensorLoggingEnabled = true;

    bool aiLoggingEnabled = true;
};