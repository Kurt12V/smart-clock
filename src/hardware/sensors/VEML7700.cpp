#include "VEML7700Sensor.h"

bool VEML7700Sensor::begin() {
    Serial.println("[VEML7700] Initializing...");

    if (!veml.begin()) {
        Serial.println("[VEML7700] ERROR: Sensor not found!");
        initialized = false;
        return false;
    }

    Serial.println("[VEML7700] Sensor found!");

    // (Опционально) Настройка параметров датчика
    // Например, можно установить время интегрирования и усиление:
    // veml.setIntegrationTime(VEML7700_IT_100MS);
    // veml.setGain(VEML7700_GAIN_1);
    // Для большинства применений настроек по умолчанию достаточно.

    initialized = true;
    return true;
}

bool VEML7700Sensor::update() {
    if (!initialized) {
        return false;
    }

    // Чтение освещённости в люксах
    float luxValue = veml.readLux();

    // Проверка на ошибку чтения (некоторые библиотеки возвращают NAN)
    if (isnan(luxValue)) {
        Serial.println("[VEML7700] ERROR: Failed to read lux value");
        return false;
    }

    lux = luxValue;
    return true;
}