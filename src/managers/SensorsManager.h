#pragma once

#include <Arduino.h>

#include "models/Sensor.h"
#include "models/SensorData.h"

#include "hardware/sensors/SHT45.h"
#include "hardware/sensors/VEML7700.h"

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

void printData() const;


private:


static constexpr int SENSOR_COUNT = 2;

SHT45Sensor sht45;
VEML7700Sensor veml7700;
Microphone microphone;

Sensor* sensors[SENSOR_COUNT];

SensorData data;

unsigned long lastUpdate = 0;


// Конвертация Celsius -> Fahrenheit
float celsiusToFahrenheit(float celsius) const;


};
