#include "CobLedManager.h"


// ============================================================
// Constructor
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
// Begin
// ============================================================

void CobLedManager::begin()
{
    _cob1.begin();
    _cob2.begin();
    _cob3.begin();
    _cob4.begin();

    offAll();
}


// ============================================================
// Update
// ============================================================

void CobLedManager::update()
{
    _cob1.update();
    _cob2.update();
    _cob3.update();
    _cob4.update();
}


// ============================================================
// Get COB
// ============================================================

CobLed& CobLedManager::getCob(
    uint8_t cob
)
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


const CobLed& CobLedManager::getCob(
    uint8_t cob
) const
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
// Set
// ============================================================

void CobLedManager::set(
    uint8_t cob,
    uint8_t brightness
)
{
    getCob(cob)
        .setBrightness(
            brightness
        );
}


// ============================================================
// Get
// ============================================================

uint8_t CobLedManager::get(
    uint8_t cob
) const
{
    return getCob(cob)
        .getBrightness();
}


// ============================================================
// Increase
// ============================================================

void CobLedManager::increase(
    uint8_t cob,
    uint8_t value
)
{
    getCob(cob)
        .increase(value);
}


// ============================================================
// Decrease
// ============================================================

void CobLedManager::decrease(
    uint8_t cob,
    uint8_t value
)
{
    getCob(cob)
        .decrease(value);
}


// ============================================================
// Fade
// ============================================================

void CobLedManager::fade(
    uint8_t cob,
    uint8_t target,
    uint32_t durationMs
)
{
    getCob(cob)
        .fadeTo(
            target,
            durationMs
        );
}


// ============================================================
// ON
// ============================================================

void CobLedManager::on(
    uint8_t cob
)
{
    getCob(cob)
        .on();
}


// ============================================================
// OFF
// ============================================================

void CobLedManager::off(
    uint8_t cob
)
{
    getCob(cob)
        .off();
}


// ============================================================
// Toggle
// ============================================================

void CobLedManager::toggle(
    uint8_t cob
)
{
    getCob(cob)
        .toggle();
}


// ============================================================
// Set all
// ============================================================

void CobLedManager::setAll(
    uint8_t brightness
)
{
    _cob1.setBrightness(brightness);
    _cob2.setBrightness(brightness);
    _cob3.setBrightness(brightness);
    _cob4.setBrightness(brightness);
}


// ============================================================
// ON all
// ============================================================

void CobLedManager::onAll()
{
    _cob1.on();
    _cob2.on();
    _cob3.on();
    _cob4.on();
}


// ============================================================
// OFF all
// ============================================================

void CobLedManager::offAll()
{
    _cob1.off();
    _cob2.off();
    _cob3.off();
    _cob4.off();
}


// ============================================================
// Is ON
// ============================================================

bool CobLedManager::isOn(
    uint8_t cob
) const
{
    return getCob(cob)
        .isOn();
}
