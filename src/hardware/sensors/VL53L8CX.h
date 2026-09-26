#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <vl53l8cx.h>

class VL53L8CXSensor
{
public:
    VL53L8CXSensor();

    bool begin();
    bool update();

    bool isInitialized() const;
    bool isDataReady() const;

    int16_t getDistance(uint8_t zone) const;
    uint8_t getTargets(uint8_t zone) const;

    const int16_t* getDistances() const;
    const uint8_t* getTargets() const;

    const VL53L8CX_ResultsData& getResults() const;

    uint8_t getResolution() const;
    uint8_t getZoneCount() const;

    const char* getName() const;

private:
    static constexpr uint8_t I2C_SDA = 8;
    static constexpr uint8_t I2C_SCL = 9;

    static constexpr uint8_t ZONE_COUNT = 64;

    VL53L8CX sensor;
    VL53L8CX_ResultsData results;

    bool initialized;

    int16_t distances[ZONE_COUNT];
    uint8_t targets[ZONE_COUNT];

    uint8_t status;
    uint8_t dataReady;

    void copyResults();
};