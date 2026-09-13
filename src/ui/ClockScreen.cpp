#include "ClockScreen.h"

#include <cstring>

ClockScreen::ClockScreen()
    : _display(nullptr),
      _root(nullptr),
      _topBar(nullptr),
      _topLabel(nullptr),
      _digit(nullptr),
      _bottomBar(nullptr),
      _bottomLabel(nullptr),
      _topLine(nullptr),
      _bottomLine(nullptr),
      _visible(true)
{
    _centerText[0] = '\0';
    _topText[0] = '\0';
    _bottomText[0] = '\0';
}

void ClockScreen::begin(
    lv_display_t* display
)
{
    _display = display;

    if (_display == nullptr)
    {
        return;
    }

    _root = lv_display_get_screen_active(
        _display
    );

    if (_root == nullptr)
    {
        return;
    }

    lv_obj_clean(_root);

    createUI();
}

void ClockScreen::createUI()
{
    // ========================================================
    // ROOT
    // ========================================================

    lv_obj_set_style_bg_color(
        _root,
        lv_color_hex(0x050608),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        _root,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    lv_obj_set_style_border_width(
        _root,
        0,
        LV_PART_MAIN
    );

    lv_obj_set_style_pad_all(
        _root,
        0,
        LV_PART_MAIN
    );

    // ========================================================
    // TOP BAR
    // ========================================================

    _topBar = lv_obj_create(_root);

    lv_obj_remove_style_all(
        _topBar
    );

    lv_obj_set_size(
        _topBar,
        172,
        38
    );

    lv_obj_align(
        _topBar,
        LV_ALIGN_TOP_MID,
        0,
        0
    );

    lv_obj_set_style_bg_color(
        _topBar,
        lv_color_hex(0x0C0E12),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        _topBar,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    // ========================================================
    // TOP TEXT
    // ========================================================

    _topLabel = lv_label_create(
        _topBar
    );

    lv_label_set_text(
        _topLabel,
        ""
    );

    lv_obj_set_width(
        _topLabel,
        152
    );

    lv_obj_set_style_text_font(
        _topLabel,
        &lv_font_montserrat_14,
        LV_PART_MAIN
    );

    lv_obj_set_style_text_color(
        _topLabel,
        lv_color_hex(0xA5AAB5),
        LV_PART_MAIN
    );

    lv_obj_set_style_text_align(
        _topLabel,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN
    );

    lv_obj_align(
        _topLabel,
        LV_ALIGN_CENTER,
        0,
        0
    );

    // ========================================================
    // TOP LINE
    // ========================================================

    _topLine = lv_obj_create(
        _root
    );

    lv_obj_remove_style_all(
        _topLine
    );

    lv_obj_set_size(
        _topLine,
        172,
        1
    );

    lv_obj_align(
        _topLine,
        LV_ALIGN_TOP_MID,
        0,
        38
    );

    lv_obj_set_style_bg_color(
        _topLine,
        lv_color_hex(0x252830),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        _topLine,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    // ========================================================
    // MAIN DIGIT
    // ========================================================

    _digit = lv_label_create(
        _root
    );

    lv_label_set_text(
        _digit,
        "0"
    );

    lv_obj_set_size(
        _digit,
        172,
        150
    );

    lv_obj_set_style_text_font(
        _digit,
        &lv_font_montserrat_20,
        LV_PART_MAIN
    );

    lv_obj_set_style_text_color(
        _digit,
        lv_color_hex(0xF4F5F7),
        LV_PART_MAIN
    );

    lv_obj_set_style_text_align(
        _digit,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN
    );

    lv_obj_align(
        _digit,
        LV_ALIGN_CENTER,
        0,
        -4
    );

    // ========================================================
    // BOTTOM BAR
    // ========================================================

    _bottomBar = lv_obj_create(
        _root
    );

    lv_obj_remove_style_all(
        _bottomBar
    );

    lv_obj_set_size(
        _bottomBar,
        172,
        52
    );

    lv_obj_align(
        _bottomBar,
        LV_ALIGN_BOTTOM_MID,
        0,
        0
    );

    lv_obj_set_style_bg_color(
        _bottomBar,
        lv_color_hex(0x0C0E12),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        _bottomBar,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    // ========================================================
    // BOTTOM TEXT
    // ========================================================

    _bottomLabel = lv_label_create(
        _bottomBar
    );

    lv_label_set_text(
        _bottomLabel,
        ""
    );

    lv_obj_set_width(
        _bottomLabel,
        152
    );

    lv_obj_set_style_text_font(
        _bottomLabel,
        &lv_font_montserrat_20,
        LV_PART_MAIN
    );

    lv_obj_set_style_text_color(
        _bottomLabel,
        lv_color_hex(0xD5D8DE),
        LV_PART_MAIN
    );

    lv_obj_set_style_text_align(
        _bottomLabel,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN
    );

    lv_obj_align(
        _bottomLabel,
        LV_ALIGN_CENTER,
        0,
        0
    );

    // ========================================================
    // BOTTOM LINE
    // ========================================================

    _bottomLine = lv_obj_create(
        _root
    );

    lv_obj_remove_style_all(
        _bottomLine
    );

    lv_obj_set_size(
        _bottomLine,
        172,
        1
    );

    lv_obj_align(
        _bottomLine,
        LV_ALIGN_BOTTOM_MID,
        0,
        -52
    );

    lv_obj_set_style_bg_color(
        _bottomLine,
        lv_color_hex(0x252830),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        _bottomLine,
        LV_OPA_COVER,
        LV_PART_MAIN
    );
}

void ClockScreen::setDigit(
    uint8_t digit
)
{
    if (digit > 9)
    {
        return;
    }

    char buffer[2];

    buffer[0] = '0' + digit;
    buffer[1] = '\0';

    setCenterText(buffer);
}

void ClockScreen::setCenterText(
    const char* text
)
{
    if (text == nullptr)
    {
        text = "";
    }

    if (strcmp(
        _centerText,
        text
    ) == 0)
    {
        return;
    }

    copyText(
        _centerText,
        sizeof(_centerText),
        text
    );

    if (_digit != nullptr)
    {
        lv_label_set_text(
            _digit,
            _centerText
        );
    }
}

void ClockScreen::setTopText(
    const char* text
)
{
    if (text == nullptr)
    {
        text = "";
    }

    if (strcmp(
        _topText,
        text
    ) == 0)
    {
        return;
    }

    copyText(
        _topText,
        sizeof(_topText),
        text
    );

    if (_topLabel != nullptr)
    {
        lv_label_set_text(
            _topLabel,
            _topText
        );
    }
}

void ClockScreen::setBottomText(
    const char* text
)
{
    if (text == nullptr)
    {
        text = "";
    }

    if (strcmp(
        _bottomText,
        text
    ) == 0)
    {
        return;
    }

    copyText(
        _bottomText,
        sizeof(_bottomText),
        text
    );

    if (_bottomLabel != nullptr)
    {
        lv_label_set_text(
            _bottomLabel,
            _bottomText
        );
    }
}

void ClockScreen::setVisible(
    bool visible
)
{
    if (_root == nullptr)
    {
        return;
    }

    _visible = visible;

    if (visible)
    {
        lv_obj_remove_flag(
            _root,
            LV_OBJ_FLAG_HIDDEN
        );
    }
    else
    {
        lv_obj_add_flag(
            _root,
            LV_OBJ_FLAG_HIDDEN
        );
    }
}

bool ClockScreen::isVisible() const
{
    return _visible;
}

void ClockScreen::copyText(
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

    destination[
        destinationSize - 1
    ] = '\0';
}