#include <Arduino.h>
#include "Config.h"
#include "Pins.h"
#include "Constants.h"
#include "Version.h"
#include <Wire.h>
#include "SensorManager.h"
#include "RTC.h"
#include "ClockManager.h"

// ============================================================
// SENSOR MANAGER
// ============================================================

SensorManager sensorManager;
ClockSystem clock;

// ============================================================
// SETUP
// ============================================================

void setup()
{
    // --------------------------------------------------------
    // SERIAL
    // --------------------------------------------------------

    Serial.begin(115200);
Serial.println(FIRMWARE_VERSION_STRING);
  Serial.println(FIRMWARE_BUILD_DATE);
  Serial.println(PROJECT_NAME);
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

Serial.println();
    
    Serial.println(
        "[SYSTEM] I2C initialized"
    );


    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    bool sensorsReady = sensorManager.begin(); 
    if (sensorsReady) { Serial.println("[MAIN] All sensors initialized successfully"); } 
    else { Serial.println("[MAIN] WARNING: Some sensors failed!"); }


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


// Текущее время
unsigned long currentTime = millis();


// Выводим данные через интервал
if (currentTime - lastSerialPrint >= SERIAL_PRINT_INTERVAL) {
    lastSerialPrint = currentTime;


    // Получаем данные
    const SensorData& data = sensorManager.getData();


    Serial.println();
    Serial.println("================================");
    Serial.println("       CURRENT SENSOR DATA");
    Serial.println("================================");


    // Температура
    Serial.print("Temperature: ");
    if (data.temperatureValid) {
        Serial.print(data.temperature, 2);
        Serial.println(" C");
    } else {
        Serial.println("--");
    }
    // Влажность
    Serial.print("Humidity:    ");

    if (data.humidityValid) {

        Serial.print(data.humidity, 2);
        Serial.println(" %");

    } else {

        Serial.println("--");
    }


    // Освещение
    Serial.print("Light:       ");

    if (data.lightValid) {

        Serial.print(data.lightLux, 2);
        Serial.println(" lux");

    } else {

        Serial.println("--");
    }


    Serial.println("================================");
}
}



















#include <Arduino.h>

#include "ClockSystem.h"

// ========================================
// MODULES
// ========================================

ClockSystem clock;

// ========================================
// SETUP
// ========================================

void setup()
{
Serial.begin(115200);

```
delay(1000);

clock.begin();
```

}

// ========================================
// LOOP
// ========================================

void loop()
{
TimeData time = clock.getTimeData();

```
DateData date = clock.getDateData();


// ====================================
// TIME
// ====================================

if (time.valid)
{
    Serial.printf(
        "%d%d:%d%d\n",

        time.getHourTens(),
        time.getHourOnes(),

        time.getMinuteTens(),
        time.getMinuteOnes()
    );
}


// ====================================
// DATE
// ====================================

if (date.valid)
{
    Serial.printf(
        "%02d.%02d.%04d\n",

        date.day,
        date.month,
        date.year
    );
}


delay(1000);
```

}
