#include "SDManager.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

SDManager::SDManager()
    : _initialized(false)
{
    Serial0.println("[SDManager] Constructor");
}


// ============================================================
// BEGIN
// ============================================================

bool SDManager::begin(
    uint8_t csPin
)
{
    Serial0.println("[SDManager] begin()");
    Serial0.printf(
        "[SDManager]   CS pin: %u\n",
        static_cast<unsigned>(csPin)
    );


    _initialized = false;


    if (!_card.begin(csPin))
    {
        Serial0.println(
            "[SDManager]   _card.begin() FAILED"
        );

        return false;
    }


    _initialized = true;


    Serial0.println(
        "[SDManager]   _card.begin() OK"
    );


    if (_card.isMounted())
    {
        Serial0.println(
            "[SDManager]   Card mounted"
        );
    }
    else
    {
        Serial0.println(
            "[SDManager]   Card NOT mounted"
        );
    }


    Serial0.println("[SDManager] begin() done");

    return true;
}


// ============================================================
// END
// ============================================================

void SDManager::end()
{
    Serial0.println("[SDManager] end()");


    if (!_initialized)
    {
        Serial0.println(
            "[SDManager]   Not initialized, skip"
        );

        return;
    }


    _card.end();

    _initialized = false;


    Serial0.println("[SDManager] end() done");
}


// ============================================================
// IS READY
// ============================================================

bool SDManager::isReady() const
{
    const bool ready =
        (
            _initialized &&
            _card.isMounted()
        );

    Serial0.printf(
        "[SDManager] isReady() -> %s\n",
        ready ? "true" : "false"
    );

    return ready;
}


// ============================================================
// CARD
// ============================================================

SDCard& SDManager::card()
{
    Serial0.println("[SDManager] card()");

    return _card;
}


const SDCard& SDManager::card() const
{
    Serial0.println("[SDManager] card() const");

    return _card;
}


// ============================================================
// INFO
// ============================================================

SDCardInfo SDManager::getInfo() const
{
    Serial0.println("[SDManager] getInfo()");


    if (!_initialized)
    {
        Serial0.println(
            "[SDManager]   Not initialized, return empty"
        );

        return SDCardInfo{};
    }


    const SDCardInfo info =
        _card.getInfo();


    Serial0.println(
        "[SDManager]   Info retrieved"
    );


    return info;
}


// ============================================================
// LIST FILES
//
// СТАРЫЙ API.
//
// Сохраняем для совместимости.
// ============================================================

size_t SDManager::listFiles(
    SDFileEntry* out,
    size_t maxFiles,
    uint8_t maxDepth,
    const char* root
) const
{
    Serial0.println("[SDManager] listFiles()");

    Serial0.printf(
        "[SDManager]   root: %s\n",
        (root != nullptr) ? root : "/"
    );

    Serial0.printf(
        "[SDManager]   maxFiles: %u\n",
        static_cast<unsigned>(maxFiles)
    );

    Serial0.printf(
        "[SDManager]   maxDepth: %u\n",
        static_cast<unsigned>(maxDepth)
    );


    if (out == nullptr)
    {
        Serial0.println(
            "[SDManager]   out == nullptr, return 0"
        );

        return 0;
    }

    if (maxFiles == 0)
    {
        Serial0.println(
            "[SDManager]   maxFiles == 0, return 0"
        );

        return 0;
    }

    if (!_initialized)
    {
        Serial0.println(
            "[SDManager]   Not initialized, return 0"
        );

        return 0;
    }

    if (!_card.isMounted())
    {
        Serial0.println(
            "[SDManager]   Card not mounted, return 0"
        );

        return 0;
    }

    if (root == nullptr)
        root = "/";


    const size_t count =
        listDir(
            root,
            out,
            maxFiles,
            0,
            0,
            maxDepth
        );


    Serial0.printf(
        "[SDManager] listFiles() -> %u\n",
        static_cast<unsigned>(count)
    );


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
    Serial0.println("[SDManager] listFilesPage()");

    Serial0.printf(
        "[SDManager]   root: %s\n",
        (root != nullptr) ? root : "/"
    );

    Serial0.printf(
        "[SDManager]   maxFiles: %u\n",
        static_cast<unsigned>(maxFiles)
    );

    Serial0.printf(
        "[SDManager]   offset: %u\n",
        static_cast<unsigned>(offset)
    );

    Serial0.printf(
        "[SDManager]   maxDepth: %u\n",
        static_cast<unsigned>(maxDepth)
    );


    hasMore = false;


    // ========================================================
    // VALIDATION
    // ========================================================

    if (out == nullptr)
    {
        Serial0.println(
            "[SDManager]   out == nullptr, return 0"
        );

        return 0;
    }

    if (maxFiles == 0)
    {
        Serial0.println(
            "[SDManager]   maxFiles == 0, return 0"
        );

        return 0;
    }

    if (!_initialized)
    {
        Serial0.println(
            "[SDManager]   Not initialized, return 0"
        );

        return 0;
    }

    if (!_card.isMounted())
    {
        Serial0.println(
            "[SDManager]   Card not mounted, return 0"
        );

        return 0;
    }

    if (root == nullptr)
        root = "/";


    // ========================================================
    // SKIPPED
    // ========================================================

    size_t skipped = 0;


    // ========================================================
    // SCAN
    // ========================================================

    const size_t count =
        listDirPage(
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


    Serial0.printf(
        "[SDManager] listFilesPage() -> count=%u, hasMore=%s\n",
        static_cast<unsigned>(count),
        hasMore ? "true" : "false"
    );


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
    if (dirname == nullptr)
        return count;

    if (out == nullptr)
        return count;

    if (count >= maxFiles)
        return count;


    Serial0.printf(
        "[SDManager]   listDir('%s', depth=%u)\n",
        dirname,
        static_cast<unsigned>(depth)
    );


    // --------------------------------------------------------
    // Открываем директорию
    // --------------------------------------------------------

    File dir =
        SD.open(dirname);

    if (!dir)
    {
        Serial0.printf(
            "[SDManager]     Cannot open '%s'\n",
            dirname
        );

        return count;
    }

    if (!dir.isDirectory())
    {
        Serial0.printf(
            "[SDManager]     '%s' is not a directory\n",
            dirname
        );

        dir.close();

        return count;
    }


    // --------------------------------------------------------
    // Перебираем содержимое
    // --------------------------------------------------------

    while (true)
    {
        if (count >= maxFiles)
            break;


        File file =
            dir.openNextFile();

        if (!file)
            break;


        const bool isDirectory =
            file.isDirectory();


        const String path =
            file.path();


        const uint64_t size =
            isDirectory
                ? 0
                : static_cast<uint64_t>(
                    file.size()
                );


        // ----------------------------------------------------
        // ADD ENTRY
        // ----------------------------------------------------

        out[count].path =
            path;

        out[count].size =
            size;

        out[count].isDir =
            isDirectory;


        Serial0.printf(
            "[SDManager]     [%u] %s%s (%llu B)\n",
            static_cast<unsigned>(count),
            path.c_str(),
            isDirectory ? "/" : "",
            static_cast<unsigned long long>(size)
        );


        ++count;


        // ----------------------------------------------------
        // RECURSIVE DIRECTORY
        // ----------------------------------------------------

        if (
            isDirectory &&
            depth < maxDepth &&
            count < maxFiles
        )
        {
            count =
                listDir(
                    path.c_str(),
                    out,
                    maxFiles,
                    count,
                    depth + 1,
                    maxDepth
                );
        }


        file.close();
    }


    dir.close();

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
    if (dirname == nullptr)
        return count;

    if (out == nullptr)
        return count;

    if (maxFiles == 0)
        return count;

    if (hasMore)
        return count;


    // ========================================================
    // OPEN DIRECTORY
    // ========================================================

    File dir =
        SD.open(dirname);

    if (!dir)
    {
        Serial0.printf(
            "[SDManager]     Cannot open '%s'\n",
            dirname
        );

        return count;
    }

    if (!dir.isDirectory())
    {
        Serial0.printf(
            "[SDManager]     '%s' is not a directory\n",
            dirname
        );

        dir.close();

        return count;
    }


    // ========================================================
    // ITERATE
    // ========================================================

    while (true)
    {
        if (hasMore)
            break;


        File file =
            dir.openNextFile();

        if (!file)
            break;


        // ====================================================
        // BASIC INFO
        // ====================================================

        const bool isDirectory =
            file.isDirectory();


        const String path =
            file.path();


        const uint64_t size =
            isDirectory
                ? 0
                : static_cast<uint64_t>(
                    file.size()
                );


        // ====================================================
        // OFFSET
        // ====================================================

        if (skipped < offset)
        {
            ++skipped;


            Serial0.printf(
                "[SDManager]     skip [%u] %s\n",
                static_cast<unsigned>(skipped - 1),
                path.c_str()
            );


            if (
                isDirectory &&
                depth < maxDepth
            )
            {
                count =
                    listDirPage(
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


        // ====================================================
        // PAGE FULL
        // ====================================================

        if (count >= maxFiles)
        {
            hasMore = true;

            Serial0.printf(
                "[SDManager]     hasMore=true (next: %s)\n",
                path.c_str()
            );


            file.close();

            break;
        }


        // ====================================================
        // ADD TO CURRENT PAGE
        // ====================================================

        out[count].path =
            path;

        out[count].size =
            size;

        out[count].isDir =
            isDirectory;

        ++count;


        Serial0.printf(
            "[SDManager]     page[%u] %s%s (%llu B)\n",
            static_cast<unsigned>(count - 1),
            path.c_str(),
            isDirectory ? "/" : "",
            static_cast<unsigned long long>(size)
        );


        // ====================================================
        // RECURSIVE DIRECTORY
        // ====================================================

        if (
            isDirectory &&
            depth < maxDepth &&
            !hasMore
        )
        {
            count =
                listDirPage(
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
    }


    // ========================================================
    // CLOSE
    // ========================================================

    dir.close();

    return count;
}


// ============================================================
// CREATE DIRECTORY
// ============================================================

bool SDManager::createDirectory(
    const String& path
)
{
    Serial0.printf(
        "[SDManager] createDirectory('%s')\n",
        path.c_str()
    );


    if (!_initialized)
    {
        Serial0.println(
            "[SDManager]   Not initialized"
        );

        return false;
    }

    if (!_card.isMounted())
    {
        Serial0.println(
            "[SDManager]   Card not mounted"
        );

        return false;
    }

    if (path.isEmpty())
    {
        Serial0.println(
            "[SDManager]   Empty path"
        );

        return false;
    }


    if (
        _card.exists(
            path.c_str()
        )
    )
    {
        Serial0.println(
            "[SDManager]   Already exists"
        );

        return true;
    }


    const bool ok =
        _card.fs().mkdir(
            path.c_str()
        );


    Serial0.printf(
        "[SDManager]   mkdir -> %s\n",
        ok ? "OK" : "FAILED"
    );


    return ok;
}


// ============================================================
// FILE EXISTS
// ============================================================

bool SDManager::fileExists(
    const String& path
) const
{
    Serial0.printf(
        "[SDManager] fileExists('%s')\n",
        path.c_str()
    );


    if (!_initialized)
    {
        Serial0.println(
            "[SDManager]   Not initialized"
        );

        return false;
    }

    if (!_card.isMounted())
    {
        Serial0.println(
            "[SDManager]   Card not mounted"
        );

        return false;
    }

    if (path.isEmpty())
    {
        Serial0.println(
            "[SDManager]   Empty path"
        );

        return false;
    }


    const bool exists =
        _card.exists(
            path.c_str()
        );


    Serial0.printf(
        "[SDManager]   -> %s\n",
        exists ? "true" : "false"
    );


    return exists;
}


// ============================================================
// READ FILE
// ============================================================

bool SDManager::readFile(
    const String& path,
    String& content
)
{
    Serial0.printf(
        "[SDManager] readFile('%s')\n",
        path.c_str()
    );


    content = "";

    if (!_initialized)
    {
        Serial0.println(
            "[SDManager]   Not initialized"
        );

        return false;
    }

    if (!_card.isMounted())
    {
        Serial0.println(
            "[SDManager]   Card not mounted"
        );

        return false;
    }

    if (path.isEmpty())
    {
        Serial0.println(
            "[SDManager]   Empty path"
        );

        return false;
    }


    File file =
        _card.fs().open(
            path.c_str(),
            FILE_READ
        );


    if (!file)
    {
        Serial0.println(
            "[SDManager]   Open FAILED"
        );

        return false;
    }


    content =
        file.readString();


    file.close();


    Serial0.printf(
        "[SDManager]   Read %u bytes\n",
        static_cast<unsigned>(content.length())
    );


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
    Serial0.printf(
        "[SDManager] writeFile('%s', %u bytes)\n",
        path.c_str(),
        static_cast<unsigned>(content.length())
    );


    if (!_initialized)
    {
        Serial0.println(
            "[SDManager]   Not initialized"
        );

        return false;
    }

    if (!_card.isMounted())
    {
        Serial0.println(
            "[SDManager]   Card not mounted"
        );

        return false;
    }

    if (path.isEmpty())
    {
        Serial0.println(
            "[SDManager]   Empty path"
        );

        return false;
    }


    File file =
        _card.fs().open(
            path.c_str(),
            FILE_WRITE
        );


    if (!file)
    {
        Serial0.println(
            "[SDManager]   Open FAILED"
        );

        return false;
    }


    const size_t written =
        file.print(content);


    file.close();


    const bool ok =
        (written == content.length());


    Serial0.printf(
        "[SDManager]   Wrote %u / %u -> %s\n",
        static_cast<unsigned>(written),
        static_cast<unsigned>(content.length()),
        ok ? "OK" : "MISMATCH"
    );


    return ok;
}


// ============================================================
// DELETE FILE
// ============================================================

bool SDManager::deleteFile(
    const String& path
)
{
    Serial0.printf(
        "[SDManager] deleteFile('%s')\n",
        path.c_str()
    );


    if (!_initialized)
    {
        Serial0.println(
            "[SDManager]   Not initialized"
        );

        return false;
    }

    if (!_card.isMounted())
    {
        Serial0.println(
            "[SDManager]   Card not mounted"
        );

        return false;
    }

    if (path.isEmpty())
    {
        Serial0.println(
            "[SDManager]   Empty path"
        );

        return false;
    }


    const bool ok =
        _card.fs().remove(
            path.c_str()
        );


    Serial0.printf(
        "[SDManager]   remove -> %s\n",
        ok ? "OK" : "FAILED"
    );


    return ok;
}