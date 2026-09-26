#include "JsonUtils.h"
#include <ArduinoJson.h>
bool JsonUtils::load(fs::FS& fs,
                     const char* path,
                     JsonDocument& document)
{
    if (!path)
        return false;

    File file = fs.open(path, FILE_READ);

    if (!file)
        return false;

    DeserializationError error =
        deserializeJson(document, file);

    file.close();

    return !error;
}

bool JsonUtils::save(fs::FS& fs,
                     const char* path,
                     const JsonDocument& document)
{
    if (!path)
        return false;

    File file = fs.open(path, FILE_WRITE);

    if (!file)
        return false;

    size_t written =
        serializeJsonPretty(document, file);

    file.close();

    return written > 0;
}

String JsonUtils::serialize(const JsonDocument& document)
{
    String output;

    serializeJson(document, output);

    return output;
}

bool JsonUtils::deserialize(const String& json,
                            JsonDocument& document)
{
    if (json.isEmpty())
        return false;

    DeserializationError error =
        deserializeJson(document, json);

    return !error;
}
