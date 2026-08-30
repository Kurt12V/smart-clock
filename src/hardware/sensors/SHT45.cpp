#include "SHT45.h"


bool SHT45Sensor::begin()
{
    Serial.println("[SHT45] Initializing...");

    if (!sht4.begin())
    {
        Serial.println("[SHT45] ERROR: Sensor not found!");

        initialized = false;

        return false;
    }

    Serial.println("[SHT45] Sensor found!");

    // Высокая точность
    sht4.setPrecision(SHT4X_HIGH_PRECISION);

    // Без встроенного нагревателя
    sht4.setHeater(SHT4X_NO_HEATER);

    initialized = true;

    return true;
}


bool SHT45Sensor::update()
{
    if (!initialized)
    {
        return false;
    }

    sensors_event_t humidityEvent;
    sensors_event_t temperatureEvent;

    if (!sht4.getEvent(
            &humidityEvent,
            &temperatureEvent
        ))
    {
        Serial.println("[SHT45] ERROR: Failed to read sensor");

        return false;
    }

    temperature = temperatureEvent.temperature;

    humidity = humidityEvent.relative_humidity;

    return true;
}


float SHT45Sensor::getTemperature()
{
    return temperature;
}


float SHT45Sensor::getHumidity()
{
    return humidity;
}