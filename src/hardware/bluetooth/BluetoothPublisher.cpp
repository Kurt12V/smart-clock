#include "BluetoothPublisher.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

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

// ============================================================
// BEGIN
// ============================================================

bool BluetoothPublisher::begin()
{
    if (_initialized)
    {
        return true;
    }

    uint32_t now =
        millis();

    _lastSensorPublish =
        now;

    _lastClockPublish =
        now;

    _lastSystemPublish =
        now;

    _initialized =
        true;

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void BluetoothPublisher::update()
{
    if (!_initialized)
    {
        return;
    }

    if (!_bluetooth.isConnected())
    {
        return;
    }

    uint32_t now =
        millis();

    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    if (
        now - _lastSensorPublish >=
        SENSOR_INTERVAL_MS
    )
    {
        _lastSensorPublish =
            now;

        publishSensors();
    }

    // --------------------------------------------------------
    // CLOCK
    // --------------------------------------------------------

    if (
        now - _lastClockPublish >=
        CLOCK_INTERVAL_MS
    )
    {
        _lastClockPublish =
            now;

        publishClock();
    }

    // --------------------------------------------------------
    // SYSTEM
    // --------------------------------------------------------

    if (
        now - _lastSystemPublish >=
        SYSTEM_INTERVAL_MS
    )
    {
        _lastSystemPublish =
            now;

        publishSystem();
    }
}

// ============================================================
// READY
// ============================================================

bool BluetoothPublisher::isReady() const
{
    return _initialized;
}

// ============================================================
// CAN PUBLISH
// ============================================================

bool BluetoothPublisher::canPublish(
    const char* topic
) const
{
    if (!_initialized)
    {
        return false;
    }

    if (!_bluetooth.isConnected())
    {
        return false;
    }

    if (!_subscriptions.isSubscribed(
        topic
    ))
    {
        return false;
    }

    return true;
}

// ============================================================
// SEND PUBLISH
// ============================================================

bool BluetoothPublisher::sendPublish(
    const char* topic,
    JsonObjectConst data
)
{
    if (!canPublish(topic))
    {
        return false;
    }

    String message =
        BluetoothProtocol::publish(
            topic,
            data
        );

    return _bluetooth.send(
        message
    );
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

    // --------------------------------------------------------
    // TEMPERATURE
    // --------------------------------------------------------

    String temperature =
        _sensors.getTemperatureC();

    if (temperature == "--")
    {
        data["temperature"] =
            nullptr;
    }
    else
    {
        data["temperature"] =
            temperature.toFloat();
    }

    // --------------------------------------------------------
    // HUMIDITY
    // --------------------------------------------------------

    String humidity =
        _sensors.getHumidity();

    if (humidity == "--")
    {
        data["humidity"] =
            nullptr;
    }
    else
    {
        data["humidity"] =
            humidity.toFloat();
    }

    // --------------------------------------------------------
    // LIGHT
    // --------------------------------------------------------

    String light =
        _sensors.getLight();

    if (light == "--")
    {
        data["light"] =
            nullptr;
    }
    else
    {
        data["light"] =
            light.toFloat();
    }

    // --------------------------------------------------------
    // DISTANCE
    // --------------------------------------------------------

    // Если в твоём SensorManager уже есть getDistance(),
    // раскомментируй этот блок.
    //
    // String distance =
    //     _sensors.getDistance();
    //
    // if (distance == "--")
    // {
    //     data["distance"] = nullptr;
    // }
    // else
    // {
    //     data["distance"] =
    //         distance.toFloat();
    // }

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
// ALARMS
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

// ============================================================
// LIGHT
// ============================================================

void BluetoothPublisher::publishLight(
    JsonObjectConst data
)
{
    sendPublish(
        BluetoothTopics::LIGHT,
        data
    );
}

// ============================================================
// SOUND
// ============================================================

void BluetoothPublisher::publishSound(
    JsonObjectConst data
)
{
    sendPublish(
        BluetoothTopics::SOUND,
        data
    );
}

// ============================================================
// TIMER
// ============================================================

void BluetoothPublisher::publishTimer(
    JsonObjectConst data
)
{
    sendPublish(
        BluetoothTopics::TIMER,
        data
    );
}

// ============================================================
// STOPWATCH
// ============================================================

void BluetoothPublisher::publishStopwatch(
    JsonObjectConst data
)
{
    sendPublish(
        BluetoothTopics::STOPWATCH,
        data
    );
}