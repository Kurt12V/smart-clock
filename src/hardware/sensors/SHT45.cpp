#include "SHT45Sensor.h"

bool SHT45Sensor::begin() {
    Serial.println("[SHT45] Initializing...");
    if (!sht4.begin()) {
        Serial.println("[SHT45] ERROR: Sensor not found!");
        initialized = false;
        return false;
    }
    Serial.println("[SHT45] Sensor found!");
    sht4.setPrecision(SHT4X_HIGH_PRECISION);
    sht4.setHeater(SHT4X_NO_HEATER);
    initialized = true;
    return true;
}

bool SHT45Sensor::update() {
    if (!initialized) {
        return false;
    }

    sensors_event_t humidityEvent, temperatureEvent;
    if (!sht4.getEvent(&humidityEvent, &temperatureEvent)) {
        Serial.println("[SHT45] ERROR: Failed to read sensor");
        return false;
    }

    temperature = temperatureEvent.temperature;
    humidity = humidityEvent.relative_humidity;
    return true;
}