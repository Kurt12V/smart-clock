#pragma once
#include "SensorData.h"
#include "SHT45Sensor.h"
#include "VEML7700Sensor.h"  // предположим, что есть такой класс

class SensorManager {
public:
    SensorManager();

    bool begin();
    void update();
    SensorData getData() const;

private:
    static constexpr int SENSOR_COUNT = 2;
    Sensor* sensors[SENSOR_COUNT];
    SHT45Sensor    sht45;
    VEML7700Sensor veml7700;

    SensorData data;
    unsigned long lastUpdate = 0;
};