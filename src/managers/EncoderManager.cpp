#include "EncoderManager.h"

// ============================================================
// GLOBAL INSTANCE FOR ISR
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
        _eventQueue[i] = Constants::Event::NONE;
    }
}


// ============================================================
// BEGIN
// ============================================================

bool EncoderManager::begin()
{
    if (_initialized)
        return true;

    pinMode(_clkPin, INPUT_PULLUP);
    pinMode(_dtPin, INPUT_PULLUP);
    pinMode(_swPin, INPUT_PULLUP);

    // --------------------------------------------------------
    // ROTARY
    // --------------------------------------------------------

    _lastCLK = digitalRead(_clkPin);

    _position = 0;
    _lastPosition = 0;
    _reportedPosition = 0;

    // --------------------------------------------------------
    // BUTTON
    // --------------------------------------------------------

    _buttonPressed =
        digitalRead(_swPin) == LOW;

    uint32_t now = millis();

    _buttonDownTime = now;
    _lastButtonChange = now;
    _lastReleaseTime = 0;

    _longPressTriggered = false;
    _doublePressPending = false;

    // --------------------------------------------------------
    // EVENT QUEUE
    // --------------------------------------------------------

    _eventHead = 0;
    _eventTail = 0;

    for (uint8_t i = 0; i < EVENT_QUEUE_SIZE; i++)
    {
        _eventQueue[i] = Constants::Event::NONE;
    }

    // --------------------------------------------------------
    // ISR
    // --------------------------------------------------------

    g_encoderInstance = this;

    attachInterrupt(
        digitalPinToInterrupt(_clkPin),
        EncoderManager::encoderISR,
        CHANGE
    );

    _initialized = true;

    return true;
}


// ============================================================
// ROTARY ISR
// ============================================================

void IRAM_ATTR EncoderManager::encoderISR()
{
    if (g_encoderInstance == nullptr)
        return;

    EncoderManager* encoder =
        g_encoderInstance;

    uint8_t clk =
        digitalRead(encoder->_clkPin);

    uint8_t dt =
        digitalRead(encoder->_dtPin);

    if (clk == encoder->_lastCLK)
        return;

    encoder->_lastCLK = clk;

    // --------------------------------------------------------
    // Только фронт HIGH
    // --------------------------------------------------------

    if (clk != HIGH)
        return;

    // --------------------------------------------------------
    // Направление
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
// ROTATION
// ============================================================

void EncoderManager::processRotation()
{
    int32_t position = _position;

    int32_t delta =
        position - _lastPosition;

    if (delta == 0)
        return;

    _lastPosition = position;

    // --------------------------------------------------------
    // CLOCKWISE
    // --------------------------------------------------------

    if (delta > 0)
    {
        for (int32_t i = 0; i < delta; i++)
        {
            addEvent(
                Constants::Event::ROTATE_CW
            );
        }
    }

    // --------------------------------------------------------
    // COUNTER CLOCKWISE
    // --------------------------------------------------------

    else
    {
        for (int32_t i = 0; i < -delta; i++)
        {
            addEvent(
                Constants::Event::ROTATE_CCW
            );
        }
    }
}


// ============================================================
// BUTTON
// ============================================================

void EncoderManager::processButton()
{
    uint32_t now = millis();

    bool currentPressed =
        digitalRead(_swPin) == LOW;


    // ========================================================
    // BUTTON STATE CHANGED
    // ========================================================

    if (currentPressed != _buttonPressed)
    {
        // ----------------------------------------------------
        // DEBOUNCE
        // ----------------------------------------------------

        if (
            now - _lastButtonChange
            < DEBOUNCE_MS
        )
        {
            return;
        }

        _lastButtonChange = now;

        _buttonPressed =
            currentPressed;


        // ====================================================
        // BUTTON DOWN
        // ====================================================

        if (_buttonPressed)
        {
            _buttonDownTime = now;

            _longPressTriggered = false;

            // Никакого PRESS здесь.
            //
            // Пока ждём, что произойдёт:
            //
            // SHORT PRESS
            // DOUBLE PRESS
            // LONG PRESS

            return;
        }


        // ====================================================
        // BUTTON UP
        // ====================================================

        addEvent(
            Constants::Event::RELEASE
        );


        // ----------------------------------------------------
        // КРИТИЧЕСКИЙ МОМЕНТ
        //
        // Если это было LONG_PRESS,
        // полностью заканчиваем эту последовательность.
        //
        // Никакого:
        //
        // PRESS
        // DOUBLE_PRESS
        //
        // после LONG_PRESS быть не должно.
        // ----------------------------------------------------

        if (_longPressTriggered)
        {
            _doublePressPending = false;

            _lastReleaseTime = 0;

            return;
        }


        // ----------------------------------------------------
        // ВТОРОЕ НАЖАТИЕ
        // ----------------------------------------------------

        if (
            _doublePressPending &&
            _lastReleaseTime != 0 &&
            (now - _lastReleaseTime)
                <= DOUBLE_PRESS_MS
        )
        {
            _doublePressPending = false;

            _lastReleaseTime = 0;

            // Только DOUBLE_PRESS
            addEvent(
                Constants::Event::DOUBLE_PRESS
            );

            return;
        }


        // ----------------------------------------------------
        // ПЕРВОЕ НАЖАТИЕ
        // ----------------------------------------------------

        _doublePressPending = true;

        _lastReleaseTime = now;

        return;
    }


    // ========================================================
    // LONG PRESS
    // ========================================================

    if (
        _buttonPressed &&
        !_longPressTriggered &&
        (now - _buttonDownTime)
            >= LONG_PRESS_MS
    )
    {
        _longPressTriggered = true;

        // ----------------------------------------------------
        // Отменяем ожидание обычного PRESS
        // ----------------------------------------------------

        _doublePressPending = false;

        _lastReleaseTime = 0;

        // ----------------------------------------------------
        // Только LONG_PRESS
        // ----------------------------------------------------

        addEvent(
            Constants::Event::LONG_PRESS
        );

        return;
    }


    // ========================================================
    // SINGLE PRESS
    // ========================================================

    if (
        _doublePressPending &&
        !_buttonPressed &&
        _lastReleaseTime != 0 &&
        (now - _lastReleaseTime)
            > DOUBLE_PRESS_MS
    )
    {
        _doublePressPending = false;

        _lastReleaseTime = 0;

        // ----------------------------------------------------
        // Только теперь подтверждаем обычный PRESS
        // ----------------------------------------------------

        addEvent(
            Constants::Event::PRESS
        );
    }
}


// ============================================================
// ADD EVENT
// ============================================================

void EncoderManager::addEvent(
    Constants::Event event
)
{
    if (
        event == Constants::Event::NONE
    )
    {
        return;
    }

    uint8_t next =
        (_eventHead + 1)
        % EVENT_QUEUE_SIZE;

    // Очередь заполнена
    if (next == _eventTail)
        return;

    _eventQueue[_eventHead] = event;

    _eventHead = next;
}


// ============================================================
// GET EVENT
// ============================================================

Constants::Event EncoderManager::getEvent()
{
    if (_eventTail == _eventHead)
    {
        return Constants::Event::NONE;
    }

    Constants::Event event =
        _eventQueue[_eventTail];

    _eventQueue[_eventTail] =
        Constants::Event::NONE;

    _eventTail =
        (_eventTail + 1)
        % EVENT_QUEUE_SIZE;

    return event;
}


// ============================================================
// POSITION
// ============================================================

int32_t EncoderManager::getPosition() const
{
    return _position;
}


int32_t EncoderManager::getDelta()
{
    int32_t position = _position;

    int32_t delta =
        position - _reportedPosition;

    _reportedPosition = position;

    return delta;
}


void EncoderManager::setPosition(
    int32_t position
)
{
    _position = position;

    _lastPosition = position;

    _reportedPosition = position;
}


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
// EVENT HELPERS
// ============================================================

bool EncoderManager::wasPressed()
{
    return getEvent()
        == Constants::Event::PRESS;
}


bool EncoderManager::wasReleased()
{
    return getEvent()
        == Constants::Event::RELEASE;
}


bool EncoderManager::wasLongPressed()
{
    return getEvent()
        == Constants::Event::LONG_PRESS;
}


bool EncoderManager::wasDoublePressed()
{
    return getEvent()
        == Constants::Event::DOUBLE_PRESS;
}


// ============================================================
// READY
// ============================================================

bool EncoderManager::isReady() const
{
    return _initialized;
}
