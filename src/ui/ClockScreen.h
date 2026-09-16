#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include "fonts/redring_clock_200.h"

// ============================================================
// CENTER MODE
// ============================================================

enum class CenterMode : uint8_t
{
    ONE_DIGIT,
    TWO_DIGITS_VERTICAL,
    TEXT
};


// ============================================================
// CLOCK SCREEN
// ============================================================

class ClockScreen
{
public:

    ClockScreen();

    bool begin(lv_display_t* display);

    // --------------------------------------------------------
    // Center
    // --------------------------------------------------------

    void setCenterMode(CenterMode mode);

    void setCenterText(const char* text);

    void setCenterText(uint8_t digit);

    void setCenterText(
        uint8_t topDigit,
        uint8_t bottomDigit
    );

    // --------------------------------------------------------
    // Top / Bottom
    // --------------------------------------------------------

    void setTopText(const char* text);

    void setBottomText(const char* text);

    // --------------------------------------------------------
    // Visibility
    // --------------------------------------------------------

    void setVisible(bool visible);

    bool isVisible() const;

private:

    // --------------------------------------------------------
    // UI
    // --------------------------------------------------------

    void createUI();

    void updateCenter();

    void updateTop();

    void updateBottom();

    // --------------------------------------------------------
    // Helpers
    // --------------------------------------------------------

    void copyText(
        char* destination,
        size_t destinationSize,
        const char* source
    );

    // --------------------------------------------------------
    // LVGL
    // --------------------------------------------------------

    lv_display_t* _display;

    lv_obj_t* _screen;

    // --------------------------------------------------------
    // Bars
    // --------------------------------------------------------

    lv_obj_t* _topBar;

    lv_obj_t* _topLine;

    lv_obj_t* _bottomBar;

    lv_obj_t* _bottomLine;

    // --------------------------------------------------------
    // Labels
    // --------------------------------------------------------

    lv_obj_t* _topLabel;

    lv_obj_t* _bottomLabel;

    lv_obj_t* _centerLabel;

    lv_obj_t* _centerTopLabel;

    lv_obj_t* _centerBottomLabel;

    // --------------------------------------------------------
    // State
    // --------------------------------------------------------

    CenterMode _centerMode;

    char _topText[64];

    char _bottomText[64];

    char _centerText[32];

    char _centerTopText[8];

    char _centerBottomText[8];

    bool _visible;

    bool _initialized;
};
