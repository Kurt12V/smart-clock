#include "BluetoothCommands.h"

namespace
{
    // ========================================================
    // RESULT
    // ========================================================

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

        if (!BluetoothCommands::isValidTopic(
            topic
        ))
        {
            return fail(
                "INVALID_TOPIC",
                "Unknown topic"
            );
        }

        if (!subscriptions.subscribe(
            topic
        ))
        {
            return fail(
                "SUBSCRIBE_FAILED",
                "Unable to subscribe"
            );
        }

        Serial0.print(
            "[BLE] Subscribed: "
        );

        Serial0.println(
            topic
        );

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

        if (!BluetoothCommands::isValidTopic(
            topic
        ))
        {
            return fail(
                "INVALID_TOPIC",
                "Unknown topic"
            );
        }

        subscriptions.unsubscribe(
            topic
        );

        Serial0.print(
            "[BLE] Unsubscribed: "
        );

        Serial0.println(
            topic
        );

        return ok();
    }

    // ========================================================
    // UNSUBSCRIBE ALL
    // ========================================================

    CommandResult handleUnsubscribeAll(
        BluetoothSubscriptionManager& subscriptions
    )
    {
        subscriptions.clear();

        Serial0.println(
            "[BLE] All subscriptions cleared"
        );

        return ok();
    }
}

// ============================================================
// HAS COMMAND
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

// ============================================================
// GET BOOL
// ============================================================

bool BluetoothCommands::getBool(
    JsonObjectConst data,
    const char* key,
    bool defaultValue
)
{
    return data[key] | defaultValue;
}

// ============================================================
// GET INT
// ============================================================

int BluetoothCommands::getInt(
    JsonObjectConst data,
    const char* key,
    int defaultValue
)
{
    return data[key] | defaultValue;
}

// ============================================================
// GET UINT
// ============================================================

uint32_t BluetoothCommands::getUInt(
    JsonObjectConst data,
    const char* key,
    uint32_t defaultValue
)
{
    return data[key] | defaultValue;
}

// ============================================================
// GET FLOAT
// ============================================================

float BluetoothCommands::getFloat(
    JsonObjectConst data,
    const char* key,
    float defaultValue
)
{
    return data[key] | defaultValue;
}

// ============================================================
// GET STRING
// ============================================================

String BluetoothCommands::getString(
    JsonObjectConst data,
    const char* key,
    const char* defaultValue
)
{
    const char* value =
        data[key] | defaultValue;

    return String(
        value
    );
}

// ============================================================
// RANGE
// ============================================================

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

// ============================================================
// VALID TIME
// ============================================================

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

// ============================================================
// VALID DATE
// ============================================================

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

// ============================================================
// VALID BRIGHTNESS
// ============================================================

bool BluetoothCommands::validBrightness(
    int value
)
{
    return inRange(
        value,
        0,
        100
    );
}

// ============================================================
// VALID VOLUME
// ============================================================

bool BluetoothCommands::validVolume(
    int value
)
{
    return inRange(
        value,
        0,
        100
    );
}

// ============================================================
// VALID COB
// ============================================================

bool BluetoothCommands::validCobId(
    int id
)
{
    return inRange(
        id,
        1,
        4
    );
}

// ============================================================
// VALID TOPIC
// ============================================================

bool BluetoothCommands::isValidTopic(
    const char* topic
)
{
    if (topic == nullptr)
        return false;

    return
        strcmp(
            topic,
            BluetoothTopics::SENSORS
        ) == 0 ||

        strcmp(
            topic,
            BluetoothTopics::CLOCK
        ) == 0 ||

        strcmp(
            topic,
            BluetoothTopics::ALARMS
        ) == 0 ||

        strcmp(
            topic,
            BluetoothTopics::LIGHT
        ) == 0 ||

        strcmp(
            topic,
            BluetoothTopics::SOUND
        ) == 0 ||

        strcmp(
            topic,
            BluetoothTopics::TIMER
        ) == 0 ||

        strcmp(
            topic,
            BluetoothTopics::STOPWATCH
        ) == 0 ||

        strcmp(
            topic,
            BluetoothTopics::SYSTEM
        ) == 0;
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

    // ========================================================
    // HELLO
    // ========================================================

    if (strcmp(
        command,
        BluetoothCommands::HELLO
    ) == 0)
    {
        result =
            handleHello();
    }

    // ========================================================
    // PING
    // ========================================================

    else if (strcmp(
        command,
        BluetoothCommands::PING
    ) == 0)
    {
        result =
            handlePing();
    }

    // ========================================================
    // SUBSCRIBE
    // ========================================================

    else if (strcmp(
        command,
        BluetoothCommands::SUBSCRIBE
    ) == 0)
    {
        result =
            handleSubscribe(
                data,
                subscriptions
            );
    }

    // ========================================================
    // UNSUBSCRIBE
    // ========================================================

    else if (strcmp(
        command,
        BluetoothCommands::UNSUBSCRIBE
    ) == 0)
    {
        result =
            handleUnsubscribe(
                data,
                subscriptions
            );
    }

    // ========================================================
    // UNSUBSCRIBE ALL
    // ========================================================

    else if (strcmp(
        command,
        BluetoothCommands::UNSUBSCRIBE_ALL
    ) == 0)
    {
        result =
            handleUnsubscribeAll(
                subscriptions
            );
    }

    // ========================================================
    // KNOWN BUT NOT IMPLEMENTED
    // ========================================================

    else if (BluetoothCommands::has(
        command
    ))
    {
        result =
            fail(
                "NOT_IMPLEMENTED",
                "Command is not implemented yet"
            );
    }

    // ========================================================
    // UNKNOWN
    // ========================================================

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

    // ========================================================
    // ERROR
    // ========================================================

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

    // ========================================================
    // SUCCESS
    // ========================================================

    response =
        BluetoothProtocol::response(
            id
        );

    return true;
}