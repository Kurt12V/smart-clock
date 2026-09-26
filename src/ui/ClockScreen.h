#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include "fonts/redring_clock_300.h"
#include "fonts/redring_clock_150.h"

enum class CenterMode : uint8_t
{
    ONE_DIGIT,
    TWO_DIGITS_VERTICAL,
    TEXT
};

class ClockScreen
{
public:

    ClockScreen();

    bool begin(lv_display_t* display);

    // ========================================================
    // CENTER
    // ========================================================

    void setCenterMode(CenterMode mode);

    void setCenterText(const char* text);

    void setCenterText(uint8_t digit);

    void setCenterText(
        uint8_t topDigit,
        uint8_t bottomDigit
    );

    // ========================================================
    // TOP / BOTTOM
    // ========================================================

    void setTopText(const char* text);

    void setBottomText(const char* text);

    // ========================================================
    // VISIBILITY
    // ========================================================

    void setVisible(bool visible);

    bool isVisible() const;

private:

    // ========================================================
    // UI
    // ========================================================

    void createUI();

    void updateCenter();

    void updateTop();

    void updateBottom();

    // ========================================================
    // STRING
    // ========================================================

    void copyText(
        char* destination,
        size_t destinationSize,
        const char* source
    );

private:

    // ========================================================
    // LVGL
    // ========================================================

    lv_display_t* _display;

    lv_obj_t* _screen;

    // ========================================================
    // TOP
    // ========================================================

    lv_obj_t* _topBar;
    lv_obj_t* _topLine;
    lv_obj_t* _topLabel;

    // ========================================================
    // CENTER CONTAINER
    // ========================================================

    lv_obj_t* _centerArea;

    // ========================================================
    // CENTER LABELS
    // ========================================================

    lv_obj_t* _centerLabel;

    lv_obj_t* _centerTopLabel;

    lv_obj_t* _centerBottomLabel;

    // ========================================================
    // BOTTOM
    // ========================================================

    lv_obj_t* _bottomBar;
    lv_obj_t* _bottomLine;
    lv_obj_t* _bottomLabel;

    // ========================================================
    // STATE
    // ========================================================

    CenterMode _centerMode;

    char _topText[64];

    char _bottomText[64];

    char _centerText[32];

    char _centerTopText[8];

    char _centerBottomText[8];

    bool _visible;

    bool _initialized;
};
