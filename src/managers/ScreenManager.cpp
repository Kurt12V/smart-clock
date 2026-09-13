#include "ScreenManager.h"

#include <cstdio>
#include <cstring>


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

        _lastDigits[i] = 255;
    }
}


// ============================================================
// BEGIN
// ============================================================

bool ScreenManager::begin()
{
    if (_initialized)
        return true;

    // ScreenManager получает LVGL displays
    // через lv_display_get_next().
    lv_display_t* display =
        lv_display_get_next(nullptr);

    uint8_t index = 0;

    while (
        display != nullptr &&
        index < SCREEN_COUNT
    )
    {
        createScreen(
            index,
            display
        );

        ++index;

        display =
            lv_display_get_next(display);
    }

    if (index != SCREEN_COUNT)
    {
        return false;
    }

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
    if (index >= SCREEN_COUNT)
        return;

    if (display == nullptr)
        return;

    // --------------------------------------------------------
    // ROOT
    // --------------------------------------------------------

    lv_obj_t* root =
        lv_display_get_screen_active(
            display
        );

    _screens[index] = root;

    lv_obj_set_style_bg_color(
        root,
        lv_color_black(),
        0
    );

    lv_obj_set_style_bg_opa(
        root,
        LV_OPA_COVER,
        0
    );

    lv_obj_set_style_border_width(
        root,
        0,
        0
    );

    lv_obj_set_style_pad_all(
        root,
        0,
        0
    );


    // ========================================================
    // TOP BAR
    // ========================================================

    lv_obj_t* topBar =
        lv_obj_create(root);

    _topBars[index] =
        topBar;

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
        lv_color_make(18, 18, 22),
        0
    );

    lv_obj_set_style_bg_opa(
        topBar,
        LV_OPA_COVER,
        0
    );


    // --------------------------------------------------------
    // TOP TEXT
    // --------------------------------------------------------

    lv_obj_t* topLabel =
        lv_label_create(topBar);

    _topLabels[index] =
        topLabel;

    lv_label_set_text(
        topLabel,
        "SMART CLOCK"
    );

    lv_obj_set_style_text_color(
        topLabel,
        lv_color_white(),
        0
    );

    lv_obj_set_style_text_font(
        topLabel,
        &lv_font_montserrat_14,
        0
    );

    lv_obj_center(
        topLabel
    );


    // ========================================================
    // DIGIT
    // ========================================================

    lv_obj_t* digit =
        lv_label_create(root);

    _digits[index] =
        digit;

    lv_label_set_text(
        digit,
        "0"
    );

    lv_obj_set_style_text_color(
        digit,
        lv_color_white(),
        0
    );

    lv_obj_set_style_text_font(
        digit,
        &lv_font_montserrat_20,
        0
    );

    lv_obj_set_style_text_align(
        digit,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_obj_align(
        digit,
        LV_ALIGN_CENTER,
        0,
        5
    );


    // ========================================================
    // BOTTOM BAR
    // ========================================================

    lv_obj_t* bottomBar =
        lv_obj_create(root);

    _bottomBars[index] =
        bottomBar;

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
        lv_color_make(18, 18, 22),
        0
    );

    lv_obj_set_style_bg_opa(
        bottomBar,
        LV_OPA_COVER,
        0
    );


    // --------------------------------------------------------
    // BOTTOM TEXT
    // --------------------------------------------------------

    lv_obj_t* bottomLabel =
        lv_label_create(bottomBar);

    _bottomLabels[index] =
        bottomLabel;

    lv_label_set_text(
        bottomLabel,
        "--"
    );

    lv_obj_set_style_text_color(
        bottomLabel,
        lv_color_make(
            180,
            180,
            185
        ),
        0
    );

    lv_obj_set_style_text_font(
        bottomLabel,
        &lv_font_montserrat_14,
        0
    );

    lv_obj_center(
        bottomLabel
    );
}


// ============================================================
// UPDATE
// ============================================================

void ScreenManager::update()
{
    if (!_initialized)
        return;

    for (uint8_t i = 0; i < SCREEN_COUNT; ++i)
    {
        updateScreen(i);
    }
}


// ============================================================
// UPDATE SCREEN
// ============================================================

void ScreenManager::updateScreen(uint8_t index)
{
    if (index >= SCREEN_COUNT)
        return;

    if (_digits[index] == nullptr)
        return;

    // ========================================================
    // CLOCK DIGIT
    // ========================================================

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
    }

    if (_lastDigits[index] != digit)
    {
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


    // ========================================================
    // BOTTOM DATA
    // ========================================================

    if (_bottomLabels[index] == nullptr)
        return;

    String bottom;


    // --------------------------------------------------------
    // SCREEN 1 — TEMPERATURE
    // --------------------------------------------------------

    if (index == 0)
    {
        bottom = _sensors.getTemperatureC();

        if (bottom.length() == 0)
        {
            bottom = "--";
        }
        else
        {
            bottom += " °C";
        }
    }


    // --------------------------------------------------------
    // SCREEN 2 — HUMIDITY
    // --------------------------------------------------------

    else if (index == 1)
    {
        bottom = _sensors.getHumidity();

        if (bottom.length() == 0)
        {
            bottom = "--";
        }
        else
        {
            bottom += " %";
        }
    }


    // --------------------------------------------------------
    // SCREEN 3 — LIGHT
    // --------------------------------------------------------

    else if (index == 2)
    {
        bottom = _sensors.getLight();

        if (bottom.length() == 0)
        {
            bottom = "--";
        }
        else
        {
            bottom += " lx";
        }
    }


    // --------------------------------------------------------
    // SCREEN 4 — DATE
    // --------------------------------------------------------

    else
    {
        char dateText[32];

        snprintf(
            dateText,
            sizeof(dateText),
            "%02u.%02u.%04u",
            _clock.day(),
            _clock.month(),
            _clock.year()
        );

        bottom = dateText;
    }


    lv_label_set_text(
        _bottomLabels[index],
        bottom.c_str()
    );


    // ========================================================
    // TOP BAR
    // ========================================================

    if (_topLabels[index] == nullptr)
        return;

    switch (index)
    {
        case 0:
            lv_label_set_text(
                _topLabels[index],
                "CLOCK"
            );
            break;

        case 1:
            lv_label_set_text(
                _topLabels[index],
                "TIME"
            );
            break;

        case 2:
            lv_label_set_text(
                _topLabels[index],
                "LIGHT"
            );
            break;

        case 3:
            lv_label_set_text(
                _topLabels[index],
                "DATE"
            );
            break;
    }
}