#pragma once

#include <Arduino.h>
#include <cstring>

#include "BluetoothProtocol.h"
#include "./managers/BluetoothSubscriptionManager.h"
#include "BluetoothTopics.h"

namespace BluetoothCommands
{
    // ========================================================
    // SYSTEM
    // ========================================================

    static constexpr const char* HELLO       = "hello";
    static constexpr const char* PING        = "ping";
    static constexpr const char* GET_INFO    = "get_info";
    static constexpr const char* GET_STATUS  = "get_status";

    // ========================================================
    // SUBSCRIPTIONS
    // ========================================================

    static constexpr const char* SUBSCRIBE =
        "subscribe";

    static constexpr const char* UNSUBSCRIBE =
        "unsubscribe";

    static constexpr const char* UNSUBSCRIBE_ALL =
        "unsubscribe_all";

    // ========================================================
    // CLOCK
    // ========================================================

    static constexpr const char* GET_TIME =
        "get_time";

    static constexpr const char* SET_TIME =
        "set_time";

    static constexpr const char* SET_DATETIME =
        "set_datetime";

    static constexpr const char* SET_TIMEZONE =
        "set_timezone";

    // ========================================================
    // SENSORS
    // ========================================================

    static constexpr const char* GET_SENSORS =
        "get_sensors";

    // ========================================================
    // ALARMS
    // ========================================================

    static constexpr const char* ALARM_LIST =
        "alarm_list";

    static constexpr const char* ALARM_CREATE =
        "alarm_create";

    static constexpr const char* ALARM_SET =
        "alarm_set";

    static constexpr const char* ALARM_ENABLE =
        "alarm_enable";

    static constexpr const char* ALARM_DELETE =
        "alarm_delete";

    static constexpr const char* ALARM_STOP =
        "alarm_stop";

    // ========================================================
    // LIGHT
    // ========================================================

    static constexpr const char* LIGHT_GET =
        "light_get";

    static constexpr const char* MATRIX_SET =
        "matrix_set";

    static constexpr const char* COB_SET =
        "cob_set";

    static constexpr const char* COB_SET_ALL =
        "cob_set_all";

    // ========================================================
    // SOUND
    // ========================================================

    static constexpr const char* SOUND_GET =
        "sound_get";

    static constexpr const char* SOUND_VOLUME =
        "sound_volume";

    static constexpr const char* SOUND_MUTE =
        "sound_mute";

    static constexpr const char* SOUND_PLAY =
        "sound_play";

    // ========================================================
    // TIMER
    // ========================================================

    static constexpr const char* TIMER_START =
        "timer_start";

    static constexpr const char* TIMER_PAUSE =
        "timer_pause";

    static constexpr const char* TIMER_RESUME =
        "timer_resume";

    static constexpr const char* TIMER_STOP =
        "timer_stop";

    // ========================================================
    // STOPWATCH
    // ========================================================

    static constexpr const char* STOPWATCH_START =
        "stopwatch_start";

    static constexpr const char* STOPWATCH_STOP =
        "stopwatch_stop";

    static constexpr const char* STOPWATCH_RESET =
        "stopwatch_reset";

    // ========================================================
    // DISPLAY
    // ========================================================

    static constexpr const char* DISPLAY_BRIGHTNESS =
        "display_brightness";

    static constexpr const char* CLOCK_FORMAT =
        "clock_format";


    bool has(
        const char* command
    );

    bool getBool(
        JsonObjectConst data,
        const char* key,
        bool defaultValue = false
    );

    int getInt(
        JsonObjectConst data,
        const char* key,
        int defaultValue = 0
    );

    uint32_t getUInt(
        JsonObjectConst data,
        const char* key,
        uint32_t defaultValue = 0
    );

    float getFloat(
        JsonObjectConst data,
        const char* key,
        float defaultValue = 0.0f
    );

    String getString(
        JsonObjectConst data,
        const char* key,
        const char* defaultValue = ""
    );

    bool inRange(
        int value,
        int minValue,
        int maxValue
    );

    bool validTime(
        int hour,
        int minute,
        int second
    );

    bool validDate(
        int year,
        int month,
        int day
    );

    bool validBrightness(
        int value
    );

    bool validVolume(
        int value
    );

    bool validCobId(
        int id
    );

    bool isValidTopic(
        const char* topic
    );
}

bool BluetoothCommands_handle(
    const BluetoothProtocol::Request& request,
    BluetoothSubscriptionManager& subscriptions,
    String& response
);
