#pragma once

#include <Arduino.h>
#include "./hardware/sd/SDcard.h"

// ============================================================
// FILE ENTRY
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

    bool begin(uint8_t csPin);
    void end();

    bool isReady() const;

    SDCard&       card();
    const SDCard& card() const;

    SDCardInfo getInfo() const;

    // --------------------------------------------------------
    // LIST FILES
    // --------------------------------------------------------

    size_t listFiles(
        SDFileEntry* out,
        size_t       maxFiles,
        uint8_t      maxDepth = 1,
        const char*  root = "/"
    ) const;

private:

    SDCard _card;
    bool   _initialized;

    size_t listDir(
        const char*  dirname,
        SDFileEntry* out,
        size_t       maxFiles,
        size_t       count,
        uint8_t      depth,
        uint8_t      maxDepth
    ) const;
};