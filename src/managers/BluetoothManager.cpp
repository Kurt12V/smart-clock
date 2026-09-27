#include "BluetoothManager.h"

// ============================================================
// INTERNAL CALLBACKS
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

        if (_manager == nullptr)
            return;

        _manager->_deviceConnected = true;

        _manager->_state =
            BluetoothConnectionState::Connected;

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

    void onDisconnect(
        NimBLEServer* server,
        NimBLEConnInfo& connInfo,
        int reason
    ) override
    {
        (void)server;
        (void)connInfo;

        if (_manager == nullptr)
            return;

        _manager->_deviceConnected = false;

        _manager->_state =
            BluetoothConnectionState::Disconnected;

        portENTER_CRITICAL(
            &_manager->_commandMux
        );

        _manager->_commandBuffer[0] = '\0';
        _manager->_commandAvailable = false;

        portEXIT_CRITICAL(
            &_manager->_commandMux
        );

        if (_manager->_subscriptionManager != nullptr)
        {
            _manager->_subscriptionManager->clear();
        }

        Serial0.println();
        Serial0.println(
            "================================"
        );
        Serial0.println(
            "[BLE] DEVICE DISCONNECTED"
        );
        Serial0.print(
            "[BLE] Disconnect reason: "
        );
        Serial0.println(reason);
        Serial0.println(
            "================================"
        );

        // ВАЖНО:
        // Здесь НЕ запускаем advertising.
        //
        // Advertising будет восстановлен
        // из BluetoothManager::update().
    }

private:

    BluetoothManager* _manager;
};


// ============================================================
// RX CALLBACKS
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
    // INIT
    // --------------------------------------------------------

    NimBLEDevice::init(
        "SmartClock"
    );

    NimBLEDevice::setPower(
        ESP_PWR_LVL_P9
    );

    // --------------------------------------------------------
    // SERVER
    // --------------------------------------------------------

    _server =
        NimBLEDevice::createServer();

    if (_server == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: createServer failed"
        );

        return false;
    }

    _server->setCallbacks(
        new ServerCallbacks(this)
    );

    // --------------------------------------------------------
    // SERVICE
    // --------------------------------------------------------

    NimBLEService* service =
        _server->createService(
            SMARTCLOCK_BLE_SERVICE_UUID
        );

    if (service == nullptr)
    {
        Serial0.println(
            "[BLE] ERROR: createService failed"
        );

        return false;
    }

    // --------------------------------------------------------
    // RX
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
            "[BLE] ERROR: RX characteristic failed"
        );

        return false;
    }

    _rx->setCallbacks(
        new RxCallbacks(this)
    );

    // --------------------------------------------------------
    // TX
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
            "[BLE] ERROR: TX characteristic failed"
        );

        return false;
    }

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
            "[BLE] ERROR: advertising unavailable"
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

    Serial0.println(
        "[BLE] Advertising started"
    );

    Serial0.println(
        "[BLE] Device name: SmartClock"
    );

    Serial0.println(
        "[BLE] Ready"
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
    static bool advertisingStarted =
        true;

    // --------------------------------------------------------
    // CONNECTED
    // --------------------------------------------------------

    if (_deviceConnected)
    {
        advertisingStarted = false;

        if (_state !=
            BluetoothConnectionState::Connected)
        {
            _state =
                BluetoothConnectionState::Connected;
        }

        return;
    }

    // --------------------------------------------------------
    // DISCONNECTED
    // --------------------------------------------------------

    if (_state !=
        BluetoothConnectionState::Disconnected)
    {
        _state =
            BluetoothConnectionState::Disconnected;
    }

    // --------------------------------------------------------
    // RESTART ADVERTISING
    // --------------------------------------------------------

    if (!advertisingStarted)
    {
        if (_advertising != nullptr)
        {
            Serial0.println(
                "[BLE] Restarting advertising..."
            );

            _advertising->start();

            advertisingStarted = true;

            Serial0.println(
                "[BLE] Advertising restarted"
            );
        }
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
    bool result;

    portENTER_CRITICAL(
        const_cast<portMUX_TYPE*>(
            &_commandMux
        )
    );

    result =
        _commandAvailable;

    portEXIT_CRITICAL(
        const_cast<portMUX_TYPE*>(
            &_commandMux
        )
    );

    return result;
}


// ============================================================
// GET COMMAND
// ============================================================

String BluetoothManager::getCommand()
{
    char buffer[
        BLUETOOTH_COMMAND_SIZE
    ];

    buffer[0] = '\0';

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

    memcpy(
        buffer,
        _commandBuffer,
        BLUETOOTH_COMMAND_SIZE
    );

    buffer[
        BLUETOOTH_COMMAND_SIZE - 1
    ] = '\0';

    _commandBuffer[0] = '\0';

    _commandAvailable = false;

    portEXIT_CRITICAL(
        &_commandMux
    );

    return String(buffer);
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
            "[BLE TX] ERROR: not connected"
        );

        return false;
    }

    if (_tx == nullptr)
    {
        Serial0.println(
            "[BLE TX] ERROR: TX unavailable"
        );

        return false;
    }

    size_t length =
        strlen(message);

    if (length == 0)
    {
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

    _tx->setValue(
        reinterpret_cast<
            const uint8_t*
        >(message),
        length
    );

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
            "[BLE RX] Message too large"
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
    // QUEUE
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
            "[BLE RX] Queue busy"
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