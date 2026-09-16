#pragma once

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "BluetoothSubscriptionManager.h"

class BluetoothManager
{
public:
    BluetoothManager();

    bool begin();
    void update();

    bool isReady() const;
    bool isConnected() const;

    bool send(const String& message);

    bool hasCommand() const;
    String getCommand();

    void setPassKey(uint32_t passKey);

    void setSubscriptionManager(
        BluetoothSubscriptionManager* manager
    );

private:
    class ServerCallbacks;
    class RXCallbacks;
    class SecurityCallbacks;

    BLEServer* _server;
    BLEService* _service;

    BLECharacteristic* _txCharacteristic;
    BLECharacteristic* _rxCharacteristic;

    bool _initialized;
    volatile bool _connected;

    String _command;

    uint32_t _passKey;

    BluetoothSubscriptionManager* _subscriptions;

    static constexpr const char* DEVICE_NAME =
        "SmartClock";

    static constexpr const char* SERVICE_UUID =
        "7A1F0001-5C3A-4D8B-9E21-123456789001";

    static constexpr const char* TX_UUID =
        "7A1F0002-5C3A-4D8B-9E21-123456789001";

    static constexpr const char* RX_UUID =
        "7A1F0003-5C3A-4D8B-9E21-123456789001";

    friend class ServerCallbacks;
    friend class RXCallbacks;
    friend class SecurityCallbacks;
};
