#include "LVGLManager.h"

#include "Pins.h"
#include "./utils/Logger.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

LVGLManager::LVGLManager(SPIManager& spi)
    : _spi(spi),

      _tft1(
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
      _lastUpdate(0)
{
    // --------------------------------------------------------
    // Context 0
    // --------------------------------------------------------

    _contexts[0] =
    {
        0,
        &_tft1,
        nullptr,
        _buffers[0],
        false
    };

    // --------------------------------------------------------
    // Context 1
    // --------------------------------------------------------

    _contexts[1] =
    {
        1,
        &_tft2,
        nullptr,
        _buffers[1],
        false
    };

    // --------------------------------------------------------
    // Context 2
    // --------------------------------------------------------

    _contexts[2] =
    {
        2,
        &_tft3,
        nullptr,
        _buffers[2],
        false
    };

    // --------------------------------------------------------
    // Context 3
    // --------------------------------------------------------

    _contexts[3] =
    {
        3,
        &_tft4,
        nullptr,
        _buffers[3],
        false
    };
}

// ============================================================
// BEGIN
// ============================================================

bool LVGLManager::begin()
{
    if (_initialized)
        return true;

    Logger::info(
        "LVGL",
        "Initializing LVGL display system..."
    );

    // ========================================================
    // SPI
    // ========================================================

    if (!_spi.isReady())
    {
        Logger::info(
            "LVGL",
            "SPI is not initialized. Starting SPI..."
        );

        if (!_spi.begin())
        {
            Logger::info(
                "LVGL",
                "SPI initialization failed"
            );

            return false;
        }
    }

    // ========================================================
    // BACKLIGHT
    // ========================================================

    pinMode(
        PIN_TFT_BL,
        OUTPUT
    );

    digitalWrite(
        PIN_TFT_BL,
        HIGH
    );

    Logger::info(
        "LVGL",
        "TFT backlight ON"
    );

    // ========================================================
    // LVGL
    // ========================================================

    lv_init();

    lv_tick_set_cb(
        []() -> uint32_t
        {
            return millis();
        }
    );

    // ========================================================
    // TFT DISPLAYS
    // ========================================================

    for (uint8_t i = 0; i < DISPLAY_COUNT; i++)
    {
        if (!initDisplay(i))
        {
            Logger::info(
                "LVGL",
                "Failed to initialize TFT"
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

// ============================================================
// INIT DISPLAY
// ============================================================

bool LVGLManager::initDisplay(
    uint8_t index
)
{
    if (index >= DISPLAY_COUNT)
        return false;

    DisplayContext& context =
        _contexts[index];

    Adafruit_ST7789& tft =
        *context.tft;

    Logger::info(
        "LVGL",
        "Initializing TFT..."
    );

    // ========================================================
    // TFT
    // ========================================================

    /*
     * ВАЖНО:
     *
     * Панель физически 172x320,
     * но контроллер ST7789 имеет RAM 240x320.
     *
     * Поэтому используем init(240,320),
     * а в flush добавляем X_OFFSET = 34.
     */

    tft.init(
        TFT_WIDTH,
        TFT_HEIGHT
    );

    tft.setRotation(0);

    tft.setSPISpeed(
        40000000
    );

    // ========================================================
    // BLACK SCREEN
    // ========================================================

    tft.fillScreen(
        ST77XX_BLACK
    );

    // ========================================================
    // LVGL DISPLAY
    // ========================================================

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

    context.lvDisplay =
        display;

    // ========================================================
    // BUFFER
    // ========================================================

    lv_display_set_buffers(
        display,

        context.buffer,
        nullptr,

        sizeof(lv_color_t) *
        BUFFER_SIZE,

        LV_DISPLAY_RENDER_MODE_PARTIAL
    );

    // ========================================================
    // FLUSH CALLBACK
    // ========================================================

    lv_display_set_flush_cb(
        display,
        LVGLManager::flushCallback
    );

    // ========================================================
    // USER DATA
    // ========================================================

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

// ============================================================
// FLUSH CALLBACK
// ============================================================

void LVGLManager::flushCallback(
    lv_display_t* display,
    const lv_area_t* area,
    uint8_t* pxMap
)
{
    if (display == nullptr)
        return;

    DisplayContext* context =
        static_cast<DisplayContext*>(
            lv_display_get_user_data(display)
        );

    if (context == nullptr)
        return;

    if (context->tft == nullptr)
        return;

    // --------------------------------------------------------
    // Context cannot directly access LVGLManager,
    // therefore all required physical mapping is done here.
    // --------------------------------------------------------

    Adafruit_ST7789& tft =
        *context->tft;

    // ========================================================
    // AREA
    // ========================================================

    const int16_t x =
        area->x1;

    const int16_t y =
        area->y1;

    const int16_t width =
        area->x2 -
        area->x1 +
        1;

    const int16_t height =
        area->y2 -
        area->y1 +
        1;

    // ========================================================
    // PHYSICAL POSITION
    // ========================================================

    const int16_t physicalX =
        x + X_OFFSET;

    const int16_t physicalY =
        y + Y_OFFSET;

    // ========================================================
    // SEND TO TFT
    // ========================================================

    tft.setAddrWindow(
        physicalX,
        physicalY,
        width,
        height
    );

    uint16_t* pixels =
        reinterpret_cast<uint16_t*>(
            pxMap
        );

    const uint32_t pixelCount =
        static_cast<uint32_t>(
            width
        ) *
        static_cast<uint32_t>(
            height
        );

    tft.writePixels(
        pixels,
        pixelCount
    );

    // ========================================================
    // LVGL READY
    // ========================================================

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

    lv_timer_handler();

    _lastUpdate = millis();
}

// ============================================================
// DISPLAY
// ============================================================

lv_display_t*
LVGLManager::display(
    uint8_t index
)
{
    if (index >= DISPLAY_COUNT)
        return nullptr;

    return _contexts[index].lvDisplay;
}

// ============================================================
// IS READY
// ============================================================

bool LVGLManager::isReady() const
{
    return _initialized;
}
