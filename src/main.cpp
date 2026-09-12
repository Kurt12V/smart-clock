#include <Arduino.h>

#include "./core/System.h"

System app;

void setup()
{
    if (!app.begin())
    {
        while (true)
        {
            delay(1000);
        }
    }
}

void loop()
{
    app.update();
}