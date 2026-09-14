#include "ScreenManager.h"

#include <cstring>


// ============================================================
// CONSTRUCTOR
// ============================================================

ScreenManager::ScreenManager(
    ClockSystem& clock,
    SensorManager& sensors,
    LVGLManager& lvgl
)
    : _clock(clock),
      _sensors(sensors),
      _lvgl(lvgl),
      _initialized(false)
{
    for (uint8_t i = 0; i < SCREEN_COUNT; ++i)
    {
        _centerModes[i] =
            CenterMode::ONE_DIGIT;
    }

    memset(
        &_notification,
        0,
        sizeof(_notification)
    );

    _notification.priority =
        Constants::NotificationPriority::NORMAL_PRIORITY;
}


// ============================================================
// BEGIN
// ============================================================

bool ScreenManager::begin()
{
    for (uint8_t i = 0; i < SCREEN_COUNT; ++i)
    {
        lv_display_t* display =
            _lvgl.display(i);

        if (display == nullptr)
            return false;

        if (!_clockScreens[i].begin(display))
            return false;

        _clockScreens[i].setCenterMode(
            _centerModes[i]
        );

        _clockScreens[i].setTopText("");
        _clockScreens[i].setBottomText("");
    }

    _initialized = true;

    return true;
}


// ============================================================
// UPDATE
// ============================================================

void ScreenManager::update()
{
    if (!_initialized)
        return;

    updateNotification();
}


// ============================================================
// SET CENTER MODE
// ============================================================

void ScreenManager::setCenterMode(
    uint8_t screen,
    CenterMode mode
)
{
    if (screen >= SCREEN_COUNT)
        return;

    _centerModes[screen] = mode;

    _clockScreens[screen].setCenterMode(mode);
}


// ============================================================
// SET CENTER TEXT — STRING
// ============================================================

void ScreenManager::setCenterText(
    uint8_t screen,
    const char* text
)
{
    if (screen >= SCREEN_COUNT)
        return;

    _clockScreens[screen].setCenterText(text);
}


// ============================================================
// SET CENTER TEXT — ONE DIGIT
// ============================================================

void ScreenManager::setCenterText(
    uint8_t screen,
    uint8_t digit
)
{
    if (screen >= SCREEN_COUNT)
        return;

    _clockScreens[screen].setCenterText(digit);
}


// ============================================================
// SET CENTER TEXT — TWO DIGITS
// ============================================================

void ScreenManager::setCenterText(
    uint8_t screen,
    uint8_t topDigit,
    uint8_t bottomDigit
)
{
    if (screen >= SCREEN_COUNT)
        return;

    _clockScreens[screen].setCenterText(
        topDigit,
        bottomDigit
    );
}


// ============================================================
// SET TOP TEXT
// ============================================================

void ScreenManager::setTopText(
    uint8_t screen,
    const char* text
)
{
    if (screen >= SCREEN_COUNT)
        return;

    _clockScreens[screen].setTopText(text);
}


// ============================================================
// SET BOTTOM TEXT
// ============================================================

void ScreenManager::setBottomText(
    uint8_t screen,
    const char* text
)
{
    if (screen >= SCREEN_COUNT)
        return;

    _clockScreens[screen].setBottomText(text);
}


// ============================================================
// SET VISIBLE
// ============================================================

void ScreenManager::setVisible(
    uint8_t screen,
    bool visible
)
{
    if (screen >= SCREEN_COUNT)
        return;

    _clockScreens[screen].setVisible(visible);
}


// ============================================================
// NOTIFICATION
// ============================================================

bool ScreenManager::notify(
    const char* text,
    uint32_t durationMs,
    Constants::NotificationPriority priority
)
{
    if (text == nullptr)
        return false;

    strncpy(
        _notification.text,
        text,
        sizeof(_notification.text) - 1
    );

    _notification.text[
        sizeof(_notification.text) - 1
    ] = '\0';

    _notification.startTime = millis();
    _notification.duration = durationMs;
    _notification.priority = priority;
    _notification.active = true;

    for (uint8_t i = 0; i < SCREEN_COUNT; ++i)
    {
        _clockScreens[i].setTopText(
            _notification.text
        );
    }

    return true;
}


// ============================================================
// UPDATE NOTIFICATION
// ============================================================

void ScreenManager::updateNotification()
{
    if (!_notification.active)
        return;

    const uint32_t now = millis();

    if ((now - _notification.startTime) <
        _notification.duration)
    {
        return;
    }

    _notification.active = false;

    for (uint8_t i = 0; i < SCREEN_COUNT; ++i)
    {
        _clockScreens[i].setTopText("");
    }
}


// ============================================================
// GET SCREEN
// ============================================================

ClockScreen& ScreenManager::screen(
    uint8_t index
)
{
    return _clockScreens[index];
}
