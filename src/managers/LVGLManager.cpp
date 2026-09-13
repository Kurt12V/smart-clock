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
    }
}


// ============================================================
// BEGIN
// ============================================================

bool LVGLManager::begin()
{
    if (_initialized)
        return true;

    Serial0.println();
    Serial0.println("==============================");
    Serial0.println("          LVGL");
    Serial0.println("==============================");

    lv_init();

    // LVGL itself reads millis().
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
    pinMode(PIN_TFT_CS1, OUTPUT);
    pinMode(PIN_TFT_CS2, OUTPUT);
    pinMode(PIN_TFT_CS3, OUTPUT);
    pinMode(PIN_TFT_CS4, OUTPUT);

    digitalWrite(PIN_TFT_CS1, HIGH);
    digitalWrite(PIN_TFT_CS2, HIGH);
    digitalWrite(PIN_TFT_CS3, HIGH);
    digitalWrite(PIN_TFT_CS4, HIGH);
    Serial0.println("[LVGL] SPI initialized");

    // --------------------------------------------------------
    // Backlight
    // --------------------------------------------------------

    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, HIGH);

    // --------------------------------------------------------
    // Displays
    // --------------------------------------------------------

    for (uint8_t i = 0; i < DISPLAY_COUNT; ++i)
    {
        Serial0.print("[LVGL] Display ");
        Serial0.print(i + 1);
        Serial0.println(" init...");

        if (!initDisplay(i))
        {
            Serial0.print("[LVGL] Display ");
            Serial0.print(i + 1);
            Serial0.println(" FAILED");

            return false;
        }

        Serial0.print("[LVGL] Display ");
        Serial0.print(i + 1);
        Serial0.println(" OK");
    }
    // _tft1.fillScreen(ST77XX_RED);
    // delay(300);

    // _tft2.fillScreen(ST77XX_GREEN);
    // delay(300);

    // _tft3.fillScreen(ST77XX_BLUE);
    // delay(300);

    // _tft4.fillScreen(ST77XX_WHITE);
    // delay(1000);
    _lastTick = millis();

    _initialized = true;

    Serial0.println("[LVGL] All displays initialized");

    return true;
}


// ============================================================
// INIT DISPLAY
// ============================================================

bool LVGLManager::initDisplay(uint8_t index)
{
    if (index >= DISPLAY_COUNT)
        return false;

    DisplayContext& context = _contexts[index];

    if (context.tft == nullptr)
        return false;

    Adafruit_ST7789& tft = *context.tft;

    // --------------------------------------------------------
    // IMPORTANT:
    //
    // The physical display is 172x320.
    //
    // Do NOT initialize it as 240x320.
    // --------------------------------------------------------

    tft.init(
        WIDTH,
        HEIGHT
    );

    tft.setRotation(0);

    tft.setSPISpeed(40000000);

    tft.fillScreen(ST77XX_BLACK);

    // --------------------------------------------------------
    // Create LVGL display
    // --------------------------------------------------------

    lv_display_t* display =
        lv_display_create(
            WIDTH,
            HEIGHT
        );

    if (display == nullptr)
        return false;

    context.lvDisplay = display;

    // --------------------------------------------------------
    // User data
    // --------------------------------------------------------

    lv_display_set_user_data(
        display,
        &context
    );

    // --------------------------------------------------------
    // Partial render buffer
    // --------------------------------------------------------

    lv_display_set_buffers(
        display,

        context.buffer,
        nullptr,

        sizeof(_buffers[index]),

        LV_DISPLAY_RENDER_MODE_PARTIAL
    );

    // --------------------------------------------------------
    // Flush callback
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
    uint8_t* px_map
)
{
    Serial0.printf(
        "[FLUSH] display=%p area=(%d,%d)-(%d,%d)\n",
        display,
        area->x1,
        area->y1,
        area->x2,
        area->y2
    );

    DisplayContext* context = nullptr;

    for (uint8_t i = 0; i < DISPLAY_COUNT; ++i)
    {
        if (_contexts[i].lvDisplay == display)
        {
            context = &_contexts[i];
            break;
        }
    }

    if (context == nullptr)
    {
        Serial0.println("[FLUSH] ERROR: context not found");

        lv_display_flush_ready(display);
        return;
    }

    Adafruit_ST7789& tft = context->tft;

    const int16_t x = area->x1;
    const int16_t y = area->y1;

    const uint16_t width =
        area->x2 - area->x1 + 1;

    const uint16_t height =
        area->y2 - area->y1 + 1;

    Serial0.printf(
        "[FLUSH] x=%d y=%d w=%d h=%d\n",
        x,
        y,
        width,
        height
    );

    tft.startWrite();

    tft.setAddrWindow(
        x,
        y,
        width,
        height
    );

    uint16_t* pixels =
        reinterpret_cast<uint16_t*>(px_map);

    const uint32_t count =
        static_cast<uint32_t>(width) *
        static_cast<uint32_t>(height);

    for (uint32_t i = 0; i < count; ++i)
    {
        tft.pushColor(pixels[i]);
    }

    tft.endWrite();

    lv_display_flush_ready(display);
}


// ============================================================
// UPDATE
// ============================================================

void LVGLManager::update()
{
    if (!_initialized)
        return;

    /*
     * Do NOT call lv_tick_inc() here.
     *
     * We already configured:
     *
     * lv_tick_set_cb([]() { return millis(); });
     *
     * Therefore LVGL gets time directly from millis().
     */

    lv_timer_handler();
}


// ============================================================
// GET DISPLAY
// ============================================================

lv_display_t* LVGLManager::display(uint8_t index)
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