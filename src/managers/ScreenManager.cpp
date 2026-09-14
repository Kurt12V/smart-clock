#include "ScreenManager.h"

#include <cstdio>
#include <cstring>
#include "./ui/fonts/redring_clock_200.h"
#include "Constants.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

ScreenManager::ScreenManager(
    ClockSystem& clock,
    SensorManager& sensors
)
    : _clock(clock),
      _sensors(sensors),
      _initialized(false)
{
    for (uint8_t i = 0; i < SCREEN_COUNT; ++i)
    {
        _screens[i] = nullptr;

        _topBars[i] = nullptr;
        _topLabels[i] = nullptr;

        _digits[i] = nullptr;

        _bottomBars[i] = nullptr;
        _bottomLabels[i] = nullptr;

        _topLines[i] = nullptr;
        _bottomLines[i] = nullptr;

        _lastDigits[i] = 255;

        _lastTopText[i][0] = '\0';
        _lastBottomText[i][0] = '\0';
    }

    _notification.active = false;
    _notification.text[0] = '\0';

    _notification.priority =
        Constants::NotificationPriority::NORMAL_PRIORITY;

    _notification.startedAt = 0;
    _notification.duration = 0;
    _notification.screenMask = 0;
}

// ============================================================
// BEGIN
// ============================================================

bool ScreenManager::begin()
{
    if (_initialized)
        return true;

    lv_display_t* display =
        lv_display_get_next(nullptr);

    uint8_t index = 0;

    while (
        display != nullptr &&
        index < SCREEN_COUNT
    )
    {
        createScreen(index, display);

        ++index;

        display =
            lv_display_get_next(display);
    }

    if (index != SCREEN_COUNT)
        return false;

    _initialized = true;

    return true;
}

// ============================================================
// CREATE SCREEN
// ============================================================

void ScreenManager::createScreen(
    uint8_t index,
    lv_display_t* display
)
{
    if (
        index >= SCREEN_COUNT ||
        display == nullptr
    )
    {
        return;
    }

    lv_obj_t* root =
        lv_display_get_screen_active(display);

    if (root == nullptr)
        return;

    _screens[index] = root;

    // ========================================================
    // ROOT
    // ========================================================

    lv_obj_set_style_bg_color(
        root,
        lv_color_hex(0x050608),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        root,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    lv_obj_set_style_border_width(
        root,
        0,
        LV_PART_MAIN
    );

    lv_obj_set_style_pad_all(
        root,
        0,
        LV_PART_MAIN
    );

    // ========================================================
    // TOP BAR
    // ========================================================

    createTopBar(
        index,
        root
    );

    // ========================================================
    // MAIN DIGIT
    // ========================================================

    createMainDigit(
        index,
        root
    );

    // ========================================================
    // BOTTOM BAR
    // ========================================================

    createBottomBar(
        index,
        root
    );
}

// ============================================================
// TOP BAR
// ============================================================

void ScreenManager::createTopBar(
    uint8_t index,
    lv_obj_t* root
)
{
    if (index >= SCREEN_COUNT)
        return;

    // ========================================================
    // BAR
    // ========================================================

    lv_obj_t* topBar =
        lv_obj_create(root);

    _topBars[index] = topBar;

    lv_obj_remove_style_all(
        topBar
    );

    lv_obj_set_size(
        topBar,
        172,
        38
    );

    lv_obj_align(
        topBar,
        LV_ALIGN_TOP_MID,
        0,
        0
    );

    lv_obj_set_style_bg_color(
        topBar,
        lv_color_hex(0x0C0E12),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        topBar,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    // ========================================================
    // LABEL
    // ========================================================

    lv_obj_t* label =
        lv_label_create(topBar);

    _topLabels[index] = label;

    lv_label_set_text(
        label,
        ""
    );

    // Allow notification to use 1-2 lines.
    lv_label_set_long_mode(
        label,
        LV_LABEL_LONG_WRAP
    );

    lv_obj_set_width(
        label,
        152
    );

    lv_obj_set_height(
        label,
        34
    );

    lv_obj_set_style_text_font(
        label,
        &lv_font_montserrat_20,
        LV_PART_MAIN
    );

    lv_obj_set_style_text_color(
        label,
        lv_color_hex(0xA5AAB5),
        LV_PART_MAIN
    );

    lv_obj_set_style_text_align(
        label,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN
    );

    lv_obj_align(
        label,
        LV_ALIGN_CENTER,
        0,
        0
    );

    // ========================================================
    // SEPARATOR
    // ========================================================

    lv_obj_t* line =
        lv_obj_create(root);

    _topLines[index] = line;

    lv_obj_remove_style_all(
        line
    );

    lv_obj_set_size(
        line,
        172,
        1
    );

    lv_obj_align(
        line,
        LV_ALIGN_TOP_MID,
        0,
        38
    );

    lv_obj_set_style_bg_color(
        line,
        lv_color_hex(0x252830),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        line,
        LV_OPA_COVER,
        LV_PART_MAIN
    );
}

// ============================================================
// MAIN DIGIT
// ============================================================

void ScreenManager::createMainDigit(
    uint8_t index,
    lv_obj_t* root
)
{
    if (index >= SCREEN_COUNT)
        return;

    lv_obj_t* digit =
        lv_label_create(root);

    _digits[index] = digit;

    lv_label_set_text(
        digit,
        "0"
    );

    lv_obj_set_size(
        digit,
        172,
        150
    );

    lv_obj_set_style_text_font(
        digit,
        &redring_clock_200,
        LV_PART_MAIN
    );

    lv_obj_set_style_text_color(
        digit,
        lv_color_hex(0xF4F5F7),
        LV_PART_MAIN
    );

    lv_obj_set_style_text_align(
        digit,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN
    );

    lv_obj_align(
        digit,
        LV_ALIGN_CENTER,
        0,
        -4
    );
}

// ============================================================
// BOTTOM BAR
// ============================================================

void ScreenManager::createBottomBar(
    uint8_t index,
    lv_obj_t* root
)
{
    if (index >= SCREEN_COUNT)
        return;

    // ========================================================
    // BAR
    // ========================================================

    lv_obj_t* bottomBar =
        lv_obj_create(root);

    _bottomBars[index] = bottomBar;

    lv_obj_remove_style_all(
        bottomBar
    );

    lv_obj_set_size(
        bottomBar,
        172,
        52
    );

    lv_obj_align(
        bottomBar,
        LV_ALIGN_BOTTOM_MID,
        0,
        0
    );

    lv_obj_set_style_bg_color(
        bottomBar,
        lv_color_hex(0x0C0E12),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        bottomBar,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    // ========================================================
    // LABEL
    // ========================================================

    lv_obj_t* label =
        lv_label_create(bottomBar);

    _bottomLabels[index] = label;

    lv_label_set_text(
        label,
        "--"
    );

    lv_obj_set_width(
        label,
        152
    );

    lv_obj_set_style_text_font(
        label,
        &lv_font_montserrat_24,
        LV_PART_MAIN
    );

    lv_obj_set_style_text_color(
        label,
        lv_color_hex(0xD5D8DE),
        LV_PART_MAIN
    );

    lv_obj_set_style_text_align(
        label,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN
    );

    lv_obj_align(
        label,
        LV_ALIGN_CENTER,
        0,
        0
    );

    // ========================================================
    // SEPARATOR
    // ========================================================

    lv_obj_t* line =
        lv_obj_create(root);

    _bottomLines[index] = line;

    lv_obj_remove_style_all(
        line
    );

    lv_obj_set_size(
        line,
        172,
        1
    );

    lv_obj_align(
        line,
        LV_ALIGN_BOTTOM_MID,
        0,
        -52
    );

    lv_obj_set_style_bg_color(
        line,
        lv_color_hex(0x252830),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        line,
        LV_OPA_COVER,
        LV_PART_MAIN
    );
}

// ============================================================
// UPDATE
// ============================================================

void ScreenManager::update()
{
    if (!_initialized)
        return;

    // --------------------------------------------------------
    // Notifications
    // --------------------------------------------------------

    updateNotification();

    // --------------------------------------------------------
    // Screens
    // --------------------------------------------------------

    for (uint8_t i = 0; i < SCREEN_COUNT; ++i)
    {
        updateScreen(i);
    }
}

// ============================================================
// UPDATE SCREEN
// ============================================================

void ScreenManager::updateScreen(
    uint8_t index
)
{
    if (index >= SCREEN_COUNT)
        return;

    updateDigit(index);
    updateTopBar(index);
    updateBottomBar(index);
}

// ============================================================
// UPDATE DIGIT
// ============================================================

void ScreenManager::updateDigit(
    uint8_t index
)
{
    if (
        index >= SCREEN_COUNT ||
        _digits[index] == nullptr
    )
    {
        return;
    }

    uint8_t digit = 0;

    switch (index)
    {
        case 0:
            digit = _clock.getHourTens();
            break;

        case 1:
            digit = _clock.getHourOnes();
            break;

        case 2:
            digit = _clock.getMinuteTens();
            break;

        case 3:
            digit = _clock.getMinuteOnes();
            break;

        default:
            return;
    }

    // No redraw if unchanged.
    if (_lastDigits[index] == digit)
        return;

    _lastDigits[index] = digit;

    char text[2];

    snprintf(
        text,
        sizeof(text),
        "%u",
        digit
    );

    lv_label_set_text(
        _digits[index],
        text
    );
}

// ============================================================
// UPDATE TOP BAR
// ============================================================
//
// Верхний бар используется ТОЛЬКО для уведомлений.
//
// Он НЕ содержит:
// - температуру
// - влажность
// - освещённость
// - дату
// - время
//
// ============================================================

void ScreenManager::updateTopBar(
    uint8_t index
)
{
    if (
        index >= SCREEN_COUNT ||
        _topLabels[index] == nullptr
    )
    {
        return;
    }

    char newText[64];

    newText[0] = '\0';

    // --------------------------------------------------------
    // Notification
    // --------------------------------------------------------

    if (
        _notification.active &&
        isScreenSelected(
            index,
            _notification.screenMask
        )
    )
    {
        copyText(
            newText,
            sizeof(newText),
            _notification.text
        );
    }

    // --------------------------------------------------------
    // No change
    // --------------------------------------------------------

    if (
        strcmp(
            _lastTopText[index],
            newText
        ) == 0
    )
    {
        return;
    }

    copyText(
        _lastTopText[index],
        sizeof(_lastTopText[index]),
        newText
    );

    lv_label_set_text(
        _topLabels[index],
        newText
    );
}

// ============================================================
// UPDATE BOTTOM BAR
// ============================================================

void ScreenManager::updateBottomBar(
    uint8_t index
)
{
    if (
        index >= SCREEN_COUNT ||
        _bottomLabels[index] == nullptr
    )
    {
        return;
    }

    char bottomText[64];

    bottomText[0] = '\0';

    // ========================================================
    // SCREEN 1
    // TEMPERATURE
    // ========================================================

    if (index == 0)
    {
        String value =
            _sensors.getTemperatureC();

        if (value.length() == 0)
        {
            snprintf(
                bottomText,
                sizeof(bottomText),
                "--"
            );
        }
        else
        {
            snprintf(
                bottomText,
                sizeof(bottomText),
                "%s %s",
                value.c_str(),
                Constants::UNIT_TEMP_C
            );
        }
    }

    // ========================================================
    // SCREEN 2
    // HUMIDITY
    // ========================================================

    else if (index == 1)
    {
        String value =
            _sensors.getHumidity();

        if (value.length() == 0)
        {
            snprintf(
                bottomText,
                sizeof(bottomText),
                "--"
            );
        }
        else
        {
            snprintf(
                bottomText,
                sizeof(bottomText),
                "%s %s",
                value.c_str(),
                Constants::UNIT_HUMID
            );
        }
    }

    // ========================================================
    // SCREEN 3
    // LIGHT
    // ========================================================

    else if (index == 2)
    {
        String value =
            _sensors.getLight();

        if (value.length() == 0)
        {
            snprintf(
                bottomText,
                sizeof(bottomText),
                "--"
            );
        }
        else
        {
            snprintf(
                bottomText,
                sizeof(bottomText),
                "%s %s",
                value.c_str(),
                Constants::UNIT_LUX
            );
        }
    }

    // ========================================================
    // SCREEN 4
    // DATE
    // ========================================================

    else if (index == 3)
    {
        const char* dayName =
            getDayOfWeekName(
                _clock.getDayOfWeek()
            );

        snprintf(
            bottomText,
            sizeof(bottomText),
            "%s %02u.%02u.%04u",
            dayName,
            _clock.day(),
            _clock.month(),
            _clock.year()
        );
    }

    // ========================================================
    // No change
    // ========================================================

    if (
        strcmp(
            _lastBottomText[index],
            bottomText
        ) == 0
    )
    {
        return;
    }

    copyText(
        _lastBottomText[index],
        sizeof(_lastBottomText[index]),
        bottomText
    );

    lv_label_set_text(
        _bottomLabels[index],
        bottomText
    );
}

// ============================================================
// NOTIFY
// ============================================================

bool ScreenManager::notify(
    const char* text,
    uint32_t durationMs,
    Constants::NotificationPriority priority
)
{
    return notifyMask(
        0x0F,
        text,
        durationMs,
        priority
    );
}

// ============================================================
// NOTIFY SCREEN
// ============================================================

bool ScreenManager::notifyScreen(
    uint8_t screen,
    const char* text,
    uint32_t durationMs,
    Constants::NotificationPriority priority
)
{
    if (screen >= SCREEN_COUNT)
        return false;

    const uint8_t mask =
        static_cast<uint8_t>(
            1U << screen
        );

    return notifyMask(
        mask,
        text,
        durationMs,
        priority
    );
}

// ============================================================
// NOTIFY MASK
// ============================================================

bool ScreenManager::notifyMask(
    uint8_t screenMask,
    const char* text,
    uint32_t durationMs,
    Constants::NotificationPriority priority
)
{
    return setNotification(
        screenMask,
        text,
        durationMs,
        priority
    );
}

// ============================================================
// SET NOTIFICATION
// ============================================================

bool ScreenManager::setNotification(
    uint8_t screenMask,
    const char* text,
    uint32_t durationMs,
    Constants::NotificationPriority priority
)
{
    if (text == nullptr)
        return false;

    if (text[0] == '\0')
        return false;

    screenMask &= 0x0F;

    if (screenMask == 0)
        return false;

    // --------------------------------------------------------
    // Priority protection
    // --------------------------------------------------------

    if (
        _notification.active &&
        !shouldReplaceNotification(priority)
    )
    {
        return false;
    }

    // --------------------------------------------------------
    // Save
    // --------------------------------------------------------

    _notification.active = true;

    copyText(
        _notification.text,
        sizeof(_notification.text),
        text
    );

    _notification.priority = priority;

    _notification.startedAt = millis();

    _notification.duration = durationMs;

    _notification.screenMask = screenMask;

    // --------------------------------------------------------
    // Apply immediately
    // --------------------------------------------------------

    for (uint8_t i = 0; i < SCREEN_COUNT; ++i)
    {
        applyNotificationToScreen(i);
    }

    return true;
}

// ============================================================
// SHOULD REPLACE
// ============================================================

bool ScreenManager::shouldReplaceNotification(
    Constants::NotificationPriority newPriority
) const
{
    if (!_notification.active)
        return true;

    const uint8_t current =
        static_cast<uint8_t>(
            _notification.priority
        );

    const uint8_t incoming =
        static_cast<uint8_t>(
            newPriority
        );

    return incoming >= current;
}

// ============================================================
// UPDATE NOTIFICATION
// ============================================================

void ScreenManager::updateNotification()
{
    if (!_notification.active)
        return;

    if (!isNotificationExpired())
        return;

    clearNotification();
}

// ============================================================
// IS EXPIRED
// ============================================================

bool ScreenManager::isNotificationExpired() const
{
    if (!_notification.active)
        return false;

    // 0 = бесконечное уведомление
    if (_notification.duration == 0)
        return false;

    const uint32_t elapsed =
        millis() - _notification.startedAt;

    return elapsed >= _notification.duration;
}

// ============================================================
// CLEAR NOTIFICATION
// ============================================================

void ScreenManager::clearNotification()
{
    _notification.active = false;

    _notification.text[0] = '\0';

    _notification.startedAt = 0;

    _notification.duration = 0;

    _notification.priority =
        Constants::NotificationPriority::NORMAL_PRIORITY;

    _notification.screenMask = 0;

    clearNotificationFromScreens();
}

// ============================================================
// CLEAR FROM SCREENS
// ============================================================

void ScreenManager::clearNotificationFromScreens()
{
    for (uint8_t i = 0; i < SCREEN_COUNT; ++i)
    {
        if (_topLabels[i] == nullptr)
            continue;

        if (_lastTopText[i][0] == '\0')
            continue;

        _lastTopText[i][0] = '\0';

        lv_label_set_text(
            _topLabels[i],
            ""
        );
    }
}

// ============================================================
// APPLY NOTIFICATION
// ============================================================

void ScreenManager::applyNotificationToScreen(
    uint8_t index
)
{
    if (index >= SCREEN_COUNT)
        return;

    if (_topLabels[index] == nullptr)
        return;

    char text[64];

    text[0] = '\0';

    if (
        _notification.active &&
        isScreenSelected(
            index,
            _notification.screenMask
        )
    )
    {
        copyText(
            text,
            sizeof(text),
            _notification.text
        );
    }

    // --------------------------------------------------------
    // No change
    // --------------------------------------------------------

    if (
        strcmp(
            _lastTopText[index],
            text
        ) == 0
    )
    {
        return;
    }

    copyText(
        _lastTopText[index],
        sizeof(_lastTopText[index]),
        text
    );

    lv_label_set_text(
        _topLabels[index],
        text
    );
}

// ============================================================
// IS SCREEN SELECTED
// ============================================================

bool ScreenManager::isScreenSelected(
    uint8_t screen,
    uint8_t screenMask
) const
{
    if (screen >= SCREEN_COUNT)
        return false;

    const uint8_t bit =
        static_cast<uint8_t>(
            1U << screen
        );

    return (screenMask & bit) != 0;
}

// ============================================================
// HAS NOTIFICATION
// ============================================================

bool ScreenManager::hasNotification() const
{
    return _notification.active;
}

// ============================================================
// GET NOTIFICATION
// ============================================================

const ScreenManager::Notification&
ScreenManager::getNotification() const
{
    return _notification;
}

// ============================================================
// GET REMAINING
// ============================================================

uint32_t ScreenManager::getNotificationRemaining() const
{
    if (!_notification.active)
        return 0;

    // Бессрочное уведомление
    if (_notification.duration == 0)
        return 0;

    const uint32_t elapsed =
        millis() - _notification.startedAt;

    if (elapsed >= _notification.duration)
        return 0;

    return _notification.duration - elapsed;
}

// ============================================================
// SCREEN ACCESS
// ============================================================

lv_obj_t* ScreenManager::screen(
    uint8_t index
) const
{
    if (index >= SCREEN_COUNT)
        return nullptr;

    return _screens[index];
}

// ============================================================
// DAY OF WEEK
// ============================================================

const char*
ScreenManager::getDayOfWeekName(
    Constants::DayOfWeek day
)
{
    switch (day)
    {
        case Constants::DayOfWeek::Sunday:
            return "ВС";

        case Constants::DayOfWeek::Monday:
            return "ПН";

        case Constants::DayOfWeek::Tuesday:
            return "ВТ";

        case Constants::DayOfWeek::Wednesday:
            return "СР";

        case Constants::DayOfWeek::Thursday:
            return "ЧТ";

        case Constants::DayOfWeek::Friday:
            return "ПТ";

        case Constants::DayOfWeek::Saturday:
            return "СБ";
    }

    return "--";
}

// ============================================================
// COPY TEXT
// ============================================================

void ScreenManager::copyText(
    char* destination,
    size_t destinationSize,
    const char* source
)
{
    if (
        destination == nullptr ||
        destinationSize == 0
    )
    {
        return;
    }

    if (source == nullptr)
    {
        destination[0] = '\0';
        return;
    }

    strncpy(
        destination,
        source,
        destinationSize - 1
    );

    destination[destinationSize - 1] = '\0';
}
