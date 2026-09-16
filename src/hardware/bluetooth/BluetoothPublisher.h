#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "./managers/BluetoothManager.h"
#include "BluetoothProtocol.h"
#include "./managers/BluetoothSubscriptionManager.h"
#include "BluetoothTopics.h"

#include "./managers/SensorsManager.h"
#include "./core/ClockSystem.h"

class BluetoothPublisher
{
public:
    static constexpr uint32_t SENSOR_INTERVAL_MS =
        1000;

    static constexpr uint32_t CLOCK_INTERVAL_MS =
        1000;

    static constexpr uint32_t SYSTEM_INTERVAL_MS =
        5000;

    BluetoothPublisher(
        BluetoothManager& bluetooth,
        BluetoothSubscriptionManager& subscriptions,
        SensorManager& sensors,
        ClockSystem& clock
    );

    bool begin();

    void update();

    bool isReady() const;

    void publishSensors();
    void publishClock();
    void publishSystem();

    void publishAlarms(
        JsonObjectConst data
    );

    void publishLight(
        JsonObjectConst data
    );

    void publishSound(
        JsonObjectConst data
    );

    void publishTimer(
        JsonObjectConst data
    );

    void publishStopwatch(
        JsonObjectConst data
    );

private:
    bool canPublish(
        const char* topic
    ) const;

    bool sendPublish(
        const char* topic,
        JsonObjectConst data
    );

private:
    BluetoothManager& _bluetooth;
    BluetoothSubscriptionManager& _subscriptions;

    SensorManager& _sensors;
    ClockSystem& _clock;

    bool _initialized;

    uint32_t _lastSensorPublish;
    uint32_t _lastClockPublish;
    uint32_t _lastSystemPublish;
};

