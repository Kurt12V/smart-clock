#include "InputManager.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

InputManager::InputManager()
    : _encoder(
          PIN_ENCODER_CLK,
          PIN_ENCODER_DT,
          PIN_ENCODER_SW
      ),
      _initialized(false)
{
}


// ============================================================
// BEGIN
// ============================================================

bool InputManager::begin()
{
    if (_initialized)
        return true;

    // --------------------------------------------------------
    // ENCODER
    // --------------------------------------------------------

    if (!_encoder.begin())
    {
        return false;
    }

    _initialized = true;

    return true;
}


// ============================================================
// UPDATE
// ============================================================

void InputManager::update()
{
    if (!_initialized)
        return;

    _encoder.update();
}


// ============================================================
// EVENTS
// ============================================================

Constants::Event InputManager::getEvent()
{
    if (!_initialized)
    {
        return Constants::Event::NONE;
    }

    return _encoder.getEvent();
}


// ============================================================
// POSITION
// ============================================================

int32_t InputManager::getPosition() const
{
    if (!_initialized)
        return 0;

    return _encoder.getPosition();
}


int32_t InputManager::getDelta()
{
    if (!_initialized)
        return 0;

    return _encoder.getDelta();
}


void InputManager::setPosition(
    int32_t position
)
{
    if (!_initialized)
        return;

    _encoder.setPosition(position);
}


void InputManager::resetPosition()
{
    if (!_initialized)
        return;

    _encoder.resetPosition();
}


// ============================================================
// BUTTON
// ============================================================

bool InputManager::isPressed() const
{
    if (!_initialized)
        return false;

    return _encoder.isPressed();
}


// ============================================================
// READY
// ============================================================

bool InputManager::isReady() const
{
    return _initialized;
}

