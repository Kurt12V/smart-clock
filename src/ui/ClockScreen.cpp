#include "ClockScreen.h"

#include <cstring>


// ============================================================
// CONSTRUCTOR
// ============================================================

ClockScreen::ClockScreen()
    : _display(nullptr),

      _root(nullptr),

      _topBar(nullptr),
      _topLabel(nullptr),

      _centerArea(nullptr),
      _centerLabel(nullptr),
      _centerTopLabel(nullptr),
      _centerBottomLabel(nullptr),

      _bottomBar(nullptr),
      _bottomLabel(nullptr),

      _topLine(nullptr),
      _bottomLine(nullptr),

      _centerMode(
          CenterMode::ONE_DIGIT
      ),

      _visible(true)
{
    _centerText[0] = '\0';

    _centerTopText[0] = '\0';

    _centerBottomText[0] = '\0';

    _topText[0] = '\0';

    _bottomText[0] = '\0';
}


// ============================================================
// BEGIN
// ============================================================

void ClockScreen::begin(
    lv_display_t* display
)
{
    _display =
        display;


    if (
        _display == nullptr
    )
    {
        return;
    }


    createUI();
}


// ============================================================
// CREATE UI
// ============================================================

void ClockScreen::createUI()
{
    // ========================================================
    // ROOT
    // ========================================================

    _root =
        lv_display_get_screen_active(
            _display
        );


    if (
        _root == nullptr
    )
    {
        return;
    }


    // ========================================================
    // ROOT STYLE
    // ========================================================

    lv_obj_set_style_bg_color(
        _root,
        lv_color_hex(0x000000),
        0
    );


    lv_obj_set_style_bg_opa(
        _root,
        LV_OPA_COVER,
        0
    );


    // ========================================================
    // TOP BAR
    // ========================================================

    _topBar =
        lv_obj_create(
            _root
        );


    lv_obj_set_size(
        _topBar,
        172,
        38
    );


    lv_obj_set_pos(
        _topBar,
        0,
        0
    );


    lv_obj_set_style_bg_opa(
        _topBar,
        LV_OPA_TRANSP,
        0
    );


    lv_obj_set_style_border_width(
        _topBar,
        0,
        0
    );


    lv_obj_set_style_pad_all(
        _topBar,
        0,
        0
    );


    // ========================================================
    // TOP LABEL
    // ========================================================

    _topLabel =
        lv_label_create(
            _topBar
        );


    lv_obj_set_width(
        _topLabel,
        172
    );


    lv_obj_set_height(
        _topLabel,
        38
    );


    lv_obj_align(
        _topLabel,
        LV_ALIGN_CENTER,
        0,
        0
    );


    lv_label_set_text(
        _topLabel,
        ""
    );


    lv_obj_set_style_text_color(
        _topLabel,
        lv_color_hex(0xFFFFFF),
        0
    );


    lv_obj_set_style_text_font(
        _topLabel,
        &lv_font_montserrat_20,
        0
    );


    lv_obj_set_style_text_align(
        _topLabel,
        LV_TEXT_ALIGN_CENTER,
        0
    );


    // ========================================================
    // TOP LINE
    // ========================================================

    _topLine =
        lv_obj_create(
            _root
        );


    lv_obj_set_size(
        _topLine,
        172,
        1
    );


    lv_obj_set_pos(
        _topLine,
        0,
        38
    );


    lv_obj_set_style_bg_color(
        _topLine,
        lv_color_hex(0x303030),
        0
    );


    lv_obj_set_style_bg_opa(
        _topLine,
        LV_OPA_COVER,
        0
    );


    lv_obj_set_style_border_width(
        _topLine,
        0,
        0
    );


    // ========================================================
    // CENTER AREA
    // ========================================================

    _centerArea =
        lv_obj_create(
            _root
        );


    lv_obj_set_size(
        _centerArea,
        172,
        228
    );


    lv_obj_set_pos(
        _centerArea,
        0,
        39
    );


    lv_obj_set_style_bg_opa(
        _centerArea,
        LV_OPA_TRANSP,
        0
    );


    lv_obj_set_style_border_width(
        _centerArea,
        0,
        0
    );


    lv_obj_set_style_pad_all(
        _centerArea,
        0,
        0
    );


    // ========================================================
    // CENTER SINGLE LABEL
    // ========================================================

    _centerLabel =
        lv_label_create(
            _centerArea
        );


    lv_obj_set_width(
        _centerLabel,
        172
    );


    lv_obj_set_height(
        _centerLabel,
        228
    );


    lv_obj_align(
        _centerLabel,
        LV_ALIGN_CENTER,
        0,
        0
    );


    lv_obj_set_style_text_font(
        _centerLabel,
        &redring_clock_200,
        0
    );


    lv_obj_set_style_text_color(
        _centerLabel,
        lv_color_hex(0xFFFFFF),
        0
    );


    lv_obj_set_style_text_align(
        _centerLabel,
        LV_TEXT_ALIGN_CENTER,
        0
    );


    lv_obj_set_style_pad_all(
        _centerLabel,
        0,
        0
    );


    lv_label_set_text(
        _centerLabel,
        ""
    );


    // ========================================================
    // VERTICAL TOP DIGIT
    // ========================================================

    _centerTopLabel =
        lv_label_create(
            _centerArea
        );


    lv_obj_set_width(
        _centerTopLabel,
        172
    );


    lv_obj_set_height(
        _centerTopLabel,
        114
    );


    lv_obj_align(
        _centerTopLabel,
        LV_ALIGN_TOP_MID,
        0,
        0
    );


    lv_obj_set_style_text_font(
        _centerTopLabel,
        &redring_clock_200,
        0
    );


    lv_obj_set_style_text_color(
        _centerTopLabel,
        lv_color_hex(0xFFFFFF),
        0
    );


    lv_obj_set_style_text_align(
        _centerTopLabel,
        LV_TEXT_ALIGN_CENTER,
        0
    );


    lv_obj_set_style_pad_all(
        _centerTopLabel,
        0,
        0
    );


    lv_label_set_text(
        _centerTopLabel,
        ""
    );


    // ========================================================
    // VERTICAL BOTTOM DIGIT
    // ========================================================

    _centerBottomLabel =
        lv_label_create(
            _centerArea
        );


    lv_obj_set_width(
        _centerBottomLabel,
        172
    );


    lv_obj_set_height(
        _centerBottomLabel,
        114
    );


    lv_obj_align(
        _centerBottomLabel,
        LV_ALIGN_BOTTOM_MID,
        0,
        0
    );


    lv_obj_set_style_text_font(
        _centerBottomLabel,
        &redring_clock_200,
        0
    );


    lv_obj_set_style_text_color(
        _centerBottomLabel,
        lv_color_hex(0xFFFFFF),
        0
    );


    lv_obj_set_style_text_align(
        _centerBottomLabel,
        LV_TEXT_ALIGN_CENTER,
        0
    );


    lv_obj_set_style_pad_all(
        _centerBottomLabel,
        0,
        0
    );


    lv_label_set_text(
        _centerBottomLabel,
        ""
    );


    // ========================================================
    // BOTTOM BAR
    // ========================================================

    _bottomBar =
        lv_obj_create(
            _root
        );


    lv_obj_set_size(
        _bottomBar,
        172,
        52
    );


    lv_obj_set_pos(
        _bottomBar,
        0,
        268
    );


    lv_obj_set_style_bg_opa(
        _bottomBar,
        LV_OPA_TRANSP,
        0
    );


    lv_obj_set_style_border_width(
        _bottomBar,
        0,
        0
    );


    lv_obj_set_style_pad_all(
        _bottomBar,
        0,
        0
    );


    // ========================================================
    // BOTTOM LABEL
    // ========================================================

    _bottomLabel =
        lv_label_create(
            _bottomBar
        );


    lv_obj_set_width(
        _bottomLabel,
        172
    );


    lv_obj_set_height(
        _bottomLabel,
        52
    );


    lv_obj_align(
        _bottomLabel,
        LV_ALIGN_CENTER,
        0,
        0
    );


    lv_label_set_text(
        _bottomLabel,
        ""
    );


    lv_obj_set_style_text_font(
        _bottomLabel,
        &lv_font_montserrat_20,
        0
    );


    lv_obj_set_style_text_color(
        _bottomLabel,
        lv_color_hex(0xFFFFFF),
        0
    );


    lv_obj_set_style_text_align(
        _bottomLabel,
        LV_TEXT_ALIGN_CENTER,
        0
    );


    // ========================================================
    // BOTTOM LINE
    // ========================================================

    _bottomLine =
        lv_obj_create(
            _root
        );


    lv_obj_set_size(
        _bottomLine,
        172,
        1
    );


    lv_obj_set_pos(
        _bottomLine,
        0,
        267
    );


    lv_obj_set_style_bg_color(
        _bottomLine,
        lv_color_hex(0x303030),
        0
    );


    lv_obj_set_style_bg_opa(
        _bottomLine,
        LV_OPA_COVER,
        0
    );


    lv_obj_set_style_border_width(
        _bottomLine,
        0,
        0
    );


    // ========================================================
    // INITIAL STATE
    // ========================================================

    updateCenter();
}


// ============================================================
// CENTER MODE
// ============================================================

void ClockScreen::setCenterMode(
    CenterMode mode
)
{
    _centerMode =
        mode;


    updateCenter();
}


// ============================================================
// CENTER TEXT
// ============================================================

void ClockScreen::setCenterText(
    const char* text
)
{
    copyText(
        _centerText,
        sizeof(_centerText),
        text
    );


    _centerMode =
        CenterMode::TEXT;


    updateCenter();
}


// ============================================================
// CENTER DIGIT
// ============================================================

void ClockScreen::setCenterText(
    uint8_t digit
)
{
    if (
        digit > 9
    )
    {
        digit = 0;
    }


    _centerText[0] =
        '0' + digit;


    _centerText[1] =
        '\0';


    _centerMode =
        CenterMode::ONE_DIGIT;


    updateCenter();
}


// ============================================================
// TWO VERTICAL DIGITS
// ============================================================

void ClockScreen::setCenterText(
    uint8_t topDigit,
    uint8_t bottomDigit
)
{
    if (
        topDigit > 9
    )
    {
        topDigit = 0;
    }


    if (
        bottomDigit > 9
    )
    {
        bottomDigit = 0;
    }


    _centerTopText[0] =
        '0' + topDigit;


    _centerTopText[1] =
        '\0';


    _centerBottomText[0] =
        '0' + bottomDigit;


    _centerBottomText[1] =
        '\0';


    _centerMode =
        CenterMode::TWO_DIGITS_VERTICAL;


    updateCenter();
}


// ============================================================
// UPDATE CENTER
// ============================================================

void ClockScreen::updateCenter()
{
    if (
        _centerLabel == nullptr
    )
    {
        return;
    }


    switch (
        _centerMode
    )
    {
        // ====================================================
        // ONE DIGIT
        // ====================================================

        case CenterMode::ONE_DIGIT:
        {
            lv_obj_remove_flag(
                _centerLabel,
                LV_OBJ_FLAG_HIDDEN
            );


            lv_obj_add_flag(
                _centerTopLabel,
                LV_OBJ_FLAG_HIDDEN
            );


            lv_obj_add_flag(
                _centerBottomLabel,
                LV_OBJ_FLAG_HIDDEN
            );


            lv_label_set_text(
                _centerLabel,
                _centerText
            );


            break;
        }


        // ====================================================
        // TWO VERTICAL DIGITS
        // ====================================================

        case CenterMode::TWO_DIGITS_VERTICAL:
        {
            lv_obj_add_flag(
                _centerLabel,
                LV_OBJ_FLAG_HIDDEN
            );


            lv_obj_remove_flag(
                _centerTopLabel,
                LV_OBJ_FLAG_HIDDEN
            );


            lv_obj_remove_flag(
                _centerBottomLabel,
                LV_OBJ_FLAG_HIDDEN
            );


            lv_label_set_text(
                _centerTopLabel,
                _centerTopText
            );


            lv_label_set_text(
                _centerBottomLabel,
                _centerBottomText
            );


            break;
        }


        // ====================================================
        // TEXT
        // ====================================================

        case CenterMode::TEXT:
        {
            lv_obj_remove_flag(
                _centerLabel,
                LV_OBJ_FLAG_HIDDEN
            );


            lv_obj_add_flag(
                _centerTopLabel,
                LV_OBJ_FLAG_HIDDEN
            );


            lv_obj_add_flag(
                _centerBottomLabel,
                LV_OBJ_FLAG_HIDDEN
            );


            lv_label_set_text(
                _centerLabel,
                _centerText
            );


            break;
        }
    }
}


// ============================================================
// TOP TEXT
// ============================================================

void ClockScreen::setTopText(
    const char* text
)
{
    copyText(
        _topText,
        sizeof(_topText),
        text
    );


    if (
        _topLabel != nullptr
    )
    {
        lv_label_set_text(
            _topLabel,
            _topText
        );
    }
}


// ============================================================
// BOTTOM TEXT
// ============================================================

void ClockScreen::setBottomText(
    const char* text
)
{
    copyText(
        _bottomText,
        sizeof(_bottomText),
        text
    );


    if (
        _bottomLabel != nullptr
    )
    {
        lv_label_set_text(
            _bottomLabel,
            _bottomText
        );
    }
}


// ============================================================
// VISIBLE
// ============================================================

void ClockScreen::setVisible(
    bool visible
)
{
    _visible =
        visible;


    if (
        _root == nullptr
    )
    {
        return;
    }


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


// ============================================================
// IS VISIBLE
// ============================================================

bool ClockScreen::isVisible() const
{
    return _visible;
}


// ============================================================
// COPY TEXT
// ============================================================

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


    if (
        source == nullptr
    )
    {
        destination[0] =
            '\0';

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
