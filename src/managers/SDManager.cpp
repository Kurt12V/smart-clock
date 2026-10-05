#include "SDManager.h"


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

bool SDManager::begin(
    uint8_t csPin
)
{
    _initialized = false;

    if (!_card.begin(csPin))
        return false;

    _initialized = true;

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
}


// ============================================================
// IS READY
// ============================================================

bool SDManager::isReady() const
{
    return (
        _initialized &&
        _card.isMounted()
    );
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
// INFO
// ============================================================

SDCardInfo SDManager::getInfo() const
{
    if (!_initialized)
        return SDCardInfo{};

    return _card.getInfo();
}


// ============================================================
// LIST FILES
// ============================================================

size_t SDManager::listFiles(
    SDFileEntry* out,
    size_t maxFiles,
    uint8_t maxDepth,
    const char* root
) const
{
    if (out == nullptr)
        return 0;

    if (maxFiles == 0)
        return 0;

    if (!_initialized)
        return 0;

    if (!_card.isMounted())
        return 0;

    if (root == nullptr)
        root = "/";

    return listDir(
        root,
        out,
        maxFiles,
        0,
        0,
        maxDepth
    );
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


    // --------------------------------------------------------
    // Открываем директорию.
    //
    // Здесь используется существующий SD API,
    // как и в твоей исходной реализации.
    // --------------------------------------------------------

    File dir =
        SD.open(dirname);

    if (!dir)
        return count;

    if (!dir.isDirectory())
    {
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
        // Добавляем запись
        // ----------------------------------------------------

        out[count].path =
            path;

        out[count].size =
            size;

        out[count].isDir =
            isDirectory;


        ++count;


        // ----------------------------------------------------
        // Рекурсивный обход
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
// CREATE DIRECTORY
// ============================================================

bool SDManager::createDirectory(
    const String& path
)
{
    if (!_initialized)
        return false;

    if (!_card.isMounted())
        return false;

    if (path.isEmpty())
        return false;


    // Уже существует

    if (
        _card.exists(
            path.c_str()
        )
    )
    {
        return true;
    }


    return _card.fs().mkdir(
        path.c_str()
    );
}


// ============================================================
// FILE EXISTS
// ============================================================

bool SDManager::fileExists(
    const String& path
) const
{
    if (!_initialized)
        return false;

    if (!_card.isMounted())
        return false;

    if (path.isEmpty())
        return false;


    return _card.exists(
        path.c_str()
    );
}


// ============================================================
// READ FILE
// ============================================================

bool SDManager::readFile(
    const String& path,
    String& content
)
{
    content = "";

    if (!_initialized)
        return false;

    if (!_card.isMounted())
        return false;

    if (path.isEmpty())
        return false;


    File file =
        _card.fs().open(
            path.c_str(),
            FILE_READ
        );


    if (!file)
        return false;


    content =
        file.readString();


    file.close();


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
    if (!_initialized)
        return false;

    if (!_card.isMounted())
        return false;

    if (path.isEmpty())
        return false;


    File file =
        _card.fs().open(
            path.c_str(),
            FILE_WRITE
        );


    if (!file)
        return false;


    const size_t written =
        file.print(content);


    file.close();


    return (
        written ==
        content.length()
    );
}


// ============================================================
// DELETE FILE
// ============================================================

bool SDManager::deleteFile(
    const String& path
)
{
    if (!_initialized)
        return false;

    if (!_card.isMounted())
        return false;

    if (path.isEmpty())
        return false;


    return _card.fs().remove(
        path.c_str()
    );
}