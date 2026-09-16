#include "ClockScreen.h"

#include <cstring>

// ============================================================
// COLORS
// ============================================================

static constexpr uint32_t COLOR_BACKGROUND = 0x000000;

static constexpr uint32_t COLOR_BAR = 0x0C0E12;

static constexpr uint32_t COLOR_LINE = 0x252830;

static constexpr uint32_t COLOR_TEXT = 0xFFFFFF;


// ============================================================
// DISPLAY SIZE
// ============================================================

static constexpr int16_t SCREEN_WIDTH = 172;

static constexpr int16_t SCREEN_HEIGHT = 320;


// ============================================================
// TOP BAR
// ============================================================

static constexpr int16_t TOP_BAR_HEIGHT = 38;

static constexpr int16_t TOP_LINE_Y = 38;


// ============================================================
// CENTER
// ============================================================

static constexpr int16_t CENTER_Y = 39;

static constexpr int16_t CENTER_HEIGHT = 228;


// ============================================================
// BOTTOM BAR
// ============================================================

static constexpr int16_t BOTTOM_LINE_Y = 267;

static constexpr int16_t BOTTOM_BAR_Y = 268;

static constexpr int16_t BOTTOM_BAR_HEIGHT = 52;


// ============================================================
// CONSTRUCTOR
// ============================================================

ClockScreen::ClockScreen()
    : _display(nullptr),
      _screen(nullptr),

      _topBar(nullptr),
      _topLine(nullptr),
      _topLabel(nullptr),

      _bottomBar(nullptr),
      _bottomLine(nullptr),
      _bottomLabel(nullptr),

      _centerLabel(nullptr),
      _centerTopLabel(nullptr),
      _centerBottomLabel(nullptr),

      _centerMode(CenterMode::ONE_DIGIT),

      _visible(true),
      _initialized(false)
{
    _topText[0] = '\0';

    _bottomText[0] = '\0';

    _centerText[0] = '\0';

    _centerTopText[0] = '\0';

    _centerBottomText[0] = '\0';
}


// ============================================================
// BEGIN
// ============================================================

bool ClockScreen::begin(
    lv_display_t* display
)
{
    if (display == nullptr)
        return false;

    if (_initialized)
        return true;

    _display = display;

    _screen =
        lv_display_get_screen_active(
            _display
        );

    if (_screen == nullptr)
        return false;

    createUI();

    _initialized = true;

    // Apply initial state
    updateTop();
    updateBottom();
    updateCenter();

    return true;
}


// ============================================================
// CREATE UI
// ============================================================

void ClockScreen::createUI()
{
    if (_screen == nullptr)
        return;


    // ========================================================
    // ROOT SCREEN
    // ========================================================

    lv_obj_set_style_bg_color(
        _screen,
        lv_color_hex(COLOR_BACKGROUND),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        _screen,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    lv_obj_set_style_border_width(
        _screen,
        0,
        LV_PART_MAIN
    );

    lv_obj_set_style_pad_all(
        _screen,
        0,
        LV_PART_MAIN
    );


    // ========================================================
    // TOP BAR
    // ========================================================

    _topBar =
        lv_obj_create(_screen);

    lv_obj_set_size(
        _topBar,
        SCREEN_WIDTH,
        TOP_BAR_HEIGHT
    );

    lv_obj_set_pos(
        _topBar,
        0,
        0
    );

    lv_obj_set_style_bg_color(
        _topBar,
        lv_color_hex(COLOR_BAR),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        _topBar,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    lv_obj_set_style_border_width(
        _topBar,
        0,
        LV_PART_MAIN
    );

    lv_obj_set_style_radius(
        _topBar,
        0,
        LV_PART_MAIN
    );

    lv_obj_set_style_pad_all(
        _topBar,
        0,
        LV_PART_MAIN
    );


    // ========================================================
    // TOP LINE
    // ========================================================

    _topLine =
        lv_obj_create(_screen);

    lv_obj_set_size(
        _topLine,
        SCREEN_WIDTH,
        1
    );

    lv_obj_set_pos(
        _topLine,
        0,
        TOP_LINE_Y
    );

    lv_obj_set_style_bg_color(
        _topLine,
        lv_color_hex(COLOR_LINE),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        _topLine,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    lv_obj_set_style_border_width(
        _topLine,
        0,
        LV_PART_MAIN
    );

    lv_obj_set_style_pad_all(
        _topLine,
        0,
        LV_PART_MAIN
    );


    // ========================================================
    // TOP LABEL
    // ========================================================

    _topLabel =
        lv_label_create(_topBar);

    lv_obj_set_width(
        _topLabel,
        SCREEN_WIDTH
    );

    lv_obj_set_style_text_color(
        _topLabel,
        lv_color_hex(COLOR_TEXT),
        LV_PART_MAIN
    );

    // Built-in LVGL font 28
    lv_obj_set_style_text_font(
        _topLabel,
        &lv_font_montserrat_28,
        LV_PART_MAIN
    );

    lv_obj_set_style_text_align(
        _topLabel,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN
    );

    lv_obj_set_style_pad_all(
        _topLabel,
        0,
        LV_PART_MAIN
    );

    lv_obj_align(
        _topLabel,
        LV_ALIGN_CENTER,
        0,
        0
    );


    // ========================================================
    // BOTTOM BAR
    // ========================================================

    _bottomBar =
        lv_obj_create(_screen);

    lv_obj_set_size(
        _bottomBar,
        SCREEN_WIDTH,
        BOTTOM_BAR_HEIGHT
    );

    lv_obj_set_pos(
        _bottomBar,
        0,
        BOTTOM_BAR_Y
    );

    lv_obj_set_style_bg_color(
        _bottomBar,
        lv_color_hex(COLOR_BAR),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        _bottomBar,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    lv_obj_set_style_border_width(
        _bottomBar,
        0,
        LV_PART_MAIN
    );

    lv_obj_set_style_radius(
        _bottomBar,
        0,
        LV_PART_MAIN
    );

    lv_obj_set_style_pad_all(
        _bottomBar,
        0,
        LV_PART_MAIN
    );


    // ========================================================
    // BOTTOM LINE
    // ========================================================

    _bottomLine =
        lv_obj_create(_screen);

    lv_obj_set_size(
        _bottomLine,
        SCREEN_WIDTH,
        1
    );

    lv_obj_set_pos(
        _bottomLine,
        0,
        BOTTOM_LINE_Y
    );

    lv_obj_set_style_bg_color(
        _bottomLine,
        lv_color_hex(COLOR_LINE),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        _bottomLine,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    lv_obj_set_style_border_width(
        _bottomLine,
        0,
        LV_PART_MAIN
    );

    lv_obj_set_style_pad_all(
        _bottomLine,
        0,
        LV_PART_MAIN
    );


    // ========================================================
    // BOTTOM LABEL
    // ========================================================

    _bottomLabel =
        lv_label_create(_bottomBar);

    lv_obj_set_width(
        _bottomLabel,
        SCREEN_WIDTH
    );

    lv_obj_set_style_text_color(
        _bottomLabel,
        lv_color_hex(COLOR_TEXT),
        LV_PART_MAIN
    );

    // Built-in LVGL font 28
    lv_obj_set_style_text_font(
        _bottomLabel,
        &lv_font_montserrat_28,
        LV_PART_MAIN
    );

    lv_obj_set_style_text_align(
        _bottomLabel,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN
    );

    lv_obj_set_style_pad_all(
        _bottomLabel,
        0,
        LV_PART_MAIN
    );

    lv_obj_align(
        _bottomLabel,
        LV_ALIGN_CENTER,
        0,
        0
    );


    // ========================================================
    // CENTER — SINGLE DIGIT / TEXT
    // ========================================================

    _centerLabel =
        lv_label_create(_screen);

    lv_obj_set_size(
        _centerLabel,
        SCREEN_WIDTH,
        CENTER_HEIGHT
    );

    lv_obj_set_style_text_color(
        _centerLabel,
        lv_color_hex(COLOR_TEXT),
        LV_PART_MAIN
    );

    // Built-in LVGL font 28
    lv_obj_set_style_text_font(
        _centerLabel,
        &lv_font_montserrat_28,
        LV_PART_MAIN
    );

    lv_obj_set_style_text_align(
        _centerLabel,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN
    );

    lv_obj_set_style_pad_all(
        _centerLabel,
        0,
        LV_PART_MAIN
    );

    /*
     * The label occupies exactly the center area:
     *
     * Y = 39
     * H = 228
     *
     * Therefore its center is:
     *
     * 39 + 228 / 2 = 153
     */

    lv_obj_align(
        _centerLabel,
        LV_ALIGN_TOP_MID,
        0,
        CENTER_Y
    );


    // ========================================================
    // CENTER TOP — TWO DIGITS
    // ========================================================

    _centerTopLabel =
        lv_label_create(_screen);

    lv_obj_set_size(
        _centerTopLabel,
        SCREEN_WIDTH,
        114
    );

    lv_obj_set_style_text_color(
        _centerTopLabel,
        lv_color_hex(COLOR_TEXT),
        LV_PART_MAIN
    );

    // Built-in LVGL font 28
    lv_obj_set_style_text_font(
        _centerTopLabel,
        &lv_font_montserrat_28,
        LV_PART_MAIN
    );

    lv_obj_set_style_text_align(
        _centerTopLabel,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN
    );

    lv_obj_set_style_pad_all(
        _centerTopLabel,
        0,
        LV_PART_MAIN
    );

    // Top half of center
    lv_obj_align(
        _centerTopLabel,
        LV_ALIGN_TOP_MID,
        0,
        CENTER_Y
    );


    // ========================================================
    // CENTER BOTTOM — TWO DIGITS
    // ========================================================

    _centerBottomLabel =
        lv_label_create(_screen);

    lv_obj_set_size(
        _centerBottomLabel,
        SCREEN_WIDTH,
        114
    );

    lv_obj_set_style_text_color(
        _centerBottomLabel,
        lv_color_hex(COLOR_TEXT),
        LV_PART_MAIN
    );

    // Built-in LVGL font 28
    lv_obj_set_style_text_font(
        _centerBottomLabel,
        &lv_font_montserrat_28,
        LV_PART_MAIN
    );

    lv_obj_set_style_text_align(
        _centerBottomLabel,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN
    );

    lv_obj_set_style_pad_all(
        _centerBottomLabel,
        0,
        LV_PART_MAIN
    );

    // Bottom half of center
    lv_obj_align(
        _centerBottomLabel,
        LV_ALIGN_TOP_MID,
        0,
        CENTER_Y + 114
    );


    // ========================================================
    // HIDE VERTICAL LABELS
    // ========================================================

    lv_obj_add_flag(
        _centerTopLabel,
        LV_OBJ_FLAG_HIDDEN
    );

    lv_obj_add_flag(
        _centerBottomLabel,
        LV_OBJ_FLAG_HIDDEN
    );
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
    if (destination == nullptr)
        return;

    if (destinationSize == 0)
        return;

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

    destination[destinationSize - 1] = '\0';
}


// ============================================================
// CENTER MODE
// ============================================================

void ClockScreen::setCenterMode(
    CenterMode mode
)
{
    _centerMode = mode;

    if (!_initialized)
        return;

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
// CENTER ONE DIGIT
// ============================================================

void ClockScreen::setCenterText(
    uint8_t digit
)
{
    if (digit > 9)
        digit = 0;

    snprintf(
        _centerText,
        sizeof(_centerText),
        "%u",
        digit
    );

    _centerMode =
        CenterMode::ONE_DIGIT;

    updateCenter();
}


// ============================================================
// CENTER TWO DIGITS VERTICAL
// ============================================================

void ClockScreen::setCenterText(
    uint8_t topDigit,
    uint8_t bottomDigit
)
{
    if (topDigit > 9)
        topDigit = 0;

    if (bottomDigit > 9)
        bottomDigit = 0;

    snprintf(
        _centerTopText,
        sizeof(_centerTopText),
        "%u",
        topDigit
    );

    snprintf(
        _centerBottomText,
        sizeof(_centerBottomText),
        "%u",
        bottomDigit
    );

    _centerMode =
        CenterMode::TWO_DIGITS_VERTICAL;

    updateCenter();
}


// ============================================================
// UPDATE CENTER
// ============================================================

void ClockScreen::updateCenter()
{
    if (!_initialized)
        return;

    if (_centerLabel == nullptr)
        return;

    if (_centerTopLabel == nullptr)
        return;

    if (_centerBottomLabel == nullptr)
        return;


    // --------------------------------------------------------
    // Hide everything first
    // --------------------------------------------------------

    lv_obj_add_flag(
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


    // --------------------------------------------------------
    // ONE DIGIT
    // --------------------------------------------------------

    if (_centerMode ==
        CenterMode::ONE_DIGIT)
    {
        lv_label_set_text(
            _centerLabel,
            _centerText
        );

        lv_obj_clear_flag(
            _centerLabel,
            LV_OBJ_FLAG_HIDDEN
        );

        return;
    }


    // --------------------------------------------------------
    // TEXT
    // --------------------------------------------------------

    if (_centerMode ==
        CenterMode::TEXT)
    {
        lv_label_set_text(
            _centerLabel,
            _centerText
        );

        lv_obj_clear_flag(
            _centerLabel,
            LV_OBJ_FLAG_HIDDEN
        );

        return;
    }


    // --------------------------------------------------------
    // TWO DIGITS VERTICAL
    // --------------------------------------------------------

    if (_centerMode ==
        CenterMode::TWO_DIGITS_VERTICAL)
    {
        lv_label_set_text(
            _centerTopLabel,
            _centerTopText
        );

        lv_label_set_text(
            _centerBottomLabel,
            _centerBottomText
        );

        lv_obj_clear_flag(
            _centerTopLabel,
            LV_OBJ_FLAG_HIDDEN
        );

        lv_obj_clear_flag(
            _centerBottomLabel,
            LV_OBJ_FLAG_HIDDEN
        );
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

    updateTop();
}


// ============================================================
// UPDATE TOP
// ============================================================

void ClockScreen::updateTop()
{
    if (!_initialized)
        return;

    if (_topLabel == nullptr)
        return;

    lv_label_set_text(
        _topLabel,
        _topText
    );
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

    updateBottom();
}


// ============================================================
// UPDATE BOTTOM
// ============================================================

void ClockScreen::updateBottom()
{
    if (!_initialized)
        return;

    if (_bottomLabel == nullptr)
        return;

    lv_label_set_text(
        _bottomLabel,
        _bottomText
    );
}


// ============================================================
// VISIBILITY
// ============================================================

void ClockScreen::setVisible(
    bool visible
)
{
    _visible = visible;

    if (!_initialized)
        return;

    if (_screen == nullptr)
        return;

    if (visible)
    {
        lv_obj_clear_flag(
            _screen,
            LV_OBJ_FLAG_HIDDEN
        );
    }
    else
    {
        lv_obj_add_flag(
            _screen,
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
