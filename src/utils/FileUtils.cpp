#include "FileUtils.h"

bool FileUtils::exists(fs::FS& fs, const char* path)
{
    if (!path)
        return false;

    return fs.exists(path);
}

bool FileUtils::createDir(fs::FS& fs, const char* path)
{
    if (!path)
        return false;

    if (fs.exists(path))
        return true;

    return fs.mkdir(path);
}

bool FileUtils::remove(fs::FS& fs, const char* path)
{
    if (!path)
        return false;

    if (!fs.exists(path))
        return true;

    return fs.remove(path);
}

bool FileUtils::writeFile(fs::FS& fs,
                          const char* path,
                          const String& data)
{
    if (!path)
        return false;

    File file = fs.open(path, FILE_WRITE);

    if (!file)
        return false;

    size_t written = file.print(data);

    file.close();

    return written == data.length();
}

bool FileUtils::appendFile(fs::FS& fs,
                           const char* path,
                           const String& data)
{
    if (!path)
        return false;

    File file = fs.open(path, FILE_APPEND);

    if (!file)
        return false;

    size_t written = file.print(data);

    file.close();

    return written == data.length();
}

String FileUtils::readFile(fs::FS& fs,
                           const char* path)
{
    if (!path)
        return String();

    File file = fs.open(path, FILE_READ);

    if (!file)
        return String();

    String data = file.readString();

    file.close();

    return data;
}

size_t FileUtils::fileSize(fs::FS& fs,
                           const char* path)
{
    if (!path)
        return 0;

    File file = fs.open(path, FILE_READ);

    if (!file)
        return 0;

    size_t size = file.size();

    file.close();

    return size;
}

bool FileUtils::copyFile(fs::FS& fs,
                         const char* source,
                         const char* destination)
{
    if (!source || !destination)
        return false;

    File src = fs.open(source, FILE_READ);

    if (!src)
        return false;

    File dst = fs.open(destination, FILE_WRITE);

    if (!dst)
    {
        src.close();
        return false;
    }

    uint8_t buffer[512];

    while (src.available())
    {
        size_t bytesRead = src.read(buffer, sizeof(buffer));

        if (bytesRead == 0)
            break;

        size_t bytesWritten = dst.write(buffer, bytesRead);

        if (bytesWritten != bytesRead)
        {
            src.close();
            dst.close();
            return false;
        }
    }

    src.close();
    dst.close();

    return true;
}