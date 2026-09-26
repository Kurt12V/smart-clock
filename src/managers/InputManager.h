
#pragma once

#include <Arduino.h>

#include "Constants.h"
#include "Pins.h"
#include "managers/EncoderManager.h"

class InputManager
{
public:
    InputManager();

    bool begin();
    void update();

    bool isReady() const;

    // ========================================================
    // EVENTS
    // ========================================================

    Constants::Event getEvent();

    // ========================================================
    // ROTATION
    // ========================================================

    int32_t getPosition() const;
    int32_t getDelta();

    void setPosition(int32_t position);
    void resetPosition();

    // ========================================================
    // BUTTON
    // ========================================================

    bool isPressed() const;

private:
    // ========================================================
    // ENCODER
    // ========================================================

    EncoderManager _encoder;

    // ========================================================
    // STATE
    // ========================================================

    bool _initialized;
};
