#pragma once

#include <Arduino.h>

#include "./core/ClockSystem.h"
#include "./managers/SensorsManager.h"

#include "./managers/LVGLManager.h"
#include "./ui/ClockScreen.h"


class ScreenManager
{
public:

    static constexpr uint8_t SCREEN_COUNT = 4;


    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    ScreenManager(
        ClockSystem& clock,
        SensorManager& sensors,
        LVGLManager& lvgl
    );


    // ========================================================
    // SYSTEM
    // ========================================================

    bool begin();

    void update();


    bool isReady() const;


    // ========================================================
    // CENTER
    // ========================================================

    void setCenterMode(
        uint8_t screen,
        ClockScreen::CenterMode mode
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
    // TOP
    // ========================================================

    void setTopText(
        uint8_t screen,
        const char* text
    );


    // ========================================================
    // BOTTOM
    // ========================================================

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
    // SCREEN ACCESS
    // ========================================================

    ClockScreen& screen(
        uint8_t index
    );


private:

    // ========================================================
    // SYSTEM REFERENCES
    // ========================================================

    ClockSystem& _clock;

    SensorManager& _sensors;

    LVGLManager& _lvgl;


    // ========================================================
    // SCREENS
    // ========================================================

    ClockScreen _screens[
        SCREEN_COUNT
    ];


    // ========================================================
    // STATE
    // ========================================================

    bool _initialized;

    uint32_t _lastUpdate;


    // ========================================================
    // DATA UPDATE
    // ========================================================

    void updateClock();

    void updateSensors();

    void updateDate();
};
