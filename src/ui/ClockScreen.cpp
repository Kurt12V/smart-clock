#include "ClockScreen.h"

#include <cstdio>
#include <cstring>

#include "fonts/redring_clock_245.h"

// ============================================================
// COLORS
// ============================================================

static constexpr uint32_t COLOR_BACKGROUND = 0x050608;
static constexpr uint32_t COLOR_BAR        = 0x0C0E12;
static constexpr uint32_t COLOR_LINE       = 0x252830;

static constexpr uint32_t COLOR_TOP_TEXT    = 0xA5AAB5;
static constexpr uint32_t COLOR_CENTER_TEXT = 0xFFFFFF;
static constexpr uint32_t COLOR_BOTTOM_TEXT = 0xD5D8DE;


// ============================================================
// CONSTRUCTOR
// ============================================================

ClockScreen::ClockScreen()
    : _display(nullptr),
      _root(nullptr),

      _topBar(nullptr),
      _topLabel(nullptr),
      _topLine(nullptr),

      _digit(nullptr),
      _digitTop(nullptr),
      _digitBottom(nullptr),

      _bottomBar(nullptr),
      _bottomLabel(nullptr),
      _bottomLine(nullptr),

      _centerMode(CenterMode::ONE_DIGIT),
      _digitValue(0),

      _visible(true)
{
    _centerText[0] = '\0';
    _lastCenterText[0] = '\0';

    _topText[0] = '\0';
    _lastTopText[0] = '\0';

    _bottomText[0] = '\0';
    _lastBottomText[0] = '\0';
}


// ============================================================
// BEGIN
// ============================================================

bool ClockScreen::begin(lv_display_t* display)
{
    if (display == nullptr)
        return false;

    _display = display;

    createUI();

    return true;
}


// ============================================================
// CREATE UI
// ============================================================

void ClockScreen::createUI()
{
    _root = lv_display_get_screen_active(_display);

    if (_root == nullptr)
        return;

    // --------------------------------------------------------
    // ROOT
    // --------------------------------------------------------

    lv_obj_set_style_bg_color(
        _root,
        lv_color_hex(COLOR_BACKGROUND),
        0
    );

    lv_obj_set_style_bg_opa(
        _root,
        LV_OPA_COVER,
        0
    );

    lv_obj_clear_flag(
        _root,
        LV_OBJ_FLAG_SCROLLABLE
    );


    // --------------------------------------------------------
    // TOP BAR
    // --------------------------------------------------------

    _topBar = lv_obj_create(_root);

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

    lv_obj_set_style_bg_color(
        _topBar,
        lv_color_hex(COLOR_BAR),
        0
    );

    lv_obj_set_style_bg_opa(
        _topBar,
        LV_OPA_COVER,
        0
    );

    lv_obj_set_style_border_width(
        _topBar,
        0,
        0
    );

    lv_obj_set_style_radius(
        _topBar,
        0,
        0
    );

    lv_obj_clear_flag(
        _topBar,
        LV_OBJ_FLAG_SCROLLABLE
    );


    // --------------------------------------------------------
    // TOP LABEL
    // --------------------------------------------------------

    _topLabel = lv_label_create(_topBar);

    lv_obj_set_width(
        _topLabel,
        152
    );

    lv_obj_align(
        _topLabel,
        LV_ALIGN_CENTER,
        0,
        0
    );

    lv_obj_set_style_text_font(
        _topLabel,
        &lv_font_montserrat_20,
        0
    );

    lv_obj_set_style_text_color(
        _topLabel,
        lv_color_hex(COLOR_TOP_TEXT),
        0
    );

    lv_obj_set_style_text_align(
        _topLabel,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_label_set_text(
        _topLabel,
        ""
    );


    // --------------------------------------------------------
    // TOP LINE
    // --------------------------------------------------------

    _topLine = lv_obj_create(_root);

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
        lv_color_hex(COLOR_LINE),
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

    lv_obj_set_style_radius(
        _topLine,
        0,
        0
    );


    // --------------------------------------------------------
    // CENTER — ONE DIGIT
    // --------------------------------------------------------

    _digit = lv_label_create(_root);

    lv_obj_set_size(
        _digit,
        172,
        180
    );

    lv_obj_set_pos(
        _digit,
        0,
        -4
    );

    lv_obj_set_style_text_font(
        _digit,
        &redring_clock_245,
        0
    );

    lv_obj_set_style_text_color(
        _digit,
        lv_color_hex(COLOR_CENTER_TEXT),
        0
    );

    lv_obj_set_style_text_align(
        _digit,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_obj_set_style_text_line_space(
        _digit,
        0,
        0
    );

    lv_label_set_text(
        _digit,
        "0"
    );


    // --------------------------------------------------------
    // CENTER — TWO DIGITS VERTICAL
    // --------------------------------------------------------

    _digitTop = lv_label_create(_root);

    lv_obj_set_size(
        _digitTop,
        172,
        125
    );

    lv_obj_set_pos(
        _digitTop,
        0,
        43
    );

    lv_obj_set_style_text_font(
        _digitTop,
        &redring_clock_245,
        0
    );

    lv_obj_set_style_text_color(
        _digitTop,
        lv_color_hex(COLOR_CENTER_TEXT),
        0
    );

    lv_obj_set_style_text_align(
        _digitTop,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_label_set_text(
        _digitTop,
        "0"
    );


    _digitBottom = lv_label_create(_root);

    lv_obj_set_size(
        _digitBottom,
        172,
        125
    );

    lv_obj_set_pos(
        _digitBottom,
        0,
        168
    );

    lv_obj_set_style_text_font(
        _digitBottom,
        &redring_clock_245,
        0
    );

    lv_obj_set_style_text_color(
        _digitBottom,
        lv_color_hex(COLOR_CENTER_TEXT),
        0
    );

    lv_obj_set_style_text_align(
        _digitBottom,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_label_set_text(
        _digitBottom,
        "0"
    );


    // --------------------------------------------------------
    // BOTTOM BAR
    // --------------------------------------------------------

    _bottomBar = lv_obj_create(_root);

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
        lv_color_hex(COLOR_BAR),
        0
    );

    lv_obj_set_style_bg_opa(
        _bottomBar,
        LV_OPA_COVER,
        0
    );

    lv_obj_set_style_border_width(
        _bottomBar,
        0,
        0
    );

    lv_obj_set_style_radius(
        _bottomBar,
        0,
        0
    );

    lv_obj_clear_flag(
        _bottomBar,
        LV_OBJ_FLAG_SCROLLABLE
    );


    // --------------------------------------------------------
    // BOTTOM LABEL
    // --------------------------------------------------------

    _bottomLabel = lv_label_create(_bottomBar);

    lv_obj_set_width(
        _bottomLabel,
        152
    );

    lv_obj_align(
        _bottomLabel,
        LV_ALIGN_CENTER,
        0,
        0
    );

    lv_obj_set_style_text_font(
        _bottomLabel,
        &lv_font_montserrat_24,
        0
    );

    lv_obj_set_style_text_color(
        _bottomLabel,
        lv_color_hex(COLOR_BOTTOM_TEXT),
        0
    );

    lv_obj_set_style_text_align(
        _bottomLabel,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_label_set_text(
        _bottomLabel,
        ""
    );


    // --------------------------------------------------------
    // BOTTOM LINE
    // --------------------------------------------------------

    _bottomLine = lv_obj_create(_root);

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
        lv_color_hex(COLOR_LINE),
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

    lv_obj_set_style_radius(
        _bottomLine,
        0,
        0
    );


    // --------------------------------------------------------
    // INITIAL STATE
    // --------------------------------------------------------

    updateCenter();
    updateTop();
    updateBottom();
}


// ============================================================
// SET CENTER MODE
// ============================================================

void ClockScreen::setCenterMode(CenterMode mode)
{
    if (_centerMode == mode)
        return;

    _centerMode = mode;

    updateCenter();
}


// ============================================================
// GET CENTER MODE
// ============================================================

ClockScreen::CenterMode ClockScreen::getCenterMode() const
{
    return _centerMode;
}


// ============================================================
// SET DIGIT
// ============================================================

void ClockScreen::setDigit(uint8_t digit)
{
    if (digit > 9)
        digit = 9;

    if (_digitValue == digit)
        return;

    _digitValue = digit;

    char buffer[2];

    buffer[0] = '0' + digit;
    buffer[1] = '\0';

    setCenterText(buffer);
}


// ============================================================
// SET CENTER TEXT
// ============================================================

void ClockScreen::setCenterText(const char* text)
{
    if (text == nullptr)
        text = "";

    if (strcmp(_centerText, text) == 0)
        return;

    strncpy(
        _centerText,
        text,
        sizeof(_centerText) - 1
    );

    _centerText[sizeof(_centerText) - 1] = '\0';

    updateCenter();
}


// ============================================================
// SET CENTER TEXT — ONE DIGIT
// ============================================================

void ClockScreen::setCenterText(uint8_t digit)
{
    if (digit > 9)
        digit = 9;

    char buffer[2];

    buffer[0] = '0' + digit;
    buffer[1] = '\0';

    setCenterText(buffer);
}


// ============================================================
// SET CENTER TEXT — TWO DIGITS
// ============================================================

void ClockScreen::setCenterText(
    uint8_t topDigit,
    uint8_t bottomDigit
)
{
    if (topDigit > 9)
        topDigit = 9;

    if (bottomDigit > 9)
        bottomDigit = 9;

    char buffer[3];

    buffer[0] = '0' + topDigit;
    buffer[1] = '0' + bottomDigit;
    buffer[2] = '\0';

    setCenterText(buffer);
}


// ============================================================
// UPDATE CENTER
// ============================================================

void ClockScreen::updateCenter()
{
    if (_digit == nullptr ||
        _digitTop == nullptr ||
        _digitBottom == nullptr)
    {
        return;
    }

    if (_centerMode == CenterMode::ONE_DIGIT)
    {
        lv_obj_clear_flag(
            _digit,
            LV_OBJ_FLAG_HIDDEN
        );

        lv_obj_add_flag(
            _digitTop,
            LV_OBJ_FLAG_HIDDEN
        );

        lv_obj_add_flag(
            _digitBottom,
            LV_OBJ_FLAG_HIDDEN
        );

        lv_label_set_text(
            _digit,
            _centerText
        );
    }
    else
    {
        lv_obj_add_flag(
            _digit,
            LV_OBJ_FLAG_HIDDEN
        );

        lv_obj_clear_flag(
            _digitTop,
            LV_OBJ_FLAG_HIDDEN
        );

        lv_obj_clear_flag(
            _digitBottom,
            LV_OBJ_FLAG_HIDDEN
        );

        const char top =
            (_centerText[0] != '\0')
                ? _centerText[0]
                : ' ';

        const char bottom =
            (_centerText[1] != '\0')
                ? _centerText[1]
                : ' ';

        char topText[2] = { top, '\0' };
        char bottomText[2] = { bottom, '\0' };

        lv_label_set_text(
            _digitTop,
            topText
        );

        lv_label_set_text(
            _digitBottom,
            bottomText
        );
    }

    strncpy(
        _lastCenterText,
        _centerText,
        sizeof(_lastCenterText) - 1
    );

    _lastCenterText[
        sizeof(_lastCenterText) - 1
    ] = '\0';
}


// ============================================================
// SET TOP TEXT
// ============================================================

void ClockScreen::setTopText(const char* text)
{
    if (text == nullptr)
        text = "";

    if (strcmp(_topText, text) == 0)
        return;

    strncpy(
        _topText,
        text,
        sizeof(_topText) - 1
    );

    _topText[sizeof(_topText) - 1] = '\0';

    updateTop();
}


// ============================================================
// UPDATE TOP
// ============================================================

void ClockScreen::updateTop()
{
    if (_topLabel == nullptr)
        return;

    lv_label_set_text(
        _topLabel,
        _topText
    );

    strncpy(
        _lastTopText,
        _topText,
        sizeof(_lastTopText) - 1
    );

    _lastTopText[
        sizeof(_lastTopText) - 1
    ] = '\0';
}


// ============================================================
// SET BOTTOM TEXT
// ============================================================

void ClockScreen::setBottomText(const char* text)
{
    if (text == nullptr)
        text = "";

    if (strcmp(_bottomText, text) == 0)
        return;

    strncpy(
        _bottomText,
        text,
        sizeof(_bottomText) - 1
    );

    _bottomText[sizeof(_bottomText) - 1] = '\0';

    updateBottom();
}


// ============================================================
// UPDATE BOTTOM
// ============================================================

void ClockScreen::updateBottom()
{
    if (_bottomLabel == nullptr)
        return;

    lv_label_set_text(
        _bottomLabel,
        _bottomText
    );

    strncpy(
        _lastBottomText,
        _bottomText,
        sizeof(_lastBottomText) - 1
    );

    _lastBottomText[
        sizeof(_lastBottomText) - 1
    ] = '\0';
}


// ============================================================
// VISIBILITY
// ============================================================

void ClockScreen::setVisible(bool visible)
{
    if (_visible == visible)
        return;

    _visible = visible;

    if (_root == nullptr)
        return;

    if (_visible)
    {
        lv_obj_clear_flag(
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
// SCREEN
// ============================================================

lv_obj_t* ClockScreen::screen() const
{
    return _root;
}
