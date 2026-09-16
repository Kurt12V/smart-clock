#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class BluetoothManager;

class BluetoothServerCallbacks : public BLEServerCallbacks
{
public:
    explicit BluetoothServerCallbacks(BluetoothManager& manager);

    void onConnect(BLEServer* server) override;
    void onDisconnect(BLEServer* server) override;

private:
    BluetoothManager& _manager;
};

class BluetoothRxCallbacks : public BLECharacteristicCallbacks
{
public:
    explicit BluetoothRxCallbacks(BluetoothManager& manager);

    void onWrite(BLECharacteristic* characteristic) override;

private:
    BluetoothManager& _manager;
};

class BluetoothManager
{
public:
    static constexpr const char* DEVICE_NAME = "SmartClock";

    static constexpr const char* SERVICE_UUID =
        "7A1F0001-5C3A-4D8B-9E21-123456789001";

    static constexpr const char* TX_CHARACTERISTIC_UUID =
        "7A1F0002-5C3A-4D8B-9E21-123456789001";

    static constexpr const char* RX_CHARACTERISTIC_UUID =
        "7A1F0003-5C3A-4D8B-9E21-123456789001";

public:
    BluetoothManager();

    bool begin();
    void update();

    bool isReady() const;
    bool isConnected() const;

    bool send(const String& data);
    bool send(const char* data);

    bool hasCommand() const;
    String getCommand();

    void clearCommand();

    void handleConnect();
    void handleDisconnect();
    void handleReceive(const String& data);

private:
    void startAdvertising();

private:
    BLEServer* _server;
    BLEService* _service;

    BLECharacteristic* _txCharacteristic;
    BLECharacteristic* _rxCharacteristic;

    BluetoothServerCallbacks* _serverCallbacks;
    BluetoothRxCallbacks* _rxCallbacks;

    volatile bool _ready;
    volatile bool _connected;

    String _command;
    volatile bool _commandAvailable;
};
