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

    void setupRoutes(
        WebServer& server
    );

    bool handleDynamicRequest(
        WebServer& server
    );


private:
void handleStop(WebServer& server);

    void sendError(
        WebServer& server,
        int code,
        const char* message
    );
    void handleGetAlarms(
        WebServer& server
    );

    void handleGetAlarm(
        WebServer& server
    );

    void handleCreateAlarm(
        WebServer& server
    );

    void handleUpdateAlarm(
        WebServer& server
    );

    void handleDeleteAlarm(
        WebServer& server
    );

    void handleEnableAlarm(
        WebServer& server
    );

    void handleDisableAlarm(
        WebServer& server
    );

    void handleRuntime(
        WebServer& server
    );

    void handleDismiss(
        WebServer& server
    );

    void handleSnooze(
        WebServer& server
    );


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


    String getAlarmIdFromRequest(
        WebServer& server
    ) const;

    bool isValidAlarmId(
        const String& id
    ) const;


private:

    AlarmManager& _alarmManager;

    AlarmController& _alarmController;
};