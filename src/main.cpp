#include <Arduino.h>

#include "Config.h"
#include "./core/App.h"

App app;

void setup()
{
    Serial0.begin(Config::SERIAL0_BAUD_RATE);
    delay(1000);

    app.begin();
}

void loop()
{
    app.update();
    delay(10);
}