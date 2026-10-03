#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "./core/ClockSystem.h"
#include "./managers/SDManager.h"
#include "./models/AlarmData.h"

class AlarmManager
{
public:
    using TriggerCallback = void (*)(const AlarmData& alarm);

    AlarmManager(
        ClockSystem& clockSystem,
        SDManager& sdManager
    );

    bool begin();
    void update();

    // ============================================================
    // ALARMS
    // ============================================================

    bool create(
        const AlarmData& alarm
    );

    bool updateAlarm(
        const AlarmData& alarm
    );

    bool remove(
        const char* id
    );

    bool setEnabled(
        const char* id,
        bool enabled
    );

    bool loadAlarm(
        const char* id,
        AlarmData& alarm
    );

    // ============================================================
    // SCHEDULE
    // ============================================================

    AlarmScheduleData* findSchedule(
        const char* id
    );

    const AlarmScheduleData* findSchedule(
        const char* id
    ) const;

    const AlarmScheduleData* getSchedule(
        uint8_t index
    ) const;

    uint8_t count() const;

    // ============================================================
    // ACTIVE ALARM
    // ============================================================

    bool hasActiveAlarm() const;

    const ActiveAlarmData& getActiveAlarm() const;

    void dismiss();

    // ============================================================
    // CALLBACK
    // ============================================================

    void setTriggerCallback(
        TriggerCallback callback
    );

private:

    static constexpr const char* ALARMS_PATH =
        "/config/alarms.json";

    static constexpr uint8_t SCHEMA_VERSION = 1;

    // ============================================================
    // DEPENDENCIES
    // ============================================================

    ClockSystem& _clockSystem;
    SDManager& _sdManager;

    // ============================================================
    // SCHEDULE
    // ============================================================

    AlarmScheduleData _schedule[
        AlarmLimits::MAX_ALARMS
    ];

    uint8_t _scheduleCount;

    // ============================================================
    // ACTIVE ALARM
    // ============================================================

    ActiveAlarmData _activeAlarm;

    // ============================================================
    // STATE
    // ============================================================

    bool _initialized;

    TriggerCallback _triggerCallback;

    // ============================================================
    // FILE
    // ============================================================

    bool ensureFileExists();

    bool createEmptyFile();

    bool readDocument(
        JsonDocument& document
    );

    bool writeDocument(
        const JsonDocument& document
    );

    // ============================================================
    // LOAD / PARSE
    // ============================================================

    bool loadSchedule();

    bool parseSchedule(
        JsonDocument& document
    );

    bool jsonToSchedule(
        JsonObjectConst object,
        AlarmScheduleData& schedule
    );

    bool jsonToAlarm(
        JsonObjectConst object,
        AlarmData& alarm
    );

    bool alarmToJson(
        JsonObject object,
        const AlarmData& alarm
    );

    // ============================================================
    // TIME
    // ============================================================

    time_t calculateNextTrigger(
        const AlarmScheduleData& alarm,
        time_t from
    ) const;

    void checkAlarms();

    void trigger(
        AlarmScheduleData& schedule
    );

    // ============================================================
    // UTILITY
    // ============================================================

    static void copyString(
        char* destination,
        size_t destinationSize,
        const char* source
    );
};