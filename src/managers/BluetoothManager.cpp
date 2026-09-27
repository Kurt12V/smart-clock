#include "BluetoothManager.h"

// ============================================================
// NIMBLE SERVER CALLBACKS
// ============================================================

class BluetoothManager::ServerCallbacks
    : public NimBLEServerCallbacks
{
public:

    explicit ServerCallbacks(
        BluetoothManager* manager
    )
        : _manager(manager)
    {
    }

    void onConnect(
        NimBLEServer* server,
        NimBLEConnInfo& connInfo
    ) override
    {
        (void)server;
        (void)connInfo;

        if (_manager != nullptr)
        {
            _manager->onConnect();
        }
    }

    void onDisconnect(
        NimBLEServer* server,
        NimBLEConnInfo& connInfo,
        int reason
    ) override
    {
        (void)server;
        (void)connInfo;
        (void)reason;

        if (_manager != nullptr)
        {
            _manager->onDisconnect();
        }
    }

private:

    BluetoothManager* _manager;
};


// ============================================================
// NIMBLE RX CALLBACKS
// ============================================================

class BluetoothManager::RxCallbacks
    : public NimBLECharacteristicCallbacks
{
public:

    explicit RxCallbacks(
        BluetoothManager* manager
    )
        : _manager(manager)
    {
    }

    void onWrite(
        NimBLECharacteristic* characteristic,
        NimBLEConnInfo& connInfo
    ) override
    {
        (void)connInfo;

        if (_manager == nullptr)
            return;

        if (characteristic == nullptr)
            return;

        std::string value =
            characteristic->getValue();

        if (value.empty())
            return;

        String message(
            value.c_str()
        );

        _manager->onReceive(
            message
        );
    }

private:

    BluetoothManager* _manager;
};


// ============================================================
// CONSTRUCTOR
// ============================================================

BluetoothManager::BluetoothManager()
    : _server(nullptr),
      _rx(nullptr),
      _tx(nullptr),
      _advertising(nullptr),
      _subscriptionManager(nullptr),
      _state(
          BluetoothConnectionState::Disconnected
      ),
      _deviceConnected(false),
      _commandBuffer{0},
      _commandAvailable(false),
      _commandMux(
          portMUX_INITIALIZER_UNLOCKED
      )
{
}


// ============================================================
// BEGIN
// ============================================================

bool BluetoothManager::begin()
{
    Serial0.println();
    Serial0.println(
        "================================"
    );
    Serial0.println(
        "[BLE] Starting NimBLE"
    );
    Serial0.println(
        "================================"
    );

    // --------------------------------------------------------
    // NIMBLE INIT
    // --------------------------------------------------------

    NimBLEDevice::init(
        "SmartClock"
    );

    NimBLEDevice::setPower(
        ESP_PWR_LVL_P9
    );

    // --------------------------------------------------------
    // CREATE SERVER
    // --------------------------------------------------------

    _server =
        NimBLEDevice::createServer();

    if (_server == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: Failed to create server"
        );

        return false;
    }

    _server->setCallbacks(
        new ServerCallbacks(this)
    );

    // --------------------------------------------------------
    // CREATE SERVICE
    // --------------------------------------------------------

    NimBLEService* service =
        _server->createService(
            SMARTCLOCK_BLE_SERVICE_UUID
        );

    if (service == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: Failed to create service"
        );

        return false;
    }

    // --------------------------------------------------------
    // RX
    // Android -> ESP32
    // --------------------------------------------------------

    _rx =
        service->createCharacteristic(
            SMARTCLOCK_BLE_RX_UUID,
            NIMBLE_PROPERTY::WRITE |
            NIMBLE_PROPERTY::WRITE_NR
        );

    if (_rx == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: Failed to create RX characteristic"
        );

        return false;
    }

    _rx->setCallbacks(
        new RxCallbacks(this)
    );

    // --------------------------------------------------------
    // TX
    // ESP32 -> Android
    // --------------------------------------------------------

    _tx =
        service->createCharacteristic(
            SMARTCLOCK_BLE_TX_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );

    if (_tx == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: Failed to create TX characteristic"
        );

        return false;
    }

    // --------------------------------------------------------
    // SERVICE
    // --------------------------------------------------------
    //
    // NimBLE-Arduino автоматически запускает сервисы
    // при запуске сервера.
    //
    // НЕ вызываем:
    //
    // service->start();
    //
    // --------------------------------------------------------

    Serial0.println(
        "[BLE] Service created"
    );

    // --------------------------------------------------------
    // START SERVER
    // --------------------------------------------------------

    _server->start();

    Serial0.println(
        "[BLE] Server started"
    );

    // --------------------------------------------------------
    // ADVERTISING
    // --------------------------------------------------------

    _advertising =
        NimBLEDevice::getAdvertising();

    if (_advertising == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: Advertising unavailable"
        );

        return false;
    }

    _advertising->addServiceUUID(
        SMARTCLOCK_BLE_SERVICE_UUID
    );

    _advertising->setName(
        "SmartClock"
    );

    _advertising->enableScanResponse(
        true
    );

    // --------------------------------------------------------
    // START ADVERTISING
    // --------------------------------------------------------

    _advertising->start();

    _state =
        BluetoothConnectionState::Disconnected;

    _deviceConnected = false;

    Serial0.println(
        "[BLE] Advertising started"
    );

    Serial0.println(
        "[BLE] Device name: SmartClock"
    );

    Serial0.println(
        "[BLE] Waiting for Android..."
    );

    Serial0.println(
        "================================"
    );

    return true;
}


// ============================================================
// UPDATE
// ============================================================

void BluetoothManager::update()
{
    if (!_deviceConnected)
    {
        if (_state !=
            BluetoothConnectionState::Disconnected)
        {
            _state =
                BluetoothConnectionState::Disconnected;
        }

        return;
    }

    if (_state !=
        BluetoothConnectionState::Connected)
    {
        _state =
            BluetoothConnectionState::Connected;
    }
}


// ============================================================
// IS CONNECTED
// ============================================================

bool BluetoothManager::isConnected() const
{
    return _deviceConnected;
}


// ============================================================
// GET STATE
// ============================================================

BluetoothConnectionState
BluetoothManager::getState() const
{
    return _state;
}


// ============================================================
// SET SUBSCRIPTION MANAGER
// ============================================================

void BluetoothManager::setSubscriptionManager(
    BluetoothSubscriptionManager& manager
)
{
    _subscriptionManager =
        &manager;

    Serial0.println(
        "[BLE] Subscription manager attached"
    );
}


// ============================================================
// ATTACH SUBSCRIPTION MANAGER
// ============================================================

void BluetoothManager::attachSubscriptionManager(
    BluetoothSubscriptionManager& manager
)
{
    setSubscriptionManager(
        manager
    );
}


// ============================================================
// HAS COMMAND
// ============================================================

bool BluetoothManager::hasCommand() const
{
    bool available;

    portENTER_CRITICAL(
        const_cast<portMUX_TYPE*>(
            &_commandMux
        )
    );

    available =
        _commandAvailable;

    portEXIT_CRITICAL(
        const_cast<portMUX_TYPE*>(
            &_commandMux
        )
    );

    return available;
}


// ============================================================
// GET COMMAND
// ============================================================

String BluetoothManager::getCommand()
{
    char localBuffer[
        BLUETOOTH_COMMAND_SIZE
    ];

    localBuffer[0] = '\0';

    portENTER_CRITICAL(
        &_commandMux
    );

    if (!_commandAvailable)
    {
        portEXIT_CRITICAL(
            &_commandMux
        );

        return String();
    }

    strncpy(
        localBuffer,
        _commandBuffer,
        BLUETOOTH_COMMAND_SIZE - 1
    );

    localBuffer[
        BLUETOOTH_COMMAND_SIZE - 1
    ] = '\0';

    _commandBuffer[0] = '\0';

    _commandAvailable = false;

    portEXIT_CRITICAL(
        &_commandMux
    );

    return String(
        localBuffer
    );
}


// ============================================================
// SEND
// ============================================================

bool BluetoothManager::send(
    const String& message
)
{
    return sendRaw(
        message.c_str()
    );
}


// ============================================================
// SEND RAW
// ============================================================

bool BluetoothManager::sendRaw(
    const char* message
)
{
    if (message == nullptr)
    {
        Serial0.println(
            "[BLE TX] ERROR: null message"
        );

        return false;
    }

    if (!_deviceConnected)
    {
        Serial0.println(
            "[BLE TX] ERROR: device not connected"
        );

        return false;
    }

    if (_tx == nullptr)
    {
        Serial0.println(
            "[BLE TX] ERROR: TX characteristic unavailable"
        );

        return false;
    }

    size_t length =
        strlen(message);

    if (length == 0)
    {
        Serial0.println(
            "[BLE TX] ERROR: empty message"
        );

        return false;
    }

    if (length >=
        BLUETOOTH_COMMAND_SIZE)
    {
        Serial0.println(
            "[BLE TX] ERROR: message too large"
        );

        return false;
    }

    Serial0.print(
        "[BLE TX] "
    );

    Serial0.println(
        message
    );

    // --------------------------------------------------------
    // SET VALUE
    // --------------------------------------------------------

    _tx->setValue(
        reinterpret_cast<
            const uint8_t*
        >(message),
        length
    );

    // --------------------------------------------------------
    // NOTIFY
    // --------------------------------------------------------

    bool result =
        _tx->notify();

    if (result)
    {
        Serial0.println(
            "[BLE TX] Notification sent"
        );
    }
    else
    {
        Serial0.println(
            "[BLE TX] Notification failed"
        );
    }

    return result;
}


// ============================================================
// SEND JSON
// ============================================================

bool BluetoothManager::sendJson(
    const JsonDocument& document
)
{
    String message;

    serializeJson(
        document,
        message
    );

    return send(
        message
    );
}


// ============================================================
// ON CONNECT
// ============================================================

void BluetoothManager::onConnect()
{
    _deviceConnected = true;

    _state =
        BluetoothConnectionState::Connected;

    // --------------------------------------------------------
    // CLEAR OLD COMMAND
    // --------------------------------------------------------

    portENTER_CRITICAL(
        &_commandMux
    );

    _commandBuffer[0] = '\0';

    _commandAvailable = false;

    portEXIT_CRITICAL(
        &_commandMux
    );

    // --------------------------------------------------------
    // LOG
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println(
        "================================"
    );
    Serial0.println(
        "[BLE] DEVICE CONNECTED"
    );
    Serial0.println(
        "================================"
    );

    Serial0.println(
        "[BLE] Connection state = CONNECTED"
    );

    Serial0.println(
        "[BLE] Waiting for Android commands..."
    );
}


// ============================================================
// ON DISCONNECT
// ============================================================

void BluetoothManager::onDisconnect()
{
    _deviceConnected = false;

    _state =
        BluetoothConnectionState::Disconnected;

    // --------------------------------------------------------
    // CLEAR COMMAND
    // --------------------------------------------------------

    portENTER_CRITICAL(
        &_commandMux
    );

    _commandBuffer[0] = '\0';

    _commandAvailable = false;

    portEXIT_CRITICAL(
        &_commandMux
    );

    // --------------------------------------------------------
    // CLEAR SUBSCRIPTIONS
    // --------------------------------------------------------

    if (_subscriptionManager != nullptr)
    {
        _subscriptionManager->clear();

        Serial0.println(
            "[BLE] Subscriptions cleared"
        );
    }

    // --------------------------------------------------------
    // LOG
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println(
        "================================"
    );
    Serial0.println(
        "[BLE] DEVICE DISCONNECTED"
    );
    Serial0.println(
        "================================"
    );

    // --------------------------------------------------------
    // RESTART ADVERTISING
    // --------------------------------------------------------

    restartAdvertising();
}


// ============================================================
// ON RECEIVE
// ============================================================

void BluetoothManager::onReceive(
    const String& message
)
{
    if (message.length() == 0)
    {
        Serial0.println(
            "[BLE RX] Empty message"
        );

        return;
    }

    if (message.length() >=
        BLUETOOTH_COMMAND_SIZE)
    {
        Serial0.println(
            "[BLE RX] ERROR: message too large"
        );

        return;
    }

    Serial0.print(
        "[BLE RX] "
    );

    Serial0.println(
        message
    );

    // --------------------------------------------------------
    // COMMAND QUEUE
    // --------------------------------------------------------

    portENTER_CRITICAL(
        &_commandMux
    );

    if (_commandAvailable)
    {
        portEXIT_CRITICAL(
            &_commandMux
        );

        Serial0.println(
            "[BLE RX] ERROR: command queue busy"
        );

        return;
    }

    size_t length =
        message.length();

    memcpy(
        _commandBuffer,
        message.c_str(),
        length
    );

    _commandBuffer[length] =
        '\0';

    _commandAvailable = true;

    portEXIT_CRITICAL(
        &_commandMux
    );

    Serial0.print(
        "[BLE] Command queued: "
    );

    Serial0.println(
        message
    );
}


// ============================================================
// RESTART ADVERTISING
// ============================================================

void BluetoothManager::restartAdvertising()
{
    if (_advertising == nullptr)
        return;

    delay(100);

    _advertising->start();

    Serial0.println(
        "[BLE] Advertising restarted"
    );
}