#include "VEML7700.h"


bool VEML7700Sensor::begin()
{
    Serial.println("[VEML7700] Initializing...");

    if (!veml.begin())
    {
        Serial.println(
            "[VEML7700] ERROR: Sensor not found!"
        );

        initialized = false;

        return false;
    }

    Serial.println(
        "[VEML7700] Sensor found!"
    );


    // Настройки чувствительности

    veml.setGain(VEML7700_GAIN_1_8);

    veml.setIntegrationTime(
        VEML7700_IT_100MS
    );


    initialized = true;

    return true;
}


bool VEML7700Sensor::update()
{
    if (!initialized)
    {
        return false;
    }

    lux = veml.readLux();

    return true;
}


float VEML7700Sensor::getLux()
{
    return lux;
}