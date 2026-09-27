#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>

#include "BluetoothSubscriptionManager.h"

// ============================================================
// UUID
// ============================================================

#define SMARTCLOCK_BLE_SERVICE_UUID \
    "7A1F0001-5C3A-4D8B-9E21-123456789001"

#define SMARTCLOCK_BLE_RX_UUID \
    "7A1F0002-5C3A-4D8B-9E21-123456789001"

#define SMARTCLOCK_BLE_TX_UUID \
    "7A1F0003-5C3A-4D8B-9E21-123456789001"

// ============================================================
// CONFIG
// ============================================================

static constexpr size_t BLUETOOTH_COMMAND_SIZE = 1024;

// ============================================================
// STATE
// ============================================================

enum class BluetoothConnectionState : uint8_t
{
    Disconnected = 0,
    Connecting,
    Connected
};

// ============================================================
// MANAGER
// ============================================================

class BluetoothManager
{
public:

    BluetoothManager();

    bool begin();

    void update();

    bool isConnected() const;

    BluetoothConnectionState getState() const;

    void setSubscriptionManager(
        BluetoothSubscriptionManager& manager
    );

    void attachSubscriptionManager(
        BluetoothSubscriptionManager& manager
    );

    bool hasCommand() const;

    String getCommand();

    bool send(
        const String& message
    );

    bool sendRaw(
        const char* message
    );

    bool sendJson(
        const JsonDocument& document
    );

private:

    class ServerCallbacks;
    class RxCallbacks;

    NimBLEServer* _server;

    NimBLECharacteristic* _rx;

    NimBLECharacteristic* _tx;

    NimBLEAdvertising* _advertising;

    BluetoothSubscriptionManager*
        _subscriptionManager;

    BluetoothConnectionState _state;

    volatile bool _deviceConnected;

    char _commandBuffer[
        BLUETOOTH_COMMAND_SIZE
    ];

    volatile bool _commandAvailable;

    portMUX_TYPE _commandMux;

    void onConnect();

    void onDisconnect();

    void onReceive(
        const String& message
    );

    void restartAdvertising();

    friend class ServerCallbacks;
    friend class RxCallbacks;
};