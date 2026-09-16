#pragma once

#include <Arduino.h>
#include <lvgl.h>

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

    bool begin(
        lv_display_t* display
    );

    // --------------------------------------------------------
    // CENTER
    // --------------------------------------------------------

    void setCenterMode(
        CenterMode mode
    );

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

    // --------------------------------------------------------
    // TOP / BOTTOM
    // --------------------------------------------------------

    void setTopText(
        const char* text
    );

    void setBottomText(
        const char* text
    );

    // --------------------------------------------------------
    // VISIBILITY
    // --------------------------------------------------------

    void setVisible(
        bool visible
    );

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
    // STRING
    // --------------------------------------------------------

    void copyText(
        char* destination,
        size_t destinationSize,
        const char* source
    );

private:

    // --------------------------------------------------------
    // LVGL
    // --------------------------------------------------------

    lv_display_t* _display;

    lv_obj_t* _screen;

    // --------------------------------------------------------
    // TOP BAR
    // --------------------------------------------------------

    lv_obj_t* _topBar;

    lv_obj_t* _topLine;

    lv_obj_t* _topLabel;

    // --------------------------------------------------------
    // BOTTOM BAR
    // --------------------------------------------------------

    lv_obj_t* _bottomBar;

    lv_obj_t* _bottomLine;

    lv_obj_t* _bottomLabel;

    // --------------------------------------------------------
    // CENTER
    // --------------------------------------------------------

    lv_obj_t* _centerLabel;

    lv_obj_t* _centerTopLabel;

    lv_obj_t* _centerBottomLabel;

    // --------------------------------------------------------
    // CENTER MODE
    // --------------------------------------------------------

    CenterMode _centerMode;

    // --------------------------------------------------------
    // TEXT
    // --------------------------------------------------------

    char _topText[64];

    char _bottomText[64];

    char _centerText[32];

    char _centerTopText[8];

    char _centerBottomText[8];

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    bool _visible;

    bool _initialized;
};
