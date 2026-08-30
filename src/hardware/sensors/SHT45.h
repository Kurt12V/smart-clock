#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT4x.h>


class SHT45Sensor
{
public:

    bool begin();

    bool update();

    float getTemperature();

    float getHumidity();

private:

    Adafruit_SHT4x sht4;

    float temperature = 0.0f;

    float humidity = 0.0f;

    bool initialized = false;
};