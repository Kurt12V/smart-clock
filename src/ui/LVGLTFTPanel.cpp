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

bool LVGLTFTPanel::begin()
{
    // ========================================================
    // TFT
    // ========================================================

    // Контроллер ST7789 обычно имеет RAM 240x320.
    // Активная область твоего дисплея — 172x320.
    _tft.init(240, 320);

    _tft.setRotation(0);

    _tft.fillScreen(ST77XX_BLACK);

    // Очистка активной области
    _tft.fillRect(
        _xOffset,
        _yOffset,
        WIDTH,
        HEIGHT,
        ST77XX_BLACK
    );

    // ========================================================
    // LVGL DISPLAY
    // ========================================================

    _display = lv_display_create(
        WIDTH,
        HEIGHT
    );

    if (_display == nullptr)
    {
        return false;
    }

    // ========================================================
    // BUFFER
    // ========================================================

    lv_display_set_buffers(
        _display,
        _buffer,
        nullptr,
        sizeof(_buffer),
        LV_DISPLAY_RENDER_MODE_PARTIAL
    );

    // ========================================================
    // FLUSH
    // ========================================================

    lv_display_set_flush_cb(
        _display,
        flushCallback
    );

    // Сохраняем this в user_data
    lv_display_set_user_data(
        _display,
        this
    );

    _ready = true;

    return true;
}

lv_display_t* LVGLTFTPanel::display() const
{
    return _display;
}

bool LVGLTFTPanel::isReady() const
{
    return _ready;
}

void LVGLTFTPanel::setBacklight(bool state)
{
    // BL управляется отдельно ScreenManager.
    // Здесь оставлено для расширения.
    (void)state;
}

void LVGLTFTPanel::flushCallback(
    lv_display_t* display,
    const lv_area_t* area,
    uint8_t* pxMap
)
{
    auto* panel =
        static_cast<LVGLTFTPanel*>(
            lv_display_get_user_data(display)
        );

    if (panel == nullptr)
    {
        lv_display_flush_ready(display);
        return;
    }

    panel->flush(
        area,
        pxMap
    );

    lv_display_flush_ready(display);
}

void LVGLTFTPanel::flush(
    const lv_area_t* area,
    uint8_t* pxMap
)
{
    const int32_t width =
        area->x2 - area->x1 + 1;

    const int32_t height =
        area->y2 - area->y1 + 1;

    if (width <= 0 || height <= 0)
    {
        return;
    }

    // LVGL 9 с RGB565.
    auto* pixels =
        reinterpret_cast<uint16_t*>(pxMap);

    _tft.drawRGBBitmap(
        area->x1 + _xOffset,
        area->y1 + _yOffset,
        pixels,
        width,
        height
    );
}