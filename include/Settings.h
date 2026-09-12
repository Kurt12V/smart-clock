#pragma once

#include <Arduino.h>
#include "Constants.h"

namespace Settings
{

// ============================================================
// DISPLAY
// ============================================================

// struct Display
// {
//     bool enabled = true;
//     uint8_t brightness = 100;
//     bool autoBrightness = false;
//     uint16_t refreshRate = 60;
// };

// ============================================================
// LED MATRIX
// ============================================================

// struct Matrix
// {
//     bool enabled = true;
//     uint8_t brightness = 50;
//     bool autoBrightness = false;

//     uint8_t width = 16;
//     uint8_t height = 16;

//     bool showEffects = true;
// };

// ============================================================
// COB LED
// ============================================================

// struct CobLed
// {
//     bool enabled = true;

//     uint8_t brightness1 = 100;
//     uint8_t brightness2 = 100;
//     uint8_t brightness3 = 100;
//     uint8_t brightness4 = 100;

//     uint16_t fadeTime = 500;

//     bool smoothTransition = true;
// };

// ============================================================
// AUDIO
// ============================================================

// struct Audio
// {
//     bool enabled = true;

//     uint8_t volume = 70;

//     bool stereo = true;

//     uint8_t sampleRate = 44;

//     bool notifications = true;
//     bool alarms = true;
//     bool timers = true;
// };

// ============================================================
// MICROPHONE
// ============================================================

// struct Microphone
// {
//     bool enabled = true;

//     uint8_t gain = 50;

//     uint32_t sampleRate = 16000;

//     bool voiceControl = false;
// };


// ============================================================
// LIGHT SENSOR
// ============================================================

// struct LightSensor
// {
//     bool enabled = true;

//     uint32_t updateInterval = 1000;

//     bool autoBrightness = true;
// };

// ============================================================
// DISTANCE SENSOR
// ============================================================

// struct DistanceSensor
// {
//     bool enabled = true;

//     uint32_t updateInterval = 500;

//     uint16_t maxDistance = 4000;

//     bool presenceDetection = false;
// };

// ============================================================
// CLOCK
// ============================================================

struct Clock
{
    Constants::UtcOffset utcOffset =
        Constants::UtcOffset::Plus3;
};

// ============================================================
// NOTIFICATIONS
// ============================================================

// struct Notifications
// {
//     bool enabled = true;

//     uint8_t duration = 5;

//     bool sound = true;

//     bool showOnDisplay = true;

//     bool showOnMatrix = true;
// };

// ============================================================
// ALARM
// ============================================================

// struct Alarm
// {
//     bool enabled = false;

//     uint8_t hour = 7;
//     uint8_t minute = 0;

//     bool repeat = false;

//     bool monday = true;
//     bool tuesday = true;
//     bool wednesday = true;
//     bool thursday = true;
//     bool friday = true;
//     bool saturday = false;
//     bool sunday = false;

//     uint8_t volume = 70;

//     bool gradualVolume = true;

//     uint16_t fadeTime = 30000;
// };

// ============================================================
// TIMER
// ============================================================

// struct Timer
// {
//     bool enabled = true;

//     uint32_t defaultDuration = 60;

//     bool sound = true;

//     bool vibration = false;
// };

// ============================================================
// WIFI
// ============================================================

// struct WiFi
// {
//     bool enabled = true;

//     bool autoConnect = true;

//     bool autoReconnect = true;

//     uint32_t reconnectInterval = 10000;

//     uint32_t connectionTimeout = 15000;

//     bool syncTime = true;
// };

// ============================================================
// POWER
// ============================================================

// struct Power
// {
//     bool sleepEnabled = true;

//     uint32_t sleepTimeout = 300000;

//     uint8_t sleepBrightness = 10;

//     bool displayOff = false;

//     bool matrixOff = true;

//     bool audioOff = true;
// };

// ============================================================
// UI
// ============================================================

// struct UI
// {
//     uint8_t brightness = 100;

//     bool animations = true;

//     bool sounds = true;

//     bool showNotifications = true;

//     bool showSensors = true;

//     uint16_t animationDuration = 300;
// };

// ============================================================
// ENCODER
// ============================================================

// struct Encoder
// {
//     bool enabled = true;

//     uint8_t stepsPerClick = 1;

//     bool acceleration = true;

//     uint16_t accelerationDelay = 100;

//     bool buttonEnabled = true;

//     uint16_t debounceTime = 50;
// };

// ============================================================
// STORAGE
// ============================================================

// struct Storage
// {
//     bool enabled = true;

//     bool useSD = true;

//     bool autoSave = true;

//     uint32_t saveInterval = 30000;
// };

// ============================================================
// SYSTEM
// ============================================================

// struct System
// {
//     bool debug = true;

//     bool serial = true;

//     uint32_t serialBaud = 115200;

//     bool watchdog = true;

//     uint32_t watchdogTimeout = 10000;
// };

// ============================================================
// COMPLETE DATA
// ============================================================

struct Data
{
//     Display display;

//     Matrix matrix;

//     CobLed cobLed;

//     Audio audio;

//     Microphone microphone;

//     Environment environment;

//     LightSensor lightSensor;

//     DistanceSensor distanceSensor;

     Clock clock;

//     Notifications notifications;

//     Alarm alarm;

//     Timer timer;

//     WiFi wifi;

//     Power power;

//     UI ui;

//     Encoder encoder;

//     Storage storage;

//     System system;
};

} // namespace Settings