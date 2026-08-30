#pragma once

#include <Arduino.h>

#include "hardware/sensors/SHT45.h"
#include "hardware/sensors/VEML7700.h"


// ============================================================
// SENSOR DATA
// ============================================================

struct SensorData
{
    float temperature;

    float humidity;

    float lightLux;
};


// ============================================================
// SENSOR MANAGER
// ============================================================

class SensorManager
{
public:

    bool begin();

    void update();

    SensorData getData();

    void printData();


private:

    SHT45Sensor sht45;

    VEML7700Sensor veml7700;


    SensorData data;

    unsigned long lastUpdate = 0;
};