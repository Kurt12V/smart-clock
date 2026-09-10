#include <Arduino.h>
#include <Wire.h>
#include <vl53l8cx.h>

// ============================================================
// ESP32-S3
// ============================================================

#define I2C_SDA 8
#define I2C_SCL 9

// LP = Low Power Enable
#define VL53L8CX_LP_PIN 14

// ============================================================
// VL53L8CX
// ============================================================

VL53L8CX sensor(
&Wire,
VL53L8CX_LP_PIN
);

VL53L8CX_ResultsData results;

uint8_t status = 0;
uint8_t dataReady = 0;

// ============================================================
// PRINT 8x8 MATRIX
// ============================================================

void printMatrix()
{
Serial.println();
Serial.println("================================================");
Serial.println("                 VL53L8CX 8x8");
Serial.println("================================================");

```
for (uint8_t y = 0; y < 8; y++)
{
    for (uint8_t x = 0; x < 8; x++)
    {
        uint8_t zone = y * 8 + x;

        uint8_t target =
            results.nb_target_detected[zone];

        if (target == 0)
        {
            Serial.print("  -- ");
        }
        else
        {
            Serial.printf(
                "%4d ",
                results.distance_mm[zone]
            );
        }
    }

    Serial.println();
}

Serial.println("================================================");
```

}

// ============================================================
// PRINT ALL ZONES
// ============================================================

void printDetailedData()
{
Serial.println();
Serial.println("ZONE DATA");
Serial.println("--------------------------------");

```
for (uint8_t zone = 0; zone < 64; zone++)
{
    uint8_t target =
        results.nb_target_detected[zone];

    int16_t distance =
        results.distance_mm[zone];

    uint8_t targetStatus =
        results.target_status[zone];

    Serial.printf(
        "Zone %02d | Distance: %4d mm | Targets: %d | Status: %d\n",
        zone,
        distance,
        target,
        targetStatus
    );
}

Serial.println("--------------------------------");
```

}

// ============================================================
// SETUP
// ============================================================

void setup()
{
Serial.begin(115200);

```
delay(1000);

Serial.println();
Serial.println();
Serial.println("========================================");
Serial.println("       VL53L8CX ESP32-S3 TEST");
Serial.println("========================================");

// --------------------------------------------------------
// I2C
// --------------------------------------------------------

Serial.println();
Serial.println("[1] Starting I2C...");

Wire.begin(
    I2C_SDA,
    I2C_SCL
);

Wire.setClock(400000);

Serial.printf(
    "[I2C] SDA = GPIO%d\n",
    I2C_SDA
);

Serial.printf(
    "[I2C] SCL = GPIO%d\n",
    I2C_SCL
);

Serial.println("[I2C] READY");

// --------------------------------------------------------
// Sensor begin
// --------------------------------------------------------

Serial.println();
Serial.println("[2] Starting VL53L8CX...");

status = sensor.begin();

Serial.printf(
    "[VL53L8CX] begin() = %u\n",
    status
);

if (status != 0)
{
    Serial.println();
    Serial.println("ERROR: VL53L8CX begin FAILED");
    Serial.println();
    Serial.println("Check:");
    Serial.println("  - VIN");
    Serial.println("  - GND");
    Serial.println("  - SDA");
    Serial.println("  - SCL");
    Serial.println("  - SPI_I2C_N = GND");
    Serial.println();

    while (true)
    {
        delay(1000);
    }
}

Serial.println("[VL53L8CX] BEGIN OK");

// --------------------------------------------------------
// Initialize sensor
// --------------------------------------------------------

Serial.println();
Serial.println("[3] Initializing sensor...");
Serial.println("    Please wait...");

status = sensor.init();

Serial.printf(
    "[VL53L8CX] init() = %u\n",
    status
);

if (status != 0)
{
    Serial.println();
    Serial.println("ERROR: VL53L8CX init FAILED");
    Serial.println();

    while (true)
    {
        delay(1000);
    }
}

Serial.println("[VL53L8CX] INITIALIZED");

// --------------------------------------------------------
// Resolution
// --------------------------------------------------------

Serial.println();
Serial.println("[4] Setting resolution 8x8...");

status =
    sensor.set_resolution(
        VL53L8CX_RESOLUTION_8X8
    );

Serial.printf(
    "[VL53L8CX] set_resolution() = %u\n",
    status
);

if (status != 0)
{
    Serial.println(
        "ERROR: Failed to set 8x8 resolution"
    );

    while (true)
    {
        delay(1000);
    }
}

Serial.println(
    "[VL53L8CX] 8x8 RESOLUTION OK"
);

// --------------------------------------------------------
// Start ranging
// --------------------------------------------------------

Serial.println();
Serial.println("[5] Starting ranging...");

status =
    sensor.start_ranging();

Serial.printf(
    "[VL53L8CX] start_ranging() = %u\n",
    status
);

if (status != 0)
{
    Serial.println(
        "ERROR: start_ranging FAILED"
    );

    while (true)
    {
        delay(1000);
    }
}

Serial.println();
Serial.println("========================================");
Serial.println("        VL53L8CX RANGING STARTED");
Serial.println("========================================");

Serial.println();
Serial.println(
    "Move your hand/object in front of sensor."
);

Serial.println();
```

}

// ============================================================
// LOOP
// ============================================================

void loop()
{
dataReady = 0;

```
// --------------------------------------------------------
// Check new measurement
// --------------------------------------------------------

status =
    sensor.check_data_ready(
        &dataReady
    );

if (status != 0)
{
    Serial.printf(
        "[ERROR] check_data_ready = %u\n",
        status
    );

    delay(100);

    return;
}

if (!dataReady)
{
    delay(5);
    return;
}

// --------------------------------------------------------
// Read measurement
// --------------------------------------------------------

status =
    sensor.get_ranging_data(
        &results
    );

if (status != 0)
{
    Serial.printf(
        "[ERROR] get_ranging_data = %u\n",
        status
    );

    delay(100);

    return;
}

// --------------------------------------------------------
// Print matrix
// --------------------------------------------------------

printMatrix();

delay(100);
```

}
