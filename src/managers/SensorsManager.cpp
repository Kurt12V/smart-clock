#include "SensorsManager.h"
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


Serial0.println();
Serial0.println("================================");
Serial0.println("         SENSOR MANAGER");
Serial0.println("================================");

bool allOk = true;

for (int i = 0; i < SENSOR_COUNT; ++i) {

    if (!sensors[i]->begin()) {

        Serial0.printf(
            "[SensorManager] %s FAILED\n",
            sensors[i]->getName()
        );

        allOk = false;

    } else {

        Serial0.printf(
            "[SensorManager] %s READY\n",
            sensors[i]->getName()
        );
    }
}

  // -------------------------
    // VL53L8CX
    // -------------------------

    Serial0.println();

    if (!vl53l8cx.begin())
    {
        Serial0.println(
            "[SensorManager] VL53L8CX FAILED"
        );
    }
    else
    {
        Serial0.println(
            "[SensorManager] VL53L8CX READY"
        );
    }

return allOk;


}

// ========================================
// UPDATE
// ========================================

void SensorManager::update() {


const unsigned long currentTime = millis();

if (currentTime - lastUpdate < Constants::SENSOR_UPDATE_INTERVAL) {
    return;
}

lastUpdate = currentTime;

for (int i = 0; i < SENSOR_COUNT; ++i) {

    if (!sensors[i]->isInitialized()) {
        continue;
    }

    sensors[i]->update(data);
}
    // VL53L8CX обновляется независимо
    vl53l8cx.update();

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
     + " ";


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
+ " ";


}

// ========================================
// HUMIDITY
// ========================================

String SensorManager::getHumidity() const {


if (!data.humidityValid) {
    return "--";
}

return String(data.humidity, 2)
+ " ";


}

// ========================================
// LIGHT
// ========================================

String SensorManager::getLight() const {


if (!data.lightValid) {
    return "--";
}

return String(data.lightLux, 2)
+ " ";


}


// =====================================================
// VL53L8CX
// =====================================================
bool SensorManager::isVL53L8CXInitialized() const
{
    return vl53l8cx.isInitialized();
}

bool SensorManager::updateVL53L8CX()
{
    return vl53l8cx.update();
}

int16_t SensorManager::getVL53L8CXDistance(
    uint8_t zone
) const
{
    return vl53l8cx.getDistance(zone);
}

uint8_t SensorManager::getVL53L8CXTargets(
    uint8_t zone
) const
{
    return vl53l8cx.getTargets(zone);
}

const int16_t*
SensorManager::getVL53L8CXDistances() const
{
    return vl53l8cx.getDistances();
}

const uint8_t*
SensorManager::getVL53L8CXTargets() const
{
    return vl53l8cx.getTargets();
}

VL53L8CXSensor&
SensorManager::getVL53L8CX()
{
    return vl53l8cx;
} 


// ========================================
// DEBUG PRINT
// ========================================

void SensorManager::printData() const {


Serial0.println();
Serial0.println("================================");
Serial0.println("         SENSOR DATA");
Serial0.println("================================");

Serial0.print("Temperature C: ");
Serial0.println(getTemperatureC());

Serial0.print("Temperature F: ");
Serial0.println(getTemperatureF());

Serial0.print("Humidity:      ");
Serial0.println(getHumidity());

Serial0.print("Light:         ");
Serial0.println(getLight());

Serial0.println("================================");


}
