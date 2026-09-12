#pragma once

#include <Arduino.h>

// #include "./managers/SPIManager.h"
// #include "./managers/SDManager.h"
// #include "DisplaySystem.h"
#include "../managers/SensorsManager.h"
#include "ClockSystem.h"
// #include "AudioSystem.h"
// #include "LEDManager.h"
// #include "EncoderManager.h"

class System
{
public:
    System();

    bool begin();
    void update();

    bool isReady() const;

    // SPIManager& spi();
    // SDManager& sd();
    // DisplaySystem& display();
    SensorManager& sensors();
    ClockSystem& clock();
    // AudioSystem& audio();
    // LEDManager& led();
    // EncoderManager& encoder();



private:
    Settings::Data _settings;
    ClockSystem _clock;
    bool _ready;

    // SPIManager _spi;
    // SDManager _sd;
    // DisplaySystem _display;
    SensorManager _sensors;
    // AudioSystem _audio;
    // LEDManager _led;
    // EncoderManager _encoder;
};