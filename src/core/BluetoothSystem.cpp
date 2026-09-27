#include "BluetoothSystem.h"

BluetoothSystem::BluetoothSystem(
    SensorManager& sensors,
    ClockSystem& clock
)
    : _sensors(sensors),
      _clock(clock),
      _bluetooth(),
      _subscriptions(),
      _publisher(
          _bluetooth,
          _subscriptions,
          _sensors,
          _clock
      ),
      _initialized(false)
{
}

// ============================================================
// BEGIN
// ============================================================

bool BluetoothSystem::begin()
{
    if (_initialized)
        return true;

    Serial0.println();
    Serial0.println(
        "================================"
    );
    Serial0.println(
        "[BT SYSTEM] Starting..."
    );
    Serial0.println(
        "================================"
    );

    // --------------------------------------------------------
    // SUBSCRIPTION MANAGER
    // --------------------------------------------------------

    _bluetooth.setSubscriptionManager(
        _subscriptions
    );

    // --------------------------------------------------------
    // BLUETOOTH / NIMBLE
    // --------------------------------------------------------

    if (!_bluetooth.begin())
    {
        Serial0.println(
            "[BT SYSTEM] Bluetooth failed"
        );

        return false;
    }

    // --------------------------------------------------------
    // PUBLISHER
    // --------------------------------------------------------

    if (!_publisher.begin())
    {
        Serial0.println(
            "[BT SYSTEM] Publisher failed"
        );

        return false;
    }

    _initialized = true;

    Serial0.println(
        "[BT SYSTEM] Ready"
    );

    Serial0.println();

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void BluetoothSystem::update()
{
    if (!_initialized)
        return;

    // --------------------------------------------------------
    // BLE
    // --------------------------------------------------------

    _bluetooth.update();

    // --------------------------------------------------------
    // ANDROID -> ESP32
    // --------------------------------------------------------

    processCommand();

    // --------------------------------------------------------
    // ESP32 -> ANDROID
    // --------------------------------------------------------

    _publisher.update();
}

// ============================================================
// READY
// ============================================================

bool BluetoothSystem::isReady() const
{
    return _initialized;
}

// ============================================================
// CONNECTED
// ============================================================

bool BluetoothSystem::isConnected() const
{
    return _bluetooth.isConnected();
}

// ============================================================
// BLUETOOTH
// ============================================================

BluetoothManager&
BluetoothSystem::bluetooth()
{
    return _bluetooth;
}

// ============================================================
// SUBSCRIPTIONS
// ============================================================

BluetoothSubscriptionManager&
BluetoothSystem::subscriptions()
{
    return _subscriptions;
}

// ============================================================
// PUBLISHER
// ============================================================

BluetoothPublisher&
BluetoothSystem::publisher()
{
    return _publisher;
}

// ============================================================
// COMMAND PROCESSING
// ============================================================

void BluetoothSystem::processCommand()
{
    if (!_bluetooth.hasCommand())
        return;

    // --------------------------------------------------------
    // GET COMMAND
    // --------------------------------------------------------

    String message =
        _bluetooth.getCommand();

    if (message.isEmpty())
        return;

    Serial0.print(
        "[BT SYSTEM] Processing: "
    );

    Serial0.println(
        message
    );

    // --------------------------------------------------------
    // JSON DOCUMENT
    // --------------------------------------------------------

    JsonDocument document;

    BluetoothProtocol::Request request;

    // --------------------------------------------------------
    // PARSE
    // --------------------------------------------------------

    if (!BluetoothProtocol::parse(
        message,
        document,
        request
    ))
    {
        Serial0.println(
            "[BT SYSTEM] Invalid request"
        );

        String error =
            BluetoothProtocol::error(
                0,
                "INVALID_REQUEST",
                "Invalid Bluetooth request"
            );

        _bluetooth.send(
            error
        );

        return;
    }

    // --------------------------------------------------------
    // COMMAND
    // --------------------------------------------------------

    String response;

    const bool success =
        BluetoothCommands_handle(
            request,
            _subscriptions,
            response
        );

    // --------------------------------------------------------
    // RESPONSE
    // --------------------------------------------------------

    if (!response.isEmpty())
    {
        _bluetooth.send(
            response
        );
    }

    // --------------------------------------------------------
    // COMMAND FAILED
    // --------------------------------------------------------

    if (!success)
    {
        return;
    }

    // --------------------------------------------------------
    // IMMEDIATE SNAPSHOTS
    // --------------------------------------------------------

    if (
        request.command ==
        BluetoothCommands::SUBSCRIBE
    )
    {
        const char* topic =
            request.data["topic"] | nullptr;

        if (topic == nullptr)
            return;

        // ----------------------------------------------------
        // SENSORS
        // ----------------------------------------------------

        if (
            strcmp(
                topic,
                BluetoothTopics::SENSORS
            ) == 0
        )
        {
            _publisher.publishSensors();
        }

        // ----------------------------------------------------
        // CLOCK
        // ----------------------------------------------------

        else if (
            strcmp(
                topic,
                BluetoothTopics::CLOCK
            ) == 0
        )
        {
            _publisher.publishClock();
        }

        // ----------------------------------------------------
        // SYSTEM
        // ----------------------------------------------------

        else if (
            strcmp(
                topic,
                BluetoothTopics::SYSTEM
            ) == 0
        )
        {
            _publisher.publishSystem();
        }

        // ----------------------------------------------------
        // OTHER TOPICS
        // ----------------------------------------------------
        //
        // They can be added later:
        //
        // ALARMS
        // LIGHT
        // SOUND
        // TIMER
        // STOPWATCH
        //
    }
}
