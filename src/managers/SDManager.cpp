#include "SDManager.h"
#include "./utils/Logger.h"

#include <cstring>

// ============================================================
// CONSTRUCTOR
// ============================================================

SDManager::SDManager()
    : _initialized(false)
{
}

// ============================================================
// BEGIN
// ============================================================

bool SDManager::begin(uint8_t csPin)
{
    if (_initialized)
        return true;

    Logger::info("SD", "Initializing SD card...");

    if (!_card.begin(csPin))
    {
        Logger::error("SD", "Failed to initialize SD card");

        _initialized = false;
        return false;
    }

    _initialized = true;

    SDCardInfo info = _card.getInfo();

    Logger::info("SD", "SD card initialized");
    Logger::info("SD", "Total: %s", info.totalSize.c_str());
    Logger::info(
        "SD",
        "Used: %s (%.1f%%)",
        info.usedSize.c_str(),
        info.usedPercent
    );
    Logger::info(
        "SD",
        "Free: %s (%.1f%%)",
        info.freeSize.c_str(),
        info.freePercent
    );

    return true;
}

// ============================================================
// END
// ============================================================

void SDManager::end()
{
    if (!_initialized)
        return;

    _card.end();

    _initialized = false;

    Logger::info("SD", "SD card unmounted");
}

// ============================================================
// IS READY
// ============================================================

bool SDManager::isReady() const
{
    return _initialized && _card.isMounted();
}

// ============================================================
// CARD
// ============================================================

SDCard& SDManager::card()
{
    return _card;
}

const SDCard& SDManager::card() const
{
    return _card;
}

// ============================================================
// GET INFO
// ============================================================

SDCardInfo SDManager::getInfo() const
{
    return _card.getInfo();
}

// ============================================================
// LIST FILES
// ============================================================

size_t SDManager::listFiles(
    SDFileEntry* out,
    size_t       maxFiles,
    uint8_t      maxDepth,
    const char*  root
) const
{
    if (!isReady())    return 0;
    if (!out)          return 0;
    if (maxFiles == 0) return 0;

    if (!root)
        root = "/";

    return listDir(root, out, maxFiles, 0, 0, maxDepth);
}

// ============================================================
// LIST DIR (recursive)
// ============================================================

size_t SDManager::listDir(
    const char*  dirname,
    SDFileEntry* out,
    size_t       maxFiles,
    size_t       count,
    uint8_t      depth,
    uint8_t      maxDepth
) const
{
    if (count >= maxFiles)
        return count;

    File dir = SD.open(dirname);

    if (!dir || !dir.isDirectory())
    {
        if (dir) dir.close();
        return count;
    }

    File entry = dir.openNextFile();

    while (entry && count < maxFiles)
    {
        const char* name = entry.name();

        bool skip =
            name == nullptr ||
            name[0] == '.' ||
            strcmp(name, "System Volume Information") == 0;

        if (!skip)
        {
            String fullPath = entry.path();

            if (!fullPath.startsWith("/"))
                fullPath = "/" + fullPath;

            bool isDir = entry.isDirectory();

            out[count].path  = fullPath;
            out[count].size  = isDir
                ? 0
                : static_cast<uint64_t>(entry.size());
            out[count].isDir = isDir;

            ++count;

            if (isDir && depth < maxDepth)
            {
                count = listDir(
                    fullPath.c_str(),
                    out,
                    maxFiles,
                    count,
                    depth + 1,
                    maxDepth
                );
            }
        }

        entry.close();
        entry = dir.openNextFile();
    }

    dir.close();

    return count;
}