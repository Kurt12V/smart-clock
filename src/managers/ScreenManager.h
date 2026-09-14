#pragma once

#include <Arduino.h>

#include "./ui/ClockScreen.h"

#include "core/ClockSystem.h"
#include "managers/SensorsManager.h"
#include "managers/LVGLManager.h"
#include "Constants.h"

class ScreenManager
{
public:
    static constexpr uint8_t SCREEN_COUNT = 4;

    using CenterMode = ClockScreen::CenterMode;

    ScreenManager(
        ClockSystem& clock,
        SensorManager& sensors,
        LVGLManager& lvgl
    );

    bool begin();
    void update();

    // --------------------------------------------------------
    // CENTER
    // --------------------------------------------------------

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

    // --------------------------------------------------------
    // TOP / BOTTOM
    // --------------------------------------------------------

    void setTopText(
        uint8_t screen,
        const char* text
    );

    void setBottomText(
        uint8_t screen,
        const char* text
    );

    // --------------------------------------------------------
    // VISIBILITY
    // --------------------------------------------------------

    void setVisible(
        uint8_t screen,
        bool visible
    );

    // --------------------------------------------------------
    // NOTIFICATION
    // --------------------------------------------------------

    bool notify(
        const char* text,
        uint32_t durationMs,
        Constants::NotificationPriority priority =
            Constants::NotificationPriority::NORMAL_PRIORITY
    );

    ClockScreen& screen(uint8_t index);

private:
    ClockSystem& _clock;
    SensorManager& _sensors;
    LVGLManager& _lvgl;

    ClockScreen _clockScreens[SCREEN_COUNT];

    CenterMode _centerModes[SCREEN_COUNT];

    bool _initialized;

    struct Notification
    {
        char text[64];

        uint32_t startTime;
        uint32_t duration;

        Constants::NotificationPriority priority;

        bool active;
    };

    Notification _notification;

    void updateNotification();
};
