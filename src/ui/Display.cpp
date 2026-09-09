#include "Display.h"
#include "Pins.h"

static constexpr int8_t TFT_CS[Display::DISPLAY_COUNT] =
{
    PIN_TFT_CS1,
    PIN_TFT_CS2,
    PIN_TFT_CS3,
    PIN_TFT_CS4
};

static constexpr int8_t TFT_RST[Display::DISPLAY_COUNT] =
{
    PIN_TFT_RST1,
    PIN_TFT_RST2,
    PIN_TFT_RST3,
    PIN_TFT_RST4
};

ST7789_172x320::ST7789_172x320(
    int8_t cs,
    int8_t dc,
    int8_t rst
)
    : Adafruit_ST7789(cs, dc, rst)
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

void ST7789_172x320::select()
{
    digitalWrite(
        _cs,
        LOW
    );
}

void ST7789_172x320::deselect()
{
    digitalWrite(
        _cs,
        HIGH
    );
}

Display::Display()
    : _display
      {
          ST7789_172x320(
              PIN_TFT_CS1,
              PIN_TFT_DC,
              PIN_TFT_RST1
          ),
          ST7789_172x320(
              PIN_TFT_CS2,
              PIN_TFT_DC,
              PIN_TFT_RST2
          ),
          ST7789_172x320(
              PIN_TFT_CS3,
              PIN_TFT_DC,
              PIN_TFT_RST3
          ),
          ST7789_172x320(
              PIN_TFT_CS4,
              PIN_TFT_DC,
              PIN_TFT_RST4
          )
      },
      _initialized(false),
      _backlight(false)
{
    setupPins();
}

void Display::setupPins()
{
    pinMode(
        PIN_TFT_DC,
        OUTPUT
    );

    digitalWrite(
        PIN_TFT_DC,
        LOW
    );

    pinMode(
        PIN_TFT_BL,
        OUTPUT
    );

    digitalWrite(
        PIN_TFT_BL,
        LOW
    );

    for (uint8_t i = 0; i < DISPLAY_COUNT; ++i)
    {
        pinMode(
            TFT_CS[i],
            OUTPUT
        );

        digitalWrite(
            TFT_CS[i],
            HIGH
        );

        pinMode(
            TFT_RST[i],
            OUTPUT
        );

        digitalWrite(
            TFT_RST[i],
            HIGH
        );
    }
}

bool Display::initializeDisplay(
    uint8_t index
)
{
    if (index >= DISPLAY_COUNT)
    {
        return false;
    }

    deselectAll();

    digitalWrite(
        TFT_RST[index],
        LOW
    );

    delay(10);

    digitalWrite(
        TFT_RST[index],
        HIGH
    );

    delay(10);

    select(index);

    _display[index].begin172x320();

    deselectAll();

    return true;
}

bool Display::begin()
{
    if (_initialized)
    {
        return true;
    }

    SPI.begin(
        PIN_TFT_SCLK,
        -1,
        PIN_TFT_MOSI,
        -1
    );

    setupPins();

    bool success = true;

    for (uint8_t i = 0; i < DISPLAY_COUNT; ++i)
    {
        if (!initializeDisplay(i))
        {
            success = false;
            break;
        }
    }

    if (!success)
    {
        deselectAll();
        setBacklight(false);
        return false;
    }

    clear(ST77XX_BLACK);

    setBacklight(true);

    _initialized = true;

    return true;
}

bool Display::isInitialized() const
{
    return _initialized;
}

ST7789_172x320& Display::get(
    uint8_t index
)
{
    if (index >= DISPLAY_COUNT)
    {
        index = 0;
    }

    return _display[index];
}

ST7789_172x320* Display::getPtr(
    uint8_t index
)
{
    if (index >= DISPLAY_COUNT)
    {
        return nullptr;
    }

    return &_display[index];
}

void Display::select(
    uint8_t index
)
{
    if (index >= DISPLAY_COUNT)
    {
        return;
    }

    deselectAll();

    digitalWrite(
        TFT_CS[index],
        LOW
    );
}

void Display::deselectAll()
{
    for (uint8_t i = 0; i < DISPLAY_COUNT; ++i)
    {
        digitalWrite(
            TFT_CS[i],
            HIGH
        );
    }
}

void Display::clear(
    uint16_t color
)
{
    for (uint8_t i = 0; i < DISPLAY_COUNT; ++i)
    {
        select(i);

        _display[i].fillScreen(color);

        deselectAll();
    }
}

void Display::clear(
    uint8_t index,
    uint16_t color
)
{
    if (index >= DISPLAY_COUNT)
    {
        return;
    }

    select(index);

    _display[index].fillScreen(color);

    deselectAll();
}

void Display::setBacklight(
    bool state
)
{
    _backlight = state;

    digitalWrite(
        PIN_TFT_BL,
        state ? HIGH : LOW
    );
}

bool Display::backlightState() const
{
    return _backlight;
}

void Display::beginTransaction()
{
    SPI.beginTransaction(
        SPISettings(
            8000000,
            MSBFIRST,
            SPI_MODE0
        )
    );
}

void Display::endTransaction()
{
    deselectAll();

    SPI.endTransaction();
}