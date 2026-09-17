#include "BluetoothManager.h"

#include "BluetoothSubscriptionManager.h"
#include "./hardware/bluetooth/BluetoothProtocol.h"
#include "./hardware/bluetooth/BluetoothCommands.h"

// ============================================================
// SERVER CALLBACKS
// ============================================================

BluetoothServerCallbacks::BluetoothServerCallbacks(
    BluetoothManager& manager
)
    : _manager(manager)
{
}

void BluetoothServerCallbacks::onConnect(
    BLEServer* server
)
{
    (void)server;

    Serial0.println();
    Serial0.println("================================");
    Serial0.println("[BLE] DEVICE CONNECTED");
    Serial0.println("================================");

    _manager.handleConnect();
}

void BluetoothServerCallbacks::onDisconnect(
    BLEServer* server
)
{
    (void)server;

    Serial0.println();
    Serial0.println("================================");
    Serial0.println("[BLE] DEVICE DISCONNECTED");
    Serial0.println("================================");

    _manager.handleDisconnect();
}

// ============================================================
// RX CALLBACKS
// ============================================================

BluetoothRxCallbacks::BluetoothRxCallbacks(
    BluetoothManager& manager
)
    : _manager(manager)
{
}

void BluetoothRxCallbacks::onWrite(
    BLECharacteristic* characteristic
)
{
    if (characteristic == nullptr)
    {
        Serial0.println(
            "[BLE RX] ERROR: null characteristic"
        );

        return;
    }

    std::string value =
        characteristic->getValue();

    if (value.empty())
    {
        Serial0.println(
            "[BLE RX] Empty packet"
        );

        return;
    }

    String command;

    command.reserve(
        value.length()
    );

    for (size_t i = 0; i < value.length(); ++i)
    {
        command +=
            static_cast<char>(
                value[i]
            );
    }

    Serial0.print(
        "[BLE RX] "
    );

    Serial0.println(
        command
    );

    _manager.handleReceive(
        command
    );
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
      _subscriptions(nullptr),
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
    {
        Serial0.println(
            "[BLE] Already initialized"
        );

        return true;
    }

    Serial0.println();
    Serial0.println("================================");
    Serial0.println("[BLE] STARTING BLUETOOTH");
    Serial0.println("================================");

    Serial0.print(
        "[BLE] Device: "
    );

    Serial0.println(
        DEVICE_NAME
    );

    Serial0.print(
        "[BLE] Service: "
    );

    Serial0.println(
        SERVICE_UUID
    );

    Serial0.print(
        "[BLE] RX: "
    );

    Serial0.println(
        RX_CHARACTERISTIC_UUID
    );

    Serial0.print(
        "[BLE] TX: "
    );

    Serial0.println(
        TX_CHARACTERISTIC_UUID
    );

    // ========================================================
    // BLE DEVICE
    // ========================================================

    BLEDevice::init(
        DEVICE_NAME
    );

    Serial0.println(
        "[BLE] BLEDevice initialized"
    );

    // ========================================================
    // SERVER
    // ========================================================

    _server =
        BLEDevice::createServer();

    if (_server == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: server creation failed"
        );

        return false;
    }

    _serverCallbacks =
        new BluetoothServerCallbacks(
            *this
        );

    if (_serverCallbacks == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: server callbacks allocation failed"
        );

        return false;
    }

    _server->setCallbacks(
        _serverCallbacks
    );

    Serial0.println(
        "[BLE] GATT server ready"
    );

    // ========================================================
    // SERVICE
    // ========================================================

    _service =
        _server->createService(
            SERVICE_UUID
        );

    if (_service == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: service creation failed"
        );

        return false;
    }

    Serial0.println(
        "[BLE] Service created"
    );

    // ========================================================
    // RX
    // Android -> ESP32
    // ========================================================

    _rxCharacteristic =
        _service->createCharacteristic(
            RX_CHARACTERISTIC_UUID,

            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR
        );

    if (_rxCharacteristic == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: RX creation failed"
        );

        return false;
    }

    _rxCallbacks =
        new BluetoothRxCallbacks(
            *this
        );

    if (_rxCallbacks == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: RX callback allocation failed"
        );

        return false;
    }

    _rxCharacteristic->setCallbacks(
        _rxCallbacks
    );

    Serial0.println(
        "[BLE] RX ready: WRITE + WRITE_NR"
    );

    // ========================================================
    // TX
    // ESP32 -> Android
    // ========================================================

    _txCharacteristic =
        _service->createCharacteristic(
            TX_CHARACTERISTIC_UUID,

            BLECharacteristic::PROPERTY_NOTIFY
        );

    if (_txCharacteristic == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: TX creation failed"
        );

        return false;
    }

    // --------------------------------------------------------
    // CCCD
    // --------------------------------------------------------

    BLE2902* cccd =
        new BLE2902();

    if (cccd == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: CCCD allocation failed"
        );

        return false;
    }

    _txCharacteristic->addDescriptor(
        cccd
    );

    Serial0.println(
        "[BLE] TX ready: NOTIFY + CCCD"
    );

    // ========================================================
    // SERVICE START
    // ========================================================

    _service->start();

    Serial0.println(
        "[BLE] Service started"
    );

    // ========================================================
    // ADVERTISING
    // ========================================================

    startAdvertising();

    // ========================================================
    // READY
    // ========================================================

    _ready = true;
    _connected = false;

    Serial0.println();
    Serial0.println("================================");
    Serial0.println("[BLE] BLUETOOTH READY");
    Serial0.println("================================");
    Serial0.println();

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
        Serial0.println(
            "[BLE] ERROR: advertising is null"
        );

        return;
    }

    advertising->addServiceUUID(
        SERVICE_UUID
    );

    advertising->setScanResponse(
        true
    );

    advertising->setMinPreferred(
        0x06
    );

    advertising->setMinPreferred(
        0x12
    );

    BLEDevice::startAdvertising();

    Serial0.println(
        "[BLE] Advertising started"
    );
}

// ============================================================
// UPDATE
// ============================================================

void BluetoothManager::update()
{
    if (!_ready)
        return;

    if (!_commandAvailable)
        return;

    String command =
        getCommand();

    if (command.isEmpty())
        return;

    processCommand(
        command
    );
}

// ============================================================
// PROCESS COMMAND
// ============================================================

void BluetoothManager::processCommand(
    const String& message
)
{
    if (_subscriptions == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: subscription manager not attached"
        );

        send(
            BluetoothProtocol::error(
                0,
                "INTERNAL_ERROR",
                "Subscription manager unavailable"
            )
        );

        return;
    }

    Serial0.println(
        "[BLE] Processing request..."
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
            "[BLE] Invalid request"
        );

        send(
            BluetoothProtocol::error(
                0,
                "INVALID_REQUEST",
                "Invalid Bluetooth request"
            )
        );

        return;
    }

    Serial0.print(
        "[BLE] Request id: "
    );

    Serial0.println(
        request.id
    );

    Serial0.print(
        "[BLE] Command: "
    );

    Serial0.println(
        request.command
    );

    // --------------------------------------------------------
    // COMMAND ROUTER
    // --------------------------------------------------------

    String response;

    bool success =
        BluetoothCommands_handle(
            request,
            *_subscriptions,
            response
        );

    // --------------------------------------------------------
    // LOG
    // --------------------------------------------------------

    Serial0.print(
        "[BLE] Command result: "
    );

    Serial0.println(
        success ? "OK" : "ERROR"
    );

    // --------------------------------------------------------
    // SEND RESPONSE
    // --------------------------------------------------------

    send(
        response
    );
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
// SUBSCRIPTION MANAGER
// ============================================================

void BluetoothManager::setSubscriptionManager(
    BluetoothSubscriptionManager* manager
)
{
    _subscriptions =
        manager;

    Serial0.print(
        "[BLE] Subscription manager: "
    );

    Serial0.println(
        manager != nullptr
            ? "attached"
            : "null"
    );
}

BluetoothSubscriptionManager*
BluetoothManager::getSubscriptionManager()
{
    return _subscriptions;
}

// ============================================================
// CONNECT
// ============================================================

void BluetoothManager::handleConnect()
{
    _connected = true;

    _command = "";
    _commandAvailable = false;

    Serial0.println(
        "[BLE] Connection state = CONNECTED"
    );

    Serial0.println(
        "[BLE] Waiting for Android commands..."
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

    if (_subscriptions != nullptr)
    {
        _subscriptions->clear();

        Serial0.println(
            "[BLE] Subscriptions cleared"
        );
    }

    Serial0.println(
        "[BLE] Connection state = DISCONNECTED"
    );

    delay(100);

    startAdvertising();
}

// ============================================================
// RECEIVE
// ============================================================

void BluetoothManager::handleReceive(
    const String& data
)
{
    if (data.isEmpty())
        return;

    _command = data;

    _commandAvailable = true;

    Serial0.print(
        "[BLE] Command queued: "
    );

    Serial0.println(
        data
    );
}

// ============================================================
// HAS COMMAND
// ============================================================

bool BluetoothManager::hasCommand() const
{
    return _commandAvailable;
}

// ============================================================
// GET COMMAND
// ============================================================

String BluetoothManager::getCommand()
{
    if (!_commandAvailable)
    {
        return String();
    }

    String result =
        _command;

    _command = "";

    _commandAvailable = false;

    return result;
}

// ============================================================
// CLEAR COMMAND
// ============================================================

void BluetoothManager::clearCommand()
{
    _command = "";

    _commandAvailable = false;
}

// ============================================================
// SEND STRING
// ============================================================

bool BluetoothManager::send(
    const String& data
)
{
    return send(
        data.c_str()
    );
}

// ============================================================
// SEND C-STRING
// ============================================================

bool BluetoothManager::send(
    const char* data
)
{
    if (!_ready)
    {
        Serial0.println(
            "[BLE TX] ERROR: Bluetooth not ready"
        );

        return false;
    }

    if (!_connected)
    {
        Serial0.println(
            "[BLE TX] ERROR: device not connected"
        );

        return false;
    }

    if (_txCharacteristic == nullptr)
    {
        Serial0.println(
            "[BLE TX] ERROR: TX unavailable"
        );

        return false;
    }

    if (data == nullptr)
    {
        Serial0.println(
            "[BLE TX] ERROR: null data"
        );

        return false;
    }

    if (strlen(data) == 0)
    {
        Serial0.println(
            "[BLE TX] ERROR: empty data"
        );

        return false;
    }

    Serial0.print(
        "[BLE TX] "
    );

    Serial0.println(
        data
    );

    _txCharacteristic->setValue(
        data
    );

    _txCharacteristic->notify();

    Serial0.println(
        "[BLE TX] Notification sent"
    );

    return true;
}