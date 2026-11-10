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
      _cob4(cob4),
      _effects(
          cob1,
          cob2,
          cob3,
          cob4
      )
{
}

// ============================================================
// BEGIN
// ============================================================

bool CobLedManager::begin()
{
    bool result = true;

    result = _cob1.begin() && result;
    result = _cob2.begin() && result;
    result = _cob3.begin() && result;
    result = _cob4.begin() && result;

    _effects.begin();

    // При запуске COB выключены.
    _effects.setEnabled(false);

    return result;
}

// ============================================================
// UPDATE
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
// VALIDATION
// ============================================================

bool CobLedManager::valid(uint8_t cob) const
{
    return cob >= 1 && cob <= 4;
}

// ============================================================
// GET COB
// ============================================================

CobLed& CobLedManager::getCob(uint8_t cob)
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

const CobLed& CobLedManager::getCob(uint8_t cob) const
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
// SET / GET BRIGHTNESS
// ============================================================

void CobLedManager::set(
    uint8_t cob,
    uint8_t brightness
)
{
    if (!valid(cob))
        return;

    _effects.setBrightness(
        cob - 1,
        brightness
    );
}

uint8_t CobLedManager::get(uint8_t cob) const
{
    if (!valid(cob))
        return 0;

    return _effects.brightness(
        cob - 1
    );
}

// ============================================================
// INCREASE / DECREASE
// ============================================================

void CobLedManager::increase(
    uint8_t cob,
    uint8_t value
)
{
    if (!valid(cob))
        return;

    uint16_t brightness =
        static_cast<uint16_t>(get(cob)) + value;

    if (brightness > 100)
        brightness = 100;

    set(
        cob,
        static_cast<uint8_t>(brightness)
    );
}

void CobLedManager::decrease(
    uint8_t cob,
    uint8_t value
)
{
    if (!valid(cob))
        return;

    uint8_t brightness = get(cob);

    if (value >= brightness)
        brightness = 0;
    else
        brightness -= value;

    set(
        cob,
        brightness
    );
}

// ============================================================
// FADE
// ============================================================

void CobLedManager::fade(
    uint8_t cob,
    uint8_t target,
    uint32_t durationMs
)
{
    if (!valid(cob))
        return;

    target = constrain(
        target,
        0,
        100
    );

    // Плавное изменение используется в статическом режиме.
    // При динамических эффектах яркостью управляет CobEffects.

    if (
        _effects.effect() == CobEffectType::Static &&
        _effects.isEnabled() &&
        _effects.isChannelEnabled(cob - 1)
    )
    {
        // Сначала сохраняем целевую яркость,
        // но не применяем её немедленно.
        _effects.setBrightness(
            cob - 1,
            target,
            false
        );

        // Затем запускаем плавное изменение PWM.
        getCob(cob).fadeTo(
            target,
            durationMs
        );
    }
    else
    {
        set(
            cob,
            target
        );
    }
}

// ============================================================
// ON / OFF
// ============================================================

void CobLedManager::on(uint8_t cob)
{
    if (!valid(cob))
        return;

    uint8_t brightness = get(cob);

    if (brightness == 0)
        brightness = 100;

    _effects.setBrightness(
        cob - 1,
        brightness
    );

    _effects.setChannelEnabled(
        cob - 1,
        true
    );
}

void CobLedManager::off(uint8_t cob)
{
    if (!valid(cob))
        return;

    _effects.setChannelEnabled(
        cob - 1,
        false
    );
}

void CobLedManager::toggle(uint8_t cob)
{
    if (!valid(cob))
        return;

    if (isOn(cob))
        off(cob);
    else
        on(cob);
}

bool CobLedManager::isOn(uint8_t cob) const
{
    if (!valid(cob))
        return false;

    // Проверяем логическое состояние канала.
    // Не проверяем физическую яркость PWM:
    // например, в нижней точке Breath она может быть нулевой.

    return
        _effects.isEnabled() &&
        _effects.isChannelEnabled(cob - 1);
}

// ============================================================
// ALL CHANNELS
// ============================================================

void CobLedManager::setAll(uint8_t brightness)
{
    _effects.setAllBrightness(
        brightness
    );
}

void CobLedManager::onAll()
{
    _effects.setEnabled(true);
}

void CobLedManager::offAll()
{
    _effects.setEnabled(false);
}

// ============================================================
// EFFECT
// ============================================================

void CobLedManager::setEffect(uint8_t effect)
{
    if (
        effect >=
        static_cast<uint8_t>(CobEffectType::COUNT)
    )
    {
        effect = 0;
    }

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
// SPEED
// ============================================================

void CobLedManager::setSpeed(uint8_t speed)
{
    _effects.setSpeed(speed);
}

uint8_t CobLedManager::speed() const
{
    return _effects.speed();
}

// ============================================================
// MASTER ENABLE
// ============================================================

void CobLedManager::setEnabled(bool enabled)
{
    _effects.setEnabled(enabled);
}

bool CobLedManager::isEnabled() const
{
    return _effects.isEnabled();
}