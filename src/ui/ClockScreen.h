#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include "fonts/redring_clock_200.h"


class ClockScreen
{
public:

    // ========================================================
    // CENTER MODES
    // ========================================================

    enum class CenterMode : uint8_t
    {
        ONE_DIGIT,

        TWO_DIGITS_VERTICAL,

        TEXT
    };


    ClockScreen();


    // ========================================================
    // INIT
    // ========================================================

    void begin(
        lv_display_t* display
    );


    // ========================================================
    // CENTER MODE
    // ========================================================

    void setCenterMode(
        CenterMode mode
    );


    // ========================================================
    // CENTER TEXT
    // ========================================================

    void setCenterText(
        const char* text
    );


    void setCenterText(
        uint8_t digit
    );


    void setCenterText(
        uint8_t topDigit,
        uint8_t bottomDigit
    );


    // ========================================================
    // TOP
    // ========================================================

    void setTopText(
        const char* text
    );


    // ========================================================
    // BOTTOM
    // ========================================================

    void setBottomText(
        const char* text
    );


    // ========================================================
    // VISIBILITY
    // ========================================================

    void setVisible(
        bool visible
    );


    bool isVisible() const;


private:

    // ========================================================
    // LVGL
    // ========================================================

    lv_display_t* _display;

    lv_obj_t* _root;


    // ========================================================
    // TOP
    // ========================================================

    lv_obj_t* _topBar;

    lv_obj_t* _topLabel;


    // ========================================================
    // CENTER
    // ========================================================

    lv_obj_t* _centerArea;

    lv_obj_t* _centerLabel;

    lv_obj_t* _centerTopLabel;

    lv_obj_t* _centerBottomLabel;


    // ========================================================
    // BOTTOM
    // ========================================================

    lv_obj_t* _bottomBar;

    lv_obj_t* _bottomLabel;


    // ========================================================
    // LINES
    // ========================================================

    lv_obj_t* _topLine;

    lv_obj_t* _bottomLine;


    // ========================================================
    // STATE
    // ========================================================

    CenterMode _centerMode;

    bool _visible;


    // ========================================================
    // TEXT
    // ========================================================

    char _centerText[16];

    char _centerTopText[8];

    char _centerBottomText[8];

    char _topText[64];

    char _bottomText[64];


    // ========================================================
    // UI
    // ========================================================

    void createUI();

    void updateCenter();


    // ========================================================
    // TEXT COPY
    // ========================================================

    static void copyText(
        char* destination,
        size_t destinationSize,
        const char* source
    );
};
