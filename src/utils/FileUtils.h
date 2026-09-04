#pragma once

#include <Arduino.h>
#include <FS.h>

class FileUtils
{
public:
    static bool exists(fs::FS& fs, const char* path);

    static bool createDir(fs::FS& fs, const char* path);

    static bool remove(fs::FS& fs, const char* path);

    static bool writeFile(fs::FS& fs,
                          const char* path,
                          const String& data);

    static bool appendFile(fs::FS& fs,
                           const char* path,
                           const String& data);

    static String readFile(fs::FS& fs,
                           const char* path);

    static size_t fileSize(fs::FS& fs,
                           const char* path);

    static bool copyFile(fs::FS& fs,
                         const char* source,
                         const char* destination);
};