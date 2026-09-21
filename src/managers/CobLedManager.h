#pragma once

#include <Arduino.h>

#include "./hardware/light/CobLed.h"

class CobLedManager
{
public:
    CobLedManager(
        CobLed& cob1,
        CobLed& cob2,
        CobLed& cob3,
        CobLed& cob4
    );

    void begin();
    void update();


    // ========================================================
    // ОБЩЕЕ УПРАВЛЕНИЕ
    // ========================================================

    void on();
    void off();
    void toggle();


    // ========================================================
    // ОБЩАЯ ЯРКОСТЬ
    // ========================================================

    void setBrightness(uint8_t brightness);

    void increaseBrightness(uint8_t step = 5);
    void decreaseBrightness(uint8_t step = 5);


    // ========================================================
    // ОТДЕЛЬНЫЙ COB
    // ========================================================

    void setBrightness(
        uint8_t cob,
        uint8_t brightness
    );

    void increaseBrightness(
        uint8_t cob,
        uint8_t step = 5
    );

    void decreaseBrightness(
        uint8_t cob,
        uint8_t step = 5
    );

    void on(uint8_t cob);
    void off(uint8_t cob);
    void toggle(uint8_t cob);


    // ========================================================
    // СОСТОЯНИЕ
    // ========================================================

    bool isOn(uint8_t cob) const;

    uint8_t brightness(uint8_t cob) const;


private:
    CobLed& _cob1;
    CobLed& _cob2;
    CobLed& _cob3;
    CobLed& _cob4;


    CobLed& getCob(uint8_t cob);
    const CobLed& getCob(uint8_t cob) const;
};
