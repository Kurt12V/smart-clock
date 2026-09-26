#include "FileUtils.h"

// ============================================================
// EXISTS
// ============================================================

bool FileUtils::exists(
    fs::FS& fs,
    const char* path
)
{
    if (!path)
        return false;

    return fs.exists(path);
}

// ============================================================
// CREATE DIRECTORY
// ============================================================

bool FileUtils::createDir(
    fs::FS& fs,
    const char* path
)
{
    if (!path)
        return false;

    if (fs.exists(path))
        return true;

    return fs.mkdir(path);
}

// ============================================================
// REMOVE FILE
// ============================================================

bool FileUtils::removeFile(
    fs::FS& fs,
    const char* path
)
{
    if (!path)
        return false;

    if (!fs.exists(path))
        return true;

    return fs.remove(path);
}

// ============================================================
// WRITE FILE
// ============================================================

bool FileUtils::writeFile(
    fs::FS& fs,
    const char* path,
    const String& data
)
{
    if (!path)
        return false;

    File file = fs.open(
        path,
        FILE_WRITE
    );

    if (!file)
        return false;

    size_t written =
        file.print(data);

    file.close();

    return written == data.length();
}

// ============================================================
// APPEND FILE
// ============================================================

bool FileUtils::appendFile(
    fs::FS& fs,
    const char* path,
    const String& data
)
{
    if (!path)
        return false;

    File file = fs.open(
        path,
        FILE_APPEND
    );

    if (!file)
        return false;

    size_t written =
        file.print(data);

    file.close();

    return written == data.length();
}

// ============================================================
// READ FILE
// ============================================================

String FileUtils::readFile(
    fs::FS& fs,
    const char* path
)
{
    if (!path)
        return String();
    
    File file = fs.open(
        path,
        FILE_READ
    );

    if (!file)
        return String();

    String data =
        file.readString();

    file.close();

    return data;
}

// ============================================================
// FILE SIZE
// ============================================================

size_t FileUtils::fileSize(
    fs::FS& fs,
    const char* path
)
{
    if (!path)
        return 0;

    File file = fs.open(
        path,
        FILE_READ
    );

    if (!file)
        return 0;

    size_t size =
        file.size();

    file.close();

    return size;
}

// ============================================================
// COPY FILE
// ============================================================

bool FileUtils::copyFile(
    fs::FS& fs,
    const char* source,
    const char* destination
)
{
    if (!source || !destination)
        return false;

    // --------------------------------------------------------
    // OPEN SOURCE
    // --------------------------------------------------------

    File src =
        fs.open(
            source,
            FILE_READ
        );

    if (!src)
        return false;

    // --------------------------------------------------------
    // OPEN DESTINATION
    // --------------------------------------------------------

    File dst =
        fs.open(
            destination,
            FILE_WRITE
        );

    if (!dst)
    {
        src.close();
        return false;
    }

    // --------------------------------------------------------
    // COPY
    // --------------------------------------------------------

    uint8_t buffer[512];

    bool success = true;

    while (src.available())
    {
        size_t bytesRead =
            src.read(
                buffer,
                sizeof(buffer)
            );

        if (bytesRead == 0)
        {
            success = false;
            break;
        }

        size_t bytesWritten =
            dst.write(
                buffer,
                bytesRead
            );

        if (bytesWritten != bytesRead)
        {
            success = false;
            break;
        }
    }

    // --------------------------------------------------------
    // CLOSE
    // --------------------------------------------------------

    src.close();
    dst.close();

    // --------------------------------------------------------
    // REMOVE BROKEN FILE
    // --------------------------------------------------------

    if (!success)
    {
        fs.remove(destination);
        return false;
    }

    return true;
}