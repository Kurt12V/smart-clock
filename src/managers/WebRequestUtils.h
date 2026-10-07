#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>

namespace WebRequestUtils
{
    bool parseJson(
        WebServer& server,
        JsonDocument& document
    );


    void sendJson(
        WebServer& server,
        int code,
        const String& body
    );


    void sendOk(
        WebServer& server
    );


    void sendError(
        WebServer& server,
        int code,
        const char* message
    );


    bool getBoolean(
        JsonObjectConst object,
        const char* key,
        bool& value
    );


    bool getUnsigned32(
        JsonObjectConst object,
        const char* key,
        uint32_t& value
    );


    bool isValidUuidV4(
        const String& id
    );


    String getPathParameter(
        const String& uri,
        const String& prefix
    );
}