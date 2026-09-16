#include "BluetoothManager.h"

#include <BLEAdvertising.h>
#include <esp_gap_ble_api.h>

// ============================================================
// SERVER CALLBACKS
// ============================================================

class BluetoothManager::ServerCallbacks
    : public BLEServerCallbacks
{
public:
    explicit ServerCallbacks(
        BluetoothManager& manager
    )
        : _manager(manager)
    {
    }

    void onConnect(BLEServer* server) override
    {
        (void)server;

        _manager._connected = true;

        Serial.println("[BLE] Client connected");
    }

    void onDisconnect(BLEServer* server) override
    {
        (void)server;

        _manager._connected = false;

        if (_manager._subscriptions != nullptr)
        {
            _manager._subscriptions->clear();
        }

        Serial.println("[BLE] Client disconnected");

        BLEDevice::startAdvertising();

        Serial.println("[BLE] Advertising restarted");
    }

private:
    BluetoothManager& _manager;
};

// ============================================================
// RX CALLBACKS
// ============================================================

class BluetoothManager::RXCallbacks
    : public BLECharacteristicCallbacks
{
public:
    explicit RXCallbacks(
        BluetoothManager& manager
    )
        : _manager(manager)
    {
    }

    void onWrite(
        BLECharacteristic* characteristic
    ) override
    {
        std::string value =
            characteristic->getValue();

        if (value.empty())
            return;

        _manager._command =
            String(value.c_str());

        Serial.print("[BLE RX] ");
        Serial.println(_manager._command);
    }

private:
    BluetoothManager& _manager;
};

// ============================================================
// SECURITY CALLBACKS
// ============================================================

class BluetoothManager::SecurityCallbacks
    : public BLESecurityCallbacks
{
public:
    explicit SecurityCallbacks(
        BluetoothManager& manager
    )
        : _manager(manager)
    {
    }

    uint32_t onPassKeyRequest()
    {
        return _manager._passKey;
    }

    void onPassKeyNotify(
        uint32_t pass_key
    )
    {
        Serial.printf(
            "[BLE] Passkey: %06lu\n",
            static_cast<unsigned long>(pass_key)
        );
    }

    bool onConfirmPIN(
        uint32_t pass_key
    )
    {
        Serial.printf(
            "[BLE] Confirm PIN: %06lu\n",
            static_cast<unsigned long>(pass_key)
        );

        return true;
    }

    bool onSecurityRequest()
    {
        return true;
    }

    void onAuthenticationComplete(
        esp_ble_auth_cmpl_t auth_cmpl
    )
    {
        if (auth_cmpl.success)
        {
            Serial.println(
                "[BLE] Authentication successful"
            );
        }
        else
        {
            Serial.printf(
                "[BLE] Authentication failed: %d\n",
                auth_cmpl.fail_reason
            );
        }
    }

private:
    BluetoothManager& _manager;
};

// ============================================================
// CONSTRUCTOR
// ============================================================

BluetoothManager::BluetoothManager()
    : _server(nullptr),
      _service(nullptr),
      _txCharacteristic(nullptr),
      _rxCharacteristic(nullptr),
      _initialized(false),
      _connected(false),
      _command(""),
      _passKey(123456),
      _subscriptions(nullptr)
{
}

// ============================================================
// BEGIN
// ============================================================

bool BluetoothManager::begin()
{
    if (_initialized)
        return true;

    Serial.println(
        "[BLE] Initializing Bluetooth..."
    );

    BLEDevice::init(DEVICE_NAME);

    // --------------------------------------------------------
    // Security
    // --------------------------------------------------------

    BLESecurity* security =
        new BLESecurity();

    security->setAuthenticationMode(
        ESP_LE_AUTH_REQ_SC_MITM_BOND
    );

    security->setCapability(
        ESP_IO_CAP_OUT
    );

    security->setInitEncryptionKey(
        ESP_BLE_ENC_KEY_MASK |
        ESP_BLE_ID_KEY_MASK
    );

    security->setKeySize(16);

    BLEDevice::setSecurityCallbacks(
        new SecurityCallbacks(*this)
    );

    // --------------------------------------------------------
    // Server
    // --------------------------------------------------------

    _server =
        BLEDevice::createServer();

    if (_server == nullptr)
        return false;

    _server->setCallbacks(
        new ServerCallbacks(*this)
    );

    // --------------------------------------------------------
    // Service
    // --------------------------------------------------------

    _service =
        _server->createService(
            SERVICE_UUID
        );

    if (_service == nullptr)
        return false;

    // --------------------------------------------------------
    // TX
    // ESP32 -> Android
    // --------------------------------------------------------

    _txCharacteristic =
        _service->createCharacteristic(
            TX_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY
        );

    if (_txCharacteristic == nullptr)
        return false;

    _txCharacteristic->addDescriptor(
        new BLE2902()
    );

    // --------------------------------------------------------
    // RX
    // Android -> ESP32
    // --------------------------------------------------------

    _rxCharacteristic =
        _service->createCharacteristic(
            RX_UUID,
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR
        );

    if (_rxCharacteristic == nullptr)
        return false;

    _rxCharacteristic->setCallbacks(
        new RXCallbacks(*this)
    );

    // --------------------------------------------------------
    // Start service
    // --------------------------------------------------------

    _service->start();

    // --------------------------------------------------------
    // Advertising
    // --------------------------------------------------------

    BLEAdvertising* advertising =
        BLEDevice::getAdvertising();

    advertising->addServiceUUID(
        SERVICE_UUID
    );

    advertising->setScanResponse(true);

    advertising->setMinPreferred(0x06);
    advertising->setMinPreferred(0x12);

    BLEDevice::startAdvertising();

    _initialized = true;

    Serial.println(
        "[BLE] Bluetooth ready"
    );

    Serial.println(
        "[BLE] Device: SmartClock"
    );

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void BluetoothManager::update()
{
    // BLE работает через callbacks.
    // Здесь пока ничего выполнять не нужно.
}

// ============================================================
// SEND
// ============================================================

bool BluetoothManager::send(
    const String& message
)
{
    if (!_initialized)
        return false;

    if (!_connected)
        return false;

    if (_txCharacteristic == nullptr)
        return false;

    _txCharacteristic->setValue(
        message.c_str()
    );

    _txCharacteristic->notify();

    return true;
}

// ============================================================
// STATE
// ============================================================

bool BluetoothManager::isReady() const
{
    return _initialized;
}

bool BluetoothManager::isConnected() const
{
    return _connected;
}

// ============================================================
// COMMAND
// ============================================================

bool BluetoothManager::hasCommand() const
{
    return !_command.isEmpty();
}

String BluetoothManager::getCommand()
{
    String result = _command;

    _command = "";

    return result;
}

// ============================================================
// PASSKEY
// ============================================================

void BluetoothManager::setPassKey(
    uint32_t passKey
)
{
    if (passKey > 999999)
        passKey = 999999;

    _passKey = passKey;
}

// ============================================================
// SUBSCRIPTIONS
// ============================================================

void BluetoothManager::setSubscriptionManager(
    BluetoothSubscriptionManager* manager
)
{
    _subscriptions = manager;
}
