
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

      // COB
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

      // MATRIX
      _matrix(PIN_MATRIX),

      _matrixBrightness(0),
      _matrixEffect(0),
      _matrixSpeed(0),
      _matrixEnabled(false),

      // STATE
      _initialized(false),
      _firstApply(true),
      _alarmOverride(false)
{
    Serial0.println("[Lighting] Constructor completed");
}

// ============================================================
// BEGIN
// ============================================================

bool LightingManager::begin()
{
    Serial0.println("[Lighting] Initializing...");

    if (_initialized)
    {
        Serial0.println("[Lighting] Already initialized");
        return true;
    }

    Serial0.println("[Lighting] Initializing COB...");
    bool cobResult = _cob.begin();

    Serial0.print("[Lighting] COB initialization: ");
    Serial0.println(cobResult ? "SUCCESS" : "FAILED");

    Serial0.println("[Lighting] Initializing LED matrix...");
    _matrix.begin();

    Serial0.println("[Lighting] LED matrix initialized");

    _initialized = true;
    _firstApply = true;

    Serial0.println("[Lighting] Applying initial settings...");
    apply();

    Serial0.println("[Lighting] Initialization completed");

    return cobResult;
}

// ============================================================
// UPDATE
// ============================================================

void LightingManager::update()
{
    if (!_initialized)
        return;

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
    if (!_initialized)
    {
        Serial0.println("[Lighting] Apply skipped: not initialized");
        return;
    }

    if (_alarmOverride)
    {
        Serial0.println("[Lighting] Apply skipped: alarm override active");
        return;
    }

    // Serial0.println("[Lighting] Applying COB settings...");
    applyCobSettings(_firstApply);

    // Serial0.println("[Lighting] Applying matrix settings...");
    applyMatrixSettings(_firstApply);

    _firstApply = false;

    // Serial0.println("[Lighting] Settings applied");
}

// ============================================================
// APPLY COB SETTINGS
// ============================================================

void LightingManager::applyCobSettings(bool force)
{
    const uint8_t brightness =
        static_cast<uint8_t>(
            _settings.get(
                SettingsManager::Param::COB_BRIGHTNESS
            )
        );

    const uint8_t effect =
        static_cast<uint8_t>(
            _settings.get(
                SettingsManager::Param::COB_EFFECT
            )
        );

    const uint8_t speed =
        static_cast<uint8_t>(
            _settings.get(
                SettingsManager::Param::COB_SPEED
            )
        );

    const bool enabled =
        _settings.get(
            SettingsManager::Param::COB_ENABLED
        ) != 0;

    // BRIGHTNESS

    if (force || brightness != _cobBrightness)
    {
        _cobBrightness = brightness;

        Serial0.print("[Lighting][COB] Brightness: ");
        Serial0.println(_cobBrightness);

        _cob.setAll(_cobBrightness);
    }

    // EFFECT

    if (force || effect != _cobEffect)
    {
        _cobEffect = effect;

        Serial0.print("[Lighting][COB] Effect: ");
        Serial0.println(_cobEffect);

        _cob.setEffect(_cobEffect);
    }

    // SPEED

    if (force || speed != _cobSpeed)
    {
        _cobSpeed = speed;

        Serial0.print("[Lighting][COB] Speed: ");
        Serial0.println(_cobSpeed);

        _cob.setSpeed(_cobSpeed);
    }

    // ENABLE

    if (force || enabled != _cobEnabled)
    {
        _cobEnabled = enabled;

        Serial0.print("[Lighting][COB] Enabled: ");
        Serial0.println(_cobEnabled ? "YES" : "NO");

        _cob.setEnabled(_cobEnabled);
    }
}

// ============================================================
// APPLY MATRIX SETTINGS
// ============================================================

void LightingManager::applyMatrixSettings(bool force)
{
    const uint8_t brightness =
        static_cast<uint8_t>(
            _settings.get(
                SettingsManager::Param::MATRIX_BRIGHTNESS
            )
        );

    const uint8_t effect =
        static_cast<uint8_t>(
            _settings.get(
                SettingsManager::Param::MATRIX_EFFECT
            )
        );

    const uint8_t speed =
        static_cast<uint8_t>(
            _settings.get(
                SettingsManager::Param::MATRIX_SPEED
            )
        );

    const bool enabled =
        _settings.get(
            SettingsManager::Param::MATRIX_ENABLED
        ) != 0;

    // BRIGHTNESS

    if (force || brightness != _matrixBrightness)
    {
        _matrixBrightness = brightness;

        Serial0.print("[Lighting][Matrix] Brightness: ");
        Serial0.println(_matrixBrightness);

        _matrix.setBrightness(_matrixBrightness);
    }

    // EFFECT

    if (force || effect != _matrixEffect)
    {
        _matrixEffect = effect;

        Serial0.print("[Lighting][Matrix] Effect: ");
        Serial0.println(_matrixEffect);

        _matrix.setEffect(
            static_cast<LedMatrixManager::Effect>(
                _matrixEffect
            )
        );
    }

    // SPEED

    if (force || speed != _matrixSpeed)
    {
        _matrixSpeed = speed;

        Serial0.print("[Lighting][Matrix] Speed: ");
        Serial0.println(_matrixSpeed);

        _matrix.setEffectSpeed(_matrixSpeed);
    }

    // ENABLE

    if (force || enabled != _matrixEnabled)
    {
        _matrixEnabled = enabled;

        Serial0.print("[Lighting][Matrix] Enabled: ");
        Serial0.println(_matrixEnabled ? "YES" : "NO");

        if (_matrixEnabled)
        {
            Serial0.println("[Lighting][Matrix] Turning ON");
            _matrix.on();
        }
        else
        {
            Serial0.println("[Lighting][Matrix] Turning OFF");
            _matrix.off();
        }
    }
}

// ============================================================
// BEGIN ALARM OVERRIDE
// ============================================================

void LightingManager::beginAlarmOverride()
{
    Serial0.println("[Lighting][Alarm] Starting alarm override");

    if (!_initialized)
    {
        Serial0.println("[Lighting][Alarm] Failed: not initialized");
        return;
    }

    if (_alarmOverride)
    {
        Serial0.println("[Lighting][Alarm] Override already active");
        return;
    }

    _alarmOverride = true;

    Serial0.println("[Lighting][Alarm] Stopping matrix effect");
    _matrix.stopEffect();

    Serial0.println("[Lighting][Alarm] Disabling COB");
    _cob.setEnabled(false);

    Serial0.println("[Lighting][Alarm] Turning off all COB channels");
    _cob.offAll();

    Serial0.println("[Lighting][Alarm] Setting matrix brightness to 255");
    _matrix.setBrightness(255);

    Serial0.println("[Lighting][Alarm] Turning matrix ON");
    _matrix.on();

    Serial0.println("[Lighting][Alarm] Clearing matrix");
    _matrix.clear();

    Serial0.println("[Lighting][Alarm] Updating matrix display");
    _matrix.show();

    Serial0.println("[Lighting][Alarm] Override started");
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
    {
        Serial0.println("[Lighting][Alarm] Matrix update skipped: override inactive");
        return;
    }

    Serial0.print("[Lighting][Alarm] Matrix brightness: ");
    Serial0.println(brightness);

    _matrix.on();

    _matrix.setBrightness(brightness);

    Serial0.print("[Lighting][Alarm] Matrix RGB: ");
    Serial0.print(red);
    Serial0.print(", ");
    Serial0.print(green);
    Serial0.print(", ");
    Serial0.println(blue);

    _matrix.fill(
        red,
        green,
        blue
    );

    _matrix.show();

    Serial0.println("[Lighting][Alarm] Matrix updated");
}

// ============================================================
// SET ALARM COB
// ============================================================

void LightingManager::setAlarmCob(uint8_t brightness)
{
    if (!_initialized || !_alarmOverride)
    {
        Serial0.println("[Lighting][Alarm] COB update skipped: override inactive");
        return;
    }

    Serial0.println("[Lighting][Alarm] Enabling COB");
    _cob.setEnabled(true);

    Serial0.print("[Lighting][Alarm] COB brightness: ");
    Serial0.println(brightness);

    for (uint8_t channel = 1; channel <= 4; channel++)
    {
        Serial0.print("[Lighting][Alarm] Setting COB channel ");
        Serial0.print(channel);
        Serial0.print(" brightness: ");
        Serial0.println(brightness);

        _cob.set(
            channel,
            brightness
        );
    }

    Serial0.println("[Lighting][Alarm] COB updated");
}

// ============================================================
// END ALARM OVERRIDE
// ============================================================

void LightingManager::endAlarmOverride()
{
    Serial0.println("[Lighting][Alarm] Ending alarm override");

    if (!_initialized)
    {
        Serial0.println("[Lighting][Alarm] Failed: not initialized");
        return;
    }

    if (!_alarmOverride)
    {
        Serial0.println("[Lighting][Alarm] Override is not active");
        return;
    }

    _alarmOverride = false;

    Serial0.println("[Lighting][Alarm] Restoring normal lighting settings");

    _firstApply = true;

    apply();

    Serial0.println("[Lighting][Alarm] Normal lighting restored");
}

// ============================================================
// STATE
// ============================================================

bool LightingManager::isAlarmOverrideActive() const
{
    return _alarmOverride;
}
