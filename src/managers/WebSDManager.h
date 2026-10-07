#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "SDManager.h"

class WebSDManager
{
public:

    explicit WebSDManager(
        SDManager& sd
    );

    void setupRoutes(
        WebServer& server
    );


private:

    void handleSD(
        WebServer& server
    );


private:

    SDManager& _sd;
};