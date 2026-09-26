#include "LVGLManager.h"

#include "Pins.h"
#include "./utils/Logger.h"

LVGLManager::LVGLManager(SPIManager& spi)
    : _spi(spi),
      _tft1(PIN_TFT_CS1, PIN_TFT_DC, PIN_TFT_RST1),
      _tft2(PIN_TFT_CS2, PIN_TFT_DC, PIN_TFT_RST2),
      _tft3(PIN_TFT_CS3, PIN_TFT_DC, PIN_TFT_RST3),
      _tft4(PIN_TFT_CS4, PIN_TFT_DC, PIN_TFT_RST4),
      _initialized(false),
      _lastUpdate(0)
{
    _contexts[0] = {
        0,
        &_tft1,
        nullptr,
        _buffers[0],
        false
    };

    _contexts[1] = {
        1,
        &_tft2,
        nullptr,
        _buffers[1],
        false
    };

    _contexts[2] = {
        2,
        &_tft3,
        nullptr,
        _buffers[2],
        false
    };

    _contexts[3] = {
        3,
        &_tft4,
        nullptr,
        _buffers[3],
        false
    };
}

bool LVGLManager::begin()
{
    if (_initialized)
        return true;

    Logger::info(
        "LVGL",
        "Initializing LVGL display system..."
    );

    if (!_spi.isReady())
    {
        if (!_spi.begin())
        {
            Logger::info(
                "LVGL",
                "SPI initialization failed"
            );

            return false;
        }
    }

    // ---------------------------------------------------------
    // Backlight
    // ---------------------------------------------------------

    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, HIGH);

    // ---------------------------------------------------------
    // LVGL
    // ---------------------------------------------------------

    lv_init();

    lv_tick_set_cb(
        []() -> uint32_t
        {
            return millis();
        }
    );

    // ---------------------------------------------------------
    // TFT displays
    // ---------------------------------------------------------

    for (uint8_t i = 0; i < DISPLAY_COUNT; i++)
    {
        if (!initDisplay(i))
        {
            Logger::info(
                "LVGL",
                "TFT initialization failed"
            );

            return false;
        }
    }

    _initialized = true;
    _lastUpdate = millis();

    Logger::info(
        "LVGL",
        "LVGL display system initialized"
    );

    return true;
}

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

    Logger::info(
        "LVGL",
        "Initializing TFT..."
    );

    // ---------------------------------------------------------
    // TFT initialization
    // ---------------------------------------------------------

    tft.init(
        TFT_WIDTH,
        TFT_HEIGHT
    );

    tft.setRotation(0);

    tft.setSPISpeed(40000000);

    tft.fillScreen(
        ST77XX_BLACK
    );

    // ---------------------------------------------------------
    // LVGL display
    // ---------------------------------------------------------

    lv_display_t* display =
        lv_display_create(
            WIDTH,
            HEIGHT
        );

    if (display == nullptr)
    {
        Logger::info(
            "LVGL",
            "lv_display_create failed"
        );

        return false;
    }

    context.lvDisplay = display;

    // ---------------------------------------------------------
    // Drawing buffer
    // ---------------------------------------------------------

    lv_display_set_buffers(
        display,
        context.buffer,
        nullptr,
        sizeof(lv_color_t) * BUFFER_SIZE,
        LV_DISPLAY_RENDER_MODE_PARTIAL
    );

    // ---------------------------------------------------------
    // Flush callback
    // ---------------------------------------------------------

    lv_display_set_flush_cb(
        display,
        LVGLManager::flushCallback
    );

    // ---------------------------------------------------------
    // User data
    // ---------------------------------------------------------

    lv_display_set_user_data(
        display,
        &context
    );

    context.initialized = true;

    Logger::info(
        "LVGL",
        "TFT initialized"
    );

    return true;
}

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
        return;

    if (context->tft == nullptr)
        return;

    Adafruit_ST7789& tft =
        *context->tft;

    // ---------------------------------------------------------
    // LVGL coordinates
    // ---------------------------------------------------------

    int16_t x =
        area->x1;

    int16_t y =
        area->y1;

    int16_t width =
        area->x2 - area->x1 + 1;

    int16_t height =
        area->y2 - area->y1 + 1;

    if (width <= 0 || height <= 0)
    {
        lv_display_flush_ready(display);
        return;
    }

    // ---------------------------------------------------------
    // Convert logical 172x320 coordinates
    // to physical 240x320 coordinates.
    //
    // 240 - 172 = 68
    // 68 / 2 = 34
    // ---------------------------------------------------------

    int16_t physicalX =
        x + X_OFFSET;

    int16_t physicalY =
        y + Y_OFFSET;

    // ---------------------------------------------------------
    // SPI transaction
    // ---------------------------------------------------------

    tft.startWrite();

    tft.setAddrWindow(
        physicalX,
        physicalY,
        width,
        height
    );

    uint16_t* pixels =
        reinterpret_cast<uint16_t*>(pxMap);

    uint32_t pixelCount =
        static_cast<uint32_t>(width) *
        static_cast<uint32_t>(height);

    tft.writePixels(
        pixels,
        pixelCount
    );

    tft.endWrite();

    // ---------------------------------------------------------
    // Tell LVGL that flush is finished
    // ---------------------------------------------------------

    lv_display_flush_ready(
        display
    );
}

void LVGLManager::update()
{
    if (!_initialized)
        return;

    lv_timer_handler();

    _lastUpdate = millis();
}

lv_display_t* LVGLManager::display(
    uint8_t index
)
{
    if (index >= DISPLAY_COUNT)
        return nullptr;

    return _contexts[index].lvDisplay;
}

bool LVGLManager::isReady() const
{
    return _initialized;
}

bool LVGLManager::testDisplays()
{
    if (!_initialized)
        return false;

    for (uint8_t i = 0; i < DISPLAY_COUNT; i++)
    {
        if (_contexts[i].tft == nullptr)
            return false;

        _contexts[i].tft->fillScreen(
            ST77XX_RED
        );
    }

    return true;
}
