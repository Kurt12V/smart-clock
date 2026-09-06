#include <Arduino.h>
#include "./core/DisplaySystem.h"

DisplaySystem displaySystem;

void setup()
{
    Serial.begin(115200);
    delay(500);

    // Инициализация дисплеев
    displaySystem.begin();

    // =========================
    // ВРЕМЯ
    // 01:23
    // =========================
displaySystem.showSplash();
}

void loop()
{
}
