#pragma once

#include <Arduino.h>
#include "SensorData.h"

class Sensor {
public:
virtual ~Sensor() = default;

// Инициализация датчика
virtual bool begin() = 0;

// Обновление данных датчика
virtual bool update(SensorData& data) = 0;

// Имя датчика
virtual const char* getName() const = 0;

// Проверка инициализации
virtual bool isInitialized() const = 0;

};
