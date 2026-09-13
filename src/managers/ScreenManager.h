#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include "./core/ClockSystem.h"
#include "./managers/SensorsManager.h"

class ScreenManager
{
public:
    static constexpr uint8_t SCREEN_COUNT = 4;

public:
    ScreenManager(
        ClockSystem& clock,
        SensorManager& sensors
    );

    bool begin();

    void update();

private:
    void createScreen(
        uint8_t index,
        lv_display_t* display
    );

    void updateScreen(
        uint8_t index
    );

private:
    ClockSystem& _clock;
    SensorManager& _sensors;

    lv_obj_t* _screens[SCREEN_COUNT];

    lv_obj_t* _topBars[SCREEN_COUNT];
    lv_obj_t* _topLabels[SCREEN_COUNT];

    lv_obj_t* _digits[SCREEN_COUNT];

    lv_obj_t* _bottomBars[SCREEN_COUNT];
    lv_obj_t* _bottomLabels[SCREEN_COUNT];

    uint8_t _lastDigits[SCREEN_COUNT];

    bool _initialized;
};