#include "SDCard.h"

SDCard::SDCard()
    : _mounted(false)
{
}

bool SDCard::begin(uint8_t csPin)
{
    if (_mounted)
        return true;

    if (!SD.begin(csPin))
    {
        _mounted = false;
        return false;
    }

    _mounted = true;

    return true;
}

bool SDCard::isMounted() const
{
    return _mounted;
}

void SDCard::end()
{
    if (!_mounted)
        return;

    SD.end();

    _mounted = false;
}

uint64_t SDCard::getTotalBytes() const
{
    if (!_mounted)
        return 0;

    return SD.totalBytes();
}

uint64_t SDCard::getUsedBytes() const
{
    if (!_mounted)
        return 0;

    return SD.usedBytes();
}

uint64_t SDCard::getFreeBytes() const
{
    if (!_mounted)
        return 0;

    uint64_t total = SD.totalBytes();
    uint64_t used = SD.usedBytes();

    if (used >= total)
        return 0;

    return total - used;
}

float SDCard::getUsedPercent() const
{
    uint64_t total = getTotalBytes();

    if (total == 0)
        return 0.0f;

    return (static_cast<float>(getUsedBytes()) /
            static_cast<float>(total)) * 100.0f;
}

float SDCard::getFreePercent() const
{
    return 100.0f - getUsedPercent();
}

String SDCard::getTotalSizeString() const
{
    uint64_t bytes = getTotalBytes();

    if (bytes >= 1024ULL * 1024ULL * 1024ULL)
    {
        return String(
            static_cast<float>(bytes) /
            (1024.0f * 1024.0f * 1024.0f),
            2
        ) + " GB";
    }

    if (bytes >= 1024ULL * 1024ULL)
    {
        return String(
            static_cast<float>(bytes) /
            (1024.0f * 1024.0f),
            2
        ) + " MB";
    }

    if (bytes >= 1024ULL)
    {
        return String(
            static_cast<float>(bytes) /
            1024.0f,
            2
        ) + " KB";
    }

    return String(bytes) + " B";
}

String SDCard::getUsedSizeString() const
{
    uint64_t bytes = getUsedBytes();

    if (bytes >= 1024ULL * 1024ULL * 1024ULL)
    {
        return String(
            static_cast<float>(bytes) /
            (1024.0f * 1024.0f * 1024.0f),
            2
        ) + " GB";
    }

    if (bytes >= 1024ULL * 1024ULL)
    {
        return String(
            static_cast<float>(bytes) /
            (1024.0f * 1024.0f),
            2
        ) + " MB";
    }

    if (bytes >= 1024ULL)
    {
        return String(
            static_cast<float>(bytes) /
            1024.0f,
            2
        ) + " KB";
    }

    return String(bytes) + " B";
}

String SDCard::getFreeSizeString() const
{
    uint64_t bytes = getFreeBytes();

    if (bytes >= 1024ULL * 1024ULL * 1024ULL)
    {
        return String(
            static_cast<float>(bytes) /
            (1024.0f * 1024.0f * 1024.0f),
            2
        ) + " GB";
    }

    if (bytes >= 1024ULL * 1024ULL)
    {
        return String(
            static_cast<float>(bytes) /
            (1024.0f * 1024.0f),
            2
        ) + " MB";
    }

    if (bytes >= 1024ULL)
    {
        return String(
            static_cast<float>(bytes) /
            1024.0f,
            2
        ) + " KB";
    }

    return String(bytes) + " B";
}

bool SDCard::exists(const char* path) const
{
    if (!_mounted || !path)
        return false;

    return SD.exists(path);
}

fs::FS& SDCard::fs()
{
    return SD;
}