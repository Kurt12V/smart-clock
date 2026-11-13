#include <Arduino.h>

#include "./core/App.h"

namespace
{
    App app;
}

void setup()
{
    Serial0.begin(115200);
    delay(500);

    app.begin();
}

void loop()
{
    app.update();
    delay(1);
}
