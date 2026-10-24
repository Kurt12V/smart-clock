#pragma once

#include <Arduino.h>

#include "ui/ClockScreen.h"

class ClockSystem;
class SensorManager;
class LVGLManager;


// ============================================================
// SCREEN MANAGER
// ============================================================

class ScreenManager
{
public:

    ScreenManager(
        ClockSystem& clock,
        SensorManager& sensors,
        LVGLManager& lvgl
    );

    bool begin();

    void update();

    bool isReady() const;


    // ========================================================
    // CENTER
    // ========================================================

    void setCenterMode(
        uint8_t screen,
        CenterMode mode
    );

    void setCenterText(
        uint8_t screen,
        const char* text
    );

    void setCenterText(
        uint8_t screen,
        uint8_t digit
    );

    void setCenterText(
        uint8_t screen,
        uint8_t topDigit,
        uint8_t bottomDigit
    );


    // ========================================================
    // TOP / BOTTOM
    // ========================================================

    void setTopText(
        uint8_t screen,
        const char* text
    );

    void setBottomText(
        uint8_t screen,
        const char* text
    );


    // ========================================================
    // VISIBILITY
    // ========================================================

    void setVisible(
        uint8_t screen,
        bool visible
    );


    // ========================================================
    // ACCESS
    // ========================================================

    ClockScreen& screen(
        uint8_t index
    );


private:

    // ========================================================
    // AUTOMATIC DATA UPDATE
    // ========================================================

    void updateClock();

    void updateSensors();

    void updateDate();


    // ========================================================
    // REFERENCES
    // ========================================================

    ClockSystem& _clock;

    SensorManager& _sensors;

    LVGLManager& _lvgl;


    // ========================================================
    // SCREENS
    // ========================================================

    ClockScreen _screens[4];


    // ========================================================
    // STATE
    // ========================================================

    bool _initialized;

    uint32_t _lastUpdate;
};
