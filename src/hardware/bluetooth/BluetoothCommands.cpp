#include "BluetoothCommands.h"

namespace
{
    struct CommandResult
    {
        bool handled;
        bool success;

        const char* errorCode;
        const char* errorMessage;
    };

    CommandResult ok()
    {
        return {
            true,
            true,
            nullptr,
            nullptr
        };
    }

    CommandResult fail(
        const char* code,
        const char* message
    )
    {
        return {
            true,
            false,
            code,
            message
        };
    }

    CommandResult unknown()
    {
        return {
            false,
            false,
            "UNKNOWN_COMMAND",
            "Unknown command"
        };
    }

    // ========================================================
    // SUBSCRIBE
    // ========================================================

    CommandResult handleSubscribe(
        JsonObjectConst data,
        BluetoothSubscriptionManager& subscriptions
    )
    {
        const char* topic =
            data["topic"] | nullptr;

        if (topic == nullptr)
        {
            return fail(
                "INVALID_TOPIC",
                "Topic is required"
            );
        }

        if (!BluetoothCommands::isValidTopic(topic))
        {
            return fail(
                "INVALID_TOPIC",
                "Unknown topic"
            );
        }

        if (!subscriptions.subscribe(topic))
        {
            return fail(
                "SUBSCRIBE_FAILED",
                "Unable to subscribe"
            );
        }

        return ok();
    }

    // ========================================================
    // UNSUBSCRIBE
    // ========================================================

    CommandResult handleUnsubscribe(
        JsonObjectConst data,
        BluetoothSubscriptionManager& subscriptions
    )
    {
        const char* topic =
            data["topic"] | nullptr;

        if (topic == nullptr)
        {
            return fail(
                "INVALID_TOPIC",
                "Topic is required"
            );
        }

        if (!BluetoothCommands::isValidTopic(topic))
        {
            return fail(
                "INVALID_TOPIC",
                "Unknown topic"
            );
        }

        subscriptions.unsubscribe(topic);

        return ok();
    }

    // ========================================================
    // HELLO
    // ========================================================

    CommandResult handleHello()
    {
        return ok();
    }

    // ========================================================
    // PING
    // ========================================================

    CommandResult handlePing()
    {
        return ok();
    }
}

// ============================================================
// HELPERS
// ============================================================

bool BluetoothCommands::has(
    const char* command
)
{
    if (command == nullptr)
        return false;

    return
        strcmp(command, HELLO) == 0 ||
        strcmp(command, PING) == 0 ||
        strcmp(command, GET_INFO) == 0 ||
        strcmp(command, GET_STATUS) == 0 ||

        strcmp(command, SUBSCRIBE) == 0 ||
        strcmp(command, UNSUBSCRIBE) == 0 ||
        strcmp(command, UNSUBSCRIBE_ALL) == 0 ||

        strcmp(command, GET_TIME) == 0 ||
        strcmp(command, SET_TIME) == 0 ||
        strcmp(command, SET_DATETIME) == 0 ||
        strcmp(command, SET_TIMEZONE) == 0 ||

        strcmp(command, GET_SENSORS) == 0 ||

        strcmp(command, ALARM_LIST) == 0 ||
        strcmp(command, ALARM_CREATE) == 0 ||
        strcmp(command, ALARM_SET) == 0 ||
        strcmp(command, ALARM_ENABLE) == 0 ||
        strcmp(command, ALARM_DELETE) == 0 ||
        strcmp(command, ALARM_STOP) == 0 ||

        strcmp(command, LIGHT_GET) == 0 ||
        strcmp(command, MATRIX_SET) == 0 ||
        strcmp(command, COB_SET) == 0 ||
        strcmp(command, COB_SET_ALL) == 0 ||

        strcmp(command, SOUND_GET) == 0 ||
        strcmp(command, SOUND_VOLUME) == 0 ||
        strcmp(command, SOUND_MUTE) == 0 ||
        strcmp(command, SOUND_PLAY) == 0 ||

        strcmp(command, TIMER_START) == 0 ||
        strcmp(command, TIMER_PAUSE) == 0 ||
        strcmp(command, TIMER_RESUME) == 0 ||
        strcmp(command, TIMER_STOP) == 0 ||

        strcmp(command, STOPWATCH_START) == 0 ||
        strcmp(command, STOPWATCH_STOP) == 0 ||
        strcmp(command, STOPWATCH_RESET) == 0 ||

        strcmp(command, DISPLAY_BRIGHTNESS) == 0 ||
        strcmp(command, CLOCK_FORMAT) == 0;
}

bool BluetoothCommands::getBool(
    JsonObjectConst data,
    const char* key,
    bool defaultValue
)
{
    return data[key] | defaultValue;
}

int BluetoothCommands::getInt(
    JsonObjectConst data,
    const char* key,
    int defaultValue
)
{
    return data[key] | defaultValue;
}

uint32_t BluetoothCommands::getUInt(
    JsonObjectConst data,
    const char* key,
    uint32_t defaultValue
)
{
    return data[key] | defaultValue;
}

float BluetoothCommands::getFloat(
    JsonObjectConst data,
    const char* key,
    float defaultValue
)
{
    return data[key] | defaultValue;
}

String BluetoothCommands::getString(
    JsonObjectConst data,
    const char* key,
    const char* defaultValue
)
{
    const char* value =
        data[key] | defaultValue;

    return String(value);
}

bool BluetoothCommands::inRange(
    int value,
    int minValue,
    int maxValue
)
{
    return
        value >= minValue &&
        value <= maxValue;
}

bool BluetoothCommands::validTime(
    int hour,
    int minute,
    int second
)
{
    return
        inRange(hour, 0, 23) &&
        inRange(minute, 0, 59) &&
        inRange(second, 0, 59);
}

bool BluetoothCommands::validDate(
    int year,
    int month,
    int day
)
{
    return
        year >= 2000 &&
        year <= 2099 &&
        month >= 1 &&
        month <= 12 &&
        day >= 1 &&
        day <= 31;
}

bool BluetoothCommands::validBrightness(
    int value
)
{
    return inRange(value, 0, 100);
}

bool BluetoothCommands::validVolume(
    int value
)
{
    return inRange(value, 0, 100);
}

bool BluetoothCommands::validCobId(
    int id
)
{
    return inRange(id, 1, 4);
}

bool BluetoothCommands::isValidTopic(
    const char* topic
)
{
    if (topic == nullptr)
        return false;

    return
        strcmp(topic, BluetoothTopics::SENSORS) == 0 ||
        strcmp(topic, BluetoothTopics::CLOCK) == 0 ||
        strcmp(topic, BluetoothTopics::ALARMS) == 0 ||
        strcmp(topic, BluetoothTopics::LIGHT) == 0 ||
        strcmp(topic, BluetoothTopics::SOUND) == 0 ||
        strcmp(topic, BluetoothTopics::TIMER) == 0 ||
        strcmp(topic, BluetoothTopics::STOPWATCH) == 0 ||
        strcmp(topic, BluetoothTopics::SYSTEM) == 0;
}

// ============================================================
// COMMAND ROUTER
// ============================================================

bool BluetoothCommands_handle(
    const BluetoothProtocol::Request& request,
    BluetoothSubscriptionManager& subscriptions,
    String& response
)
{
    const uint32_t id =
        request.id;

    const char* command =
        request.command.c_str();

    JsonObjectConst data =
        request.data;

    CommandResult result =
        unknown();

    // --------------------------------------------------------
    // SYSTEM
    // --------------------------------------------------------

    if (strcmp(command, BluetoothCommands::HELLO) == 0)
    {
        result = handleHello();
    }
    else if (
        strcmp(command, BluetoothCommands::PING) == 0
    )
    {
        result = handlePing();
    }

    // --------------------------------------------------------
    // SUBSCRIPTIONS
    // --------------------------------------------------------

    else if (
        strcmp(command, BluetoothCommands::SUBSCRIBE) == 0
    )
    {
        result =
            handleSubscribe(
                data,
                subscriptions
            );
    }
    else if (
        strcmp(command, BluetoothCommands::UNSUBSCRIBE) == 0
    )
    {
        result =
            handleUnsubscribe(
                data,
                subscriptions
            );
    }
    else if (
        strcmp(command, BluetoothCommands::UNSUBSCRIBE_ALL) == 0
    )
    {
        subscriptions.clear();

        result = ok();
    }

    // --------------------------------------------------------
    // NOT IMPLEMENTED YET
    // --------------------------------------------------------

    else if (
        BluetoothCommands::has(command)
    )
    {
        result =
            fail(
                "NOT_IMPLEMENTED",
                "Command is not implemented yet"
            );
    }

    // --------------------------------------------------------
    // UNKNOWN
    // --------------------------------------------------------

    if (!result.handled)
    {
        response =
            BluetoothProtocol::error(
                id,
                "UNKNOWN_COMMAND",
                "Unknown command"
            );

        return false;
    }

    if (!result.success)
    {
        response =
            BluetoothProtocol::error(
                id,
                result.errorCode,
                result.errorMessage
            );

        return false;
    }

    response =
        BluetoothProtocol::response(id);

    return true;
}
