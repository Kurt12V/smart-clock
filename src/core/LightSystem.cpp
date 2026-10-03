#include "LightSystem.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

LightSystem::LightSystem(
    CobLedManager& cob,
    LedMatrixManager& matrix
)
    : _cob(cob),
      _matrix(matrix),

      _initialized(false),

      _enabled(true),

      _matrixEnabled(true),
      _cobEnabled(true),

      _brightness(255),

      _matrixBrightness(255),
      _cobBrightness(255)
{
}

// ============================================================
// BEGIN
// ============================================================

bool LightSystem::begin()
{
    if (_initialized)
        return true;

    Serial0.println("[LIGHT] Initializing light system...");

    // Matrix begin() возвращает void
    _matrix.begin();

    // COB manager возвращает bool
    if (!_cob.begin())
    {
        Serial0.println("[LIGHT] COB manager initialization failed");
        return false;
    }

    _initialized = true;

    applyState();

    Serial0.println("[LIGHT] Light system initialized");

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void LightSystem::update()
{
    if (!_initialized)
        return;

    _cob.update();
    _matrix.update();
}

// ============================================================
// MASTER ENABLE
// ============================================================

void LightSystem::setEnabled(
    bool enabled
)
{
    if (_enabled == enabled)
        return;

    _enabled = enabled;

    applyState();
}

// ============================================================
// IS ENABLED
// ============================================================

bool LightSystem::isEnabled() const
{
    return _enabled;
}

// ============================================================
// OUTPUT
// ============================================================

void LightSystem::setOutput(
    Output output
)
{
    switch (output)
    {
        case Output::None:

            _matrixEnabled = false;
            _cobEnabled = false;

            break;

        case Output::Matrix:

            _matrixEnabled = true;
            _cobEnabled = false;

            break;

        case Output::Cob:

            _matrixEnabled = false;
            _cobEnabled = true;

            break;

        case Output::Both:

            _matrixEnabled = true;
            _cobEnabled = true;

            break;

        default:

            _matrixEnabled = true;
            _cobEnabled = true;

            break;
    }

    applyState();
}

// ============================================================
// GET OUTPUT
// ============================================================

LightSystem::Output LightSystem::output() const
{
    if (_matrixEnabled && _cobEnabled)
        return Output::Both;

    if (_matrixEnabled)
        return Output::Matrix;

    if (_cobEnabled)
        return Output::Cob;

    return Output::None;
}

// ============================================================
// MATRIX ENABLED
// ============================================================

bool LightSystem::matrixEnabled() const
{
    return
        _enabled &&
        _matrixEnabled;
}

// ============================================================
// COB ENABLED
// ============================================================

bool LightSystem::cobEnabled() const
{
    return
        _enabled &&
        _cobEnabled;
}

// ============================================================
// ENABLE MATRIX
// ============================================================

void LightSystem::enableMatrix(
    bool enabled
)
{
    _matrixEnabled = enabled;

    applyMatrixState();
}

// ============================================================
// ENABLE COB
// ============================================================

void LightSystem::enableCob(
    bool enabled
)
{
    _cobEnabled = enabled;

    applyCobState();
}

// ============================================================
// GLOBAL BRIGHTNESS
// ============================================================

void LightSystem::setBrightness(
    uint8_t brightness
)
{
    _brightness = brightness;

    applyMatrixState();
    applyCobState();
}

// ============================================================
// GET GLOBAL BRIGHTNESS
// ============================================================

uint8_t LightSystem::brightness() const
{
    return _brightness;
}

// ============================================================
// MATRIX MANAGER
// ============================================================

LedMatrixManager& LightSystem::matrix()
{
    return _matrix;
}

const LedMatrixManager& LightSystem::matrix() const
{
    return _matrix;
}

// ============================================================
// COB MANAGER
// ============================================================

CobLedManager& LightSystem::cob()
{
    return _cob;
}

const CobLedManager& LightSystem::cob() const
{
    return _cob;
}

// ============================================================
// MATRIX BRIGHTNESS
// ============================================================

void LightSystem::setMatrixBrightness(
    uint8_t brightness
)
{
    _matrixBrightness = brightness;

    applyMatrixState();
}

// ============================================================
// MATRIX BRIGHTNESS GET
// ============================================================

uint8_t LightSystem::matrixBrightness() const
{
    return _matrixBrightness;
}

// ============================================================
// COB BRIGHTNESS
// ============================================================

void LightSystem::setCobBrightness(
    uint8_t brightness
)
{
    _cobBrightness = brightness;

    applyCobState();
}

// ============================================================
// COB BRIGHTNESS GET
// ============================================================

uint8_t LightSystem::cobBrightness() const
{
    return _cobBrightness;
}

// ============================================================
// ALL ON
// ============================================================

void LightSystem::on()
{
    _enabled = true;

    applyState();
}

// ============================================================
// ALL OFF
// ============================================================

void LightSystem::off()
{
    _enabled = false;

    applyState();
}

// ============================================================
// TOGGLE
// ============================================================

void LightSystem::toggle()
{
    setEnabled(!_enabled);
}

// ============================================================
// MATRIX ON
// ============================================================

void LightSystem::matrixOn()
{
    _matrixEnabled = true;

    applyMatrixState();
}

// ============================================================
// MATRIX OFF
// ============================================================

void LightSystem::matrixOff()
{
    _matrixEnabled = false;

    applyMatrixState();
}

// ============================================================
// COB ON
// ============================================================

void LightSystem::cobOn()
{
    _cobEnabled = true;

    applyCobState();
}

// ============================================================
// COB OFF
// ============================================================

void LightSystem::cobOff()
{
    _cobEnabled = false;

    applyCobState();
}

// ============================================================
// MATRIX STATE
// ============================================================

bool LightSystem::isMatrixOn() const
{
    return
        _enabled &&
        _matrixEnabled;
}

// ============================================================
// COB STATE
// ============================================================

bool LightSystem::isCobOn() const
{
    return
        _enabled &&
        _cobEnabled;
}

// ============================================================
// APPLY COMPLETE STATE
// ============================================================

void LightSystem::applyState()
{
    applyMatrixState();
    applyCobState();
}

// ============================================================
// APPLY MATRIX STATE
// ============================================================

void LightSystem::applyMatrixState()
{
    if (!_initialized)
        return;

    if (!_enabled || !_matrixEnabled)
    {
        _matrix.off();
        return;
    }

    _matrix.setBrightness(
        _matrixBrightness
    );

    _matrix.on();
}

// ============================================================
// APPLY COB STATE
// ============================================================

void LightSystem::applyCobState()
{
    if (!_initialized)
        return;

    if (!_enabled || !_cobEnabled)
    {
        _cob.offAll();
        return;
    }

    uint16_t brightness =
        (
            (uint16_t)_cobBrightness *
            (uint16_t)_brightness
        ) / 255;

    _cob.setAll(
        (uint8_t)brightness
    );

    _cob.onAll();
}