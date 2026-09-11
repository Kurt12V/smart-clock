#pragma once

#include <Arduino.h>
#include "CobLed.h"

class CobLedManager
{
public:
    CobLedManager(
        CobLed& cob1,
        CobLed& cob2
    );

    void begin();
    void update();

    // Общее управление двумя COB
    void on();
    void off();
    void toggle();

    // Общая яркость
    void setBrightness(uint8_t brightness);
    void increaseBrightness(uint8_t step = 5);
    void decreaseBrightness(uint8_t step = 5);

    // Управление отдельно
    void setBrightness(uint8_t cob, uint8_t brightness);

    void on(uint8_t cob);
    void off(uint8_t cob);
    void toggle(uint8_t cob);

    // Состояние
    bool isOn(uint8_t cob) const;
    uint8_t brightness(uint8_t cob) const;

private:
    CobLed& _cob1;
    CobLed& _cob2;

    CobLed& getCob(uint8_t cob);
    const CobLed& getCob(uint8_t cob) const;
};