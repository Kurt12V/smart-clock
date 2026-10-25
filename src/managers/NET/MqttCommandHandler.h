#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "SmartClockMqttClient.h"

class MqttCommandHandler {
public:

    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    MqttCommandHandler();


    // ========================================================
    // INITIALIZATION
    // ========================================================

    bool begin(
        SmartClockMqttClient& mqttClient
    );


    // ========================================================
    // COMMAND CALLBACK
    // ========================================================

    static void onMqttEvent(
        SmartClockMqttClient::Event event,
        const char* topic,
        const char* payload
    );


private:

    // ========================================================
    // INSTANCE
    // ========================================================

    static MqttCommandHandler* _instance;


    // ========================================================
    // MQTT CLIENT
    // ========================================================

    SmartClockMqttClient* _mqttClient;


    // ========================================================
    // COMMAND PROCESSING
    // ========================================================

    void handleCommand(
        const char* payload
    );


    // ========================================================
    // COMMANDS
    // ========================================================

    void executePing(
        const char* requestId
    );

    void executeGetStatus(
        const char* requestId
    );

    void executeRestart(
        const char* requestId
    );


    // ========================================================
    // ACK
    // ========================================================

    bool sendAck(
        const char* requestId,
        const char* command,
        bool success,
        const char* message
    );


    // ========================================================
    // UNKNOWN COMMAND
    // ========================================================

    void executeUnknownCommand(
        const char* requestId,
        const char* command
    );
};