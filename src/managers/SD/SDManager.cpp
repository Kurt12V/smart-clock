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
// LIST FILES PAGE
//
// Пагинация.
//
// Пример:
//
// offset = 0,  maxFiles = 10
//     → записи 0..9
//
// offset = 10, maxFiles = 10
//     → записи 10..19
//
// offset = 20, maxFiles = 10
//     → записи 20..29
//
// ВАЖНО:
//
// Мы НЕ создаём массив:
//
//     SDFileEntry[offset + maxFiles]
//
// SDManager просто проходит записи и пропускает первые
// offset элементов.
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
    hasMore = false;


    // ========================================================
    // VALIDATION
    // ========================================================

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


    // ========================================================
    // SKIPPED
    //
    // Сколько записей уже пропущено.
    // ========================================================

    size_t skipped = 0;


    // ========================================================
    // SCAN
    // ========================================================

    return listDirPage(
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
}


// ============================================================
// LIST DIRECTORY
//
// Обычный полный/ограниченный обход.
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
    // Открываем директорию
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
        // ADD ENTRY
        // ----------------------------------------------------

        out[count].path =
            path;

        out[count].size =
            size;

        out[count].isDir =
            isDirectory;


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
//
// Основная логика пагинации.
//
// Мы идём по SD последовательно:
//
// 1. Если запись ещё находится до offset:
//       пропускаем.
//
// 2. Если offset уже достигнут:
//       сохраняем запись.
//
// 3. Если страница заполнена и обнаружена ещё одна запись:
//       hasMore = true.
//
// В результате не требуется хранить предыдущие страницы.
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
        return count;

    if (!dir.isDirectory())
    {
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
        //
        // Пока не пропустили нужное количество записей,
        // текущая запись нам не нужна.
        // ====================================================

        if (skipped < offset)
        {
            ++skipped;

            // ------------------------------------------------
            // Даже пропущенная директория должна быть
            // обработана рекурсивно.
            // ------------------------------------------------

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
        //
        // Все maxFiles элементов уже собраны.
        //
        // Текущая запись означает, что существует ещё один
        // элемент после страницы.
        // ====================================================

        if (count >= maxFiles)
        {
            hasMore = true;

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


        // ====================================================
        // PAGE FULL
        // ====================================================

        if (
            count >= maxFiles &&
            !hasMore
        )
        {
            // Пока мы ещё не знаем, есть ли следующий
            // элемент.
            //
            // Поэтому продолжаем один шаг цикла.
            //
            // Следующая запись установит hasMore=true.
        }
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
    if (!_initialized)
        return false;

    if (!_card.isMounted())
        return false;

    if (path.isEmpty())
        return false;


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
