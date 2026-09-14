#pragma once

#include <Arduino.h>

#include "core/ClockSystem.h"
#include "managers/SensorsManager.h"
#include "managers/LVGLManager.h"
#include "managers/ScreenManager.h"

class DisplaySystem
{
public:
    DisplaySystem(
        ClockSystem& clock,
        SensorManager& sensors
    );

    bool begin();
    void update();

    ScreenManager& screens();

private:
    ClockSystem& _clock;
    SensorManager& _sensors;

    LVGLManager _lvgl;
    ScreenManager _screens;

    bool _initialized;

    // Обновление содержимого экранов
    void updateScreens();

    // Обновление времени
    void updateTime();

    // Обновление показаний датчиков
    void updateSensors();

    // Обновление даты
    void updateDate();
};
