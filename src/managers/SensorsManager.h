#pragma once

#include <Arduino.h>

#include "Sensor.h"
#include "SensorData.h"

#include "SHT45Sensor.h"
#include "VEML7700Sensor.h"

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


void printData() const;


private:


static constexpr int SENSOR_COUNT = 2;

SHT45Sensor sht45;
VEML7700Sensor veml7700;

Sensor* sensors[SENSOR_COUNT];

SensorData data;

unsigned long lastUpdate = 0;


// Конвертация Celsius -> Fahrenheit
float celsiusToFahrenheit(float celsius) const;


};
