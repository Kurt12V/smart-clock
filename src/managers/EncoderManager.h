#pragma once

#include <Arduino.h>

class EncoderManager
{
public:

    // ========================================================
    // EVENTS
    // ========================================================

    enum class Event : uint8_t
    {
        NONE,

        ROTATE_CW,
        ROTATE_CCW,

        PRESS,
        RELEASE,

        LONG_PRESS,
        DOUBLE_PRESS
    };


    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    EncoderManager(
        uint8_t clkPin,
        uint8_t dtPin,
        uint8_t swPin
    );


    // ========================================================
    // LIFECYCLE
    // ========================================================

    bool begin();

    void update();

    bool isReady() const;


    // ========================================================
    // EVENTS
    // ========================================================

    Event getEvent();


    // ========================================================
    // ROTATION
    // ========================================================

    int32_t getPosition() const;

    int32_t getDelta();

    void setPosition(
        int32_t position
    );

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
    // INTERRUPT
    // ========================================================

    static void IRAM_ATTR encoderISR();


    // ========================================================
    // INTERNAL
    // ========================================================

    void processRotation();

    void processButton();

    void addEvent(
        Event event
    );


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

    bool _doublePressPending;


    // ========================================================
    // EVENT QUEUE
    // ========================================================

    static constexpr uint8_t EVENT_QUEUE_SIZE = 16;

    Event _eventQueue[EVENT_QUEUE_SIZE];

    uint8_t _eventHead;

    uint8_t _eventTail;


    // ========================================================
    // SETTINGS
    // ========================================================

    static constexpr uint32_t DEBOUNCE_MS = 30;

    static constexpr uint32_t LONG_PRESS_MS = 800;

    static constexpr uint32_t DOUBLE_PRESS_MS = 350;
};
