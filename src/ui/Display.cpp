#include "Display.h"
#include "Pins.h"

#include <Arduino.h>
#include <SPI.h>


// ============================================================
// Массивы GPIO
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
// Инициализация одного дисплея
// ============================================================

void ST7789_172x320::begin172x320()
{
    // Инициализация физического дисплея
    init(
        172,
        320,
        SPI_MODE0
    );

    // Для конкретного ST7789 172x320
    // требуется смещение по X.
    setColRowStart(
        34,
        0
    );

    // Безопасная скорость SPI
    setSPISpeed(
        8000000
    );

    // Ориентация
    setRotation(
        0
    );

    // Инверсия цветов
    invertDisplay(
        true
    );
}


// ============================================================
// Display constructor
// ============================================================

Display::Display()
    :
      _display
      {
          ST7789_172x320(
              PIN_TFT_CS1,
              PIN_TFT_DC,
              PIN_TFT_RST1,
              PIN_TFT_MOSI,
              PIN_TFT_SCLK
          ),

          ST7789_172x320(
              PIN_TFT_CS2,
              PIN_TFT_DC,
              PIN_TFT_RST2,
              PIN_TFT_MOSI,
              PIN_TFT_SCLK
          ),

          ST7789_172x320(
              PIN_TFT_CS3,
              PIN_TFT_DC,
              PIN_TFT_RST3,
              PIN_TFT_MOSI,
              PIN_TFT_SCLK
          ),

          ST7789_172x320(
              PIN_TFT_CS4,
              PIN_TFT_DC,
              PIN_TFT_RST4,
              PIN_TFT_MOSI,
              PIN_TFT_SCLK
          )
      },

      _initialized(false),
      _backlight(false)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        pinMode(
            TFT_CS[i],
            OUTPUT
        );

        /*
         * Все дисплеи выключены.
         */
        digitalWrite(
            TFT_CS[i],
            HIGH
        );
    }


    /*
     * --------------------------------------------------------
     * RST
     * --------------------------------------------------------
     */

    for (uint8_t i = 0; i < COUNT; i++)
    {
        pinMode(
            TFT_RST[i],
            OUTPUT
        );

        /*
         * Не держим дисплей в reset.
         */
        digitalWrite(
            TFT_RST[i],
            HIGH
        );
    }
}


// ============================================================
// Выключить все дисплеи
// ============================================================

void Display::disableAllDisplays()
{
    for (uint8_t i = 0; i < 4; i++)
    {
        if (_display[i] != nullptr)
        {
            delete _display[i];
            _display[i] = nullptr;
        }
    }
}


// ============================================================
// Инициализация одного дисплея
// ============================================================

bool Display::initializeDisplay(
    uint8_t index
)
{
    if (index >= COUNT)
    {
        return false;
    }


    /*
     * Очень важно:
     *
     * перед инициализацией конкретного
     * дисплея все остальные CS должны
     * быть HIGH.
     */

    disableAllDisplays();


    /*
     * Выбираем только этот дисплей.
     */

    digitalWrite(
        TFT_CS[index],
        LOW
    );


    /*
     * Инициализация ST7789.
     */

    _display[index].begin172x320();


    /*
     * После init снова снимаем CS.
     */

    digitalWrite(
        TFT_CS[index],
        HIGH
    );


    return true;
}


// ============================================================
// begin()
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

    pinMode(
        PIN_TFT_BL,
        OUTPUT
    );

    digitalWrite(
        PIN_TFT_BL,
        HIGH
    );


    // --------------------------------------------------------
    // SPI
    // --------------------------------------------------------
    //
    // Все четыре дисплея используют одну SPI-шину.
    //
    // SCLK  -> общий
    // MOSI  -> общий
    // DC    -> общий
    //
    // CS    -> отдельный
    // RST   -> отдельный
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
    // INITIALIZE FOUR DISPLAYS
    // --------------------------------------------------------

    for (uint8_t i = 0; i < 4; i++)
    {
        // Создаем объект дисплея
        _display[i] =
            new ST7789_172x320(
                TFT_CS[i],
                PIN_TFT_DC,
                TFT_RST[i],
                PIN_TFT_MOSI,
                PIN_TFT_SCLK
            );


        // ----------------------------------------------------
        // CS
        // ----------------------------------------------------

        pinMode(
            TFT_CS[i],
            OUTPUT
        );

        digitalWrite(
            TFT_CS[i],
            HIGH
        );


        // ----------------------------------------------------
        // RST
        // ----------------------------------------------------

        pinMode(
            TFT_RST[i],
            OUTPUT
        );

        digitalWrite(
            TFT_RST[i],
            HIGH
        );


        // ----------------------------------------------------
        // INITIALIZE ST7789
        // ----------------------------------------------------

        _display[i]->begin172x320();


        // ----------------------------------------------------
        // CLEAR DISPLAY
        // ----------------------------------------------------

        _display[i]->fillScreen(
            ST77XX_BLACK
        );
    }


    // --------------------------------------------------------
    // FINISHED
    // --------------------------------------------------------

    _initialized = true;

    return true;
}


// ============================================================
// Проверка инициализации
// ============================================================

bool Display::isInitialized() const
{
    return _initialized;
}


// ============================================================
// Получить дисплей
// ============================================================

ST7789_172x320& Display::get(uint8_t index)
{
    if (index >= 4)
    {
        index = 0;
    }

    return _display[index];
}


// ============================================================
// Получить указатель
// ============================================================

ST7789_172x320* Display::getPtr(
    uint8_t index
)
{
    if (index >= COUNT)
    {
        return nullptr;
    }

    return &_display[index];
}


// ============================================================
// Выбрать дисплей
// ============================================================

void Display::select(
    uint8_t index
)
{
    if (index >= COUNT)
    {
        return;
    }


    /*
     * Сначала выключаем все.
     */
    disableAllDisplays();


    /*
     * Затем выбираем только один.
     */
    digitalWrite(
        TFT_CS[index],
        LOW
    );
}


// ============================================================
// Снять выбор со всех
// ============================================================

void Display::deselectAll()
{
    disableAllDisplays();
}


// ============================================================
// Очистить все дисплеи
// ============================================================

void Display::clear(
    uint16_t color
)
{
    for (uint8_t i = 0; i < COUNT; i++)
    {
        select(i);

        _display[i].fillScreen(
            color
        );
    }

    deselectAll();
}


// ============================================================
// Очистить один дисплей
// ============================================================

void Display::clear(
    uint8_t index,
    uint16_t color
)
{
    if (index >= COUNT)
    {
        return;
    }

    select(index);

    _display[index].fillScreen(
        color
    );

    digitalWrite(
        TFT_CS[index],
        HIGH
    );
}


// ============================================================
// Backlight
// ============================================================

void Display::setBacklight(bool state)
{
    digitalWrite(
        PIN_TFT_BL,
        state ? HIGH : LOW
    );
}

