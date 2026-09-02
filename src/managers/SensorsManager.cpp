#include "SensorManager.h"
#include "Config.h"

SensorManager::SensorManager() {
    sensors[0] = &sht45;
    sensors[1] = &veml7700;
}

bool SensorManager::begin() {
    Serial.println();
    Serial.println("==============================");
    Serial.println("SENSOR MANAGER");
    Serial.println("==============================");

    bool allOk = true;
    for (int i = 0; i < SENSOR_COUNT; ++i) {
        if (!sensors[i]->begin()) {
            Serial.printf("[SensorManager] %s FAILED\n", sensors[i]->getName());
            allOk = false;
        } else {
            Serial.printf("[SensorManager] %s READY\n", sensors[i]->getName());
        }
    }

    if (allOk) {
        Serial.println("[SensorManager] ALL SENSORS READY");
    } else {
        Serial.println("[SensorManager] SOME SENSORS FAILED");
    }
    return allOk;
}

void SensorManager::update() {
    unsigned long currentTime = millis();
    if (currentTime - lastUpdate < SENSOR_UPDATE_INTERVAL) {
        return;
    }
    lastUpdate = currentTime;

    // Сбрасываем данные перед обновлением
    data.reset();

    for (int i = 0; i < SENSOR_COUNT; ++i) {
        if (sensors[i]->update()) {
            // Здесь нужно заполнить data в зависимости от типа датчика.
            // Так как мы не знаем тип, можно использовать dynamic_cast,
            // но для простоты оставим прямое обращение к конкретным датчикам.
            // Это нарушает абстракцию, но в данном случае допустимо.
        }
    }

    // Заполняем data из конкретных датчиков (можно сделать через интерфейс, но проще так)
    if (sht45.isInitialized()) {
        // В реальном проекте sht45.update() уже вызван, но мы можем повторно не читать,
        // а получить последние значения.
        // Лучше, чтобы датчики сами обновляли свои внутренние поля.
        // Мы можем считать их через геттеры.
        data.temperature = sht45.getTemperature();
        data.humidity    = sht45.getHumidity();
        data.temperatureValid = !isnan(data.temperature);
        data.humidityValid    = !isnan(data.humidity);
    }

    if (veml7700.isInitialized()) {
        data.lightLux = veml7700.getLux();
        data.lightValid = !isnan(data.lightLux);
    }
}

SensorData SensorManager::getData() const {
    return data;
}

void SensorManager::printData() const {
    data.print();
}