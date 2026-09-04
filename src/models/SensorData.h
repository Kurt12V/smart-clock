#pragma once

#include <Arduino.h>
#include <math.h>

struct SensorData {

// =========================
// SHT45
// =========================

float temperature = NAN;
float humidity = NAN;

bool temperatureValid = false;
bool humidityValid = false;


// =========================
// VEML7700
// =========================

float lightLux = NAN;

bool lightValid = false;


// =========================
// Будущие датчики
// =========================

float coPpm = NAN;
bool coValid = false;

float distance = NAN;
bool distanceValid = false;

bool presenceDetected = false;
bool presenceValid = false;


// =========================
// Сброс данных
// =========================

void reset() {

    temperature = NAN;
    humidity = NAN;
    lightLux = NAN;

    coPpm = NAN;
    distance = NAN;

    presenceDetected = false;

    temperatureValid = false;
    humidityValid = false;
    lightValid = false;

    coValid = false;
    distanceValid = false;
    presenceValid = false;
}


// =========================
// Debug вывод
// =========================

void print() const {

    Serial.println();
    Serial.println("================================");
    Serial.println("          SENSOR DATA");
    Serial.println("================================");

    // Temperature
    Serial.print("Temperature: ");

    if (temperatureValid) {
        Serial.print(temperature, 2);
        Serial.println(" C");
    } else {
        Serial.println("--");
    }


    // Humidity
    Serial.print("Humidity:    ");

    if (humidityValid) {
        Serial.print(humidity, 2);
        Serial.println(" %");
    } else {
        Serial.println("--");
    }


    // Light
    Serial.print("Light:       ");

    if (lightValid) {
        Serial.print(lightLux, 2);
        Serial.println(" lux");
    } else {
        Serial.println("--");
    }


    // CO
    Serial.print("CO:          ");

    if (coValid) {
        Serial.print(coPpm, 2);
        Serial.println(" ppm");
    } else {
        Serial.println("--");
    }


    // Distance
    Serial.print("Distance:    ");

    if (distanceValid) {
        Serial.print(distance, 2);
        Serial.println(" mm");
    } else {
        Serial.println("--");
    }


    // Presence
    Serial.print("Presence:    ");

    if (presenceValid) {
        Serial.println(
            presenceDetected ? "DETECTED" : "NOT DETECTED"
        );
    } else {
        Serial.println("--");
    }

    Serial.println("================================");
}

};
