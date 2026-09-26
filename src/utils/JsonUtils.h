#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>

class JsonUtils
{
public:
    static bool load(fs::FS& fs,
                     const char* path,
                     JsonDocument& document);

    static bool save(fs::FS& fs,
                     const char* path,
                     const JsonDocument& document);

    static String serialize(const JsonDocument& document);

    static bool deserialize(const String& json,
                            JsonDocument& document);
};
