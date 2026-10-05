#pragma once

#include <Arduino.h>

#include "./hardware/sd/SDcard.h"


// ============================================================
// SD FILE ENTRY
// ============================================================

struct SDFileEntry
{
    String   path;
    uint64_t size;
    bool     isDir;
};


// ============================================================
// SD MANAGER
// ============================================================

class SDManager
{
public:

    SDManager();


    // ========================================================
    // LIFECYCLE
    // ========================================================

    bool begin(
        uint8_t csPin
    );

    void end();

    bool isReady() const;


    // ========================================================
    // CARD
    // ========================================================

    SDCard& card();

    const SDCard& card() const;


    // ========================================================
    // INFO
    // ========================================================

    SDCardInfo getInfo() const;


    // ========================================================
    // FILE LIST
    // ========================================================

    size_t listFiles(
        SDFileEntry* out,
        size_t maxFiles,
        uint8_t maxDepth = 1,
        const char* root = "/"
    ) const;


    // ========================================================
    // FILE OPERATIONS
    //
    // Добавлены для AlarmManager.
    // Работа с FS остаётся внутри SDManager.
    // ========================================================

    bool createDirectory(
        const String& path
    );

    bool fileExists(
        const String& path
    ) const;

    bool readFile(
        const String& path,
        String& content
    );

    bool writeFile(
        const String& path,
        const String& content
    );

    bool deleteFile(
        const String& path
    );


private:

    // ========================================================
    // DATA
    // ========================================================

    SDCard _card;

    bool _initialized;


    // ========================================================
    // DIRECTORY SCAN
    // ========================================================

    size_t listDir(
        const char* dirname,
        SDFileEntry* out,
        size_t maxFiles,
        size_t count,
        uint8_t depth,
        uint8_t maxDepth
    ) const;
};