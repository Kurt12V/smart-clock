#include "SplashScreen.h"

#include <Arduino.h>

SplashScreen::SplashScreen(DisplayManager& display)
    : _display(display),
      _startTime(0)
{
}

void SplashScreen::show()
{
    if (!_display.isInitialized())
        return;

    randomSeed(
        micros() ^
        analogRead(0) ^
        millis()
    );

    _display.clear(ST77XX_BLACK);

    initializeColumns();

    _startTime = millis();

    uint32_t lastFrame = 0;

    while (millis() - _startTime < DURATION)
    {
        uint32_t now = millis();

        if (now - lastFrame >= FRAME_TIME)
        {
            lastFrame = now;

            drawFrame();
        }

        delay(1);
    }

    finalFlash();

    _display.clear(ST77XX_BLACK);
}


// ============================================================
// ИНИЦИАЛИЗАЦИЯ КОЛОНОК
// ============================================================

void SplashScreen::initializeColumns()
{
    for (uint16_t i = 0; i < COLUMNS; i++)
    {
        Column& column = _columns[i];

        column.active = random(0, 100) < 65;

        column.head = random(
            -ROWS,
            ROWS
        );

        column.length = random(
            5,
            18
        );

        column.speed = random(
            1,
            4
        );

        column.counter = random(
            0,
            column.speed
        );
    }
}


// ============================================================
// КАДР MATRIX
// ============================================================

void SplashScreen::drawFrame()
{
    for (uint16_t columnIndex = 0;
         columnIndex < COLUMNS;
         columnIndex++)
    {
        Column& column = _columns[columnIndex];

        if (!column.active)
        {
            // Иногда запускаем новую колонку
            if (random(0, 100) < 3)
            {
                column.active = true;

                column.head = -random(1, 15);

                column.length = random(
                    6,
                    20
                );

                column.speed = random(
                    1,
                    4
                );

                column.counter = 0;
            }

            continue;
        }

        // ----------------------------------------------------
        // Двигаем колонку не каждый кадр
        // ----------------------------------------------------

        column.counter++;

        if (column.counter < column.speed)
            continue;

        column.counter = 0;

        // ----------------------------------------------------
        // Стираем хвост
        // ----------------------------------------------------

        int16_t oldTail =
            column.head - column.length;

        if (oldTail >= 0 &&
            oldTail < ROWS)
        {
            clearCharacter(
                columnIndex,
                oldTail
            );
        }

        // ----------------------------------------------------
        // Стираем старую голову,
        // чтобы избежать артефактов
        // ----------------------------------------------------

        if (column.head >= 0 &&
            column.head < ROWS)
        {
            drawCharacter(
                columnIndex,
                column.head,
                randomCharacter(),
                greenColor(110)
            );
        }

        // ----------------------------------------------------
        // Новая позиция
        // ----------------------------------------------------

        column.head += 1;

        // ----------------------------------------------------
        // Рисуем хвост
        // ----------------------------------------------------

        for (int16_t i = 0;
             i < column.length;
             i++)
        {
            int16_t row =
                column.head - i;

            if (row < 0 ||
                row >= ROWS)
            {
                continue;
            }

            uint8_t brightness;

            if (i == 0)
            {
                // Голова — самая яркая
                brightness = 255;
            }
            else if (i == 1)
            {
                brightness = 220;
            }
            else if (i < 4)
            {
                brightness = 170;
            }
            else
            {
                int value =
                    150 -
                    ((i - 4) * 8);

                if (value < 30)
                    value = 30;

                brightness =
                    static_cast<uint8_t>(value);
            }

            drawCharacter(
                columnIndex,
                row,
                randomCharacter(),
                greenColor(brightness)
            );
        }

        // ----------------------------------------------------
        // Если колонка ушла вниз —
        // запускаем её снова сверху
        // ----------------------------------------------------

        if (column.head - column.length > ROWS)
        {
            column.active = false;

            // Небольшая случайная пауза
            column.head =
                -random(5, 30);
        }
    }
}


// ============================================================
// РИСОВАНИЕ СИМВОЛА
// ============================================================

void SplashScreen::drawCharacter(
    uint16_t column,
    int16_t row,
    char character,
    uint16_t color
)
{
    if (column >= COLUMNS)
        return;

    if (row < 0 || row >= ROWS)
        return;

    int16_t globalX =
        column * CELL_WIDTH;

    int16_t globalY =
        row * CELL_HEIGHT;

    _display.drawText(
        String(character).c_str(),
        globalX,
        globalY + 7,
        color,
        nullptr
    );
}


// ============================================================
// ОЧИСТКА СИМВОЛА
// ============================================================

void SplashScreen::clearCharacter(
    uint16_t column,
    int16_t row
)
{
    if (column >= COLUMNS)
        return;

    if (row < 0 || row >= ROWS)
        return;

    int16_t x =
        column * CELL_WIDTH;

    int16_t y =
        row * CELL_HEIGHT;

    _display.fillRect(
        x,
        y,
        CELL_WIDTH,
        CELL_HEIGHT,
        ST77XX_BLACK
    );
}


// ============================================================
// MATRIX CHARACTERS
// ============================================================

char SplashScreen::randomCharacter()
{
    static const char characters[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "@#$%&*+-=<>[]{}"
        "/\\|";

    constexpr uint16_t count =
        sizeof(characters) - 1;

    return characters[
        random(0, count)
    ];
}


// ============================================================
// RGB565 GREEN
// ============================================================

uint16_t SplashScreen::greenColor(
    uint8_t brightness
)
{
    /*
        RGB565:

        R = 5 bit
        G = 6 bit
        B = 5 bit

        Для Matrix используем только зелёный.
    */

    uint8_t green =
        map(
            brightness,
            0,
            255,
            0,
            63
        );

    return static_cast<uint16_t>(
        green << 5
    );
}


// ============================================================
// ФИНАЛЬНАЯ ВСПЫШКА
// ============================================================

void SplashScreen::finalFlash()
{
    _display.clear(
        greenColor(80)
    );

    delay(40);

    _display.clear(
        greenColor(25)
    );

    delay(40);

    _display.clear(
        ST77XX_BLACK
    );

    delay(100);
}