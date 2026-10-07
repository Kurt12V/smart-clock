#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>

class WebPageManager
{
public:

    WebPageManager();

    void setupRoutes(
        WebServer& server
    );

    void handleRoot(
        WebServer& server
    );

    bool handleNotFound(
        WebServer& server
    );


private:

    bool sendFile(
        WebServer& server,
        const String& path
    );

    const char* getContentType(
        const String& path
    ) const;
};