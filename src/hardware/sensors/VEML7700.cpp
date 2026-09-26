#include "VEML7700.h"

bool VEML7700Sensor::begin() {


Serial.println("[VEML7700] Initializing...");

if (!veml.begin()) {

    Serial.println("[VEML7700] ERROR: Sensor not found!");

    initialized = false;

    return false;
}

Serial.println("[VEML7700] Sensor found!");

initialized = true;

Serial.println("[VEML7700] READY");

return true;


}

bool VEML7700Sensor::update(SensorData& data) {


if (!initialized) {
    return false;
}

float lux = veml.readLux();

if (isnan(lux) || lux < 0) {

    Serial.println("[VEML7700] ERROR: Failed to read light");

    data.lightValid = false;

    return false;
}

data.lightLux = lux;

data.lightValid = true;

return true;


}
