#pragma once
#include <Arduino.h>

struct SensorData {
    float temperature = NAN;   // °C
    float humidity    = NAN;   // %
    float lightLux    = NAN;   // lux

    bool temperatureValid = false;
    bool humidityValid    = false;
    bool lightValid       = false;

    // Метод для сброса всех полей
    void reset() {
        temperature = humidity = lightLux = NAN;
        temperatureValid = humidityValid = lightValid = false;
    }

    // Метод для вывода в Serial (можно переопределить формат)
    // void print() const {
    //     Serial.println();
    //     Serial.println("================================");
    //     Serial.println("           SENSOR DATA");
    //     Serial.println("================================");

    //     if (temperatureValid) {
    //         Serial.print("Temperature: ");
    //         Serial.print(temperature, 2);
    //         Serial.println(" C");
    //     } else {
    //         Serial.println("Temperature: --");
    //     }

    //     if (humidityValid) {
    //         Serial.print("Humidity:    ");
    //         Serial.print(humidity, 2);
    //         Serial.println(" %");
    //     } else {
    //         Serial.println("Humidity:    --");
    //     }

    //     if (lightValid) {
    //         Serial.print("Light:       ");
    //         Serial.print(lightLux, 2);
    //         Serial.println(" lux");
    //     } else {
    //         Serial.println("Light:       --");
    //     }

    //     Serial.println("================================");
    // }
};