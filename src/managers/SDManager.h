#pragma once

#include <Arduino.h>
#include "./hardware/sd/SDcard.h"

class SDManager
{
public:

    SDManager();

    bool begin(uint8_t csPin);
    void end();

    bool isReady() const;

    SDCard& card();
    const SDCard& card() const;

    SDCardInfo getInfo() const;

private:

    SDCard _card;

    bool _initialized;
};