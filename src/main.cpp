```cpp
#include <Arduino.h>
#include "DisplaySystem.h"

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

    displaySystem.clock().setTime(
        0, 1,
        2, 3
    );

    // =========================
    // ДАТА
    // MON
    // 06.09.2026
    // =========================

    displaySystem.clock().setDate(
        "mon",
        6,
        9,
        2026
    );

    

    // =========================
    // ДАТЧИКИ
    // =========================

    displaySystem.clock().setSensors(
        "22.5",   // temperature
        "47.0",   // humidity
        "125.0",  // light
        "87.0"    // battery
    );

    // =========================
    // УВЕДОМЛЕНИЕ
    // =========================

    displaySystem.clock().notify(
        "Доброе утро"
    );

    // =========================
    // ОТРИСОВКА
    // =========================

    displaySystem.clock().draw();
}

void loop()
{
}
```
