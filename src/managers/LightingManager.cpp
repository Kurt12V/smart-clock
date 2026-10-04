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

      // ======================================================
      // COB
      // ======================================================

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

      _cobBrightness(0),
      _cobEffect(0),
      _cobSpeed(0),
      _cobEnabled(false),

      // ======================================================
      // MATRIX
      // ======================================================

      _matrix(
          PIN_MATRIX
      ),

      _matrixBrightness(0),
      _matrixEffect(0),
      _matrixSpeed(0),
      _matrixEnabled(false),

      // ======================================================
      // STATE
      // ======================================================

      _initialized(false),
      _firstApply(true)
{
}


// ============================================================
// Begin
// ============================================================

bool LightingManager::begin()
{
    _cob.begin();
    _matrix.begin();

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

    // Сначала применяем изменения настроек.
    apply();

    // Затем запускаем эффекты.
    _cob.update();
    _matrix.update();
}


// ============================================================
// Apply settings
// ============================================================

void LightingManager::apply()
{
    if (!_initialized)
        return;


    // ========================================================
    // COB SETTINGS
    // ========================================================

    const uint8_t cobBrightness =
        static_cast<uint8_t>(
            _settings.get(
                Param::COB_BRIGHTNESS
            )
        );

    const uint8_t cobEffect =
        static_cast<uint8_t>(
            _settings.get(
                Param::COB_EFFECT
            )
        );

    const uint8_t cobSpeed =
        static_cast<uint8_t>(
            _settings.get(
                Param::COB_SPEED
            )
        );

    const bool cobEnabled =
        _settings.get(
            Param::COB_ENABLED
        ) != 0;


    // ========================================================
    // MATRIX SETTINGS
    // ========================================================

    const uint8_t matrixBrightness =
        static_cast<uint8_t>(
            _settings.get(
                Param::MATRIX_BRIGHTNESS
            )
        );

    const uint8_t matrixEffect =
        static_cast<uint8_t>(
            _settings.get(
                Param::MATRIX_EFFECT
            )
        );

    const uint8_t matrixSpeed =
        static_cast<uint8_t>(
            _settings.get(
                Param::MATRIX_SPEED
            )
        );

    const bool matrixEnabled =
        _settings.get(
            Param::MATRIX_ENABLED
        ) != 0;


    // ========================================================
    // FIRST APPLY
    // ========================================================

    if (_firstApply)
    {
        // ----------------------------------------------------
        // COB
        // ----------------------------------------------------

        _cobBrightness = cobBrightness;
        _cobEffect = cobEffect;
        _cobSpeed = cobSpeed;
        _cobEnabled = cobEnabled;

        _cob.setAll(_cobBrightness);
        _cob.setEffect(_cobEffect);
        _cob.setSpeed(_cobSpeed);
        _cob.setEnabled(_cobEnabled);


        // ----------------------------------------------------
        // MATRIX
        // ----------------------------------------------------

        _matrixBrightness = matrixBrightness;
        _matrixEffect = matrixEffect;
        _matrixSpeed = matrixSpeed;
        _matrixEnabled = matrixEnabled;

        _matrix.setBrightness(_matrixBrightness);
        _matrix.setEffect(
            static_cast<LedMatrixManager::Effect>(
                _matrixEffect
            )
        );
        _matrix.setEffectSpeed(_matrixSpeed);

        if (_matrixEnabled)
            _matrix.on();
        else
            _matrix.off();


        _firstApply = false;

        return;
    }


    // ========================================================
    // COB BRIGHTNESS
    // ========================================================

    if (_cobBrightness != cobBrightness)
    {
        _cobBrightness = cobBrightness;

        _cob.setAll(
            _cobBrightness
        );
    }


    // ========================================================
    // COB EFFECT
    // ========================================================

    if (_cobEffect != cobEffect)
    {
        _cobEffect = cobEffect;

        _cob.setEffect(
            _cobEffect
        );
    }


    // ========================================================
    // COB SPEED
    // ========================================================

    if (_cobSpeed != cobSpeed)
    {
        _cobSpeed = cobSpeed;

        _cob.setSpeed(
            _cobSpeed
        );
    }


    // ========================================================
    // COB ENABLED
    // ========================================================

    if (_cobEnabled != cobEnabled)
    {
        _cobEnabled = cobEnabled;

        _cob.setEnabled(
            _cobEnabled
        );
    }


    // ========================================================
    // MATRIX BRIGHTNESS
    // ========================================================

    if (_matrixBrightness != matrixBrightness)
    {
        _matrixBrightness = matrixBrightness;

        _matrix.setBrightness(
            _matrixBrightness
        );
    }


    // ========================================================
    // MATRIX EFFECT
    // ========================================================

    if (_matrixEffect != matrixEffect)
    {
        _matrixEffect = matrixEffect;

        _matrix.setEffect(
            static_cast<LedMatrixManager::Effect>(
                _matrixEffect
            )
        );
    }


    // ========================================================
    // MATRIX SPEED
    // ========================================================

    if (_matrixSpeed != matrixSpeed)
    {
        _matrixSpeed = matrixSpeed;

        _matrix.setEffectSpeed(
            _matrixSpeed
        );
    }


    // ========================================================
    // MATRIX ENABLED
    // ========================================================

    if (_matrixEnabled != matrixEnabled)
    {
        _matrixEnabled = matrixEnabled;

        if (_matrixEnabled)
            _matrix.on();
        else
            _matrix.off();
    }
}