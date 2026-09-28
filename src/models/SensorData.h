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

    Serial0.println();
    Serial0.println("================================");
    Serial0.println("          SENSOR DATA");
    Serial0.println("================================");

    // Temperature
    Serial0.print("Temperature: ");

    if (temperatureValid) {
        Serial0.print(temperature, 2);
        Serial0.println(" C");
    } else {
        Serial0.println("--");
    }


    // Humidity
    Serial0.print("Humidity:    ");

    if (humidityValid) {
        Serial0.print(humidity, 2);
        Serial0.println(" %");
    } else {
        Serial0.println("--");
    }


    // Light
    Serial0.print("Light:       ");

    if (lightValid) {
        Serial0.print(lightLux, 2);
        Serial0.println(" lux");
    } else {
        Serial0.println("--");
    }


    // CO
    Serial0.print("CO:          ");

    if (coValid) {
        Serial0.print(coPpm, 2);
        Serial0.println(" ppm");
    } else {
        Serial0.println("--");
    }


    // Distance
    Serial0.print("Distance:    ");

    if (distanceValid) {
        Serial0.print(distance, 2);
        Serial0.println(" mm");
    } else {
        Serial0.println("--");
    }


    // Presence
    Serial0.print("Presence:    ");

    if (presenceValid) {
        Serial0.println(
            presenceDetected ? "DETECTED" : "NOT DETECTED"
        );
    } else {
        Serial0.println("--");
    }

    Serial0.println("================================");
}

};
