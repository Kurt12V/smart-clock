#include "SensorManager.h"

#include "Config.h"


bool SensorManager::begin()
{
    Serial.println();
    Serial.println("==============================");
    Serial.println("SENSOR MANAGER");
    Serial.println("==============================");


    // --------------------------------------------------------
    // START SHT45
    // --------------------------------------------------------

    bool sht45Status = sht45.begin();


    // --------------------------------------------------------
    // START VEML7700
    // --------------------------------------------------------

    bool vemlStatus = veml7700.begin();


    // --------------------------------------------------------
    // RESULT
    // --------------------------------------------------------

    if (!sht45Status)
    {
        Serial.println(
            "[SensorManager] SHT45 FAILED"
        );
    }

    if (!vemlStatus)
    {
        Serial.println(
            "[SensorManager] VEML7700 FAILED"
        );
    }


    if (sht45Status && vemlStatus)
    {
        Serial.println(
            "[SensorManager] ALL SENSORS READY"
        );

        return true;
    }

    return false;
}


// ============================================================
// UPDATE
// ============================================================

void SensorManager::update()
{
    unsigned long currentTime = millis();


    // Проверяем интервал обновления

    if (
        currentTime - lastUpdate <
        SENSOR_UPDATE_INTERVAL
    )
    {
        return;
    }


    lastUpdate = currentTime;


    // --------------------------------------------------------
    // UPDATE SHT45
    // --------------------------------------------------------

    if (sht45.update())
    {
        data.temperature =
            sht45.getTemperature();

        data.humidity =
            sht45.getHumidity();
    }


    // --------------------------------------------------------
    // UPDATE VEML7700
    // --------------------------------------------------------

    if (veml7700.update())
    {
        data.lightLux =
            veml7700.getLux();
    }
}


// ============================================================
// GET DATA
// ============================================================

SensorData SensorManager::getData()
{
    return data;
}


// ============================================================
// PRINT DATA
// ============================================================

void SensorManager::printData()
{
    Serial.println();

    Serial.println(
        "================================"
    );

    Serial.println(
        "           SENSOR DATA"
    );

    Serial.println(
        "================================"
    );


    Serial.print("Temperature: ");

    Serial.print(data.temperature, 2);

    Serial.println(" C");


    Serial.print("Humidity:    ");

    Serial.print(data.humidity, 2);

    Serial.println(" %");


    Serial.print("Light:       ");

    Serial.print(data.lightLux, 2);

    Serial.println(" lux");


    Serial.println(
        "================================"
    );
}