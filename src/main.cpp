#include <Arduino.h>

#include "Config.h"
#include "Pins.h"
#include "Constants.h"
#include "Version.h"

#include "./core/ClockSystem.h"
#include "./managers/SensorsManager.h"

#include "./managers/BluetoothManager.h"
#include "./managers/BluetoothSubscriptionManager.h"
#include "./hardware/bluetooth/BluetoothPublisher.h"


// ============================================================
// GLOBAL OBJECTS
// ============================================================
Settings::Clock clockSettings;
ClockSystem clockSystem(clockSettings);
SensorManager sensorManager;

BluetoothManager bluetoothManager;
BluetoothSubscriptionManager bluetoothSubscriptions;

BluetoothPublisher bluetoothPublisher(
    bluetoothManager,
    bluetoothSubscriptions,
    sensorManager,
    clockSystem
);


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println("      SMART CLOCK ESP32-S3");
    Serial.println("================================");

    // --------------------------------------------------------
    // CLOCK
    // --------------------------------------------------------

    Serial.println("[INIT] ClockSystem...");

    if (!clockSystem.begin())
    {
        Serial.println("[INIT] ClockSystem FAILED");
    }
    else
    {
        Serial.println("[INIT] ClockSystem OK");
    }


    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    Serial.println("[INIT] SensorManager...");

    if (!sensorManager.begin())
    {
        Serial.println("[INIT] SensorManager FAILED");
    }
    else
    {
        Serial.println("[INIT] SensorManager OK");
    }


    // --------------------------------------------------------
    // BLUETOOTH
    // --------------------------------------------------------

    Serial.println("[INIT] BluetoothManager...");

    if (!bluetoothManager.begin())
    {
        Serial.println("[INIT] BluetoothManager FAILED");
    }
    else
    {
        Serial.println("[INIT] BluetoothManager OK");
    }


    // --------------------------------------------------------
    // BLUETOOTH PUBLISHER
    // --------------------------------------------------------

    Serial.println("[INIT] BluetoothPublisher...");

    if (!bluetoothPublisher.begin())
    {
        Serial.println("[INIT] BluetoothPublisher FAILED");
    }
    else
    {
        Serial.println("[INIT] BluetoothPublisher OK");
    }


    // --------------------------------------------------------
    // READY
    // --------------------------------------------------------

    Serial.println();
    Serial.println("================================");
    Serial.println("       SMART CLOCK READY");
    Serial.println("================================");
    Serial.println();

    Serial.println("[SYSTEM] BLE waiting for connection...");
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // CLOCK
    // --------------------------------------------------------

    clockSystem.update();


    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    sensorManager.update();


    // --------------------------------------------------------
    // BLUETOOTH COMMAND PROCESSING
    // --------------------------------------------------------
    //
    // Здесь обрабатываются команды:
    //
    // hello
    // ping
    // subscribe
    // unsubscribe
    // unsubscribe_all
    // get_info
    // get_status
    // ...
    //
    // BluetoothManager получает JSON из BLE callback,
    // а здесь уже разбирает и выполняет его.
    // --------------------------------------------------------

    bluetoothManager.update();


    // --------------------------------------------------------
    // BLUETOOTH PUBLISHER
    // --------------------------------------------------------
    //
    // Публикует:
    //
    // sensors -> раз в 1 сек
    // clock   -> раз в 1 сек
    // system  -> раз в 5 сек
    //
    // Только если Android подписан на соответствующий topic.
    // --------------------------------------------------------

    bluetoothPublisher.update();


    // --------------------------------------------------------
    // SMALL DELAY
    // --------------------------------------------------------

    delay(1);
}