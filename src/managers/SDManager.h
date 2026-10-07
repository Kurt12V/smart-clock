#pragma once

#include <Arduino.h>

#include "./hardware/sd/SDcard.h"


// ============================================================
// SD FILE ENTRY
// ============================================================

struct SDFileEntry
{
    String   path;
    uint64_t size = 0;
    bool     isDir = false;
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
    //
    // Старый API.
    //
    // Возвращает максимум maxFiles записей.
    // Сохраняется для совместимости с существующим кодом.
    // ========================================================

    size_t listFiles(
        SDFileEntry* out,
        size_t maxFiles,
        uint8_t maxDepth = 1,
        const char* root = "/"
    ) const;


    // ========================================================
    // PAGINATED FILE LIST
    //
    // Возвращает одну страницу файлов.
    //
    // offset:
    //     Сколько записей пропустить.
    //
    // maxFiles:
    //     Максимальное количество записей вернуть.
    //
    // hasMore:
    //     true, если после текущей страницы есть ещё записи.
    //
    // ВАЖНО:
    // Метод НЕ создаёт массив размером offset + maxFiles.
    // ========================================================

    size_t listFilesPage(
        SDFileEntry* out,
        size_t maxFiles,
        size_t offset,
        bool& hasMore,
        uint8_t maxDepth = 1,
        const char* root = "/"
    ) const;


    // ========================================================
    // FILE OPERATIONS
    //
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
    // NORMAL DIRECTORY SCAN
    // ========================================================

    size_t listDir(
        const char* dirname,
        SDFileEntry* out,
        size_t maxFiles,
        size_t count,
        uint8_t depth,
        uint8_t maxDepth
    ) const;


    // ========================================================
    // PAGINATED DIRECTORY SCAN
    // ========================================================

    size_t listDirPage(
        const char* dirname,
        SDFileEntry* out,
        size_t maxFiles,
        size_t count,
        size_t& skipped,
        size_t offset,
        bool& hasMore,
        uint8_t depth,
        uint8_t maxDepth
    ) const;
};
