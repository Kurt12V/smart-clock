#include <Arduino.h>
#include "./managers/LedMatrixManager.h"

// Используй GPIO, к которому подключена матрица WS2812.
constexpr uint8_t MATRIX_DATA_PIN = 48;

LedMatrixManager matrix(MATRIX_DATA_PIN);

void setup()
{
    Serial0.begin(115200);
    delay(1000);

    Serial0.println();
    Serial0.println("=== LED MATRIX BRIGHTNESS TEST ===");

    matrix.begin();

    // Отключаем эффекты, чтобы они не перезаписывали тестовый цвет.
    matrix.stopEffect();

    matrix.on();
    matrix.clear();

    // Устанавливаем красный цвет.
    matrix.fill(255, 0, 0);

    // Проверяем несколько значений яркости.
    constexpr uint8_t brightnessLevels[] =
    {
        10, 20, 30, 40, 60, 100, 150
    };

    constexpr size_t levelCount =
        sizeof(brightnessLevels) / sizeof(brightnessLevels[0]);

    for (size_t i = 0; i < levelCount; ++i)
    {
        const uint8_t brightness = brightnessLevels[i];

        matrix.setBrightness(brightness);
        matrix.show();

        Serial0.printf(
            "Brightness: %u / 255\n",
            static_cast<unsigned>(brightness)
        );

        // Оставляем каждый уровень на 3 секунды.
        delay(3000);
    }

    Serial0.println("=== TEST FINISHED ===");
}

void loop()
{
    // Ничего не меняем после завершения теста.
}