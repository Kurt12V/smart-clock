#include <Arduino.h>
#include "Config.h"
#include "./core/ClockSystem.h"
#include "./ui/screens/ClockScreen.h"
#include "./core/DisplaySystem.h"
#include "./managers/SensorsManager.h"

DisplaySystem displaySystem;



void setup()
{
    Serial.begin(115200);
    delay(500);

    displaySystem.begin();

    // =========================
    // ВРЕМЯ
    // 01:23
    // =========================
displaySystem.startSplash();
}

void loop()
{

}
