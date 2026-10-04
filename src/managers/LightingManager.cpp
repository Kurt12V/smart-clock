
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

      _initialized(false)
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

    _cob.update();

    apply();
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


    // --------------------------------------------------------
    // Brightness
    // --------------------------------------------------------

    _cob.setAll(
        brightness
    );


    // --------------------------------------------------------
    // Effect
    // --------------------------------------------------------

    _cob.setEffect(
        effect
    );


    // --------------------------------------------------------
    // Speed
    // --------------------------------------------------------

    _cob.setSpeed(
        speed
    );


    // --------------------------------------------------------
    // Enabled
    // --------------------------------------------------------

    if (enabled)
        _cob.onAll();
    else
        _cob.offAll();
}
