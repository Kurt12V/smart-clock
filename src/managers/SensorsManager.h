#pragma once

#include <Arduino.h>

#include "models/Sensor.h"
#include "models/SensorData.h"

#include "./hardware/sensors/SHT45.h"
#include "./hardware/sensors/VEML7700.h"
#include "./hardware/sensors/VL53L8CX.h"
#include "./hardware/audio/Microphone.h"

class SensorManager {
public:
SensorManager();

bool begin();
void update();

const SensorData& getData() const;


// =========================
// Готовые значения
// =========================

String getTemperatureC() const;

String getTemperatureF() const;

String getHumidity() const;

String getLight() const;

    // =====================================================
    // MICROPHONE
    // =====================================================

    bool isMicrophoneInitialized() const;

    bool isMicrophoneEnabled() const;

    void setMicrophoneEnabled(
        bool enabled
    );


    bool startListening();

    void stopListening();

    bool isListening() const;


    bool updateAudio();


    float getMicrophoneRMS() const;

    float getMicrophoneLevel() const;

    int16_t getMicrophonePeak() const;


    /*
     * Доступ к Microphone для AudioRecorder.
     */

    Microphone& getMicrophone();

bool isVL53L8CXInitialized() const;

bool updateVL53L8CX();

int16_t getVL53L8CXDistance(uint8_t zone) const;
uint8_t getVL53L8CXTargets(uint8_t zone) const;

const int16_t* getVL53L8CXDistances() const;
const uint8_t* getVL53L8CXTargets() const;

VL53L8CXSensor& getVL53L8CX();

void printData() const;


private:


static constexpr int SENSOR_COUNT = 2;

SHT45Sensor sht45;
VEML7700Sensor veml7700;
VL53L8CXSensor vl53l8cx;
Microphone microphone;

Sensor* sensors[SENSOR_COUNT];

SensorData data;

unsigned long lastUpdate = 0;


// Конвертация Celsius -> Fahrenheit
float celsiusToFahrenheit(float celsius) const;


};
