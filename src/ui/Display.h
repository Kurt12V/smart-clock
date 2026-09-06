#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>


// ============================================================
// ST7789 172x320
// ============================================================

class ST7789_172x320 : public Adafruit_ST7789
{
public:

    ST7789_172x320(
        int8_t cs,
        int8_t dc,
        int8_t rst,
        int8_t mosi,
        int8_t sclk
    );

    void begin172x320();

    void select();

    void deselect();
};

class Display
{
public:

    static constexpr uint8_t COUNT = 4;

    static constexpr int16_t WIDTH  = 172;
    static constexpr int16_t HEIGHT = 320;

    static constexpr uint32_t SPI_FREQUENCY = 8000000UL;


    Display();


    // --------------------------------------------------------
    // Инициализация
    // --------------------------------------------------------

    bool begin();

    bool isInitialized() const;


    // --------------------------------------------------------
    // Доступ к физическому дисплею
    // --------------------------------------------------------

    ST7789_172x320& get(uint8_t index);

    ST7789_172x320* getPtr(uint8_t index);


    // --------------------------------------------------------
    // CS
    // --------------------------------------------------------

    void select(uint8_t index);

    void deselectAll();


    // --------------------------------------------------------
    // Очистка
    // --------------------------------------------------------

    void clear(
        uint16_t color = ST77XX_BLACK
    );

    void clear(
        uint8_t index,
        uint16_t color = ST77XX_BLACK
    );


    // --------------------------------------------------------
    // Backlight
    // --------------------------------------------------------

    void setBacklight(bool state);

    bool backlightState() const;


    // --------------------------------------------------------
    // SPI transaction
    // --------------------------------------------------------

    void beginTransaction();

    void endTransaction();


private:

    ST7789_172x320 _display[COUNT];

    bool _initialized;
    bool _backlight;


    // --------------------------------------------------------
    // Внутренние функции
    // --------------------------------------------------------

    void setupPins();

    void disableAllDisplays();

    bool initializeDisplay(
        uint8_t index
    );
};