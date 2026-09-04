#include "Logger.h"

Logger::Level Logger::_level =
    Logger::Level::Debug;

void Logger::begin(unsigned long baud)
{
    Serial.begin(baud);
}

void Logger::setLevel(Level level)
{
    _level = level;
}

Logger::Level Logger::getLevel()
{
    return _level;
}

const char* Logger::levelName(Level level)
{
    switch (level)
    {
        case Level::Debug:
            return "DEBUG";

        case Level::Info:
            return "INFO";

        case Level::Warning:
            return "WARNING";

        case Level::Error:
            return "ERROR";

        default:
            return "UNKNOWN";
    }
}

void Logger::print(Level level,
                   const char* format,
                   va_list args)
{
    if (level < _level)
        return;

    Serial.print("[");
    Serial.print(millis());
    Serial.print("] ");

    Serial.print("[");
    Serial.print(levelName(level));
    Serial.print("] ");

    char buffer[256];

    vsnprintf(
        buffer,
        sizeof(buffer),
        format,
        args
    );

    Serial.println(buffer);
}

void Logger::debug(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    print(Level::Debug, format, args);

    va_end(args);
}

void Logger::info(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    print(Level::Info, format, args);

    va_end(args);
}

void Logger::warning(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    print(Level::Warning, format, args);

    va_end(args);
}

void Logger::error(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    print(Level::Error, format, args);

    va_end(args);
}