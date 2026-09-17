#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class BluetoothProtocol
{
public:

    static constexpr uint8_t VERSION = 1;

    static constexpr size_t JSON_SIZE = 1024;

    struct Request
    {
        uint8_t version = 0;

        uint32_t id = 0;

        String command;

        JsonObject data;
    };

    static bool parse(
        const String& message,
        JsonDocument& document,
        Request& request
    );

    static String response(
        uint32_t id
    );

    static String response(
        uint32_t id,
        JsonObjectConst data
    );

    static String error(
        uint32_t id,
        const char* code,
        const char* message
    );

    static String publish(
        const char* topic
    );

    static String publish(
        const char* topic,
        JsonObjectConst data
    );

private:

    BluetoothProtocol() = delete;
};