
#include "LightingManager.h"

#include "Pins.h"
#include "Config.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

LightingManager::LightingManager(
    SettingsManager& settings
)
    : _settings(settings),

      // ------------------------------------------------------
      // COB
      // ------------------------------------------------------

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

      // ------------------------------------------------------
      // MATRIX
      // ------------------------------------------------------

      _matrix(
          PIN_MATRIX
      ),

      _matrixBrightness(0),
      _matrixEffect(0),
      _matrixSpeed(0),
      _matrixEnabled(false),

      // ------------------------------------------------------
      // STATE
      // ------------------------------------------------------

      _initialized(false),
      _firstApply(true),
      _alarmOverride(false)
{
}

// ============================================================
// BEGIN
// ============================================================

bool LightingManager::begin()
{
    if (_initialized)
        return true;

    _cob.begin();
    _matrix.begin();

    _initialized = true;
    _firstApply = true;

    apply();

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void LightingManager::update()
{
    if (!_initialized)
        return;

    // Временно передали управление рассвету.
    // Нельзя применять обычные настройки или запускать
    // обычные световые эффекты до завершения override.
    if (_alarmOverride)
        return;

    apply();

    _cob.update();
    _matrix.update();
}

// ============================================================
// APPLY SETTINGS
// ============================================================

void LightingManager::apply()
{
    if (!_initialized || _alarmOverride)
        return;

    // ========================================================
    // READ COB SETTINGS
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
    // READ MATRIX SETTINGS
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
    // FIRST APPLY OR RESTORE
    // ========================================================

    if (_firstApply)
    {
        _cobBrightness = cobBrightness;
        _cobEffect = cobEffect;
        _cobSpeed = cobSpeed;
        _cobEnabled = cobEnabled;

        _cob.setEnabled(false);
        _cob.offAll();
        _cob.setAll(_cobBrightness);
        _cob.setSpeed(_cobSpeed);
        _cob.setEffect(_cobEffect);
        _cob.setEnabled(_cobEnabled);

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

// ============================================================
// BEGIN ALARM OVERRIDE
// ============================================================

void LightingManager::beginAlarmOverride()
{
    if (!_initialized || _alarmOverride)
        return;

    _alarmOverride = true;

    // Останавливаем обычный эффект матрицы.
    _matrix.stopEffect();

    // Отключаем обычное управление COB.
    _cob.setEnabled(false);
    _cob.offAll();

    // Готовим матрицу к непосредственному управлению.
    _matrix.setBrightness(255);
    _matrix.on();
    _matrix.clear();
    _matrix.show();
}

// ============================================================
// SET ALARM MATRIX
// ============================================================

void LightingManager::setAlarmMatrix(
    uint8_t brightness,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    if (!_initialized || !_alarmOverride)
        return;

    _matrix.on();

    _matrix.setBrightness(
        brightness
    );

    _matrix.fill(
        red,
        green,
        blue
    );

    _matrix.show();
}

// ============================================================
// SET ALARM COB
// ============================================================

void LightingManager::setAlarmCob(
    uint8_t brightness
)
{
    if (!_initialized || !_alarmOverride)
        return;

    _cob.setEnabled(true);

    for (uint8_t channel = 1; channel <= 4; ++channel)
    {
        _cob.set(
            channel,
            brightness
        );
    }
}

// ============================================================
// END ALARM OVERRIDE
// ============================================================

void LightingManager::endAlarmOverride()
{
    if (!_initialized || !_alarmOverride)
        return;

    // Сначала прекращаем временное управление.
    _alarmOverride = false;

    // Принудительно применяем актуальные настройки.
    // Это важно, поскольку внутренние кэши могут содержать
    // значения, которые были до рассвета.
    _firstApply = true;

    apply();
}

// ============================================================
// STATE
// ============================================================

bool LightingManager::isAlarmOverrideActive() const
{
    return _alarmOverride;
}