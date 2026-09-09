#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

class ST7789_172x320 : public Adafruit_ST7789
{
public:
    ST7789_172x320(
        int8_t cs,
        int8_t dc,
        int8_t rst
    );

    void begin172x320();

    void select();

    void deselect();
};


class Display
{
public:

    Display();


    // --------------------------------------------------------
    // Инициализация
    // --------------------------------------------------------

    // Запускает все четыре экрана. Возвращает true при успехе.
    bool begin();

    bool isInitialized() const;


    // --------------------------------------------------------
    // Доступ к физическому дисплею
    // --------------------------------------------------------

    // Возвращает экран по номеру: 0, 1, 2 или 3.
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

    // Включает или выключает общую подсветку экранов.
    void setBacklight(bool state);

    bool backlightState() const;


    // --------------------------------------------------------
    // SPI transaction
    // --------------------------------------------------------

    void beginTransaction();

    void endTransaction();


private:

    ST7789_172x320* _display[4];

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


// Если в остальных файлах проекта используется старое имя:
using DisplayManager = Display;
