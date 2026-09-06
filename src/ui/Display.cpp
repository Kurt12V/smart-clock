#include "Display.h"
#include "Pins.h"

#include <SPI.h>


// ============================================================
// Пины 4 дисплеев
// ============================================================

static const int8_t TFT_CS[4] =
{
    PIN_TFT_CS1,
    PIN_TFT_CS2,
    PIN_TFT_CS3,
    PIN_TFT_CS4
};

static const int8_t TFT_RST[4] =
{
    PIN_TFT_RST1,
    PIN_TFT_RST2,
    PIN_TFT_RST3,
    PIN_TFT_RST4
};


// ============================================================
// ST7789_172x320
// ============================================================

ST7789_172x320::ST7789_172x320(
    int8_t cs,
    int8_t dc,
    int8_t rst,
    int8_t mosi,
    int8_t sclk
)
    : Adafruit_ST7789(
        cs,
        dc,
        mosi,
        sclk,
        rst
    )
{
}


void ST7789_172x320::begin172x320()
{
    init(
        172,
        320,
        SPI_MODE0
    );

    setColRowStart(
        34,
        0
    );

    setSPISpeed(
        8000000
    );

    setRotation(0);

    invertDisplay(true);
}


// ============================================================
// Display
// ============================================================

Display::Display()
    : _initialized(false)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        _display[i] = nullptr;
    }
}


Display::~Display()
{
    for (uint8_t i = 0; i < 4; i++)
    {
        delete _display[i];
        _display[i] = nullptr;
    }
}


// ============================================================
// Инициализация
// ============================================================

bool Display::begin()
{
    // Подсветка
    pinMode(
        PIN_TFT_BL,
        OUTPUT
    );

    digitalWrite(
        PIN_TFT_BL,
        HIGH
    );


    // Инициализация 4 дисплеев
    for (uint8_t i = 0; i < 4; i++)
    {
        _display[i] = new ST7789_172x320(
            TFT_CS[i],
            PIN_TFT_DC,
            TFT_RST[i],
            PIN_TFT_MOSI,
            PIN_TFT_SCLK
        );

        delay(50);

        _display[i]->begin172x320();

        delay(50);

        _display[i]->fillScreen(
            ST77XX_BLACK
        );

        delay(30);
    }


    _initialized = true;

    return true;
}


// ============================================================
// Получить дисплей
// ============================================================

ST7789_172x320& Display::get(
    uint8_t index
)
{
    if (index >= 4)
    {
        index = 0;
    }

    return *_display[index];
}


// ============================================================
// Подсветка
// ============================================================

void Display::setBacklight(
    bool state
)
{
    digitalWrite(
        PIN_TFT_BL,
        state ? HIGH : LOW
    );
}