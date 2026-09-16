#include "BluetoothPublisher.h"

BluetoothPublisher::BluetoothPublisher(
    BluetoothManager& bluetooth,
    BluetoothSubscriptionManager& subscriptions,
    SensorManager& sensors,
    ClockSystem& clock
)
    : _bluetooth(bluetooth),
      _subscriptions(subscriptions),
      _sensors(sensors),
      _clock(clock),
      _initialized(false),
      _lastSensorPublish(0),
      _lastClockPublish(0),
      _lastSystemPublish(0)
{
}

bool BluetoothPublisher::begin()
{
    if (_initialized)
        return true;

    _lastSensorPublish = millis();
    _lastClockPublish = millis();
    _lastSystemPublish = millis();

    _initialized = true;

    return true;
}

void BluetoothPublisher::update()
{
    if (!_initialized)
        return;

    if (!_bluetooth.isConnected())
        return;

    const uint32_t now =
        millis();

    if (
        now - _lastSensorPublish >=
        SENSOR_INTERVAL_MS
    )
    {
        _lastSensorPublish = now;

        publishSensors();
    }

    if (
        now - _lastClockPublish >=
        CLOCK_INTERVAL_MS
    )
    {
        _lastClockPublish = now;

        publishClock();
    }

    if (
        now - _lastSystemPublish >=
        SYSTEM_INTERVAL_MS
    )
    {
        _lastSystemPublish = now;

        publishSystem();
    }
}

bool BluetoothPublisher::isReady() const
{
    return _initialized;
}

// ============================================================
// CHECK
// ============================================================

bool BluetoothPublisher::canPublish(
    const char* topic
) const
{
    if (!_initialized)
        return false;

    if (!_bluetooth.isConnected())
        return false;

    if (!_subscriptions.isSubscribed(topic))
        return false;

    return true;
}

// ============================================================
// SEND
// ============================================================

bool BluetoothPublisher::sendPublish(
    const char* topic,
    JsonObjectConst data
)
{
    if (!canPublish(topic))
        return false;

    String message =
        BluetoothProtocol::publish(
            topic,
            data
        );

    return _bluetooth.send(message);
}

// ============================================================
// SENSORS
// ============================================================

void BluetoothPublisher::publishSensors()
{
    if (!canPublish(
        BluetoothTopics::SENSORS
    ))
    {
        return;
    }

    JsonDocument document;

    JsonObject data =
        document.to<JsonObject>();

    String temperature =
        _sensors.getTemperatureC();

    String humidity =
        _sensors.getHumidity();

    String light =
        _sensors.getLight();

    if (temperature == "--")
        data["temperature"] = nullptr;
    else
        data["temperature"] =
            temperature.toFloat();

    if (humidity == "--")
        data["humidity"] = nullptr;
    else
        data["humidity"] =
            humidity.toFloat();

    if (light == "--")
        data["light"] = nullptr;
    else
        data["light"] =
            light.toFloat();

    sendPublish(
        BluetoothTopics::SENSORS,
        data
    );
}

// ============================================================
// CLOCK
// ============================================================

void BluetoothPublisher::publishClock()
{
    if (!canPublish(
        BluetoothTopics::CLOCK
    ))
    {
        return;
    }

    JsonDocument document;

    JsonObject data =
        document.to<JsonObject>();

    data["hour"] =
        _clock.hour();

    data["minute"] =
        _clock.minute();

    data["second"] =
        _clock.second();

    data["day"] =
        _clock.day();

    data["month"] =
        _clock.month();

    data["year"] =
        _clock.year();

    data["valid"] =
        _clock.isTimeValid();

    data["dayOfWeek"] =
        static_cast<uint8_t>(
            _clock.getDayOfWeek()
        );

    sendPublish(
        BluetoothTopics::CLOCK,
        data
    );
}

// ============================================================
// SYSTEM
// ============================================================

void BluetoothPublisher::publishSystem()
{
    if (!canPublish(
        BluetoothTopics::SYSTEM
    ))
    {
        return;
    }

    JsonDocument document;

    JsonObject data =
        document.to<JsonObject>();

    data["connected"] =
        _bluetooth.isConnected();

    data["uptime"] =
        millis() / 1000UL;

    data["subscriptions"] =
        _subscriptions.count();

    sendPublish(
        BluetoothTopics::SYSTEM,
        data
    );
}

// ============================================================
// EVENT TOPICS
// ============================================================

void BluetoothPublisher::publishAlarms(
    JsonObjectConst data
)
{
    sendPublish(
        BluetoothTopics::ALARMS,
        data
    );
}

void BluetoothPublisher::publishLight(
    JsonObjectConst data
)
{
    sendPublish(
        BluetoothTopics::LIGHT,
        data
    );
}

void BluetoothPublisher::publishSound(
    JsonObjectConst data
)
{
    sendPublish(
        BluetoothTopics::SOUND,
        data
    );
}

void BluetoothPublisher::publishTimer(
    JsonObjectConst data
)
{
    sendPublish(
        BluetoothTopics::TIMER,
        data
    );
}

void BluetoothPublisher::publishStopwatch(
    JsonObjectConst data
)
{
    sendPublish(
        BluetoothTopics::STOPWATCH,
        data
    );
}
