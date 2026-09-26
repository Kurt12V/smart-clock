#include "VL53L8CX.h"

VL53L8CXSensor::VL53L8CXSensor()
    : sensor(&Wire, -1),
      initialized(false),
      status(0),
      dataReady(0)
{
    for (uint8_t i = 0; i < ZONE_COUNT; i++)
    {
        distances[i] = 0;
        targets[i] = 0;
    }
}

bool VL53L8CXSensor::begin()
{
    Serial0.println();
    Serial0.println("================================");
    Serial0.println("          VL53L8CX");
    Serial0.println("================================");

    Serial0.printf(
        "[VL53L8CX] SDA = GPIO%d\n",
        I2C_SDA
    );

    Serial0.printf(
        "[VL53L8CX] SCL = GPIO%d\n",
        I2C_SCL
    );

    Serial0.println(
        "[VL53L8CX] I2C = 400 kHz"
    );

    // -------------------------
    // I2C
    // -------------------------

    Wire.begin(
        I2C_SDA,
        I2C_SCL
    );

    Wire.setClock(400000);

    // -------------------------
    // BEGIN
    // -------------------------

    Serial0.println(
        "[VL53L8CX] begin..."
    );

    status = sensor.begin();

    if (status != 0)
    {
        Serial0.print(
            "[VL53L8CX] begin FAILED, status = "
        );

        Serial0.println(status);

        initialized = false;

        return false;
    }

    Serial0.println(
        "[VL53L8CX] begin OK"
    );

    // -------------------------
    // INIT
    // -------------------------

    Serial0.println(
        "[VL53L8CX] init..."
    );

    status = sensor.init();

    if (status != 0)
    {
        Serial0.print(
            "[VL53L8CX] init FAILED, status = "
        );

        Serial0.println(status);

        initialized = false;

        return false;
    }

    Serial0.println(
        "[VL53L8CX] init OK"
    );

    // -------------------------
    // 8x8
    // -------------------------

    status =
        sensor.set_resolution(
            VL53L8CX_RESOLUTION_8X8
        );

    if (status != 0)
    {
        Serial0.print(
            "[VL53L8CX] resolution FAILED, status = "
        );

        Serial0.println(status);

        initialized = false;

        return false;
    }

    Serial0.println(
        "[VL53L8CX] Resolution: 8x8"
    );

    // -------------------------
    // RANGING
    // -------------------------

    status =
        sensor.start_ranging();

    if (status != 0)
    {
        Serial0.print(
            "[VL53L8CX] start ranging FAILED, status = "
        );

        Serial0.println(status);

        initialized = false;

        return false;
    }

    Serial0.println(
        "[VL53L8CX] Ranging STARTED"
    );

    initialized = true;

    return true;
}

bool VL53L8CXSensor::update()
{
    if (!initialized)
        return false;

    status =
        sensor.check_data_ready(
            &dataReady
        );

    if (status != 0)
    {
        return false;
    }

    if (!dataReady)
    {
        return false;
    }

    status =
        sensor.get_ranging_data(
            &results
        );

    if (status != 0)
    {
        return false;
    }

    copyResults();

    return true;
}

void VL53L8CXSensor::copyResults()
{
    for (uint8_t i = 0; i < ZONE_COUNT; i++)
    {
        distances[i] =
            results.distance_mm[i];

        targets[i] =
            results.nb_target_detected[i];
    }
}

bool VL53L8CXSensor::isInitialized() const
{
    return initialized;
}

bool VL53L8CXSensor::isDataReady() const
{
    return dataReady != 0;
}

int16_t VL53L8CXSensor::getDistance(
    uint8_t zone
) const
{
    if (zone >= ZONE_COUNT)
        return -1;

    return distances[zone];
}

uint8_t VL53L8CXSensor::getTargets(
    uint8_t zone
) const
{
    if (zone >= ZONE_COUNT)
        return 0;

    return targets[zone];
}

const int16_t* VL53L8CXSensor::getDistances() const
{
    return distances;
}

const uint8_t* VL53L8CXSensor::getTargets() const
{
    return targets;
}

const VL53L8CX_ResultsData&
VL53L8CXSensor::getResults() const
{
    return results;
}

uint8_t VL53L8CXSensor::getResolution() const
{
    return 64;
}

uint8_t VL53L8CXSensor::getZoneCount() const
{
    return ZONE_COUNT;
}

const char* VL53L8CXSensor::getName() const
{
    return "VL53L8CX";
}