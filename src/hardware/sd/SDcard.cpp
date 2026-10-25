#include "SDcard.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

SDCard::SDCard()
    : _mounted(false),
      _csPin(255)
{
}

// ============================================================
// BEGIN
// ============================================================

bool SDCard::begin(uint8_t csPin)
{
    if (_mounted)
        return true;

    _csPin = csPin;

    // Убедимся, что SD-карта не выбрана до начала обмена.
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);

    // Низкая частота для первичной диагностики.
    constexpr uint32_t SD_SPI_FREQUENCY = 400000;

    if (!SD.begin(_csPin, SPI, SD_SPI_FREQUENCY))
    {
        _mounted = false;

        Serial0.println("[SD] ERROR: SD.begin() failed");
        Serial0.println("[SD] Check wiring, card module and SPI signals");

        return false;
    }

    _mounted = true;

    Serial0.println("[SD] Card mounted successfully");

    return true;
}

// ============================================================
// IS MOUNTED
// ============================================================

bool SDCard::isMounted() const
{
    return _mounted;
}

// ============================================================
// END
// ============================================================

void SDCard::end()
{
    if (!_mounted)
        return;

    SD.end();

    _mounted = false;
}

// ============================================================
// GET INFO
// ============================================================

SDCardInfo SDCard::getInfo() const
{
    SDCardInfo info;

    if (!_mounted)
        return info;

    // --------------------------------------------------------
    // BYTES
    // --------------------------------------------------------

    info.totalBytes = SD.totalBytes();
    info.usedBytes  = SD.usedBytes();

    if (info.usedBytes >= info.totalBytes)
    {
        info.freeBytes = 0;
    }
    else
    {
        info.freeBytes =
            info.totalBytes -
            info.usedBytes;
    }

    // --------------------------------------------------------
    // PERCENT
    // --------------------------------------------------------

    if (info.totalBytes > 0)
    {
        info.usedPercent =
            (
                static_cast<float>(info.usedBytes) /
                static_cast<float>(info.totalBytes)
            ) * 100.0f;

        info.freePercent =
            100.0f - info.usedPercent;
    }

    // --------------------------------------------------------
    // FORMATTED SIZE
    // --------------------------------------------------------

    info.totalSize =
        formatBytes(info.totalBytes);

    info.usedSize =
        formatBytes(info.usedBytes);

    info.freeSize =
        formatBytes(info.freeBytes);

    return info;
}

// ============================================================
// FORMAT BYTES
// ============================================================

String SDCard::formatBytes(uint64_t bytes) const
{
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

// ============================================================
// EXISTS
// ============================================================

bool SDCard::exists(const char* path) const
{
    if (!_mounted || !path)
        return false;

    return SD.exists(path);
}

// ============================================================
// FILE SYSTEM
// ============================================================

fs::FS& SDCard::fs()
{
    return SD;
}