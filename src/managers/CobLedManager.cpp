#include "CobLedManager.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

CobLedManager::CobLedManager(
    CobLed& cob1,
    CobLed& cob2,
    CobLed& cob3,
    CobLed& cob4
)
    : _cob1(cob1),
      _cob2(cob2),
      _cob3(cob3),
      _cob4(cob4)
{
}


// ============================================================
// BEGIN
// ============================================================

void CobLedManager::begin()
{
    _cob1.begin();
    _cob2.begin();
    _cob3.begin();
    _cob4.begin();
}


// ============================================================
// UPDATE
// ============================================================

void CobLedManager::update()
{
    // Будущее:
    //
    // - плавное изменение яркости
    // - автояркость
    // - ночной режим
    // - расписание
    // - сцены
    // - fade in / fade out
    // - управление от будильника
    // - аварийное мигание
}


// ============================================================
// GET COB
// ============================================================

CobLed& CobLedManager::getCob(uint8_t cob)
{
    switch (cob)
    {
        case 1:
            return _cob1;

        case 2:
            return _cob2;

        case 3:
            return _cob3;

        case 4:
            return _cob4;

        default:
            // Для защиты от неправильного номера.
            //
            // В дальнейшем можно сделать отдельную
            // проверку/enum.
            return _cob1;
    }
}


const CobLed& CobLedManager::getCob(uint8_t cob) const
{
    switch (cob)
    {
        case 1:
            return _cob1;

        case 2:
            return _cob2;

        case 3:
            return _cob3;

        case 4:
            return _cob4;

        default:
            return _cob1;
    }
}


// ============================================================
// ВСЕ COB — ON
// ============================================================

void CobLedManager::on()
{
    _cob1.on();
    _cob2.on();
    _cob3.on();
    _cob4.on();
}


// ============================================================
// ВСЕ COB — OFF
// ============================================================

void CobLedManager::off()
{
    _cob1.off();
    _cob2.off();
    _cob3.off();
    _cob4.off();
}


// ============================================================
// ВСЕ COB — TOGGLE
// ============================================================

void CobLedManager::toggle()
{
    _cob1.toggle();
    _cob2.toggle();
    _cob3.toggle();
    _cob4.toggle();
}


// ============================================================
// ОБЩАЯ ЯРКОСТЬ
// ============================================================

void CobLedManager::setBrightness(uint8_t brightness)
{
    _cob1.setBrightness(brightness);
    _cob2.setBrightness(brightness);
    _cob3.setBrightness(brightness);
    _cob4.setBrightness(brightness);
}


// ============================================================
// УВЕЛИЧИТЬ ЯРКОСТЬ ВСЕХ
// ============================================================

void CobLedManager::increaseBrightness(uint8_t step)
{
    _cob1.increase(step);
    _cob2.increase(step);
    _cob3.increase(step);
    _cob4.increase(step);
}


// ============================================================
// УМЕНЬШИТЬ ЯРКОСТЬ ВСЕХ
// ============================================================

void CobLedManager::decreaseBrightness(uint8_t step)
{
    _cob1.decrease(step);
    _cob2.decrease(step);
    _cob3.decrease(step);
    _cob4.decrease(step);
}


// ============================================================
// ОТДЕЛЬНЫЙ COB — BRIGHTNESS
// ============================================================

void CobLedManager::setBrightness(
    uint8_t cob,
    uint8_t brightness
)
{
    getCob(cob).setBrightness(brightness);
}


// ============================================================
// ОТДЕЛЬНЫЙ COB — INCREASE
// ============================================================

void CobLedManager::increaseBrightness(
    uint8_t cob,
    uint8_t step
)
{
    getCob(cob).increase(step);
}


// ============================================================
// ОТДЕЛЬНЫЙ COB — DECREASE
// ============================================================

void CobLedManager::decreaseBrightness(
    uint8_t cob,
    uint8_t step
)
{
    getCob(cob).decrease(step);
}


// ============================================================
// ОТДЕЛЬНЫЙ COB — ON
// ============================================================

void CobLedManager::on(uint8_t cob)
{
    getCob(cob).on();
}


// ============================================================
// ОТДЕЛЬНЫЙ COB — OFF
// ============================================================

void CobLedManager::off(uint8_t cob)
{
    getCob(cob).off();
}


// ============================================================
// ОТДЕЛЬНЫЙ COB — TOGGLE
// ============================================================

void CobLedManager::toggle(uint8_t cob)
{
    getCob(cob).toggle();
}


// ============================================================
// СОСТОЯНИЕ
// ============================================================

bool CobLedManager::isOn(uint8_t cob) const
{
    return getCob(cob).isOn();
}


// ============================================================
// ЯРКОСТЬ
// ============================================================

uint8_t CobLedManager::brightness(uint8_t cob) const
{
    return getCob(cob).getBrightness();
}
