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

  // -------------------------
    // VL53L8CX
    // -------------------------

    Serial.println();

    if (!vl53l8cx.begin())
    {
        Serial.println(
            "[SensorManager] VL53L8CX FAILED"
        );
    }
    else
    {
        Serial.println(
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

// =====================================================
// MICROPHONE
// =====================================================

bool SensorManager::isMicrophoneInitialized() const
{
    return microphone.isInitialized();
}


bool SensorManager::isMicrophoneEnabled() const
{
    return microphone.isEnabled();
}


void SensorManager::setMicrophoneEnabled(
    bool enabled
)
{
    microphone.setEnabled(
        enabled
    );
}


bool SensorManager::startListening()
{
    return microphone.startListening();
}


void SensorManager::stopListening()
{
    microphone.stopListening();
}


bool SensorManager::isListening() const
{
    return microphone.isListening();
}


bool SensorManager::updateAudio()
{
    return microphone.updateAudio();
}


float SensorManager::getMicrophoneRMS() const
{
    return microphone.getRMS();
}


float SensorManager::getMicrophoneLevel() const
{
    return microphone.getLevel();
}


int16_t SensorManager::getMicrophonePeak() const
{
    return microphone.getPeak();
}


Microphone& SensorManager::getMicrophone()
{
    return microphone;
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
    // ========================================
    // MICROPHONE
    // ========================================

    Serial.println();

    Serial.print("Microphone:    ");

    if (!isMicrophoneInitialized())
    {
        Serial.println("NOT INITIALIZED");
    }
    else if (!isMicrophoneEnabled())
    {
        Serial.println("DISABLED");
    }
    else
    {
        Serial.println("READY");
    };


    Serial.print("Mic level:     ");
    Serial.println(
        getMicrophoneLevel(),
        4
    );


    Serial.print("Mic RMS:       ");
    Serial.println(
        getMicrophoneRMS(),
        2
    );


    Serial.print("Mic peak:      ");
    Serial.println(
        getMicrophonePeak()
    );


    Serial.println(
        "================================"
    );

}
