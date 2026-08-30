#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VEML7700.h>


class VEML7700Sensor
{
public:

    bool begin();

    bool update();

    float getLux();

private:

    Adafruit_VEML7700 veml;

    float lux = 0.0f;

    bool initialized = false;
};