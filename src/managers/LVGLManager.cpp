#include "LVGLManager.h"

#include <SPI.h>


// ============================================================
// CONSTRUCTOR
// ============================================================

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
    // --------------------------------------------------------
    // TFT -> CONTEXT
    // --------------------------------------------------------

    _contexts[0].index = 0;
    _contexts[0].tft   = &_tft1;

    _contexts[1].index = 1;
    _contexts[1].tft   = &_tft2;

    _contexts[2].index = 2;
    _contexts[2].tft   = &_tft3;

    _contexts[3].index = 3;
    _contexts[3].tft   = &_tft4;


    // --------------------------------------------------------
    // BUFFER -> CONTEXT
    // --------------------------------------------------------

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


    // --------------------------------------------------------
    // LVGL
    // --------------------------------------------------------

    lv_init();

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
    // CS
    // --------------------------------------------------------

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
    // BACKLIGHT
    // --------------------------------------------------------

    pinMode(PIN_TFT_BL, OUTPUT);

    digitalWrite(
        PIN_TFT_BL,
        HIGH
    );


    // --------------------------------------------------------
    // INITIALIZE DISPLAYS
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


    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    _lastTick = millis();

    _initialized = true;


    Serial0.println("[LVGL] All displays initialized");

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


    if (context.tft == nullptr)
        return false;


    Adafruit_ST7789& tft =
        *context.tft;


    // --------------------------------------------------------
    // TFT INITIALIZATION
    // --------------------------------------------------------

    tft.init(
        WIDTH,
        HEIGHT
    );


    /*
        Все дисплеи пока имеют одинаковую ориентацию.

        НЕ ставим rotation(2) для TFT2.

        Так как проблема наблюдается только
        в top/bottom bar, физическую ориентацию
        дисплея сейчас не меняем.
    */

    tft.setRotation(0);


    // --------------------------------------------------------
    // SPI SPEED
    // --------------------------------------------------------

    tft.setSPISpeed(
        40000000
    );


    // --------------------------------------------------------
    // CLEAR DISPLAY
    // --------------------------------------------------------

    tft.fillScreen(
        ST77XX_BLACK
    );


    // --------------------------------------------------------
    // CREATE LVGL DISPLAY
    // --------------------------------------------------------

    lv_display_t* display =
        lv_display_create(
            WIDTH,
            HEIGHT
        );


    if (display == nullptr)
    {
        Serial0.print(
            "[LVGL] Failed to create display "
        );

        Serial0.println(
            index + 1
        );

        return false;
    }


    context.lvDisplay =
        display;


    // --------------------------------------------------------
    // USER DATA
    // --------------------------------------------------------

    lv_display_set_user_data(
        display,
        &context
    );


    // --------------------------------------------------------
    // LVGL BUFFER
    // --------------------------------------------------------

    lv_display_set_buffers(
        display,

        context.buffer,

        nullptr,

        BUFFER_SIZE * sizeof(lv_color_t),

        LV_DISPLAY_RENDER_MODE_PARTIAL
    );


    // --------------------------------------------------------
    // FLUSH CALLBACK
    // --------------------------------------------------------

    lv_display_set_flush_cb(
        display,
        LVGLManager::flushCallback
    );


    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    context.initialized = true;


    Serial0.print(
        "[LVGL] Context "
    );

    Serial0.print(
        index + 1
    );

    Serial0.print(
        " buffer = 0x"
    );

    Serial0.println(
        reinterpret_cast<uintptr_t>(
            context.buffer
        ),
        HEX
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
    // --------------------------------------------------------
    // CHECK DISPLAY
    // --------------------------------------------------------

    if (display == nullptr)
    {
        Serial0.println(
            "[FLUSH] ERROR: display == nullptr"
        );

        return;
    }


    // --------------------------------------------------------
    // GET CONTEXT
    // --------------------------------------------------------

    DisplayContext* context =
        static_cast<DisplayContext*>(
            lv_display_get_user_data(
                display
            )
        );


    if (
        context == nullptr ||
        context->tft == nullptr
    )
    {
        Serial0.println(
            "[FLUSH] ERROR: context not found"
        );

        lv_display_flush_ready(
            display
        );

        return;
    }


    if (area == nullptr)
    {
        Serial0.println(
            "[FLUSH] ERROR: area == nullptr"
        );

        lv_display_flush_ready(
            display
        );

        return;
    }


    if (pxMap == nullptr)
    {
        Serial0.println(
            "[FLUSH] ERROR: pxMap == nullptr"
        );

        lv_display_flush_ready(
            display
        );

        return;
    }


    // --------------------------------------------------------
    // TFT
    // --------------------------------------------------------

    Adafruit_ST7789& tft =
        *context->tft;


    // --------------------------------------------------------
    // AREA
    // --------------------------------------------------------

    const int16_t x =
        area->x1;

    const int16_t y =
        area->y1;


    const uint16_t width =
        static_cast<uint16_t>(
            area->x2 -
            area->x1 +
            1
        );


    const uint16_t height =
        static_cast<uint16_t>(
            area->y2 -
            area->y1 +
            1
        );


    const uint32_t pixelCount =
        static_cast<uint32_t>(width) *
        static_cast<uint32_t>(height);


    // --------------------------------------------------------
    // DEBUG
    // --------------------------------------------------------

    /*
        Пока оставляем диагностику.

        Она позволит увидеть:

        TFT1 -> какие области
        TFT2 -> какие области
        TFT3 -> какие области
        TFT4 -> какие области
    */

    Serial0.printf(
        "[FLUSH] TFT%u "
        "x=%d y=%d "
        "w=%u h=%u "
        "pixels=%lu\n",

        context->index + 1,

        x,
        y,

        width,
        height,

        static_cast<unsigned long>(
            pixelCount
        )
    );


    // --------------------------------------------------------
    // VALIDATE AREA
    // --------------------------------------------------------

    if (
        x < 0 ||
        y < 0 ||
        x >= WIDTH ||
        y >= HEIGHT
    )
    {
        Serial0.println(
            "[FLUSH] ERROR: invalid start"
        );

        lv_display_flush_ready(
            display
        );

        return;
    }


    if (
        area->x2 >= WIDTH ||
        area->y2 >= HEIGHT
    )
    {
        Serial0.println(
            "[FLUSH] ERROR: invalid end"
        );

        lv_display_flush_ready(
            display
        );

        return;
    }


    // --------------------------------------------------------
    // SEND PIXELS TO TFT
    // --------------------------------------------------------

    tft.startWrite();


    tft.setAddrWindow(
        x,
        y,
        width,
        height
    );


    uint16_t* pixels =
        reinterpret_cast<uint16_t*>(
            pxMap
        );


    tft.writePixels(
        pixels,
        pixelCount
    );


    tft.endWrite();


    // --------------------------------------------------------
    // TELL LVGL THAT FLUSH IS COMPLETE
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


    /*
        lv_tick_set_cb(millis) используется
        вместо lv_tick_inc().
    */

    lv_timer_handler();
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
