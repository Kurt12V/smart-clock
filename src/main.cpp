#include <Arduino.h>
#include "managers/Led/LedMatrixManager.h"

// Укажи тот же GPIO, который передаёшь в LightingManager.
constexpr uint8_t MATRIX_PIN = 14;

LedMatrixManager matrix(MATRIX_PIN);

void setup()
{
    Serial0.begin(115200);
    delay(500);

    matrix.begin();
    matrix.on();
    matrix.setBrightness(20);

    matrix.fill(255, 0, 0);
    matrix.show();

    Serial0.println("[TEST] Matrix red");
}

void loop()
{
    matrix.update();
    delay(1);
}