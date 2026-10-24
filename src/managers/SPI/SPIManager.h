#pragma once

#include <Arduino.h>
#include <SPI.h>

class SPIManager
{
public:

    SPIManager();

    bool begin();

    void end();

    bool isReady() const;

    SPIClass& bus();

    void select(uint8_t csPin);
    void deselect(uint8_t csPin);

private:

    bool _initialized;
};