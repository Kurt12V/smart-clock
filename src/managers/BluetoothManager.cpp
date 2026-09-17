#include "BluetoothManager.h"

#include "BluetoothSubscriptionManager.h"

// ============================================================
// SERVER CALLBACKS
// ============================================================

BluetoothServerCallbacks::BluetoothServerCallbacks(
    BluetoothManager& manager)
    : _manager(manager)
{
}

void BluetoothServerCallbacks::onConnect(
    BLEServer* server)
{
    (void)server;

    Serial0.println();
    Serial0.println("================================");
    Serial0.println("[BLE] DEVICE CONNECTED");
    Serial0.println("================================");

    _manager.handleConnect();
}

void BluetoothServerCallbacks::onDisconnect(
    BLEServer* server)
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
    BluetoothManager& manager)
    : _manager(manager)
{
}

void BluetoothRxCallbacks::onWrite(
    BLECharacteristic* characteristic)
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

    command.reserve(value.length());

    for (size_t i = 0; i < value.length(); ++i)
    {
        command +=
            static_cast<char>(value[i]);
    }

    Serial0.print("[BLE RX] ");
    Serial0.println(command);

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

    // --------------------------------------------------------
    // DEVICE INFORMATION
    // --------------------------------------------------------

    Serial0.print("[BLE] Device name: ");
    Serial0.println(DEVICE_NAME);

    Serial0.print("[BLE] Service UUID: ");
    Serial0.println(SERVICE_UUID);

    Serial0.print("[BLE] RX UUID: ");
    Serial0.println(RX_CHARACTERISTIC_UUID);

    Serial0.print("[BLE] TX UUID: ");
    Serial0.println(TX_CHARACTERISTIC_UUID);

    // --------------------------------------------------------
    // BLE INIT
    // --------------------------------------------------------

    Serial0.println(
        "[BLE] Initializing BLE..."
    );

    BLEDevice::init(
        DEVICE_NAME
    );

    Serial0.println(
        "[BLE] BLE initialized"
    );

    // --------------------------------------------------------
    // SERVER
    // --------------------------------------------------------

    Serial0.println(
        "[BLE] Creating GATT server..."
    );

    _server =
        BLEDevice::createServer();

    if (_server == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: failed to create GATT server"
        );

        return false;
    }

    Serial0.println(
        "[BLE] GATT server created"
    );

    // --------------------------------------------------------
    // SERVER CALLBACKS
    // --------------------------------------------------------

    _serverCallbacks =
        new BluetoothServerCallbacks(
            *this
        );

    if (_serverCallbacks == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: failed to create server callbacks"
        );

        return false;
    }

    _server->setCallbacks(
        _serverCallbacks
    );

    Serial0.println(
        "[BLE] Server callbacks installed"
    );

    // --------------------------------------------------------
    // SERVICE
    // --------------------------------------------------------

    Serial0.println(
        "[BLE] Creating service..."
    );

    _service =
        _server->createService(
            SERVICE_UUID
        );

    if (_service == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: failed to create service"
        );

        return false;
    }

    Serial0.println(
        "[BLE] Service created"
    );

    // ========================================================
    // RX CHARACTERISTIC
    //
    // Android -> ESP32
    //
    // UUID ...0002
    //
    // WRITE
    // WRITE_NR
    // ========================================================

    Serial0.println(
        "[BLE] Creating RX characteristic..."
    );

    _rxCharacteristic =
        _service->createCharacteristic(
            RX_CHARACTERISTIC_UUID,

            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR
        );

    if (_rxCharacteristic == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: failed to create RX characteristic"
        );

        return false;
    }

    // --------------------------------------------------------
    // RX CALLBACK
    // --------------------------------------------------------

    _rxCallbacks =
        new BluetoothRxCallbacks(
            *this
        );

    if (_rxCallbacks == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: failed to create RX callbacks"
        );

        return false;
    }

    _rxCharacteristic->setCallbacks(
        _rxCallbacks
    );

    Serial0.println(
        "[BLE] RX characteristic created"
    );

    Serial0.print(
        "[BLE] RX UUID: "
    );

    Serial0.println(
        RX_CHARACTERISTIC_UUID
    );

    Serial0.println(
        "[BLE] RX properties:"
    );

    Serial0.println(
        "       WRITE"
    );

    Serial0.println(
        "       WRITE_NR"
    );

    // ========================================================
    // TX CHARACTERISTIC
    //
    // ESP32 -> Android
    //
    // UUID ...0003
    //
    // NOTIFY
    // ========================================================

    Serial0.println(
        "[BLE] Creating TX characteristic..."
    );

    _txCharacteristic =
        _service->createCharacteristic(
            TX_CHARACTERISTIC_UUID,

            BLECharacteristic::PROPERTY_NOTIFY
        );

    if (_txCharacteristic == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: failed to create TX characteristic"
        );

        return false;
    }

    Serial0.println(
        "[BLE] TX characteristic created"
    );

    Serial0.print(
        "[BLE] TX UUID: "
    );

    Serial0.println(
        TX_CHARACTERISTIC_UUID
    );

    Serial0.println(
        "[BLE] TX properties:"
    );

    Serial0.println(
        "       NOTIFY"
    );

    // --------------------------------------------------------
    // CCCD
    //
    // Required by Android notifications
    // UUID = 0x2902
    // --------------------------------------------------------

    Serial0.println(
        "[BLE] Adding TX CCCD descriptor..."
    );

    BLE2902* cccd =
        new BLE2902();

    if (cccd == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: failed to allocate BLE2902"
        );

        return false;
    }

    _txCharacteristic->addDescriptor(
        cccd
    );

    Serial0.println(
        "[BLE] TX CCCD added"
    );

    Serial0.println(
        "[BLE] TX notifications enabled"
    );

    // --------------------------------------------------------
    // INITIAL TX VALUE
    // --------------------------------------------------------

    _txCharacteristic->setValue(
        "{\"v\":1,\"type\":\"hello\",\"device\":\"SmartClock\"}"
    );

    Serial0.println(
        "[BLE] Initial TX value configured"
    );

    // ========================================================
    // START SERVICE
    // ========================================================

    Serial0.println(
        "[BLE] Starting GATT service..."
    );

    _service->start();

    Serial0.println(
        "[BLE] GATT service started"
    );

    // ========================================================
    // ADVERTISING
    // ========================================================

    startAdvertising();

    // --------------------------------------------------------
    // READY
    // --------------------------------------------------------

    _ready = true;
    _connected = false;

    Serial0.println();
    Serial0.println("================================");
    Serial0.println("[BLE] BLUETOOTH READY");
    Serial0.println("================================");

    Serial0.print("[BLE] Device: ");
    Serial0.println(DEVICE_NAME);

    Serial0.print("[BLE] Service: ");
    Serial0.println(SERVICE_UUID);

    Serial0.print("[BLE] RX: ");
    Serial0.println(RX_CHARACTERISTIC_UUID);

    Serial0.print("[BLE] TX: ");
    Serial0.println(TX_CHARACTERISTIC_UUID);

    Serial0.println();

    return true;
}

// ============================================================
// ADVERTISING
// ============================================================

void BluetoothManager::startAdvertising()
{
    Serial0.println(
        "[BLE] Starting advertising..."
    );

    BLEAdvertising* advertising =
        BLEDevice::getAdvertising();

    if (advertising == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: advertising object is null"
        );

        return;
    }

    // --------------------------------------------------------
    // SERVICE UUID
    // --------------------------------------------------------

    advertising->addServiceUUID(
        SERVICE_UUID
    );

    // --------------------------------------------------------
    // SCAN RESPONSE
    // --------------------------------------------------------

    advertising->setScanResponse(
        true
    );

    // --------------------------------------------------------
    // CONNECTION PARAMETERS
    // --------------------------------------------------------

    advertising->setMinPreferred(
        0x06
    );

    advertising->setMinPreferred(
        0x12
    );

    // --------------------------------------------------------
    // START
    // --------------------------------------------------------

    BLEDevice::startAdvertising();

    Serial0.println(
        "[BLE] Advertising started"
    );

    Serial0.print(
        "[BLE] Device name: "
    );

    Serial0.println(
        DEVICE_NAME
    );

    Serial0.print(
        "[BLE] Advertising service: "
    );

    Serial0.println(
        SERVICE_UUID
    );
}

// ============================================================
// UPDATE
// ============================================================

void BluetoothManager::update()
{
    if (!_ready)
    {
        return;
    }

    // Reserved for future BLE processing.
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
    BluetoothSubscriptionManager* manager)
{
    _subscriptions = manager;

    Serial0.print(
        "[BLE] Subscription manager: "
    );

    if (_subscriptions != nullptr)
    {
        Serial0.println(
            "attached"
        );
    }
    else
    {
        Serial0.println(
            "null"
        );
    }
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

    Serial0.println(
        "[BLE] Connection state = CONNECTED"
    );

    Serial0.println(
        "[BLE] Waiting for Android to enable TX notifications..."
    );
}

// ============================================================
// DISCONNECT
// ============================================================

void BluetoothManager::handleDisconnect()
{
    _connected = false;

    // --------------------------------------------------------
    // CLEAR RX COMMAND
    // --------------------------------------------------------

    _command = "";
    _commandAvailable = false;

    // --------------------------------------------------------
    // CLEAR SUBSCRIPTIONS
    // --------------------------------------------------------

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

    // --------------------------------------------------------
    // RESTART ADVERTISING
    // --------------------------------------------------------

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
    {
        return;
    }

    _command = data;
    _commandAvailable = true;

    Serial0.print(
        "[BLE] Command received: "
    );

    Serial0.println(
        data
    );
}

// ============================================================
// COMMAND
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
    const String& data)
{
    return send(
        data.c_str()
    );
}

// ============================================================
// SEND C-STRING
// ============================================================

bool BluetoothManager::send(
    const char* data)
{
    // --------------------------------------------------------
    // CHECK READY
    // --------------------------------------------------------

    if (!_ready)
    {
        Serial0.println(
            "[BLE TX] ERROR: Bluetooth not ready"
        );

        return false;
    }

    // --------------------------------------------------------
    // CHECK CONNECTION
    // --------------------------------------------------------

    if (!_connected)
    {
        Serial0.println(
            "[BLE TX] ERROR: no connected device"
        );

        return false;
    }

    // --------------------------------------------------------
    // CHECK CHARACTERISTIC
    // --------------------------------------------------------

    if (_txCharacteristic == nullptr)
    {
        Serial0.println(
            "[BLE TX] ERROR: TX characteristic unavailable"
        );

        return false;
    }

    // --------------------------------------------------------
    // CHECK DATA
    // --------------------------------------------------------

    if (data == nullptr)
    {
        Serial0.println(
            "[BLE TX] ERROR: null data"
        );

        return false;
    }

    // --------------------------------------------------------
    // CHECK LENGTH
    // --------------------------------------------------------

    size_t length =
        strlen(data);

    if (length == 0)
    {
        Serial0.println(
            "[BLE TX] ERROR: empty data"
        );

        return false;
    }

    // --------------------------------------------------------
    // LOG
    // --------------------------------------------------------

    Serial0.print(
        "[BLE TX] "
    );

    Serial0.println(
        data
    );

    // --------------------------------------------------------
    // SET VALUE
    // --------------------------------------------------------

    _txCharacteristic->setValue(
        data
    );

    // --------------------------------------------------------
    // NOTIFY
    // --------------------------------------------------------

    _txCharacteristic->notify();

    Serial0.println(
        "[BLE TX] Notification sent"
    );

    return true;
}