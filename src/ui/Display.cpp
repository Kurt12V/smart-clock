#include "Display.h"
#include "Pins.h"

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>


// ============================================================
// DISPLAY PINS
// ============================================================

// У каждого экрана отдельный выбор устройства и сброс.
static const int8_t TFT_CS[DISPLAY_COUNT] =
{
    PIN_TFT_CS1,
    PIN_TFT_CS2,
    PIN_TFT_CS3,
    PIN_TFT_CS4
};

static const int8_t TFT_RST[DISPLAY_COUNT] =
{
    PIN_TFT_RST1,
    PIN_TFT_RST2,
    PIN_TFT_RST3,
    PIN_TFT_RST4
};


// ============================================================
// ST7789 172x320
// ============================================================

ST7789_172x320::ST7789_172x320(
    int8_t cs,
    int8_t dc,
    int8_t rst
)
    : Adafruit_ST7789(
        cs,
        dc,
        rst
    )
{
}


// ============================================================
// DISPLAY INITIALIZATION
// ============================================================

void ST7789_172x320::begin172x320()
{
    /*
     * 40 МГц подходит для коротких качественных проводов.
     * Если на экране появятся полосы, мерцание или искажённые
     * символы, замените 40000000 на 20000000.
     */
    setSPISpeed(40000000);

    /*
     * Реальный размер видимой области данной панели:
     * 172 x 320 пикселей.
     */
    init(
        172,
        320,
        SPI_MODE0
    );

    /*
     * Для большинства ST7789 172x320 нужен сдвиг области
     * изображения на 34 пикселя по горизонтали.
     */
    setColRowStart(
        34,
        0
    );

    // Вертикальная ориентация: 172 пикселя в ширину, 320 в высоту.
    setRotation(0);

    /*
     * У многих панелей ST7789 цвета выглядят правильно только
     * с инверсией. Если чёрный фон выглядит зелёным или цвета
     * выглядят негативом, замените true на false.
     */
    invertDisplay(true);

    fillScreen(ST77XX_BLACK);
}


// ============================================================
// CONSTRUCTOR
// ============================================================

Display::Display()
    : _initialized(false)
{
    for (uint8_t i = 0; i < DISPLAY_COUNT; i++)
    {
        _display[i] = nullptr;
    }
}


// ============================================================
// DESTRUCTOR
// ============================================================

Display::~Display()
{
    for (uint8_t i = 0; i < DISPLAY_COUNT; i++)
    {
        delete _display[i];
        _display[i] = nullptr;
    }
}


// ============================================================
// BEGIN
// ============================================================

bool Display::begin()
{
    if (_initialized)
    {
        return true;
    }


    // --------------------------------------------------------
    // BACKLIGHT
    // --------------------------------------------------------

    pinMode(PIN_TFT_BL, OUTPUT);

    /*
     * Обычный вариант: HIGH включает подсветку.
     * Если дисплеи светятся при LOW, поменяйте HIGH и LOW
     * в этой строке и в setBacklight().
     */
    digitalWrite(PIN_TFT_BL, HIGH);


    // --------------------------------------------------------
    // SHARED SPI BUS
    // --------------------------------------------------------
    //
    // SCLK — общий тактовый провод.
    // MOSI — общий провод данных от платы к дисплеям.
    // DC   — общий провод выбора «команда или данные».
    //
    // CS и RST — отдельные для каждого дисплея.
    //
    // --------------------------------------------------------

    SPI.begin(
        PIN_TFT_SCLK,
        -1,             // MISO не нужен: дисплеи только получают данные.
        PIN_TFT_MOSI,
        -1
    );

    pinMode(PIN_TFT_DC, OUTPUT);
    digitalWrite(PIN_TFT_DC, LOW);


    // --------------------------------------------------------
    // PREPARE ALL CHIP-SELECT AND RESET PINS
    // --------------------------------------------------------

    for (uint8_t i = 0; i < DISPLAY_COUNT; i++)
    {
        pinMode(TFT_CS[i], OUTPUT);
        digitalWrite(TFT_CS[i], HIGH);

        pinMode(TFT_RST[i], OUTPUT);
        digitalWrite(TFT_RST[i], HIGH);
    }


    // --------------------------------------------------------
    // INITIALIZE DISPLAYS ONE AT A TIME
    // --------------------------------------------------------

    for (uint8_t i = 0; i < DISPLAY_COUNT; i++)
    {
        _display[i] = new ST7789_172x320(
            TFT_CS[i],
            PIN_TFT_DC,
            TFT_RST[i]
        );

        if (_display[i] == nullptr)
        {
            return false;
        }

        _display[i]->begin172x320();
        _display[i]->fillScreen(ST77XX_BLACK);
    }


    _initialized = true;

    return true;
}


// ============================================================
// GET DISPLAY
// ============================================================

ST7789_172x320& Display::get(uint8_t index)
{
    // Защита от случайного номера вне диапазона 0–3.
    if (index >= DISPLAY_COUNT)
    {
        index = 0;
    }

    return *_display[index];
}


// ============================================================
// BACKLIGHT
// ============================================================

void Display::setBacklight(bool state)
{
    digitalWrite(
        PIN_TFT_BL,
        state ? HIGH : LOW
    );
}
