#include "SHT45.h"

bool SHT45Sensor::begin() {


Serial.println("[SHT45] Initializing...");

if (!sht4.begin()) {

    Serial.println("[SHT45] ERROR: Sensor not found!");

    initialized = false;

    return false;
}

Serial.println("[SHT45] Sensor found!");

// Высокая точность
sht4.setPrecision(SHT4X_HIGH_PRECISION);

// Нагреватель выключен
sht4.setHeater(SHT4X_NO_HEATER);

initialized = true;

Serial.println("[SHT45] READY");

return true;


}

bool SHT45Sensor::update(SensorData& data) {

if (!initialized) {
    return false;
}

sensors_event_t humidityEvent;
sensors_event_t temperatureEvent;

if (!sht4.getEvent(
        &humidityEvent,
        &temperatureEvent
    )) {

    Serial.println("[SHT45] ERROR: Failed to read sensor");

    data.temperatureValid = false;
    data.humidityValid = false;

    return false;
}

// Температура
data.temperature = temperatureEvent.temperature;

// Влажность
data.humidity = humidityEvent.relative_humidity;

data.temperatureValid = true;
data.humidityValid = true;

return true;

}
