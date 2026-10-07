#include "LVGLManager.h"

#include "Pins.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

LVGLManager::LVGLManager(
    SPIManager& spi
)
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
      _lastUpdate(0),
      _brightness(100),
      _applied(255)
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

// ============================================================
// BEGIN
// ============================================================

bool LVGLManager::begin()
{
    if (_initialized)
        return true;

    Serial0.println(
        "[LVGL] Initializing display system..."
    );

    // ========================================================
    // SPI
    // ========================================================

    if (!_spi.isReady())
    {
        Serial0.println(
            "[LVGL] SPI is not ready"
        );

        if (!_spi.begin())
        {
            Serial0.println(
                "[LVGL] SPI initialization failed"
            );

            return false;
        }
    }

    // ========================================================
    // BACKLIGHT PWM (ESP32 Arduino Core 2.x)
    // ========================================================

    ledcSetup(
        BL_CHANNEL,
        5000,
        8
    );

    ledcAttachPin(
        PIN_TFT_BL,
        BL_CHANNEL
    );

    // Keep displays dark while initializing.
    ledcWrite(
        BL_CHANNEL,
        0
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

    for (
        uint8_t i = 0;
        i < DISPLAY_COUNT;
        i++
    )
    {
        Serial0.print(
            "[LVGL] Initializing TFT "
        );

        Serial0.println(i);

        if (!initDisplay(i))
        {
            Serial0.println(
                "[LVGL] TFT initialization failed"
            );

            return false;
        }
    }

    // ========================================================
    // IMPORTANT
    // Clear physical displays once more
    // ========================================================

    clearDisplays();

    // ========================================================
    // READY
    // ========================================================

    _initialized = true;

    _lastUpdate = millis();

    // ========================================================
    // APPLY BRIGHTNESS
    // ========================================================

    _applied = 255;   // сбросить кэш
    applyBrightness();

    Serial0.println(
        "[LVGL] Display system initialized"
    );

    return true;
}

// ============================================================
// SET BRIGHTNESS
// ============================================================

void LVGLManager::setBrightness(uint8_t percent)
{
    if (percent > 100)
        percent = 100;

    _brightness = percent;

    applyBrightness();
}

// ============================================================
// GET BRIGHTNESS
// ============================================================

uint8_t LVGLManager::brightness() const
{
    return _brightness;
}

// ============================================================
// APPLY BRIGHTNESS (PWM)
// ============================================================

void LVGLManager::applyBrightness()
{
    if (!_initialized)
        return;

    if (_brightness == _applied)
        return;

    _applied = _brightness;

    // 0..100% -> 0..255 duty
    uint8_t duty = static_cast<uint8_t>(
        (static_cast<uint16_t>(_brightness) * 255U) / 100U
    );

    ledcWrite(BL_CHANNEL, duty);

    Serial0.print("[LVGL] Brightness: ");
    Serial0.print(_brightness);
    Serial0.println("%");
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

    if (context.tft == nullptr)
        return false;

    Adafruit_ST7789& tft =
        *context.tft;

    // ========================================================
    // TFT
    // ========================================================

    tft.init(
        TFT_WIDTH,
        TFT_HEIGHT
    );

    tft.setRotation(2);

    tft.setSPISpeed(
        40000000
    );

    // ========================================================
    // IMPORTANT
    // Clear TFT GRAM immediately
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
        Serial0.println(
            "[LVGL] lv_display_create failed"
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
        sizeof(lv_color_t) * BUFFER_SIZE,
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

    return true;
}

// ============================================================
// CLEAR DISPLAYS
// ============================================================

void LVGLManager::clearDisplays()
{
    if (!_spi.isReady())
        return;

    for (
        uint8_t i = 0;
        i < DISPLAY_COUNT;
        i++
    )
    {
        DisplayContext& context =
            _contexts[i];

        if (!context.initialized)
            continue;

        if (context.tft == nullptr)
            continue;

        context.tft->fillScreen(
            ST77XX_BLACK
        );
    }
}

// ============================================================
// FORCE REFRESH
// ============================================================

void LVGLManager::refresh()
{
    if (!_initialized)
        return;

    for (
        uint8_t i = 0;
        i < DISPLAY_COUNT;
        i++
    )
    {
        lv_display_t* display =
            _contexts[i].lvDisplay;

        if (display == nullptr)
            continue;

        lv_obj_invalidate(
            lv_display_get_screen_active(
                display
            )
        );
    }

    lv_timer_handler();
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

    if (area == nullptr)
        return;

    if (pxMap == nullptr)
        return;

    DisplayContext* context =
        static_cast<DisplayContext*>(
            lv_display_get_user_data(
                display
            )
        );

    if (context == nullptr)
        return;

    if (context->tft == nullptr)
        return;

    Adafruit_ST7789& tft =
        *context->tft;

    // ========================================================
    // LVGL AREA
    // ========================================================

    int16_t x =
        area->x1;

    int16_t y =
        area->y1;

    int16_t width =
        area->x2 -
        area->x1 +
        1;

    int16_t height =
        area->y2 -
        area->y1 +
        1;

    if (
        width <= 0 ||
        height <= 0
    )
    {
        lv_display_flush_ready(
            display
        );

        return;
    }

    // ========================================================
    // PHYSICAL COORDINATES
    // ========================================================

    int16_t physicalX =
        x + X_OFFSET;

    int16_t physicalY =
        y + Y_OFFSET;

    // ========================================================
    // SPI
    // ========================================================

    tft.startWrite();

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

    uint32_t pixelCount =
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

    tft.endWrite();

    // ========================================================
    // FLUSH READY
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

    _lastUpdate =
        millis();
}

// ============================================================
// DISPLAY
// ============================================================

lv_display_t*
LVGLManager::display(
    uint8_t index
)
{
    if (
        index >= DISPLAY_COUNT
    )
    {
        return nullptr;
    }

    return _contexts[index].lvDisplay;
}

// ============================================================
// READY
// ============================================================

bool LVGLManager::isReady() const
{
    return _initialized;
}

// ============================================================
// TEST DISPLAYS
// ============================================================

bool LVGLManager::testDisplays()
{
    if (!_initialized)
        return false;

    for (
        uint8_t i = 0;
        i < DISPLAY_COUNT;
        i++
    )
    {
        if (
            _contexts[i].tft ==
            nullptr
        )
        {
            return false;
        }

        _contexts[i].tft->fillScreen(
            ST77XX_RED
        );
    }

    return true;
}