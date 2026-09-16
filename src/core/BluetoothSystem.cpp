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

bool BluetoothSystem::begin()
{
    if (_initialized)
        return true;

    Serial.println(
        "[BT SYSTEM] Starting..."
    );

    // --------------------------------------------------------
    // Bluetooth
    // --------------------------------------------------------

    if (!_bluetooth.begin())
    {
        Serial.println(
            "[BT SYSTEM] Bluetooth failed"
        );

        return false;
    }

    // --------------------------------------------------------
    // Subscription manager
    // --------------------------------------------------------

    _bluetooth.setSubscriptionManager(
        &_subscriptions
    );

    // --------------------------------------------------------
    // Publisher
    // --------------------------------------------------------

    if (!_publisher.begin())
    {
        Serial.println(
            "[BT SYSTEM] Publisher failed"
        );

        return false;
    }

    _initialized = true;

    Serial.println(
        "[BT SYSTEM] Ready"
    );

    return true;
}

void BluetoothSystem::update()
{
    if (!_initialized)
        return;

    // BLE callbacks
    _bluetooth.update();

    // Android -> ESP32
    processCommand();

    // ESP32 -> Android
    _publisher.update();
}

bool BluetoothSystem::isReady() const
{
    return _initialized;
}

bool BluetoothSystem::isConnected() const
{
    return _bluetooth.isConnected();
}

BluetoothManager&
BluetoothSystem::bluetooth()
{
    return _bluetooth;
}

BluetoothSubscriptionManager&
BluetoothSystem::subscriptions()
{
    return _subscriptions;
}

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

    String message =
        _bluetooth.getCommand();

    // --------------------------------------------------------
    // JSON document
    // --------------------------------------------------------

    JsonDocument document;

    BluetoothProtocol::Request request;

    if (!BluetoothProtocol::parse(
        message,
        document,
        request
    ))
    {
        String error =
            BluetoothProtocol::error(
                0,
                "INVALID_REQUEST",
                "Invalid Bluetooth request"
            );

        _bluetooth.send(error);

        return;
    }

    // --------------------------------------------------------
    // Command
    // --------------------------------------------------------

    String response;

    BluetoothCommands_handle(
        request,
        _subscriptions,
        response
    );

    if (!response.isEmpty())
    {
        _bluetooth.send(response);
    }

    // --------------------------------------------------------
    // Immediate snapshots
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

        // После subscribe сразу отправляем
        // актуальное состояние.

        if (
            strcmp(
                topic,
                BluetoothTopics::SENSORS
            ) == 0
        )
        {
            _publisher.publishSensors();
        }
        else if (
            strcmp(
                topic,
                BluetoothTopics::CLOCK
            ) == 0
        )
        {
            _publisher.publishClock();
        }
        else if (
            strcmp(
                topic,
                BluetoothTopics::SYSTEM
            ) == 0
        )
        {
            _publisher.publishSystem();
        }
    }
}

