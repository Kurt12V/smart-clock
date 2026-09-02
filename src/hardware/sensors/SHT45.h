#pragma once
#include "Sensor.h"
#include <Adafruit_SHT4x.h>

class SHT45Sensor : public Sensor {
public:
    SHT45Sensor() = default;

    bool begin() override;
    bool update() override;
    const char* getName() const override { return "SHT45"; }
    bool isInitialized() const override { return initialized; }

    // Дополнительные методы для получения конкретных значений
    float getTemperature() const { return temperature; }
    float getHumidity() const    { return humidity; }

private:
    Adafruit_SHT4x sht4;
    bool initialized = false;
    float temperature = NAN;
    float humidity    = NAN;
};