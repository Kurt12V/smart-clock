#include "LedMatrixManager.h"

#include "Config.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

LedMatrixManager::LedMatrixManager(
    uint8_t dataPin,
    SettingsManager& settings
)
    : _matrix(
        dataPin
    ),
      _effects(
          _matrix
      ),
      _settings(
          settings
      ),

      _mode(
          Mode::Effect
      ),

      _isOn(false),
      _initialized(false),

      _brightness(50),

      _effect(0),
      _speed(50),

      _r(255),
      _g(255),
      _b(255),

      _lastSettingsEnabled(false),
      _lastSettingsBrightness(255),
      _lastSettingsEffect(255),
      _lastSettingsSpeed(255)
{
}

// ============================================================
// BEGIN
// ============================================================

void LedMatrixManager::begin()
{
    _matrix.begin();

    _effects.begin();

    _initialized =
        true;

    // ----------------------------------------------------------
    // Read settings
    // ----------------------------------------------------------

    _isOn =
        _settings.isMatrixEnabled();

    _brightness =
        _settings.getMatrixBrightness();

    _effect =
        _settings.getMatrixEffect();

    _speed =
        _settings.getMatrixSpeed();

    // ----------------------------------------------------------
    // Cache
    // ----------------------------------------------------------

    _lastSettingsEnabled =
        _isOn;

    _lastSettingsBrightness =
        _brightness;

    _lastSettingsEffect =
        _effect;

    _lastSettingsSpeed =
        _speed;

    // ----------------------------------------------------------
    // Hardware brightness
    // ----------------------------------------------------------

    _matrix.setBrightness(
        _brightness
    );

    // ----------------------------------------------------------
    // Effect configuration
    // ----------------------------------------------------------

    _effects.setSpeed(
        _speed
    );

    _effects.setType(
        effectToType(
            _effect
        )
    );

    // ----------------------------------------------------------
    // Initial render
    // ----------------------------------------------------------

    if (!_isOn)
    {
        clear();
        show();
        return;
    }

    if (_mode == Mode::Lighting)
    {
        renderLighting();
    }
    else
    {
        _effects.update();
    }

    show();
}

// ============================================================
// UPDATE
// ============================================================

void LedMatrixManager::update()
{
    if (!_initialized)
        return;

    // ----------------------------------------------------------
    // Settings
    // ----------------------------------------------------------

    synchronizeSettings();

    // ----------------------------------------------------------
    // OFF
    // ----------------------------------------------------------

    if (!_isOn)
        return;

    // ----------------------------------------------------------
    // LIGHTING
    // ----------------------------------------------------------

    if (_mode == Mode::Lighting)
    {
        return;
    }

    // ----------------------------------------------------------
    // EFFECT
    // ----------------------------------------------------------

    uint32_t before =
        _matrix.pixel(0);

    _effects.update();

    /*
     * Effects render directly into the matrix buffer.
     *
     * We always show after update.
     *
     * LedMatrixEffects itself controls its
     * internal update interval.
     */

    (void)before;

    show();
}

// ============================================================
// SYNCHRONIZE SETTINGS
// ============================================================

void LedMatrixManager::synchronizeSettings()
{
    const bool newEnabled =
        _settings.isMatrixEnabled();

    const uint8_t newBrightness =
        _settings.getMatrixBrightness();

    const uint8_t newEffect =
        _settings.getMatrixEffect();

    const uint8_t newSpeed =
        _settings.getMatrixSpeed();

    // ==========================================================
    // ENABLE
    // ==========================================================

    if (
        newEnabled !=
        _lastSettingsEnabled
    )
    {
        _lastSettingsEnabled =
            newEnabled;

        _isOn =
            newEnabled;

        if (!_isOn)
        {
            clear();
            show();
            return;
        }
    }

    // ==========================================================
    // BRIGHTNESS
    // ==========================================================

    if (
        newBrightness !=
        _lastSettingsBrightness
    )
    {
        _lastSettingsBrightness =
            newBrightness;

        _brightness =
            newBrightness;

        _matrix.setBrightness(
            _brightness
        );
    }

    // ==========================================================
    // EFFECT
    // ==========================================================

    if (
        newEffect !=
        _lastSettingsEffect
    )
    {
        _lastSettingsEffect =
            newEffect;

        _effect =
            newEffect;

        _effects.setType(
            effectToType(
                _effect
            )
        );

        _mode =
            Mode::Effect;
    }

    // ==========================================================
    // SPEED
    // ==========================================================

    if (
        newSpeed !=
        _lastSettingsSpeed
    )
    {
        _lastSettingsSpeed =
            newSpeed;

        _speed =
            newSpeed;

        _effects.setSpeed(
            _speed
        );
    }
}

// ============================================================
// EFFECT CONVERSION
// ============================================================

LedMatrixEffects::Type
LedMatrixManager::effectToType(
    uint8_t effect
) const
{
    if (
        effect >=
        static_cast<uint8_t>(
            LedMatrixEffects::Type::COUNT
        )
    )
    {
        effect = 0;
    }

    return static_cast<
        LedMatrixEffects::Type
    >(effect);
}

// ============================================================
// RENDER LIGHTING
// ============================================================

void LedMatrixManager::renderLighting()
{
    _matrix.fill(
        _matrix.color(
            _r,
            _g,
            _b
        )
    );

    show();
}

// ============================================================
// ON
// ============================================================

void LedMatrixManager::on()
{
    _settings.setMatrixEnabled(
        true
    );

    _isOn =
        true;

    _lastSettingsEnabled =
        true;

    if (_mode == Mode::Lighting)
    {
        renderLighting();
    }
}

// ============================================================
// OFF
// ============================================================

void LedMatrixManager::off()
{
    _settings.setMatrixEnabled(
        false
    );

    _isOn =
        false;

    _lastSettingsEnabled =
        false;

    clear();

    show();
}

// ============================================================
// IS ON
// ============================================================

bool LedMatrixManager::isOn() const
{
    return _isOn;
}

// ============================================================
// SET BRIGHTNESS
// ============================================================

void LedMatrixManager::setBrightness(
    uint8_t brightness
)
{
    brightness =
        constrain(
            brightness,
            Config::MATRIX_BRIGHTNESS_MIN,
            Config::MATRIX_BRIGHTNESS_MAX
        );

    _settings.setMatrixBrightness(
        brightness
    );

    _brightness =
        brightness;

    _lastSettingsBrightness =
        brightness;

    _matrix.setBrightness(
        brightness
    );
}

// ============================================================
// GET BRIGHTNESS
// ============================================================

uint8_t LedMatrixManager::brightness() const
{
    return _brightness;
}

// ============================================================
// SET LIGHTING
// ============================================================

void LedMatrixManager::setLighting(
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    _r = r;
    _g = g;
    _b = b;

    _mode =
        Mode::Lighting;

    if (!_isOn)
        return;

    renderLighting();
}

// ============================================================
// RGB GETTERS
// ============================================================

uint8_t LedMatrixManager::red() const
{
    return _r;
}

uint8_t LedMatrixManager::green() const
{
    return _g;
}

uint8_t LedMatrixManager::blue() const
{
    return _b;
}

// ============================================================
// SET EFFECT
// ============================================================

void LedMatrixManager::setEffect(
    uint8_t effect
)
{
    effect =
        constrain(
            effect,
            Config::MATRIX_EFFECT_MIN,
            Config::MATRIX_EFFECT_MAX
        );

    _settings.setMatrixEffect(
        effect
    );

    _effect =
        effect;

    _lastSettingsEffect =
        effect;

    _mode =
        Mode::Effect;

    _effects.setType(
        effectToType(
            effect
        )
    );

    _effects.setSpeed(
        _speed
    );

    if (_isOn)
    {
        clear();
        show();
    }
}

// ============================================================
// GET EFFECT
// ============================================================

uint8_t LedMatrixManager::effect() const
{
    return _effect;
}

// ============================================================
// SET SPEED
// ============================================================

void LedMatrixManager::setSpeed(
    uint8_t speed
)
{
    speed =
        constrain(
            speed,
            Config::MATRIX_SPEED_MIN,
            Config::MATRIX_SPEED_MAX
        );

    _settings.setMatrixSpeed(
        speed
    );

    _speed =
        speed;

    _lastSettingsSpeed =
        speed;

    _effects.setSpeed(
        speed
    );
}

// ============================================================
// GET SPEED
// ============================================================

uint8_t LedMatrixManager::speed() const
{
    return _speed;
}

// ============================================================
// FIRE DIRECTION BOOL
// ============================================================

void LedMatrixManager::setFireDirection(
    bool forward
)
{
    setFireDirection(
        forward
            ? LedMatrixEffects::FireDirection::LeftToRight
            : LedMatrixEffects::FireDirection::RightToLeft
    );
}

// ============================================================
// FIRE DIRECTION ENUM
// ============================================================

void LedMatrixManager::setFireDirection(
    LedMatrixEffects::FireDirection direction
)
{
    _effects.setFireDirection(
        direction
    );
}

// ============================================================
// SET MODE
// ============================================================

void LedMatrixManager::setMode(
    Mode mode
)
{
    if (_mode == mode)
        return;

    _mode =
        mode;

    if (_mode == Mode::Effect)
    {
        _effects.setType(
            effectToType(
                _effect
            )
        );

        _effects.setSpeed(
            _speed
        );
    }
    else
    {
        if (_isOn)
            renderLighting();
    }
}

// ============================================================
// GET MODE
// ============================================================

LedMatrixManager::Mode
LedMatrixManager::mode() const
{
    return _mode;
}

// ============================================================
// EFFECTS
// ============================================================

LedMatrixEffects&
LedMatrixManager::effects()
{
    return _effects;
}

const LedMatrixEffects&
LedMatrixManager::effects() const
{
    return _effects;
}

// ============================================================
// MATRIX
// ============================================================

LedMatrix&
LedMatrixManager::matrix()
{
    return _matrix;
}

const LedMatrix&
LedMatrixManager::matrix() const
{
    return _matrix;
}

// ============================================================
// CLEAR
// ============================================================

void LedMatrixManager::clear()
{
    _matrix.clear();
}

// ============================================================
// SHOW
// ============================================================

void LedMatrixManager::show()
{
    _matrix.show();
}