#include "LightingManager.h"

#include "Pins.h"
#include "Config.h"


// ============================================================
// Constructor
// ============================================================

LightingManager::LightingManager(
    SettingsManager& settings
)
    : _settings(settings),

      _cob1(
          PIN_COB1,
          1,
          Config::COB_PWM_FREQUENCY,
          Config::COB_PWM_RESOLUTION
      ),

      _cob2(
          PIN_COB2,
          2,
          Config::COB_PWM_FREQUENCY,
          Config::COB_PWM_RESOLUTION
      ),

      _cob3(
          PIN_COB3,
          3,
          Config::COB_PWM_FREQUENCY,
          Config::COB_PWM_RESOLUTION
      ),

      _cob4(
          PIN_COB4,
          4,
          Config::COB_PWM_FREQUENCY,
          Config::COB_PWM_RESOLUTION
      ),

      _cob(
          _cob1,
          _cob2,
          _cob3,
          _cob4
      ),

      _initialized(false),
      _firstApply(true),
      _brightness(0),
      _effect(0),
      _speed(0),
      _enabled(false)
{
}


// ============================================================
// Begin
// ============================================================

bool LightingManager::begin()
{
    _cob.begin();

    _initialized = true;

    apply();

    return true;
}


// ============================================================
// Update
// ============================================================

void LightingManager::update()
{
    if (!_initialized)
        return;

    // Только проверяет изменения настроек.
    // Сам эффект продолжает работать внутри CobLedManager.
    apply();

    _cob.update();
}


// ============================================================
// Apply settings
// ============================================================

void LightingManager::apply()
{
    if (!_initialized)
        return;

    const uint8_t brightness =
        static_cast<uint8_t>(
            _settings.get(
                Param::COB_BRIGHTNESS
            )
        );

    const uint8_t effect =
        static_cast<uint8_t>(
            _settings.get(
                Param::COB_EFFECT
            )
        );

    const uint8_t speed =
        static_cast<uint8_t>(
            _settings.get(
                Param::COB_SPEED
            )
        );

    const bool enabled =
        _settings.get(
            Param::COB_ENABLED
        ) != 0;


    // ========================================================
    // First apply
    // ========================================================

    if (_firstApply)
    {
        _brightness = brightness;
        _effect = effect;
        _speed = speed;
        _enabled = enabled;

        _cob.setAll(_brightness);
        _cob.setEffect(_effect);
        _cob.setSpeed(_speed);
        _cob.setEnabled(_enabled);

        _firstApply = false;

        return;
    }


    // ========================================================
    // Brightness changed
    // ========================================================

    if (_brightness != brightness)
    {
        _brightness = brightness;

        _cob.setAll(
            _brightness
        );
    }


    // ========================================================
    // Effect changed
    // ========================================================

    if (_effect != effect)
    {
        _effect = effect;

        _cob.setEffect(
            _effect
        );
    }


    // ========================================================
    // Speed changed
    // ========================================================

    if (_speed != speed)
    {
        _speed = speed;

        _cob.setSpeed(
            _speed
        );
    }


    // ========================================================
    // Enabled changed
    // ========================================================

    if (_enabled != enabled)
    {
        _enabled = enabled;

        _cob.setEnabled(
            _enabled
        );
    }
}
