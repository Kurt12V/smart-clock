#pragma once

#include <Arduino.h>
#include <lvgl.h>

class ClockScreen
{
public:
    enum class CenterMode : uint8_t
    {
        ONE_DIGIT = 0,
        TWO_DIGITS_VERTICAL = 1
    };

    ClockScreen();

    bool begin(lv_display_t* display);

    // --------------------------------------------------------
    // CENTER
    // --------------------------------------------------------

    void setDigit(uint8_t digit);

    void setCenterText(const char* text);
    void setCenterText(uint8_t digit);
    void setCenterText(uint8_t topDigit, uint8_t bottomDigit);

    void setCenterMode(CenterMode mode);
    CenterMode getCenterMode() const;

    // --------------------------------------------------------
    // TOP / BOTTOM
    // --------------------------------------------------------

    void setTopText(const char* text);
    void setBottomText(const char* text);

    // --------------------------------------------------------
    // VISIBILITY
    // --------------------------------------------------------

    void setVisible(bool visible);
    bool isVisible() const;

    lv_obj_t* screen() const;

private:
    lv_display_t* _display;
    lv_obj_t* _root;

    // TOP
    lv_obj_t* _topBar;
    lv_obj_t* _topLabel;
    lv_obj_t* _topLine;

    // CENTER
    lv_obj_t* _digit;
    lv_obj_t* _digitTop;
    lv_obj_t* _digitBottom;

    // BOTTOM
    lv_obj_t* _bottomBar;
    lv_obj_t* _bottomLabel;
    lv_obj_t* _bottomLine;

    CenterMode _centerMode;

    uint8_t _digitValue;

    char _centerText[16];
    char _lastCenterText[16];

    char _topText[64];
    char _lastTopText[64];

    char _bottomText[64];
    char _lastBottomText[64];

    bool _visible;

    void createUI();

    void updateCenter();
    void updateTop();
    void updateBottom();
};
