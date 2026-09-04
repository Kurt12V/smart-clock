#pragma once

#include "Sensor.h"
#include <Adafruit_VEML7700.h>

class VEML7700Sensor : public Sensor {
public:

VEML7700Sensor() = default;

bool begin() override;

bool update(SensorData& data) override;

const char* getName() const override {
    return "VEML7700";
}

bool isInitialized() const override {
    return initialized;
}


private:


Adafruit_VEML7700 veml;

bool initialized = false;


};
