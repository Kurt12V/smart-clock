#pragma once

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class BluetoothManager;
class BluetoothSubscriptionManager;

// ============================================================
// SERVER CALLBACKS
// ============================================================

class BluetoothServerCallbacks : public BLEServerCallbacks
{
public:
    explicit BluetoothServerCallbacks(
        BluetoothManager& manager
    );

    void onConnect(
        BLEServer* server
    ) override;

    void onDisconnect(
        BLEServer* server
    ) override;

private:
    BluetoothManager& _manager;
};

// ============================================================
// RX CALLBACKS
// ============================================================

class BluetoothRxCallbacks : public BLECharacteristicCallbacks
{
public:
    explicit BluetoothRxCallbacks(
        BluetoothManager& manager
    );

    void onWrite(
        BLECharacteristic* characteristic
    ) override;

private:
    BluetoothManager& _manager;
};

// ============================================================
// BLUETOOTH MANAGER
// ============================================================

class BluetoothManager
{
public:

    static constexpr const char* DEVICE_NAME =
        "SmartClock";

    // --------------------------------------------------------
    // SERVICE
    // --------------------------------------------------------

    static constexpr const char* SERVICE_UUID =
        "7A1F0001-5C3A-4D8B-9E21-123456789001";

    // --------------------------------------------------------
    // RX
    // Android -> ESP32
    // WRITE / WRITE_NR
    // --------------------------------------------------------

    static constexpr const char* RX_CHARACTERISTIC_UUID =
        "7A1F0002-5C3A-4D8B-9E21-123456789001";

    // --------------------------------------------------------
    // TX
    // ESP32 -> Android
    // NOTIFY
    // --------------------------------------------------------

    static constexpr const char* TX_CHARACTERISTIC_UUID =
        "7A1F0003-5C3A-4D8B-9E21-123456789001";

public:

    BluetoothManager();

    bool begin();

    void update();

    bool isReady() const;

    bool isConnected() const;

    // --------------------------------------------------------
    // SUBSCRIPTIONS
    // --------------------------------------------------------

    void setSubscriptionManager(
        BluetoothSubscriptionManager* manager
    );

    BluetoothSubscriptionManager*
    getSubscriptionManager();

    // --------------------------------------------------------
    // TX
    // --------------------------------------------------------

    bool send(
        const String& data
    );

    bool send(
        const char* data
    );

    // --------------------------------------------------------
    // RX
    // --------------------------------------------------------

    bool hasCommand() const;

    String getCommand();

    void clearCommand();

    // --------------------------------------------------------
    // CALLBACKS
    // --------------------------------------------------------

    void handleConnect();

    void handleDisconnect();

    void handleReceive(
        const String& data
    );

private:

    void startAdvertising();

    void processCommand(
        const String& message
    );

private:

    BLEServer* _server;

    BLEService* _service;

    // ESP32 -> Android
    BLECharacteristic* _txCharacteristic;

    // Android -> ESP32
    BLECharacteristic* _rxCharacteristic;

    BLEServerCallbacks* _serverCallbacks;

    BluetoothRxCallbacks* _rxCallbacks;

    BluetoothSubscriptionManager* _subscriptions;

    volatile bool _ready;

    volatile bool _connected;

    String _command;

    volatile bool _commandAvailable;
};