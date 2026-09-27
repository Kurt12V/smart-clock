#pragma once

#include <Arduino.h>
#include "Constants.h"

namespace Settings
{

// ============================================================
// DISPLAY
// ============================================================

struct Display
{
    uint8_t brightness = 100;
};

// ============================================================
// LED MATRIX
// ============================================================

struct Matrix
{
    bool enabled = true;
    uint8_t brightness = 50;
};

// ============================================================
// COB LED
// ============================================================

struct CobLed
{
    bool enabled = true;
    uint8_t brightness = 100;
};

// ============================================================
// AUDIO
// ============================================================

struct Audio
{
    uint8_t volume = 70;
};

// ============================================================
// MICROPHONE
// ============================================================

struct Microphone
{
    bool enabled = true;
};

// ============================================================
// CLOCK
// ============================================================

struct Clock
{
    Constants::UtcOffset utcOffset =
        Constants::UtcOffset::Plus3;
};


// ============================================================
// WIFI
// ============================================================

struct WiFi
{
    String ssid = "tpl47";
    String password = "12713714";
};


struct Data
{
     Display display;

     Matrix matrix;

     CobLed cobLed;

     Audio audio;

     Microphone microphone;

     Clock clock;
     WiFi wifi;
};

} // namespace Settings