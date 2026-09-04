#pragma once

#include "Sensor.h"
#include <Adafruit_SHT4x.h>

class SHT45Sensor : public Sensor {
public:

SHT45Sensor() = default;

bool begin() override;

bool update(SensorData& data) override;

const char* getName() const override {
    return "SHT45";
}

bool isInitialized() const override {
    return initialized;
}


private:
Adafruit_SHT4x sht4;

bool initialized = false;

};
