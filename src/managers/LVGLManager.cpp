#include "LVGLManager.h"

#include <SPI.h>

LVGLManager::LVGLManager()
    : _tft1(
        PIN_TFT_CS1,
        PIN_TFT_DC,
        PIN_TFT_RST1
    ),
      _tft2(
        PIN_TFT_CS2,
        PIN_TFT_DC,
        PIN_TFT_RST2
    ),
      _tft3(
        PIN_TFT_CS3,
        PIN_TFT_DC,
        PIN_TFT_RST3
    ),
      _tft4(
        PIN_TFT_CS4,
        PIN_TFT_DC,
        PIN_TFT_RST4
    ),
      _initialized(false),
      _lastTick(0)
{
    _contexts[0].tft = &_tft1;
    _contexts[1].tft = &_tft2;
    _contexts[2].tft = &_tft3;
    _contexts[3].tft = &_tft4;

    for (uint8_t i = 0; i < DISPLAY_COUNT; ++i)
    {
        _contexts[i].buffer = _buffers[i];

        _contexts[i].xOffset = 34;
        _contexts[i].yOffset = 0;
    }
}


// ============================================================
// BEGIN
// ============================================================

bool LVGLManager::begin()
{
    if (_initialized)
        return true;

    // --------------------------------------------------------
    // LVGL
    // --------------------------------------------------------

    lv_init();

    // LVGL сама будет брать время через millis().
    lv_tick_set_cb(
        []() -> uint32_t
        {
            return millis();
        }
    );

    // --------------------------------------------------------
    // SPI
    // --------------------------------------------------------

    SPI.begin(
        PIN_SCLK,
        PIN_SD_MISO,
        PIN_MOSI
    );

    // --------------------------------------------------------
    // BACKLIGHT
    // --------------------------------------------------------

    pinMode(PIN_TFT_BL, OUTPUT);

    digitalWrite(
        PIN_TFT_BL,
        HIGH
    );

    // --------------------------------------------------------
    // DISPLAYS
    // --------------------------------------------------------

    for (uint8_t i = 0; i < DISPLAY_COUNT; ++i)
    {
        if (!initDisplay(i))
        {
            return false;
        }
    }

    _lastTick = millis();

    _initialized = true;

    return true;
}


// ============================================================
// INIT ONE DISPLAY
// ============================================================

bool LVGLManager::initDisplay(uint8_t index)
{
    if (index >= DISPLAY_COUNT)
        return false;

    DisplayContext& context =
        _contexts[index];

    if (context.tft == nullptr)
        return false;

    Adafruit_ST7789& tft =
        *context.tft;

    // --------------------------------------------------------
    // ST7789
    // --------------------------------------------------------

    tft.init(
        240,
        320
    );

    tft.setRotation(0);

    tft.fillScreen(
        ST77XX_BLACK
    );

    // --------------------------------------------------------
    // LVGL DISPLAY
    // --------------------------------------------------------

    lv_display_t* display =
        lv_display_create(
            WIDTH,
            HEIGHT
        );

    if (display == nullptr)
    {
        return false;
    }

    context.lvDisplay = display;

    // Передаём LVGL указатель
    // на наш DisplayContext.
    lv_display_set_user_data(
        display,
        &context
    );

    // --------------------------------------------------------
    // BUFFER
    // --------------------------------------------------------

    lv_display_set_buffers(
        display,

        context.buffer,

        nullptr,

        sizeof(_buffers[index]),

        LV_DISPLAY_RENDER_MODE_PARTIAL
    );

    // --------------------------------------------------------
    // FLUSH CALLBACK
    // --------------------------------------------------------

    lv_display_set_flush_cb(
        display,
        LVGLManager::flushCallback
    );

    context.initialized = true;

    return true;
}


// ============================================================
// FLUSH
// ============================================================

void LVGLManager::flushCallback(
    lv_display_t* display,
    const lv_area_t* area,
    uint8_t* pxMap
)
{
    if (display == nullptr)
        return;

    if (area == nullptr)
        return;

    if (pxMap == nullptr)
        return;

    DisplayContext* context =
        static_cast<DisplayContext*>(
            lv_display_get_user_data(display)
        );

    if (context == nullptr)
    {
        lv_display_flush_ready(display);
        return;
    }

    if (context->tft == nullptr)
    {
        lv_display_flush_ready(display);
        return;
    }

    Adafruit_ST7789& tft =
        *context->tft;

    // --------------------------------------------------------
    // AREA
    // --------------------------------------------------------

    const int16_t width =
        area->x2 - area->x1 + 1;

    const int16_t height =
        area->y2 - area->y1 + 1;

    const int16_t x =
        area->x1 + context->xOffset;

    const int16_t y =
        area->y1 + context->yOffset;

    if (width <= 0 || height <= 0)
    {
        lv_display_flush_ready(display);
        return;
    }

    // --------------------------------------------------------
    // SEND PIXELS
    // --------------------------------------------------------

    tft.startWrite();

    tft.setAddrWindow(
        x,
        y,
        width,
        height
    );

    uint16_t* pixels =
        reinterpret_cast<uint16_t*>(pxMap);

    const uint32_t count =
        static_cast<uint32_t>(width) *
        static_cast<uint32_t>(height);

    // Adafruit_ST7789 в текущей
    // установленной версии имеет pushColor(),
    // но не pushColors().
    for (uint32_t i = 0; i < count; ++i)
    {
        tft.pushColor(
            pixels[i]
        );
    }

    tft.endWrite();

    // --------------------------------------------------------
    // TELL LVGL
    // --------------------------------------------------------

    lv_display_flush_ready(
        display
    );
}


// ============================================================
// UPDATE
// ============================================================

void LVGLManager::update()
{
    if (!_initialized)
        return;

    const uint32_t now =
        millis();

    const uint32_t elapsed =
        now - _lastTick;

    if (elapsed > 0)
    {
        lv_tick_inc(
            elapsed
        );

        _lastTick = now;
    }

    lv_timer_handler();
}


// ============================================================
// TICK
// ============================================================

void LVGLManager::tick(
    uint32_t elapsedMs
)
{
    if (!_initialized)
        return;

    lv_tick_inc(
        elapsedMs
    );
}


// ============================================================
// GET DISPLAY
// ============================================================

lv_display_t* LVGLManager::display(
    uint8_t index
)
{
    if (index >= DISPLAY_COUNT)
        return nullptr;

    return _contexts[index].lvDisplay;
}


// ============================================================
// READY
// ============================================================

bool LVGLManager::isReady() const
{
    return _initialized;
}