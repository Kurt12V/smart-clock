#include "EncoderManager.h"


// ============================================================
// GLOBAL ENCODER INSTANCE
// ============================================================

static EncoderManager* g_encoderInstance = nullptr;


// ============================================================
// CONSTRUCTOR
// ============================================================

EncoderManager::EncoderManager(
    uint8_t clkPin,
    uint8_t dtPin,
    uint8_t swPin
)
    : _clkPin(clkPin),
      _dtPin(dtPin),
      _swPin(swPin),

      _initialized(false),

      _buttonPressed(false),

      _position(0),
      _lastPosition(0),
      _reportedPosition(0),

      _lastCLK(HIGH),

      _buttonDownTime(0),
      _lastButtonChange(0),
      _lastReleaseTime(0),

      _longPressTriggered(false),
      _doublePressPending(false),

      _eventHead(0),
      _eventTail(0)
{
    for (uint8_t i = 0; i < EVENT_QUEUE_SIZE; i++)
    {
        _eventQueue[i] = Event::NONE;
    }
}


// ============================================================
// BEGIN
// ============================================================

bool EncoderManager::begin()
{
    if (_initialized)
        return true;


    // --------------------------------------------------------
    // GPIO
    // --------------------------------------------------------

    pinMode(
        _clkPin,
        INPUT_PULLUP
    );

    pinMode(
        _dtPin,
        INPUT_PULLUP
    );

    pinMode(
        _swPin,
        INPUT_PULLUP
    );


    // --------------------------------------------------------
    // Initial encoder state
    // --------------------------------------------------------

    _lastCLK =
        digitalRead(_clkPin);

    _position = 0;

    _lastPosition = 0;

    _reportedPosition = 0;


    // --------------------------------------------------------
    // Initial button state
    // --------------------------------------------------------

    _buttonPressed =
        digitalRead(_swPin) == LOW;

    _buttonDownTime =
        millis();

    _lastButtonChange =
        millis();

    _lastReleaseTime = 0;

    _longPressTriggered = false;

    _doublePressPending = false;


    // --------------------------------------------------------
    // Save instance for ISR
    // --------------------------------------------------------

    g_encoderInstance =
        this;


    // --------------------------------------------------------
    // Encoder interrupt
    // --------------------------------------------------------

    attachInterrupt(
        digitalPinToInterrupt(_clkPin),
        EncoderManager::encoderISR,
        CHANGE
    );


    _initialized = true;

    return true;
}


// ============================================================
// ENCODER ISR
// ============================================================

void IRAM_ATTR EncoderManager::encoderISR()
{
    if (g_encoderInstance == nullptr)
        return;


    EncoderManager* encoder =
        g_encoderInstance;


    uint8_t clk =
        digitalRead(
            encoder->_clkPin
        );

    uint8_t dt =
        digitalRead(
            encoder->_dtPin
        );


    // --------------------------------------------------------
    // Detect CLK transition
    // --------------------------------------------------------

    if (clk == encoder->_lastCLK)
        return;


    encoder->_lastCLK = clk;


    // --------------------------------------------------------
    // Count only rising edge
    // --------------------------------------------------------

    if (clk != HIGH)
        return;


    // --------------------------------------------------------
    // Direction
    // --------------------------------------------------------

    if (dt == LOW)
    {
        encoder->_position++;
    }
    else
    {
        encoder->_position--;
    }
}


// ============================================================
// UPDATE
// ============================================================

void EncoderManager::update()
{
    if (!_initialized)
        return;


    processRotation();

    processButton();
}


// ============================================================
// PROCESS ROTATION
// ============================================================

void EncoderManager::processRotation()
{
    int32_t position =
        _position;


    int32_t delta =
        position - _lastPosition;


    if (delta == 0)
        return;


    _lastPosition =
        position;


    // --------------------------------------------------------
    // Clockwise
    // --------------------------------------------------------

    if (delta > 0)
    {
        for (int32_t i = 0; i < delta; i++)
        {
            addEvent(
                Event::ROTATE_CW
            );
        }
    }


    // --------------------------------------------------------
    // Counter-clockwise
    // --------------------------------------------------------

    else
    {
        for (int32_t i = 0; i < -delta; i++)
        {
            addEvent(
                Event::ROTATE_CCW
            );
        }
    }
}


// ============================================================
// PROCESS BUTTON
// ============================================================

void EncoderManager::processButton()
{
    uint32_t now =
        millis();


    bool currentPressed =
        digitalRead(_swPin) == LOW;


    // ========================================================
    // BUTTON STATE CHANGE
    // ========================================================

    if (currentPressed != _buttonPressed)
    {
        // ----------------------------------------------------
        // Debounce
        // ----------------------------------------------------

        if (
            now - _lastButtonChange
            < DEBOUNCE_MS
        )
        {
            return;
        }


        _lastButtonChange =
            now;


        _buttonPressed =
            currentPressed;


        // ====================================================
        // BUTTON PRESSED
        // ====================================================

        if (_buttonPressed)
        {
            _buttonDownTime =
                now;

            _longPressTriggered =
                false;


            addEvent(
                Event::PRESS
            );

            return;
        }


        // ====================================================
        // BUTTON RELEASED
        // ====================================================

        addEvent(
            Event::RELEASE
        );


        // ----------------------------------------------------
        // Check second click
        // ----------------------------------------------------

        if (
            _doublePressPending &&
            _lastReleaseTime != 0 &&
            (now - _lastReleaseTime)
                <= DOUBLE_PRESS_MS
        )
        {
            // -----------------------------------------------
            // Double click detected
            // -----------------------------------------------

            _doublePressPending =
                false;

            addEvent(
                Event::DOUBLE_PRESS
            );
        }
        else
        {
            // -----------------------------------------------
            // First click
            // -----------------------------------------------

            _doublePressPending =
                true;
        }


        // ----------------------------------------------------
        // Save current release time
        // ----------------------------------------------------

        _lastReleaseTime =
            now;
    }


    // ========================================================
    // LONG PRESS
    // ========================================================

    if (
        _buttonPressed &&
        !_longPressTriggered
    )
    {
        if (
            now - _buttonDownTime
            >= LONG_PRESS_MS
        )
        {
            _longPressTriggered =
                true;


            addEvent(
                Event::LONG_PRESS
            );
        }
    }


    // ========================================================
    // DOUBLE CLICK TIMEOUT
    // ========================================================

    if (
        _doublePressPending &&
        !_buttonPressed &&
        _lastReleaseTime != 0
    )
    {
        if (
            now - _lastReleaseTime
            > DOUBLE_PRESS_MS
        )
        {
            _doublePressPending =
                false;
        }
    }
}


// ============================================================
// ADD EVENT
// ============================================================

void EncoderManager::addEvent(
    Event event
)
{
    if (event == Event::NONE)
        return;


    uint8_t next =
        (_eventHead + 1)
        % EVENT_QUEUE_SIZE;


    // --------------------------------------------------------
    // Queue full
    // --------------------------------------------------------

    if (next == _eventTail)
        return;


    _eventQueue[_eventHead] =
        event;


    _eventHead =
        next;
}


// ============================================================
// GET EVENT
// ============================================================

EncoderManager::Event
EncoderManager::getEvent()
{
    if (
        _eventTail ==
        _eventHead
    )
    {
        return Event::NONE;
    }


    Event event =
        _eventQueue[_eventTail];


    _eventQueue[_eventTail] =
        Event::NONE;


    _eventTail =
        (_eventTail + 1)
        % EVENT_QUEUE_SIZE;


    return event;
}


// ============================================================
// GET POSITION
// ============================================================

int32_t EncoderManager::getPosition() const
{
    return _position;
}


// ============================================================
// GET DELTA
// ============================================================

int32_t EncoderManager::getDelta()
{
    int32_t position =
        _position;


    int32_t delta =
        position - _reportedPosition;


    _reportedPosition =
        position;


    return delta;
}


// ============================================================
// SET POSITION
// ============================================================

void EncoderManager::setPosition(
    int32_t position
)
{
    _position =
        position;

    _lastPosition =
        position;

    _reportedPosition =
        position;
}


// ============================================================
// RESET POSITION
// ============================================================

void EncoderManager::resetPosition()
{
    setPosition(0);
}


// ============================================================
// BUTTON STATE
// ============================================================

bool EncoderManager::isPressed() const
{
    return _buttonPressed;
}


// ============================================================
// WAS PRESSED
// ============================================================

bool EncoderManager::wasPressed()
{
    return getEvent()
        == Event::PRESS;
}


// ============================================================
// WAS RELEASED
// ============================================================

bool EncoderManager::wasReleased()
{
    return getEvent()
        == Event::RELEASE;
}


// ============================================================
// WAS LONG PRESSED
// ============================================================

bool EncoderManager::wasLongPressed()
{
    return getEvent()
        == Event::LONG_PRESS;
}


// ============================================================
// WAS DOUBLE PRESSED
// ============================================================

bool EncoderManager::wasDoublePressed()
{
    return getEvent()
        == Event::DOUBLE_PRESS;
}


// ============================================================
// READY
// ============================================================

bool EncoderManager::isReady() const
{
    return _initialized;
}
