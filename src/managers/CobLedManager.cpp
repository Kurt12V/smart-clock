#include "CobLedManager.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

CobLedManager::CobLedManager(
    CobLed& cob1,
    CobLed& cob2,
    CobLed& cob3,
    CobLed& cob4,
    SettingsManager& settings
)
    : _cob1(cob1),
      _cob2(cob2),
      _cob3(cob3),
      _cob4(cob4),

      _settings(settings),

      _effects(
          cob1,
          cob2,
          cob3,
          cob4
      ),

      _mode(Mode::Normal),

      _initialized(false),

      _lastBrightness(255),
      _lastEnabled(false),
      _lastEffect(255),
      _lastSpeed(255)
{
}

// ============================================================
// BEGIN
// ============================================================

bool CobLedManager::begin()
{
    bool result1 =
        _cob1.begin();

    bool result2 =
        _cob2.begin();

    bool result3 =
        _cob3.begin();

    bool result4 =
        _cob4.begin();

    _effects.begin();

    _initialized = true;

    // Force initial SettingsManager application.
    _lastBrightness = 255;
    _lastEnabled = false;
    _lastEffect = 255;
    _lastSpeed = 255;

    applySettings();

    return
        result1 &&
        result2 &&
        result3 &&
        result4;
}

// ============================================================
// UPDATE
// ============================================================

void CobLedManager::update()
{
    if (!_initialized)
        return;

    if (_mode == Mode::Effect)
    {
        updateEffects();
        return;
    }

    updateNormal();
}

// ============================================================
// NORMAL UPDATE
// ============================================================

void CobLedManager::updateNormal()
{
    _cob1.update();
    _cob2.update();
    _cob3.update();
    _cob4.update();

    applySettings();
}

// ============================================================
// EFFECT UPDATE
// ============================================================

void CobLedManager::updateEffects()
{
    _effects.update();
}

// ============================================================
// GET COB
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

// ============================================================
// GET COB CONST
// ============================================================

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
// SET
// ============================================================

void CobLedManager::set(
    uint8_t cob,
    uint8_t brightness
)
{
    if (cob < 1 || cob > 4)
        return;

    if (_mode == Mode::Effect)
        restoreNormalMode();

    getCob(cob).setBrightness(
        brightness
    );
}

// ============================================================
// GET
// ============================================================

uint8_t CobLedManager::get(
    uint8_t cob
) const
{
    if (cob < 1 || cob > 4)
        return 0;

    return getCob(cob).getBrightness();
}

// ============================================================
// INCREASE
// ============================================================

void CobLedManager::increase(
    uint8_t cob,
    uint8_t value
)
{
    if (cob < 1 || cob > 4)
        return;

    if (_mode == Mode::Effect)
        restoreNormalMode();

    getCob(cob).increase(value);
}

// ============================================================
// DECREASE
// ============================================================

void CobLedManager::decrease(
    uint8_t cob,
    uint8_t value
)
{
    if (cob < 1 || cob > 4)
        return;

    if (_mode == Mode::Effect)
        restoreNormalMode();

    getCob(cob).decrease(value);
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
    if (cob < 1 || cob > 4)
        return;

    if (_mode == Mode::Effect)
        restoreNormalMode();

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
    if (cob < 1 || cob > 4)
        return;

    if (_mode == Mode::Effect)
        restoreNormalMode();

    getCob(cob).on();
}

// ============================================================
// OFF
// ============================================================

void CobLedManager::off(
    uint8_t cob
)
{
    if (cob < 1 || cob > 4)
        return;

    if (_mode == Mode::Effect)
        restoreNormalMode();

    getCob(cob).off();
}

// ============================================================
// TOGGLE
// ============================================================

void CobLedManager::toggle(
    uint8_t cob
)
{
    if (cob < 1 || cob > 4)
        return;

    if (_mode == Mode::Effect)
        restoreNormalMode();

    getCob(cob).toggle();
}

// ============================================================
// IS ON
// ============================================================

bool CobLedManager::isOn(
    uint8_t cob
) const
{
    if (cob < 1 || cob > 4)
        return false;

    return getCob(cob).isOn();
}

// ============================================================
// SET ALL
// ============================================================

void CobLedManager::setAll(
    uint8_t brightness
)
{
    if (_mode == Mode::Effect)
        restoreNormalMode();

    _cob1.setBrightness(brightness);
    _cob2.setBrightness(brightness);
    _cob3.setBrightness(brightness);
    _cob4.setBrightness(brightness);
}

// ============================================================
// ON ALL
// ============================================================

void CobLedManager::onAll()
{
    if (_mode == Mode::Effect)
        restoreNormalMode();

    _cob1.on();
    _cob2.on();
    _cob3.on();
    _cob4.on();
}

// ============================================================
// OFF ALL
// ============================================================

void CobLedManager::offAll()
{
    if (_mode == Mode::Effect)
        restoreNormalMode();

    _cob1.off();
    _cob2.off();
    _cob3.off();
    _cob4.off();
}

// ============================================================
// SET EFFECT
// ============================================================

void CobLedManager::setEffect(
    CobEffectType effect
)
{
    _effects.setEffect(
        effect
    );

    _mode =
        Mode::Effect;

    _effects.setEnabled(true);
}

// ============================================================
// GET EFFECT
// ============================================================

CobEffectType CobLedManager::getEffect() const
{
    return _effects.effect();
}

// ============================================================
// SET SPEED
// ============================================================

void CobLedManager::setSpeed(
    uint8_t speed
)
{
    _effects.setSpeed(speed);
}

// ============================================================
// GET SPEED
// ============================================================

uint8_t CobLedManager::getSpeed() const
{
    return _effects.speed();
}

// ============================================================
// ENABLE EFFECTS
// ============================================================

void CobLedManager::enableEffects()
{
    _effects.setEnabled(true);

    _mode =
        Mode::Effect;
}

// ============================================================
// DISABLE EFFECTS
// ============================================================

void CobLedManager::disableEffects()
{
    _effects.setEnabled(false);

    restoreNormalMode();
}

// ============================================================
// EFFECTS ENABLED
// ============================================================

bool CobLedManager::effectsEnabled() const
{
    return _effects.isEnabled();
}

// ============================================================
// EFFECT ENGINE
// ============================================================

CobEffects& CobLedManager::effects()
{
    return _effects;
}

const CobEffects& CobLedManager::effects() const
{
    return _effects;
}

// ============================================================
// SET MODE
// ============================================================

void CobLedManager::setMode(
    Mode mode
)
{
    if (_mode == mode)
        return;

    _mode = mode;

    if (_mode == Mode::Normal)
    {
        _effects.setEnabled(false);

        applySettings();
    }
    else
    {
        _effects.setEnabled(true);
    }
}

// ============================================================
// GET MODE
// ============================================================

CobLedManager::Mode CobLedManager::getMode() const
{
    return _mode;
}

// ============================================================
// IS EFFECT MODE
// ============================================================

bool CobLedManager::isEffectMode() const
{
    return _mode == Mode::Effect;
}

// ============================================================
// APPLY SETTINGS
// ============================================================

void CobLedManager::applySettings()
{
    if (!_initialized)
        return;

    // --------------------------------------------------------
    // SettingsManager controls only NORMAL mode.
    // --------------------------------------------------------

    if (_mode != Mode::Normal)
        return;

    const bool enabled =
        _settings.isCobEnabled();

    const uint8_t brightness =
        _settings.getCobBrightness();

    const uint8_t effect =
        _settings.getCobEffect();

    const uint8_t speed =
        _settings.getCobSpeed();

    // --------------------------------------------------------
    // ENABLE
    // --------------------------------------------------------

    if (enabled != _lastEnabled)
    {
        _lastEnabled =
            enabled;

        if (!enabled)
        {
            _cob1.off();
            _cob2.off();
            _cob3.off();
            _cob4.off();
        }
        else
        {
            _cob1.setBrightness(brightness);
            _cob2.setBrightness(brightness);
            _cob3.setBrightness(brightness);
            _cob4.setBrightness(brightness);
        }
    }

    // --------------------------------------------------------
    // BRIGHTNESS
    // --------------------------------------------------------

    if (brightness != _lastBrightness)
    {
        _lastBrightness =
            brightness;

        if (enabled)
        {
            _cob1.setBrightness(brightness);
            _cob2.setBrightness(brightness);
            _cob3.setBrightness(brightness);
            _cob4.setBrightness(brightness);
        }
    }

    // --------------------------------------------------------
    // EFFECT SETTING
    //
    // In normal mode Static is the normal state.
    //
    // Non-static effects from SettingsManager are supported,
    // but they do NOT take control of the manager.
    //
    // If you want an effect to run continuously, use
    // setEffect().
    // --------------------------------------------------------

    if (effect != _lastEffect)
    {
        _lastEffect =
            effect;

        if (effect <
            static_cast<uint8_t>(
                CobEffectType::COUNT
            ))
        {
            _effects.setEffect(
                static_cast<CobEffectType>(
                    effect
                )
            );
        }
    }

    // --------------------------------------------------------
    // SPEED
    // --------------------------------------------------------

    if (speed != _lastSpeed)
    {
        _lastSpeed =
            speed;

        _effects.setSpeed(
            speed
        );
    }
}

// ============================================================
// RESTORE NORMAL MODE
// ============================================================

void CobLedManager::restoreNormalMode()
{
    _mode =
        Mode::Normal;

    _effects.setEnabled(false);

    // Force SettingsManager to be applied again.
    _lastBrightness = 255;
    _lastEnabled = false;
    _lastEffect = 255;
    _lastSpeed = 255;

    applySettings();
}