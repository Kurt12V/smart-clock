#pragma once

#include <Arduino.h>
#include "Constants.h"

class EncoderManager
{
public:
    EncoderManager(
        uint8_t clkPin,
        uint8_t dtPin,
        uint8_t swPin
    );

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

    bool wasPressed();
    bool wasReleased();
    bool wasLongPressed();
    bool wasDoublePressed();

private:
    // ========================================================
    // ROTATION
    // ========================================================

    static void IRAM_ATTR encoderISR();

    void processRotation();

    // ========================================================
    // BUTTON
    // ========================================================

    void processButton();

    // ========================================================
    // EVENTS
    // ========================================================

    void addEvent(Constants::Event event);

private:
    // ========================================================
    // PINS
    // ========================================================

    uint8_t _clkPin;
    uint8_t _dtPin;
    uint8_t _swPin;

    // ========================================================
    // STATE
    // ========================================================

    bool _initialized;
    bool _buttonPressed;

    // ========================================================
    // ROTATION
    // ========================================================

    volatile int32_t _position;

    int32_t _lastPosition;
    int32_t _reportedPosition;

    volatile uint8_t _lastCLK;

    // ========================================================
    // BUTTON TIMING
    // ========================================================

    uint32_t _buttonDownTime;
    uint32_t _lastButtonChange;
    uint32_t _lastReleaseTime;

    // ========================================================
    // BUTTON STATE
    // ========================================================

    bool _longPressTriggered;

    // TRUE = ждём второе нажатие
    bool _doublePressPending;

    // ========================================================
    // EVENT QUEUE
    // ========================================================

    static constexpr uint8_t EVENT_QUEUE_SIZE = 16;

    Constants::Event _eventQueue[EVENT_QUEUE_SIZE];

    uint8_t _eventHead;
    uint8_t _eventTail;

    // ========================================================
    // TIMING
    // ========================================================

    static constexpr uint32_t DEBOUNCE_MS = 30;

    static constexpr uint32_t LONG_PRESS_MS = 800;

    static constexpr uint32_t DOUBLE_PRESS_MS = 350;
};

