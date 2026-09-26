#include "LVGLTFTPanel.h"

LVGLTFTPanel::LVGLTFTPanel(
    Adafruit_ST7789& tft,
    int16_t xOffset,
    int16_t yOffset
)
    : _tft(tft),
      _xOffset(xOffset),
      _yOffset(yOffset),
      _display(nullptr),
      _ready(false)
{
}

// ============================================================
// BEGIN
// ============================================================

bool LVGLTFTPanel::begin()
{
    _tft.init(240, 320);

    _tft.setRotation(0);

    _tft.fillScreen(ST77XX_BLACK);

    _tft.fillRect(
        _xOffset,
        _yOffset,
        WIDTH,
        HEIGHT,
        ST77XX_BLACK
    );

    _display =
        lv_display_create(
            WIDTH,
            HEIGHT
        );

    if (_display == nullptr)
        return false;

    lv_display_set_buffers(
        _display,
        _buffer,
        nullptr,
        sizeof(_buffer),
        LV_DISPLAY_RENDER_MODE_PARTIAL
    );

    lv_display_set_flush_cb(
        _display,
        flushCallback
    );

    lv_display_set_user_data(
        _display,
        this
    );

    _ready = true;

    return true;
}

// ============================================================
// DISPLAY
// ============================================================

lv_display_t*
LVGLTFTPanel::display() const
{
    return _display;
}

// ============================================================
// READY
// ============================================================

bool LVGLTFTPanel::isReady() const
{
    return _ready;
}

// ============================================================
// BACKLIGHT
// ============================================================

void LVGLTFTPanel::setBacklight(
    bool state
)
{
    (void)state;
}

// ============================================================
// FLUSH CALLBACK
// ============================================================

void LVGLTFTPanel::flushCallback(
    lv_display_t* display,
    const lv_area_t* area,
    uint8_t* pxMap
)
{
    auto* panel =
        static_cast<LVGLTFTPanel*>(
            lv_display_get_user_data(
                display
            )
        );

    if (panel == nullptr)
    {
        lv_display_flush_ready(
            display
        );

        return;
    }

    panel->flush(
        area,
        pxMap
    );

    lv_display_flush_ready(
        display
    );
}

// ============================================================
// FLUSH
// ============================================================

void LVGLTFTPanel::flush(
    const lv_area_t* area,
    uint8_t* pxMap
)
{
    if (area == nullptr ||
        pxMap == nullptr)
    {
        return;
    }

    const int32_t width =
        area->x2 -
        area->x1 +
        1;

    const int32_t height =
        area->y2 -
        area->y1 +
        1;

    if (width <= 0 ||
        height <= 0)
    {
        return;
    }

    auto* pixels =
        reinterpret_cast<uint16_t*>(
            pxMap
        );

    _tft.drawRGBBitmap(
        area->x1 + _xOffset,
        area->y1 + _yOffset,
        pixels,
        width,
        height
    );
}
