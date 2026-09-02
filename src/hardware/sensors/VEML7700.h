#pragma once
#include "Sensor.h"
#include <Adafruit_VEML7700.h>

class VEML7700Sensor : public Sensor {
public:
    VEML7700Sensor() = default;

    // Реализация виртуальных методов базового класса
    bool begin() override;
    bool update() override;
    const char* getName() const override { return "VEML7700"; }
    bool isInitialized() const override { return initialized; }

    // Специфичный для датчика метод получения освещённости (в люксах)
    float getLux() const { return lux; }

private:
    Adafruit_VEML7700 veml;   // объект библиотеки
    bool initialized = false;
    float lux = NAN;          // последнее успешно считанное значение
};