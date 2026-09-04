#include "SensorManager.h"
#include "Config.h"
#include "Constants.h"

// ========================================
// CONSTRUCTOR
// ========================================

SensorManager::SensorManager() {


sensors[0] = &sht45;
sensors[1] = &veml7700;


}

// ========================================
// INITIALIZATION
// ========================================

bool SensorManager::begin() {


Serial.println();
Serial.println("================================");
Serial.println("         SENSOR MANAGER");
Serial.println("================================");

bool allOk = true;

for (int i = 0; i < SENSOR_COUNT; ++i) {

    if (!sensors[i]->begin()) {

        Serial.printf(
            "[SensorManager] %s FAILED\n",
            sensors[i]->getName()
        );

        allOk = false;

    } else {

        Serial.printf(
            "[SensorManager] %s READY\n",
            sensors[i]->getName()
        );
    }
}

return allOk;


}

// ========================================
// UPDATE
// ========================================

void SensorManager::update() {


const unsigned long currentTime = millis();

if (currentTime - lastUpdate < SENSOR_UPDATE_INTERVAL) {
    return;
}

lastUpdate = currentTime;

for (int i = 0; i < SENSOR_COUNT; ++i) {

    if (!sensors[i]->isInitialized()) {
        continue;
    }

    sensors[i]->update(data);
}


}

// ========================================
// RAW DATA
// ========================================

const SensorData& SensorManager::getData() const {
return data;
}

// ========================================
// TEMPERATURE
// ========================================

String SensorManager::getTemperatureC() const {


if (!data.temperatureValid) {
    return "--";
}

return String(data.temperature, 2)
    + " "
    + Constants::UNIT_TEMP_C;


}

float SensorManager::celsiusToFahrenheit(float celsius) const {


return (celsius * 9.0f / 5.0f) + 32.0f;


}

String SensorManager::getTemperatureF() const {


if (!data.temperatureValid) {
    return "--";
}

float fahrenheit = celsiusToFahrenheit(data.temperature);

return String(fahrenheit, 2)
    + " "
    + Constants::UNIT_TEMP_F;


}

// ========================================
// HUMIDITY
// ========================================

String SensorManager::getHumidity() const {


if (!data.humidityValid) {
    return "--";
}

return String(data.humidity, 2)
    + " "
    + Constants::UNIT_HUMID;


}

// ========================================
// LIGHT
// ========================================

String SensorManager::getLight() const {


if (!data.lightValid) {
    return "--";
}

return String(data.lightLux, 2)
    + " "
    + Constants::UNIT_LUX;


}

// ========================================
// DEBUG PRINT
// ========================================

void SensorManager::printData() const {


Serial.println();
Serial.println("================================");
Serial.println("         SENSOR DATA");
Serial.println("================================");

Serial.print("Temperature C: ");
Serial.println(getTemperatureC());

Serial.print("Temperature F: ");
Serial.println(getTemperatureF());

Serial.print("Humidity:      ");
Serial.println(getHumidity());

Serial.print("Light:         ");
Serial.println(getLight());

Serial.println("================================");
```

}
