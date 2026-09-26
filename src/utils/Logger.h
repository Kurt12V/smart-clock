#pragma once

#include <Arduino.h>

class Logger
{
public:

    enum class Level : uint8_t
    {
        Debug = 0,
        Info,
        Warning,
        Error,
        None
    };

    static void begin(unsigned long baud = 115200);

    static void setLevel(Level level);

    static Level getLevel();

    static void debug(const char* format, ...);

    static void info(const char* format, ...);

    static void warning(const char* format, ...);

    static void error(const char* format, ...);

private:

    static Level _level;

    static void print(Level level,
                      const char* format,
                      va_list args);

    static const char* levelName(Level level);
};