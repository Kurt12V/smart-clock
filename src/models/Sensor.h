#pragma once
#include <Arduino.h>

class Sensor {
public:
    virtual ~Sensor() = default;

    // Инициализация датчика, возвращает true при успехе
    virtual bool begin() = 0;

    // Обновление данных, возвращает true, если чтение успешно
    virtual bool update() = 0;

    // Получить имя датчика (для отладки)
    virtual const char* getName() const = 0;

    // Проверить, инициализирован ли датчик
    virtual bool isInitialized() const = 0;
};