#include "BluetoothManager.h"

#include <cstring>

#include "../../utils/Logger.h"

// ============================================================
// SERVER CALLBACKS
// ============================================================

BluetoothServerCallbacks::BluetoothServerCallbacks(
    BluetoothManager& manager)
    : _manager(manager)
{
}

void BluetoothServerCallbacks::onConnect(BLEServer* server)
{
    (void)server;

    _manager.handleConnect();
}

void BluetoothServerCallbacks::onDisconnect(BLEServer* server)
{
    (void)server;

    _manager.handleDisconnect();
}

// ============================================================
// RX CALLBACKS
// ============================================================

BluetoothRxCallbacks::BluetoothRxCallbacks(
    BluetoothManager& manager)
    : _manager(manager)
{
}

void BluetoothRxCallbacks::onWrite(
    BLECharacteristic* characteristic)
{
    if (characteristic == nullptr)
        return;

    std::string value = characteristic->getValue();

    if (value.empty())
        return;

    String command;

    for (size_t i = 0; i < value.length(); ++i)
    {
        command += static_cast<char>(value[i]);
    }

    _manager.handleReceive(command);
}

// ============================================================
// CONSTRUCTOR
// ============================================================

BluetoothManager::BluetoothManager()
    : _server(nullptr),
      _service(nullptr),
      _txCharacteristic(nullptr),
      _rxCharacteristic(nullptr),
      _serverCallbacks(nullptr),
      _rxCallbacks(nullptr),
      _ready(false),
      _connected(false),
      _command(),
      _commandAvailable(false)
{
}

// ============================================================
// BEGIN
// ============================================================

bool BluetoothManager::begin()
{
    if (_ready)
        return true;

    Serial0.println("BLE", "Starting Bluetooth...");
    Serial0.println("BLE", DEVICE_NAME);

    // --------------------------------------------------------
    // INIT BLE
    // --------------------------------------------------------

    BLEDevice::init(DEVICE_NAME);

    // --------------------------------------------------------
    // CREATE SERVER
    // --------------------------------------------------------

    _server = BLEDevice::createServer();

    if (_server == nullptr)
    {
        Serial0.println("BLE", "Failed to create BLE server");
        return false;
    }

    _serverCallbacks =
        new BluetoothServerCallbacks(*this);

    _server->setCallbacks(_serverCallbacks);

    // --------------------------------------------------------
    // CREATE SERVICE
    // --------------------------------------------------------

    _service =
        _server->createService(SERVICE_UUID);

    if (_service == nullptr)
    {
        Serial0.println("BLE", "Failed to create service");
        return false;
    }

    // --------------------------------------------------------
    // TX
    //
    // ESP32 -> Android
    // --------------------------------------------------------

    _txCharacteristic =
        _service->createCharacteristic(
            TX_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY
        );

    if (_txCharacteristic == nullptr)
    {
        Serial0.println("BLE", "Failed to create TX characteristic");
        return false;
    }

    // CCCD descriptor for notifications

    _txCharacteristic->addDescriptor(
        new BLE2902()
    );

    _txCharacteristic->setValue(
        "{\"v\":1,\"type\":\"hello\",\"device\":\"SmartClock\"}"
    );

    // --------------------------------------------------------
    // RX
    //
    // Android -> ESP32
    // --------------------------------------------------------

    _rxCharacteristic =
        _service->createCharacteristic(
            RX_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR
        );

    if (_rxCharacteristic == nullptr)
    {
        Serial0.println("BLE", "Failed to create RX characteristic");
        return false;
    }

    _rxCallbacks =
        new BluetoothRxCallbacks(*this);

    _rxCharacteristic->setCallbacks(_rxCallbacks);

    // --------------------------------------------------------
    // START SERVICE
    // --------------------------------------------------------

    _service->start();

    Serial0.println("BLE", "GATT service started");

    // --------------------------------------------------------
    // ADVERTISING
    // --------------------------------------------------------

    startAdvertising();

    _ready = true;

    Serial0.println("BLE", "Bluetooth ready");

    return true;
}

// ============================================================
// ADVERTISING
// ============================================================

void BluetoothManager::startAdvertising()
{
    BLEAdvertising* advertising =
        BLEDevice::getAdvertising();

    if (advertising == nullptr)
    {
        Serial0.println("BLE", "Advertising object unavailable");
        return;
    }

    // Clear old configuration if supported by current library.
    // We intentionally do not call removeServiceUUID() here,
    // because API differs between BLE library versions.

    advertising->addServiceUUID(SERVICE_UUID);

    advertising->setScanResponse(true);

    advertising->setMinPreferred(0x06);
    advertising->setMinPreferred(0x12);

    BLEDevice::startAdvertising();

    Serial0.println("BLE", "Advertising started");
}

// ============================================================
// UPDATE
// ============================================================

void BluetoothManager::update()
{
    if (!_ready)
        return;

    // Nothing periodic is required here yet.
}

// ============================================================
// STATE
// ============================================================

bool BluetoothManager::isReady() const
{
    return _ready;
}

bool BluetoothManager::isConnected() const
{
    return _connected;
}

// ============================================================
// CONNECT
// ============================================================

void BluetoothManager::handleConnect()
{
    _connected = true;

    Serial0.println(
        "BLE",
        "Android device connected"
    );
}

// ============================================================
// DISCONNECT
// ============================================================

void BluetoothManager::handleDisconnect()
{
    _connected = false;

    _command = "";
    _commandAvailable = false;

    Serial0.println(
        "BLE",
        "Android device disconnected"
    );

    delay(100);

    startAdvertising();
}

// ============================================================
// RECEIVE
// ============================================================

void BluetoothManager::handleReceive(
    const String& data)
{
    if (data.length() == 0)
        return;

    _command = data;
    _commandAvailable = true;

    Serial0.println(
        "BLE RX",
        data.c_str()
    );
}

// ============================================================
// COMMAND
// ============================================================

bool BluetoothManager::hasCommand() const
{
    return _commandAvailable;
}

String BluetoothManager::getCommand()
{
    if (!_commandAvailable)
        return String();

    String result = _command;

    _command = "";
    _commandAvailable = false;

    return result;
}

void BluetoothManager::clearCommand()
{
    _command = "";
    _commandAvailable = false;
}

// ============================================================
// SEND
// ============================================================

bool BluetoothManager::send(
    const String& data)
{
    return send(data.c_str());
}

bool BluetoothManager::send(
    const char* data)
{
    if (!_ready)
    {
        Serial0.println(
            "BLE TX",
            "Bluetooth is not ready"
        );

        return false;
    }

    if (!_connected)
    {
        Serial0.println(
            "BLE TX",
            "No connected device"
        );

        return false;
    }

    if (_txCharacteristic == nullptr)
    {
        Serial0.println(
            "BLE TX",
            "TX characteristic unavailable"
        );

        return false;
    }

    if (data == nullptr)
        return false;

    _txCharacteristic->setValue(data);

    _txCharacteristic->notify();

    Serial0.println(
        "BLE TX",
        data
    );

    return true;
}
