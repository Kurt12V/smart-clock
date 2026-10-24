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
      _cob4(cob4),
      _effects(
          _cob1,
          _cob2,
          _cob3,
          _cob4
      )
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

    _effects.begin();

    offAll();
}


// ============================================================
// Update
// ============================================================

void CobLedManager::update()
{
    _effects.update();

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
        case 1: return _cob1;
        case 2: return _cob2;
        case 3: return _cob3;
        case 4: return _cob4;

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
        case 1: return _cob1;
        case 2: return _cob2;
        case 3: return _cob3;
        case 4: return _cob4;

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
    getCob(cob).setBrightness(
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
    return getCob(cob).getBrightness();
}


// ============================================================
// Increase
// ============================================================

void CobLedManager::increase(
    uint8_t cob,
    uint8_t value
)
{
    getCob(cob).increase(value);
}


// ============================================================
// Decrease
// ============================================================

void CobLedManager::decrease(
    uint8_t cob,
    uint8_t value
)
{
    getCob(cob).decrease(value);
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
    getCob(cob).fadeTo(
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
    getCob(cob).on();
}


// ============================================================
// OFF
// ============================================================

void CobLedManager::off(
    uint8_t cob
)
{
    getCob(cob).off();
}


// ============================================================
// Toggle
// ============================================================

void CobLedManager::toggle(
    uint8_t cob
)
{
    getCob(cob).toggle();
}


// ============================================================
// Is ON
// ============================================================

bool CobLedManager::isOn(
    uint8_t cob
) const
{
    return getCob(cob).isOn();
}


// ============================================================
// Set all
// ============================================================

void CobLedManager::setAll(
    uint8_t brightness
)
{
    _effects.setAllBrightness(
        brightness
    );
}


// ============================================================
// ON all
// ============================================================

void CobLedManager::onAll()
{
    _effects.setEnabled(true);
}


// ============================================================
// OFF all
// ============================================================

void CobLedManager::offAll()
{
    _effects.setEnabled(false);

    _cob1.off();
    _cob2.off();
    _cob3.off();
    _cob4.off();
}


// ============================================================
// Effect
// ============================================================

void CobLedManager::setEffect(
    uint8_t effect
)
{
    _effects.setEffect(
        static_cast<CobEffectType>(effect)
    );
}


uint8_t CobLedManager::effect() const
{
    return static_cast<uint8_t>(
        _effects.effect()
    );
}


// ============================================================
// Speed
// ============================================================

void CobLedManager::setSpeed(
    uint8_t speed
)
{
    _effects.setSpeed(speed);
}


uint8_t CobLedManager::speed() const
{
    return _effects.speed();
}


// ============================================================
// Enabled
// ============================================================

void CobLedManager::setEnabled(
    bool enabled
)
{
    _effects.setEnabled(enabled);

    if (!enabled)
    {
        _cob1.off();
        _cob2.off();
        _cob3.off();
        _cob4.off();
    }
}


bool CobLedManager::isEnabled() const
{
    return _effects.isEnabled();
}
