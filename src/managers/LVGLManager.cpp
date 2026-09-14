#include "LVGLManager.h"

#include "Pins.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

LVGLManager::LVGLManager()
    : _tft{
        Adafruit_ST7789(
            PIN_TFT_CS1,
            PIN_TFT_DC,
            PIN_TFT_RST1
        ),

        Adafruit_ST7789(
            PIN_TFT_CS2,
            PIN_TFT_DC,
            PIN_TFT_RST2
        ),

        Adafruit_ST7789(
            PIN_TFT_CS3,
            PIN_TFT_DC,
            PIN_TFT_RST3
        ),

        Adafruit_ST7789(
            PIN_TFT_CS4,
            PIN_TFT_DC,
            PIN_TFT_RST4
        )
    },

      _panels{
        LVGLTFTPanel(
            _tft[0],
            34,
            0
        ),

        LVGLTFTPanel(
            _tft[1],
            34,
            0
        ),

        LVGLTFTPanel(
            _tft[2],
            34,
            0
        ),

        LVGLTFTPanel(
            _tft[3],
            34,
            0
        )
    },

    _initialized(false)
{
}

// ============================================================
// BEGIN
// ============================================================

bool LVGLManager::begin()
{
    if (_initialized)
        return true;

    Serial.println(
        "LVGL: initializing..."
    );

    // --------------------------------------------------------
    // SPI
    // --------------------------------------------------------

    SPI.begin(
        PIN_SCLK,
        -1,
        PIN_MOSI,
        -1
    );

    // --------------------------------------------------------
    // LVGL
    // --------------------------------------------------------

    lv_init();

    // --------------------------------------------------------
    // FOUR TFT
    // --------------------------------------------------------

    for (uint8_t i = 0;
         i < DISPLAY_COUNT;
         ++i)
    {
        Serial.print(
            "LVGL: TFT "
        );

        Serial.print(i);

        Serial.println(
            " init..."
        );

        if (!_panels[i].begin())
        {
            Serial.print(
                "LVGL: TFT "
            );

            Serial.print(i);

            Serial.println(
                " ERROR"
            );

            return false;
        }
    }

    _initialized = true;

    Serial.println(
        "LVGL: initialized"
    );

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void LVGLManager::update()
{
    if (!_initialized)
        return;

    lv_timer_handler();
}

// ============================================================
// DISPLAY
// ============================================================

lv_display_t*
LVGLManager::display(
    uint8_t index
) const
{
    if (index >= DISPLAY_COUNT)
        return nullptr;

    return _panels[index].display();
}

// ============================================================
// READY
// ============================================================

bool LVGLManager::isReady() const
{
    return _initialized;
}
