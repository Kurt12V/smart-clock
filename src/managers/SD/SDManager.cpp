#include "SDManager.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

SDManager::SDManager()
    : _initialized(false)
{
    Serial0.println();
    Serial0.println("[SDM][CONSTRUCTOR] SDManager created");
    Serial0.printf("[SDM][CONSTRUCTOR] Initialized: %s\n",
                   _initialized ? "true" : "false");
}

// ============================================================
// BEGIN
// ============================================================

bool SDManager::begin(
    uint8_t csPin
)
{
    Serial0.println();
    Serial0.println("================================================");
    Serial0.println("[SDM][BEGIN] Initializing SDManager");
    Serial0.println("================================================");

    Serial0.printf("[SDM][BEGIN] CS pin: GPIO %u\n", csPin);
    Serial0.printf("[SDM][BEGIN] Previous initialized state: %s\n",
                   _initialized ? "true" : "false");

    _initialized = false;

    Serial0.println("[SDM][BEGIN] Calling _card.begin()...");

    const unsigned long startTime = millis();

    const bool result = _card.begin(csPin);

    const unsigned long elapsedTime = millis() - startTime;

    Serial0.printf("[SDM][BEGIN] _card.begin() result: %s\n",
                   result ? "true" : "false");

    Serial0.printf("[SDM][BEGIN] Initialization duration: %lu ms\n",
                   elapsedTime);

    if (!result)
    {
        Serial0.println("[SDM][BEGIN][ERROR] SDCard initialization failed");
        Serial0.println("[SDM][BEGIN][ERROR] Manager remains uninitialized");
        Serial0.printf("[SDM][BEGIN] Initialized: %s\n",
                       _initialized ? "true" : "false");
        Serial0.println("================================================");

        return false;
    }

    _initialized = true;

    Serial0.println("[SDM][BEGIN] SDCard initialized successfully");
    Serial0.printf("[SDM][BEGIN] Initialized: %s\n",
                   _initialized ? "true" : "false");

    Serial0.printf("[SDM][BEGIN] Card mounted: %s\n",
                   _card.isMounted() ? "true" : "false");

    Serial0.printf("[SDM][BEGIN] Manager ready: %s\n",
                   isReady() ? "true" : "false");

    Serial0.println("================================================");

    return true;
}

// ============================================================
// END
// ============================================================

void SDManager::end()
{
    Serial0.println();
    Serial0.println("[SDM][END] Stopping SDManager");

    Serial0.printf("[SDM][END] Initialized: %s\n",
                   _initialized ? "true" : "false");

    if (!_initialized)
    {
        Serial0.println("[SDM][END] Manager is not initialized");
        Serial0.println("[SDM][END] Nothing to stop");
        return;
    }

    Serial0.println("[SDM][END] Calling _card.end()...");

    _card.end();

    _initialized = false;

    Serial0.println("[SDM][END] _card.end() completed");
    Serial0.printf("[SDM][END] Initialized: %s\n",
                   _initialized ? "true" : "false");

    Serial0.printf("[SDM][END] Card mounted: %s\n",
                   _card.isMounted() ? "true" : "false");
}

// ============================================================
// IS READY
// ============================================================

bool SDManager::isReady() const
{
    const bool cardMounted = _card.isMounted();

    const bool result =
        _initialized &&
        cardMounted;

    Serial0.printf(
        "[SDM][IS_READY] Initialized=%s, Mounted=%s, Result=%s\n",
        _initialized ? "true" : "false",
        cardMounted ? "true" : "false",
        result ? "true" : "false"
    );

    return result;
}

// ============================================================
// CARD
// ============================================================

SDCard& SDManager::card()
{
    Serial0.println("[SDM][CARD] Returning mutable SDCard reference");

    return _card;
}

const SDCard& SDManager::card() const
{
    Serial0.println("[SDM][CARD] Returning const SDCard reference");

    return _card;
}

// ============================================================
// INFO
// ============================================================

SDCardInfo SDManager::getInfo() const
{
    Serial0.println();
    Serial0.println("[SDM][INFO] Requesting card information");

    Serial0.printf("[SDM][INFO] Initialized: %s\n",
                   _initialized ? "true" : "false");

    if (!_initialized)
    {
        Serial0.println("[SDM][INFO][ERROR] Manager is not initialized");
        Serial0.println("[SDM][INFO] Returning empty SDCardInfo");

        return SDCardInfo{};
    }

    Serial0.println("[SDM][INFO] Calling _card.getInfo()...");

    SDCardInfo info = _card.getInfo();

    Serial0.println("[SDM][INFO] Received card information:");

    Serial0.printf("[SDM][INFO] Total bytes: %llu\n",
                   static_cast<unsigned long long>(info.totalBytes));

    Serial0.printf("[SDM][INFO] Used bytes: %llu\n",
                   static_cast<unsigned long long>(info.usedBytes));

    Serial0.printf("[SDM][INFO] Free bytes: %llu\n",
                   static_cast<unsigned long long>(info.freeBytes));

    Serial0.printf("[SDM][INFO] Used: %.2f%%\n",
                   info.usedPercent);

    Serial0.printf("[SDM][INFO] Free: %.2f%%\n",
                   info.freePercent);

    Serial0.printf("[SDM][INFO] Total size: %s\n",
                   info.totalSize.c_str());

    Serial0.printf("[SDM][INFO] Used size: %s\n",
                   info.usedSize.c_str());

    Serial0.printf("[SDM][INFO] Free size: %s\n",
                   info.freeSize.c_str());

    return info;
}

// ============================================================
// LIST FILES
//
// СТАРЫЙ API.
// Сохраняем для совместимости.
// ============================================================

size_t SDManager::listFiles(
    SDFileEntry* out,
    size_t maxFiles,
    uint8_t maxDepth,
    const char* root
) const
{
    Serial0.println();
    Serial0.println("[SDM][LIST] Starting file listing");

    Serial0.printf("[SDM][LIST] Output buffer: %s\n",
                   out != nullptr ? "valid" : "nullptr");

    Serial0.printf("[SDM][LIST] Maximum entries: %u\n",
                   static_cast<unsigned int>(maxFiles));

    Serial0.printf("[SDM][LIST] Maximum depth: %u\n", maxDepth);

    Serial0.printf("[SDM][LIST] Initialized: %s\n",
                   _initialized ? "true" : "false");

    Serial0.printf("[SDM][LIST] Card mounted: %s\n",
                   _card.isMounted() ? "true" : "false");

    if (out == nullptr)
    {
        Serial0.println("[SDM][LIST][ERROR] Output buffer is nullptr");
        return 0;
    }

    if (maxFiles == 0)
    {
        Serial0.println("[SDM][LIST][ERROR] maxFiles is zero");
        return 0;
    }

    if (!_initialized)
    {
        Serial0.println("[SDM][LIST][ERROR] Manager is not initialized");
        return 0;
    }

    if (!_card.isMounted())
    {
        Serial0.println("[SDM][LIST][ERROR] Card is not mounted");
        return 0;
    }

    if (root == nullptr)
    {
        Serial0.println("[SDM][LIST] Root is nullptr; using /");
        root = "/";
    }

    Serial0.printf("[SDM][LIST] Root directory: %s\n", root);

    const unsigned long startTime = millis();

    const size_t count = listDir(
        root,
        out,
        maxFiles,
        0,
        0,
        maxDepth
    );

    Serial0.printf("[SDM][LIST] Entries found: %u\n",
                   static_cast<unsigned int>(count));

    Serial0.printf("[SDM][LIST] Duration: %lu ms\n",
                   millis() - startTime);

    Serial0.printf("[SDM][LIST] Buffer limit reached: %s\n",
                   count >= maxFiles ? "true" : "false");

    return count;
}

// ============================================================
// LIST FILES PAGE
// ============================================================

size_t SDManager::listFilesPage(
    SDFileEntry* out,
    size_t maxFiles,
    size_t offset,
    bool& hasMore,
    uint8_t maxDepth,
    const char* root
) const
{
    Serial0.println();
    Serial0.println("[SDM][PAGE] Starting paginated file listing");

    Serial0.printf("[SDM][PAGE] Output buffer: %s\n",
                   out != nullptr ? "valid" : "nullptr");

    Serial0.printf("[SDM][PAGE] Page size: %u\n",
                   static_cast<unsigned int>(maxFiles));

    Serial0.printf("[SDM][PAGE] Offset: %u\n",
                   static_cast<unsigned int>(offset));

    Serial0.printf("[SDM][PAGE] Maximum depth: %u\n", maxDepth);

    hasMore = false;

    Serial0.println("[SDM][PAGE] hasMore initialized to false");

    if (out == nullptr)
    {
        Serial0.println("[SDM][PAGE][ERROR] Output buffer is nullptr");
        return 0;
    }

    if (maxFiles == 0)
    {
        Serial0.println("[SDM][PAGE][ERROR] maxFiles is zero");
        return 0;
    }

    if (!_initialized)
    {
        Serial0.println("[SDM][PAGE][ERROR] Manager is not initialized");
        return 0;
    }

    if (!_card.isMounted())
    {
        Serial0.println("[SDM][PAGE][ERROR] Card is not mounted");
        return 0;
    }

    if (root == nullptr)
    {
        Serial0.println("[SDM][PAGE] Root is nullptr; using /");
        root = "/";
    }

    Serial0.printf("[SDM][PAGE] Root: %s\n", root);

    size_t skipped = 0;

    Serial0.println("[SDM][PAGE] Starting directory scan");

    const unsigned long startTime = millis();

    const size_t count = listDirPage(
        root,
        out,
        maxFiles,
        0,
        skipped,
        offset,
        hasMore,
        0,
        maxDepth
    );

    Serial0.printf("[SDM][PAGE] Entries returned: %u\n",
                   static_cast<unsigned int>(count));

    Serial0.printf("[SDM][PAGE] Entries skipped: %u\n",
                   static_cast<unsigned int>(skipped));

    Serial0.printf("[SDM][PAGE] Has more entries: %s\n",
                   hasMore ? "true" : "false");

    Serial0.printf("[SDM][PAGE] Duration: %lu ms\n",
                   millis() - startTime);

    return count;
}

// ============================================================
// LIST DIRECTORY
// ============================================================

size_t SDManager::listDir(
    const char* dirname,
    SDFileEntry* out,
    size_t maxFiles,
    size_t count,
    uint8_t depth,
    uint8_t maxDepth
) const
{
    Serial0.printf(
        "[SDM][DIR] Enter: path=%s, count=%u, depth=%u, maxDepth=%u\n",
        dirname != nullptr ? dirname : "(nullptr)",
        static_cast<unsigned int>(count),
        depth,
        maxDepth
    );

    if (dirname == nullptr)
    {
        Serial0.println("[SDM][DIR][ERROR] dirname is nullptr");
        return count;
    }

    if (out == nullptr)
    {
        Serial0.println("[SDM][DIR][ERROR] Output buffer is nullptr");
        return count;
    }

    if (count >= maxFiles)
    {
        Serial0.println("[SDM][DIR] Entry limit reached");
        return count;
    }

    Serial0.printf("[SDM][DIR] Opening: %s\n", dirname);

    File dir = SD.open(dirname);

    if (!dir)
    {
        Serial0.printf("[SDM][DIR][ERROR] Failed to open: %s\n",
                       dirname);
        return count;
    }

    if (!dir.isDirectory())
    {
        Serial0.printf("[SDM][DIR][ERROR] Not a directory: %s\n",
                       dirname);

        dir.close();

        return count;
    }

    Serial0.printf("[SDM][DIR] Directory opened: %s\n", dirname);

    while (true)
    {
        if (count >= maxFiles)
        {
            Serial0.println("[SDM][DIR] Maximum entry count reached");
            break;
        }

        File file = dir.openNextFile();

        if (!file)
        {
            Serial0.printf("[SDM][DIR] No more entries in: %s\n",
                           dirname);
            break;
        }

        const bool isDirectory = file.isDirectory();
        const String path = file.path();

        const uint64_t size =
            isDirectory
                ? 0
                : static_cast<uint64_t>(file.size());

        Serial0.printf("[SDM][DIR] Entry: %s\n", path.c_str());

        Serial0.printf("[SDM][DIR] Type: %s\n",
                       isDirectory ? "directory" : "file");

        Serial0.printf("[SDM][DIR] Size: %llu bytes\n",
                       static_cast<unsigned long long>(size));

        out[count].path = path;
        out[count].size = size;
        out[count].isDir = isDirectory;

        Serial0.printf("[SDM][DIR] Stored at index: %u\n",
                       static_cast<unsigned int>(count));

        ++count;

        Serial0.printf("[SDM][DIR] Total entries stored: %u\n",
                       static_cast<unsigned int>(count));

        if (
            isDirectory &&
            depth < maxDepth &&
            count < maxFiles
        )
        {
            Serial0.printf("[SDM][DIR] Recursing into: %s\n",
                           path.c_str());

            count = listDir(
                path.c_str(),
                out,
                maxFiles,
                count,
                depth + 1,
                maxDepth
            );

            Serial0.printf("[SDM][DIR] Returned from subdirectory; count=%u\n",
                           static_cast<unsigned int>(count));
        }

        file.close();

        Serial0.printf("[SDM][DIR] Entry closed: %s\n", path.c_str());
    }

    dir.close();

    Serial0.printf("[SDM][DIR] Directory closed: %s\n", dirname);
    Serial0.printf("[SDM][DIR] Returning count: %u\n",
                   static_cast<unsigned int>(count));

    return count;
}

// ============================================================
// LIST DIRECTORY PAGE
// ============================================================

size_t SDManager::listDirPage(
    const char* dirname,
    SDFileEntry* out,
    size_t maxFiles,
    size_t count,
    size_t& skipped,
    size_t offset,
    bool& hasMore,
    uint8_t depth,
    uint8_t maxDepth
) const
{
    Serial0.printf(
        "[SDM][DIR_PAGE] Enter: path=%s, count=%u, skipped=%u, offset=%u, depth=%u\n",
        dirname != nullptr ? dirname : "(nullptr)",
        static_cast<unsigned int>(count),
        static_cast<unsigned int>(skipped),
        static_cast<unsigned int>(offset),
        depth
    );

    if (dirname == nullptr)
    {
        Serial0.println("[SDM][DIR_PAGE][ERROR] dirname is nullptr");
        return count;
    }

    if (out == nullptr)
    {
        Serial0.println("[SDM][DIR_PAGE][ERROR] Output buffer is nullptr");
        return count;
    }

    if (maxFiles == 0)
    {
        Serial0.println("[SDM][DIR_PAGE][ERROR] maxFiles is zero");
        return count;
    }

    if (hasMore)
    {
        Serial0.println("[SDM][DIR_PAGE] hasMore already true");
        return count;
    }

    Serial0.printf("[SDM][DIR_PAGE] Opening directory: %s\n",
                   dirname);

    File dir = SD.open(dirname);

    if (!dir)
    {
        Serial0.printf("[SDM][DIR_PAGE][ERROR] Failed to open: %s\n",
                       dirname);
        return count;
    }

    if (!dir.isDirectory())
    {
        Serial0.printf("[SDM][DIR_PAGE][ERROR] Not a directory: %s\n",
                       dirname);

        dir.close();

        return count;
    }

    while (true)
    {
        if (hasMore)
        {
            Serial0.println("[SDM][DIR_PAGE] Stopping: hasMore=true");
            break;
        }

        File file = dir.openNextFile();

        if (!file)
        {
            Serial0.printf("[SDM][DIR_PAGE] No more entries: %s\n",
                           dirname);
            break;
        }

        const bool isDirectory = file.isDirectory();
        const String path = file.path();

        const uint64_t size =
            isDirectory
                ? 0
                : static_cast<uint64_t>(file.size());

        Serial0.printf("[SDM][DIR_PAGE] Entry: %s\n", path.c_str());

        Serial0.printf("[SDM][DIR_PAGE] Type: %s\n",
                       isDirectory ? "directory" : "file");

        Serial0.printf("[SDM][DIR_PAGE] Size: %llu bytes\n",
                       static_cast<unsigned long long>(size));

        // ----------------------------------------------------
        // OFFSET
        // ----------------------------------------------------

        if (skipped < offset)
        {
            ++skipped;

            Serial0.printf(
                "[SDM][DIR_PAGE] Skipping entry: skipped=%u/%u\n",
                static_cast<unsigned int>(skipped),
                static_cast<unsigned int>(offset)
            );

            if (
                isDirectory &&
                depth < maxDepth
            )
            {
                Serial0.printf(
                    "[SDM][DIR_PAGE] Scanning skipped subdirectory: %s\n",
                    path.c_str()
                );

                count = listDirPage(
                    path.c_str(),
                    out,
                    maxFiles,
                    count,
                    skipped,
                    offset,
                    hasMore,
                    depth + 1,
                    maxDepth
                );
            }

            file.close();

            if (hasMore)
                break;

            continue;
        }

        // ----------------------------------------------------
        // PAGE FULL
        // ----------------------------------------------------

        if (count >= maxFiles)
        {
            hasMore = true;

            Serial0.println(
                "[SDM][DIR_PAGE] Another entry found after page filled"
            );

            Serial0.println("[SDM][DIR_PAGE] hasMore=true");

            file.close();

            break;
        }

        // ----------------------------------------------------
        // ADD ENTRY
        // ----------------------------------------------------

        out[count].path = path;
        out[count].size = size;
        out[count].isDir = isDirectory;

        Serial0.printf(
            "[SDM][DIR_PAGE] Stored entry at index %u\n",
            static_cast<unsigned int>(count)
        );

        ++count;

        Serial0.printf("[SDM][DIR_PAGE] Current page count: %u/%u\n",
                       static_cast<unsigned int>(count),
                       static_cast<unsigned int>(maxFiles));

        // ----------------------------------------------------
        // RECURSIVE DIRECTORY
        // ----------------------------------------------------

        if (
            isDirectory &&
            depth < maxDepth &&
            !hasMore
        )
        {
            Serial0.printf(
                "[SDM][DIR_PAGE] Entering subdirectory: %s\n",
                path.c_str()
            );

            count = listDirPage(
                path.c_str(),
                out,
                maxFiles,
                count,
                skipped,
                offset,
                hasMore,
                depth + 1,
                maxDepth
            );
        }

        file.close();

        Serial0.printf("[SDM][DIR_PAGE] Closed entry: %s\n",
                       path.c_str());
    }

    dir.close();

    Serial0.printf("[SDM][DIR_PAGE] Closed directory: %s\n", dirname);

    Serial0.printf("[SDM][DIR_PAGE] Returning count: %u\n",
                   static_cast<unsigned int>(count));

    Serial0.printf("[SDM][DIR_PAGE] Skipped count: %u\n",
                   static_cast<unsigned int>(skipped));

    Serial0.printf("[SDM][DIR_PAGE] hasMore: %s\n",
                   hasMore ? "true" : "false");

    return count;
}

// ============================================================
// CREATE DIRECTORY
// ============================================================

bool SDManager::createDirectory(
    const String& path
)
{
    Serial0.println();
    Serial0.println("[SDM][MKDIR] Creating directory");

    Serial0.printf("[SDM][MKDIR] Path: %s\n", path.c_str());

    Serial0.printf("[SDM][MKDIR] Initialized: %s\n",
                   _initialized ? "true" : "false");

    if (!_initialized)
    {
        Serial0.println("[SDM][MKDIR][ERROR] Manager is not initialized");
        return false;
    }

    Serial0.printf("[SDM][MKDIR] Card mounted: %s\n",
                   _card.isMounted() ? "true" : "false");

    if (!_card.isMounted())
    {
        Serial0.println("[SDM][MKDIR][ERROR] Card is not mounted");
        return false;
    }

    if (path.isEmpty())
    {
        Serial0.println("[SDM][MKDIR][ERROR] Path is empty");
        return false;
    }

    if (_card.exists(path.c_str()))
    {
        Serial0.println("[SDM][MKDIR] Path already exists");
        Serial0.println("[SDM][MKDIR] Returning true");

        return true;
    }

    Serial0.println("[SDM][MKDIR] Calling filesystem mkdir()...");

    const bool result = _card.fs().mkdir(path.c_str());

    Serial0.printf("[SDM][MKDIR] mkdir() result: %s\n",
                   result ? "true" : "false");

    const bool existsAfter = _card.exists(path.c_str());

    Serial0.printf("[SDM][MKDIR] Path exists after operation: %s\n",
                   existsAfter ? "true" : "false");

    return result;
}

// ============================================================
// FILE EXISTS
// ============================================================

bool SDManager::fileExists(
    const String& path
) const
{
    Serial0.println();
    Serial0.println("[SDM][EXISTS] Checking path");

    Serial0.printf("[SDM][EXISTS] Path: %s\n", path.c_str());

    Serial0.printf("[SDM][EXISTS] Initialized: %s\n",
                   _initialized ? "true" : "false");

    if (!_initialized)
    {
        Serial0.println("[SDM][EXISTS][ERROR] Manager is not initialized");
        return false;
    }

    Serial0.printf("[SDM][EXISTS] Card mounted: %s\n",
                   _card.isMounted() ? "true" : "false");

    if (!_card.isMounted())
    {
        Serial0.println("[SDM][EXISTS][ERROR] Card is not mounted");
        return false;
    }

    if (path.isEmpty())
    {
        Serial0.println("[SDM][EXISTS][ERROR] Path is empty");
        return false;
    }

    const bool result = _card.exists(path.c_str());

    Serial0.printf("[SDM][EXISTS] Result: %s\n",
                   result ? "true" : "false");

    return result;
}

// ============================================================
// READ FILE
// ============================================================

bool SDManager::readFile(
    const String& path,
    String& content
)
{
    Serial0.println();
    Serial0.println("================================================");
    Serial0.println("[SDM][READ] Starting file read");
    Serial0.println("================================================");

    Serial0.printf("[SDM][READ] Path: %s\n", path.c_str());

    Serial0.printf("[SDM][READ] Initialized: %s\n",
                   _initialized ? "true" : "false");

    content = "";

    Serial0.println("[SDM][READ] Output content cleared");

    if (!_initialized)
    {
        Serial0.println("[SDM][READ][ERROR] Manager is not initialized");
        return false;
    }

    Serial0.printf("[SDM][READ] Card mounted: %s\n",
                   _card.isMounted() ? "true" : "false");

    if (!_card.isMounted())
    {
        Serial0.println("[SDM][READ][ERROR] Card is not mounted");
        return false;
    }

    if (path.isEmpty())
    {
        Serial0.println("[SDM][READ][ERROR] Path is empty");
        return false;
    }

    Serial0.println("[SDM][READ] Checking whether path exists...");

    const bool existsBefore = _card.exists(path.c_str());

    Serial0.printf("[SDM][READ] Exists before opening: %s\n",
                   existsBefore ? "true" : "false");

    Serial0.println("[SDM][READ] Opening file with FILE_READ...");

    const unsigned long startTime = millis();

    File file = _card.fs().open(
        path.c_str(),
        FILE_READ
    );

    Serial0.printf("[SDM][READ] File handle valid: %s\n",
                   file ? "true" : "false");

    if (!file)
    {
        Serial0.println("[SDM][READ][ERROR] Failed to open file");
        Serial0.println("================================================");

        return false;
    }

    Serial0.printf("[SDM][READ] Is directory: %s\n",
                   file.isDirectory() ? "true" : "false");

    const size_t sizeBefore = file.size();

    Serial0.printf("[SDM][READ] File size before reading: %u bytes\n",
                   static_cast<unsigned int>(sizeBefore));

    Serial0.println("[SDM][READ] Calling readString()...");

    content = file.readString();

    Serial0.printf("[SDM][READ] Content length: %u bytes\n",
                   static_cast<unsigned int>(content.length()));

    Serial0.printf("[SDM][READ] File position after read: %u\n",
                   static_cast<unsigned int>(file.position()));

    Serial0.printf("[SDM][READ] File size after reading: %u bytes\n",
                   static_cast<unsigned int>(file.size()));

    file.close();

    Serial0.println("[SDM][READ] File closed");

    Serial0.printf("[SDM][READ] Duration: %lu ms\n",
                   millis() - startTime);

    Serial0.printf("[SDM][READ] Content empty: %s\n",
                   content.isEmpty() ? "true" : "false");

    Serial0.printf("[SDM][READ] Result: %s\n",
                   "true (read operation completed)");

    if (content.length() != sizeBefore)
    {
        Serial0.println(
            "[SDM][READ][WARNING] Content length differs from file size"
        );
    }

    Serial0.println("================================================");

    return true;
}

// ============================================================
// WRITE FILE
// ============================================================

bool SDManager::writeFile(
    const String& path,
    const String& content
)
{
    Serial0.println();
    Serial0.println("================================================");
    Serial0.println("[SDM][WRITE] Starting file write");
    Serial0.println("================================================");

    Serial0.printf("[SDM][WRITE] Path: %s\n", path.c_str());

    Serial0.printf("[SDM][WRITE] Content length: %u bytes\n",
                   static_cast<unsigned int>(content.length()));

    Serial0.printf("[SDM][WRITE] Initialized: %s\n",
                   _initialized ? "true" : "false");

    if (!_initialized)
    {
        Serial0.println("[SDM][WRITE][ERROR] Manager is not initialized");
        return false;
    }

    Serial0.printf("[SDM][WRITE] Card mounted: %s\n",
                   _card.isMounted() ? "true" : "false");

    if (!_card.isMounted())
    {
        Serial0.println("[SDM][WRITE][ERROR] Card is not mounted");
        return false;
    }

    if (path.isEmpty())
    {
        Serial0.println("[SDM][WRITE][ERROR] Path is empty");
        return false;
    }

    // --------------------------------------------------------
    // PATH STATUS
    // --------------------------------------------------------

    const bool existsBefore = _card.exists(path.c_str());

    Serial0.printf("[SDM][WRITE] File exists before opening: %s\n",
                   existsBefore ? "true" : "false");

    if (existsBefore)
    {
        Serial0.println(
            "[SDM][WRITE][INFO] FILE_WRITE may append to the existing file"
        );
    }

    // --------------------------------------------------------
    // OPEN FILE
    // --------------------------------------------------------

    Serial0.println("[SDM][WRITE] Opening file with FILE_WRITE...");

    const unsigned long startTime = millis();

    File file = _card.fs().open(
        path.c_str(),
        FILE_WRITE
    );

    Serial0.printf("[SDM][WRITE] File handle valid: %s\n",
                   file ? "true" : "false");

    if (!file)
    {
        Serial0.println("[SDM][WRITE][ERROR] Failed to open file");
        Serial0.println("================================================");

        return false;
    }

    Serial0.printf("[SDM][WRITE] Is directory: %s\n",
                   file.isDirectory() ? "true" : "false");

    Serial0.printf("[SDM][WRITE] Initial file size: %u bytes\n",
                   static_cast<unsigned int>(file.size()));

    Serial0.printf("[SDM][WRITE] Initial file position: %u\n",
                   static_cast<unsigned int>(file.position()));

    // --------------------------------------------------------
    // WRITE DATA
    // --------------------------------------------------------

    Serial0.println("[SDM][WRITE] Calling file.print()...");

    const size_t expectedBytes = content.length();

    const size_t written = file.print(content);

    Serial0.printf("[SDM][WRITE] Expected bytes: %u\n",
                   static_cast<unsigned int>(expectedBytes));

    Serial0.printf("[SDM][WRITE] Bytes reported written: %u\n",
                   static_cast<unsigned int>(written));

    Serial0.printf("[SDM][WRITE] File size immediately after print: %u bytes\n",
                   static_cast<unsigned int>(file.size()));

    Serial0.printf("[SDM][WRITE] File position after print: %u\n",
                   static_cast<unsigned int>(file.position()));

    Serial0.printf("[SDM][WRITE] Write count matches expected: %s\n",
                   written == expectedBytes ? "true" : "false");

    // --------------------------------------------------------
    // CLOSE FILE
    // --------------------------------------------------------

    Serial0.println("[SDM][WRITE] Closing file...");

    file.close();

    Serial0.println("[SDM][WRITE] file.close() completed");

    Serial0.printf("[SDM][WRITE] Duration before verification: %lu ms\n",
                   millis() - startTime);

    // --------------------------------------------------------
    // VERIFY PATH
    // --------------------------------------------------------

    Serial0.println("[SDM][WRITE] Verifying file after close...");

    const bool existsAfter = _card.exists(path.c_str());

    Serial0.printf("[SDM][WRITE] File exists after close: %s\n",
                   existsAfter ? "true" : "false");

    // --------------------------------------------------------
    // VERIFY STORED SIZE
    // --------------------------------------------------------

    size_t storedSize = 0;

    if (existsAfter)
    {
        Serial0.println("[SDM][WRITE] Reopening file for verification...");

        File verifyFile = _card.fs().open(
            path.c_str(),
            FILE_READ
        );

        Serial0.printf("[SDM][WRITE] Verification handle valid: %s\n",
                       verifyFile ? "true" : "false");

        if (verifyFile)
        {
            storedSize = verifyFile.size();

            Serial0.printf(
                "[SDM][WRITE] Stored file size after close: %u bytes\n",
                static_cast<unsigned int>(storedSize)
            );

            verifyFile.close();

            Serial0.println("[SDM][WRITE] Verification file closed");
        }
        else
        {
            Serial0.println(
                "[SDM][WRITE][WARNING] Could not reopen file for verification"
            );
        }
    }
    else
    {
        Serial0.println(
            "[SDM][WRITE][ERROR] File does not exist after close"
        );
    }

    // --------------------------------------------------------
    // RESULT
    // --------------------------------------------------------

    const bool writeCountCorrect =
        written == expectedBytes;

    const bool storedSizeCorrect =
        existsAfter &&
        storedSize >= expectedBytes;

    const bool result =
        writeCountCorrect &&
        storedSizeCorrect;

    Serial0.println("[SDM][WRITE] Verification summary:");

    Serial0.printf("[SDM][WRITE] Expected bytes: %u\n",
                   static_cast<unsigned int>(expectedBytes));

    Serial0.printf("[SDM][WRITE] Reported bytes: %u\n",
                   static_cast<unsigned int>(written));

    Serial0.printf("[SDM][WRITE] Stored file size: %u\n",
                   static_cast<unsigned int>(storedSize));

    Serial0.printf("[SDM][WRITE] Write count correct: %s\n",
                   writeCountCorrect ? "true" : "false");

    Serial0.printf("[SDM][WRITE] Stored size sufficient: %s\n",
                   storedSizeCorrect ? "true" : "false");

    Serial0.printf("[SDM][WRITE] Final result: %s\n",
                   result ? "SUCCESS" : "FAILURE");

    Serial0.printf("[SDM][WRITE] Total duration: %lu ms\n",
                   millis() - startTime);

    if (!writeCountCorrect)
    {
        Serial0.println(
            "[SDM][WRITE][ERROR] file.print() returned an unexpected byte count"
        );
    }

    if (!existsAfter)
    {
        Serial0.println(
            "[SDM][WRITE][ERROR] File is missing after close"
        );
    }
    else if (!storedSizeCorrect)
    {
        Serial0.println(
            "[SDM][WRITE][ERROR] Stored size is smaller than expected"
        );
    }

    Serial0.println("================================================");

    return result;
}

// ============================================================
// DELETE FILE
// ============================================================

bool SDManager::deleteFile(
    const String& path
)
{
    Serial0.println();
    Serial0.println("[SDM][DELETE] Deleting file");

    Serial0.printf("[SDM][DELETE] Path: %s\n", path.c_str());

    Serial0.printf("[SDM][DELETE] Initialized: %s\n",
                   _initialized ? "true" : "false");

    if (!_initialized)
    {
        Serial0.println("[SDM][DELETE][ERROR] Manager is not initialized");
        return false;
    }

    Serial0.printf("[SDM][DELETE] Card mounted: %s\n",
                   _card.isMounted() ? "true" : "false");

    if (!_card.isMounted())
    {
        Serial0.println("[SDM][DELETE][ERROR] Card is not mounted");
        return false;
    }

    if (path.isEmpty())
    {
        Serial0.println("[SDM][DELETE][ERROR] Path is empty");
        return false;
    }

    const bool existsBefore = _card.exists(path.c_str());

    Serial0.printf("[SDM][DELETE] Exists before delete: %s\n",
                   existsBefore ? "true" : "false");

    if (!existsBefore)
    {
        Serial0.println(
            "[SDM][DELETE][ERROR] Path does not exist; remove not attempted"
        );

        return false;
    }

    Serial0.println("[SDM][DELETE] Calling filesystem remove()...");

    const bool result = _card.fs().remove(path.c_str());

    Serial0.printf("[SDM][DELETE] remove() result: %s\n",
                   result ? "true" : "false");

    const bool existsAfter = _card.exists(path.c_str());

    Serial0.printf("[SDM][DELETE] Exists after delete: %s\n",
                   existsAfter ? "true" : "false");

    if (existsAfter)
    {
        Serial0.println(
            "[SDM][DELETE][WARNING] Path still exists after remove()"
        );
    }

    return result && !existsAfter;
}