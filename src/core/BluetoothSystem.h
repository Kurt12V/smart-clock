#pragma once

#include <Arduino.h>

#include "./managers/BluetoothManager.h"
#include "./hardware/bluetooth/BluetoothProtocol.h"
#include "./hardware/bluetooth/BluetoothCommands.h"
#include "./managers/BluetoothSubscriptionManager.h"
#include "./hardware/bluetooth/BluetoothPublisher.h"

#include "./managers/SensorsManager.h"
#include "./core/ClockSystem.h"

class BluetoothSystem
{
public:
    BluetoothSystem(
        SensorManager& sensors,
        ClockSystem& clock
    );

    bool begin();

    void update();

    bool isReady() const;
    bool isConnected() const;

    BluetoothManager& bluetooth();

    BluetoothSubscriptionManager&
    subscriptions();

    BluetoothPublisher& publisher();

private:
    void processCommand();

private:
    SensorManager& _sensors;
    ClockSystem& _clock;

    BluetoothManager _bluetooth;

    BluetoothSubscriptionManager
        _subscriptions;

    BluetoothPublisher
        _publisher;

    bool _initialized;
};

