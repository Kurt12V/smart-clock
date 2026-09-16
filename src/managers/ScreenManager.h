#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include "./core/ClockSystem.h"
#include "Constants.h"
#include "./managers/SensorsManager.h"

class ScreenManager
{
public:

    static constexpr uint8_t SCREEN_COUNT = 4;

    // ============================================================
    // NOTIFICATION
    // ============================================================

    struct Notification
    {
        bool active = false;

        char text[64] = "";

        Constants::NotificationPriority priority =
            Constants::NotificationPriority::NORMAL_PRIORITY;

        uint32_t startedAt = 0;
        uint32_t duration = 0;

        // Bit mask:
        // bit 0 -> screen 1
        // bit 1 -> screen 2
        // bit 2 -> screen 3
        // bit 3 -> screen 4
        //
        // 0x01 = screen 1
        // 0x02 = screen 2
        // 0x04 = screen 3
        // 0x08 = screen 4
        // 0x0F = all screens
        uint8_t screenMask = 0x0F;
    };

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

    // ============================================================
    // NOTIFICATION API
    // ============================================================

    /**
     * Показать уведомление на всех экранах.
     *
     * durationMs:
     *  0 = бессрочно
     *  >0 = время отображения в миллисекундах
     */
    bool notify(
        const char* text,
        uint32_t durationMs = 5000,
        Constants::NotificationPriority priority =
            Constants::NotificationPriority::NORMAL_PRIORITY
    );

    /**
     * Показать уведомление только на одном экране.
     *
     * screen:
     *  0 = экран 1
     *  1 = экран 2
     *  2 = экран 3
     *  3 = экран 4
     */
    bool notifyScreen(
        uint8_t screen,
        const char* text,
        uint32_t durationMs = 5000,
        Constants::NotificationPriority priority =
            Constants::NotificationPriority::NORMAL_PRIORITY
    );

    /**
     * Показать уведомление по маске экранов.
     *
     * Например:
     * 0x01 -> экран 1
     * 0x05 -> экраны 1 и 3
     * 0x0F -> все 4 экрана
     */
    bool notifyMask(
        uint8_t screenMask,
        const char* text,
        uint32_t durationMs = 5000,
        Constants::NotificationPriority priority =
            Constants::NotificationPriority::NORMAL_PRIORITY
    );

    /**
     * Удалить текущее уведомление.
     */
    void clearNotification();

    /**
     * Проверить наличие активного уведомления.
     */
    bool hasNotification() const;

    /**
     * Получить текущее уведомление.
     */
    const Notification& getNotification() const;

    /**
     * Получить оставшееся время уведомления.
     *
     * 0 = бессрочное или уже закончилось.
     */
    uint32_t getNotificationRemaining() const;

    // ============================================================
    // SCREEN ACCESS
    // ============================================================

    lv_obj_t* screen(uint8_t index) const;

private:

    // ============================================================
    // INTERNAL
    // ============================================================

    ClockSystem& _clock;

    SensorManager& _sensors;

    bool _initialized;

    // LVGL root screens
    lv_obj_t* _screens[SCREEN_COUNT];

    // Top bar
    lv_obj_t* _topBars[SCREEN_COUNT];
    lv_obj_t* _topLabels[SCREEN_COUNT];

    // Main digit
    lv_obj_t* _digits[SCREEN_COUNT];

    // Bottom bar
    lv_obj_t* _bottomBars[SCREEN_COUNT];
    lv_obj_t* _bottomLabels[SCREEN_COUNT];

    // Separators
    lv_obj_t* _topLines[SCREEN_COUNT];
    lv_obj_t* _bottomLines[SCREEN_COUNT];

    // ============================================================
    // CACHED VALUES
    // ============================================================

    uint8_t _lastDigits[SCREEN_COUNT];

    char _lastTopText[SCREEN_COUNT][64];
    char _lastBottomText[SCREEN_COUNT][64];

    // ============================================================
    // NOTIFICATION
    // ============================================================

    Notification _notification;

    // ============================================================
    // SCREEN CREATION
    // ============================================================

    void createScreen(
        uint8_t index,
        lv_display_t* display
    );

    void createTopBar(
        uint8_t index,
        lv_obj_t* root
    );

    void createMainDigit(
        uint8_t index,
        lv_obj_t* root
    );

    void createBottomBar(
        uint8_t index,
        lv_obj_t* root
    );

    // ============================================================
    // UPDATE
    // ============================================================

    void updateScreen(uint8_t index);

    void updateDigit(uint8_t index);

    void updateTopBar(uint8_t index);

    void updateBottomBar(uint8_t index);

    void updateNotification();

    void clearNotificationFromScreens();

    void applyNotificationToScreen(
        uint8_t index
    );

    // ============================================================
    // NOTIFICATION HELPERS
    // ============================================================

    bool setNotification(
        uint8_t screenMask,
        const char* text,
        uint32_t durationMs,
        Constants::NotificationPriority priority
    );

    bool isNotificationExpired() const;

    bool shouldReplaceNotification(
        Constants::NotificationPriority newPriority
    ) const;

    bool isScreenSelected(
        uint8_t screen,
        uint8_t screenMask
    ) const;

    // ============================================================
    // TEXT HELPERS
    // ============================================================

    static void copyText(
        char* destination,
        size_t destinationSize,
        const char* source
    );

    static const char* getDayOfWeekName(
        Constants::DayOfWeek day
    );
};