#pragma once

#include <Arduino.h>
#include <lvgl.h>

class ClockScreen
{
public:

    ClockScreen();

    void begin(
        lv_display_t* display
    );

    void setDigit(
        uint8_t digit
    );

    void setCenterText(
        const char* text
    );

    void setTopText(
        const char* text
    );

    void setBottomText(
        const char* text
    );

    void setVisible(
        bool visible
    );

    bool isVisible() const;

private:

    lv_display_t* _display;

    lv_obj_t* _root;

    lv_obj_t* _topBar;
    lv_obj_t* _topLabel;

    lv_obj_t* _digit;

    lv_obj_t* _bottomBar;
    lv_obj_t* _bottomLabel;

    lv_obj_t* _topLine;
    lv_obj_t* _bottomLine;

    char _centerText[16];
    char _topText[64];
    char _bottomText[64];

    bool _visible;

    void createUI();

    static void copyText(
        char* destination,
        size_t destinationSize,
        const char* source
    );
};