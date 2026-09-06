#include <Arduino.h>
#include "DisplaySystem.h"

DisplaySystem displaySystem;

void setup()
{
    Serial.begin(115200);
    delay(500);

    displaySystem.begin();
    displaySystem.showSplash();

    displaySystem.clock().setTime(12, 34);
    displaySystem.clock().setDate(6, 9, 2026);

    displaySystem.clock().setValue(0, "22.5", "TEMPERATURE");
    displaySystem.clock().setValue(1, "47", "HUMIDITY");
    displaySystem.clock().setValue(2, "125", "LIGHT");
    displaySystem.clock().setValue(3, "87", "BATTERY");

    displaySystem.clock().setStatus("ONLINE");
    displaySystem.clock().notify("SYSTEM READY");

    displaySystem.showClock();
}

void loop()
{
    delay(1000);
>>>>>>> dc146c51bd4400ae6cb9d747586280e2a7c03913
}