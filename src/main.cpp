#include <Arduino.h>

#include <Wire.h>

#include "Config.h"

#include "managers/SensorManager.h"


// ============================================================
// SENSOR MANAGER
// ============================================================

SensorManager sensorManager;


// ============================================================
// SETUP
// ============================================================

void setup()
{
    // --------------------------------------------------------
    // SERIAL
    // --------------------------------------------------------

    Serial.begin(115200);

    delay(1000);


    Serial.println();
    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        "ESP32 SMART CLOCK"
    );

    Serial.println(
        "Starting system..."
    );

    Serial.println(
        "================================"
    );


    // --------------------------------------------------------
    // I2C
    // --------------------------------------------------------

    Wire.begin(
        I2C_SDA,
        I2C_SCL
    );

    Wire.setClock(
        I2C_FREQUENCY
    );


    Serial.println();

    Serial.println(
        "[SYSTEM] I2C initialized"
    );


    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    sensorManager.begin();


    Serial.println();

    Serial.println(
        "[SYSTEM] Setup complete"
    );
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    // Обновляем датчики

    sensorManager.update();


    // Получаем данные

    SensorData data =
        sensorManager.getData();


    // Выводим данные каждые 5 секунд

    static unsigned long lastPrint = 0;


    if (
        millis() - lastPrint >= 5000
    )
    {
        lastPrint = millis();


        sensorManager.printData();
    }
}