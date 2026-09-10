#include <Arduino.h>
#include <Wire.h>
#include <vl53l8cx.h>

// =========================
// I2C
// =========================

#define I2C_SDA 8
#define I2C_SCL 9

// LP физически подключен к 3.3V
// GPIO для LP НЕ используется.

// =========================
// VL53L8CX
// =========================

// В зависимости от версии библиотеки конструктор может отличаться.
// Для версии ST Arduino обычно используется такой вариант:
VL53L8CX sensor(&Wire, -1);

VL53L8CX_ResultsData results;

uint8_t status = 0;
uint8_t dataReady = 0;


// =========================
// SETUP
// =========================

void setup()
{
    // UART0
    Serial0.begin(115200);

    delay(1000);

    Serial0.println();
    Serial0.println("================================");
    Serial0.println("       VL53L8CX TEST");
    Serial0.println("================================");

    // I2C
    Wire.begin(
        I2C_SDA,
        I2C_SCL
    );

    Wire.setClock(400000);

    Serial0.println("[I2C] SDA = GPIO8");
    Serial0.println("[I2C] SCL = GPIO9");
    Serial0.println("[I2C] Speed = 400 kHz");

    // =========================
    // SENSOR BEGIN
    // =========================

    Serial0.println();
    Serial0.println("[VL53L8CX] begin...");

    status = sensor.begin();

    if (status != 0)
    {
        Serial0.print("[VL53L8CX] begin FAILED, status = ");
        Serial0.println(status);

        while (true)
        {
            delay(1000);
        }
    }

    Serial0.println("[VL53L8CX] begin OK");

    // =========================
    // SENSOR INIT
    // =========================

    Serial0.println("[VL53L8CX] init...");

    status = sensor.init();

    if (status != 0)
    {
        Serial0.print("[VL53L8CX] init FAILED, status = ");
        Serial0.println(status);

        while (true)
        {
            delay(1000);
        }
    }

    Serial0.println("[VL53L8CX] init OK");

    // =========================
    // 8x8 RESOLUTION
    // =========================

    status =
        sensor.set_resolution(
            VL53L8CX_RESOLUTION_8X8
        );

    if (status != 0)
    {
        Serial0.print("[VL53L8CX] resolution FAILED, status = ");
        Serial0.println(status);

        while (true)
        {
            delay(1000);
        }
    }

    Serial0.println("[VL53L8CX] Resolution: 8x8");

    // =========================
    // START RANGING
    // =========================

    status =
        sensor.start_ranging();

    if (status != 0)
    {
        Serial0.print("[VL53L8CX] start ranging FAILED, status = ");
        Serial0.println(status);

        while (true)
        {
            delay(1000);
        }
    }

    Serial0.println("[VL53L8CX] Ranging STARTED");
    Serial0.println();
}


// =========================
// LOOP
// =========================

void loop()
{
    // Проверяем, готовы ли новые данные
    status =
        sensor.check_data_ready(
            &dataReady
        );

    if (status != 0)
    {
        Serial0.print("[VL53L8CX] check_data_ready error: ");
        Serial0.println(status);

        delay(100);
        return;
    }

    if (!dataReady)
    {
        delay(5);
        return;
    }

    // Получаем данные
    status =
        sensor.get_ranging_data(
            &results
        );

    if (status != 0)
    {
        Serial0.print("[VL53L8CX] get_ranging_data error: ");
        Serial0.println(status);

        delay(50);
        return;
    }

    // =========================
    // PRINT 8x8
    // =========================

    Serial0.println();
    Serial0.println("---------- DISTANCE ----------");

    for (uint8_t y = 0; y < 8; y++)
    {
        for (uint8_t x = 0; x < 8; x++)
        {
            uint8_t zone =
                y * 8 + x;

            int16_t distance =
                results.distance_mm[zone];

            uint8_t targets =
                results.nb_target_detected[zone];

            if (targets == 0)
            {
                Serial0.print(" ----");
            }
            else
            {
                if (distance < 1000)
                    Serial0.print(" ");

                if (distance < 100)
                    Serial0.print(" ");

                Serial0.print(distance);
            }

            Serial0.print(" ");
        }

        Serial0.println();
    }

    Serial0.println("------------------------------");

    delay(50);
}