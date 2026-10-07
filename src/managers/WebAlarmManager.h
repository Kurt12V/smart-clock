#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "AlarmManager.h"
#include "AlarmController.h"

class WebAlarmManager
{
public:
    WebAlarmManager(
        AlarmManager& alarmManager,
        AlarmController& alarmController
    );

    void setupRoutes(WebServer& server);

    bool handleDynamicRequest(WebServer& server);

private:
    // ========================================================
    // REST API
    // ========================================================

    void handleGetAlarms(WebServer& server);
    void handleGetAlarm(WebServer& server);
    void handleCreateAlarm(WebServer& server);
    void handleUpdateAlarm(WebServer& server);
    void handleDeleteAlarm(WebServer& server);
    void handleSetEnabled(WebServer& server);

    // ========================================================
    // Runtime API
    // ========================================================

    void handleRuntime(WebServer& server);
    void handleDismiss(WebServer& server);
    void handleSnooze(WebServer& server);
    void handleStop(WebServer& server);

    // ========================================================
    // Serialization
    // ========================================================

    bool parseAlarmFromRequest(
        WebServer& server,
        Alarm& alarm
    );

    void sendAlarm(
        WebServer& server,
        const Alarm& alarm
    );

    void sendAlarmList(
        WebServer& server
    );

    // ========================================================
    // Helpers
    // ========================================================

    String getAlarmIdFromRequest(
        WebServer& server
    ) const;

    bool isValidAlarmId(
        const String& id
    ) const;

    void sendError(
        WebServer& server,
        int code,
        const char* message
    );

    void sendOk(
        WebServer& server
    );

private:
    AlarmManager& _alarmManager;
    AlarmController& _alarmController;
};
