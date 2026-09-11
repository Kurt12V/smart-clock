#include "CobLedManager.h"

CobLedManager::CobLedManager(
    CobLed& cob1,
    CobLed& cob2
)
    : _cob1(cob1),
      _cob2(cob2)
{
}

void CobLedManager::begin()
{
    _cob1.begin();
    _cob2.begin();
}

void CobLedManager::update()
{
    // В будущем:
    // - автояркость от VEML7700
    // - ночной режим
    // - плавное изменение
    // - расписание
}

CobLed& CobLedManager::getCob(uint8_t cob)
{
    return (cob == 1) ? _cob1 : _cob2;
}

const CobLed& CobLedManager::getCob(uint8_t cob) const
{
    return (cob == 1) ? _cob1 : _cob2;
}


// =========================
// ОБА COB
// =========================

void CobLedManager::on()
{
    _cob1.on();
    _cob2.on();
}

void CobLedManager::off()
{
    _cob1.off();
    _cob2.off();
}

void CobLedManager::toggle()
{
    _cob1.toggle();
    _cob2.toggle();
}

void CobLedManager::setBrightness(uint8_t brightness)
{
    _cob1.setBrightness(brightness);
    _cob2.setBrightness(brightness);
}

void CobLedManager::increaseBrightness(uint8_t step)
{
    _cob1.increase(step);
    _cob2.increase(step);
}

void CobLedManager::decreaseBrightness(uint8_t step)
{
    _cob1.decrease(step);
    _cob2.decrease(step);
}


// =========================
// ОТДЕЛЬНЫЙ COB
// =========================

void CobLedManager::setBrightness(
    uint8_t cob,
    uint8_t brightness
)
{
    getCob(cob).setBrightness(brightness);
}

void CobLedManager::on(uint8_t cob)
{
    getCob(cob).on();
}

void CobLedManager::off(uint8_t cob)
{
    getCob(cob).off();
}

void CobLedManager::toggle(uint8_t cob)
{
    getCob(cob).toggle();
}


// =========================
// СОСТОЯНИЕ
// =========================

bool CobLedManager::isOn(uint8_t cob) const
{
    return getCob(cob).isOn();
}

uint8_t CobLedManager::brightness(uint8_t cob) const
{
    return getCob(cob).getBrightness();
}