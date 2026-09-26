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
    // Управление отдельным COB
    // ========================================================

    void set(
        uint8_t cob,
        uint8_t brightness
    );

    uint8_t get(
        uint8_t cob
    ) const;

    void increase(
        uint8_t cob,
        uint8_t value
    );

    void decrease(
        uint8_t cob,
        uint8_t value
    );

    void fade(
        uint8_t cob,
        uint8_t target,
        uint32_t durationMs
    );

    void on(
        uint8_t cob
    );

    void off(
        uint8_t cob
    );

    void toggle(
        uint8_t cob
    );

    // ========================================================
    // Все COB
    // ========================================================

    void setAll(
        uint8_t brightness
    );

    void onAll();

    void offAll();

    // ========================================================
    // Состояние
    // ========================================================

    bool isOn(
        uint8_t cob
    ) const;

private:

    CobLed& _cob1;
    CobLed& _cob2;
    CobLed& _cob3;
    CobLed& _cob4;

    CobLed& getCob(
        uint8_t cob
    );

    const CobLed& getCob(
        uint8_t cob
    ) const;
};
