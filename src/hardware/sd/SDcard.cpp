#include "SDcard.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

SDCard::SDCard()
    : _mounted(false),
      _csPin(255)
{
    Serial0.println();
    Serial0.println("[SD][CONSTRUCTOR] SDCard object created");
    Serial0.printf("[SD][CONSTRUCTOR] Initial mounted state: %s\n",
                   _mounted ? "true" : "false");
    Serial0.printf("[SD][CONSTRUCTOR] Initial CS pin: %u\n",
                   _csPin);
}

// ============================================================
// BEGIN
// ============================================================

bool SDCard::begin(uint8_t csPin)
{
    Serial0.println();
    Serial0.println("================================================");
    Serial0.println("[SD][BEGIN] Starting SD card initialization");
    Serial0.println("================================================");

    Serial0.printf("[SD][BEGIN] Requested CS pin: GPIO %u\n", csPin);
    Serial0.printf("[SD][BEGIN] Previous mounted state: %s\n",
                   _mounted ? "true" : "false");

    if (_mounted)
    {
        Serial0.println("[SD][BEGIN] Card is already mounted");
        Serial0.println("[SD][BEGIN] Initialization skipped");
        return true;
    }

    _csPin = csPin;

    Serial0.printf("[SD][BEGIN] Stored CS pin: GPIO %u\n", _csPin);

    // --------------------------------------------------------
    // CHIP SELECT
    // --------------------------------------------------------

    Serial0.println("[SD][BEGIN] Configuring chip select pin");

    pinMode(_csPin, OUTPUT);

    Serial0.printf("[SD][BEGIN] GPIO %u configured as OUTPUT\n",
                   _csPin);

    digitalWrite(_csPin, HIGH);

    Serial0.printf("[SD][BEGIN] GPIO %u set HIGH (card deselected)\n",
                   _csPin);

    Serial0.printf("[SD][BEGIN] CS pin level after setup: %d\n",
                   digitalRead(_csPin));

    // --------------------------------------------------------
    // SPI CONFIGURATION
    // --------------------------------------------------------

    constexpr uint32_t SD_SPI_FREQUENCY = 400000;

    Serial0.println("[SD][BEGIN] SPI configuration:");
    Serial0.printf("[SD][BEGIN]   SPI bus: SPI\n");
    Serial0.printf("[SD][BEGIN]   CS pin: GPIO %u\n", _csPin);
    Serial0.printf("[SD][BEGIN]   SPI frequency: %lu Hz\n",
                   static_cast<unsigned long>(SD_SPI_FREQUENCY));
    Serial0.printf("[SD][BEGIN]   SPI frequency: %.2f kHz\n",
                   SD_SPI_FREQUENCY / 1000.0f);

    Serial0.println("[SD][BEGIN] Calling SD.begin()...");
    Serial0.println("[SD][BEGIN] Waiting for card response");

    // --------------------------------------------------------
    // MOUNT CARD
    // --------------------------------------------------------

    const unsigned long startTime = millis();

    const bool beginResult =
        SD.begin(_csPin, SPI, SD_SPI_FREQUENCY);

    const unsigned long elapsedTime = millis() - startTime;

    Serial0.printf("[SD][BEGIN] SD.begin() returned: %s\n",
                   beginResult ? "true" : "false");

    Serial0.printf("[SD][BEGIN] Initialization duration: %lu ms\n",
                   elapsedTime);

    Serial0.printf("[SD][BEGIN] CS pin level after SD.begin(): %d\n",
                   digitalRead(_csPin));

    if (!beginResult)
    {
        _mounted = false;

        Serial0.println();
        Serial0.println("[SD][ERROR] SD.begin() failed");
        Serial0.println("[SD][ERROR] Card was not mounted");
        Serial0.printf("[SD][ERROR] CS pin: GPIO %u\n", _csPin);
        Serial0.printf("[SD][ERROR] SPI frequency: %lu Hz\n",
                       static_cast<unsigned long>(SD_SPI_FREQUENCY));

        Serial0.println("[SD][ERROR] Check the following:");
        Serial0.println("[SD][ERROR] 1. SD card wiring");
        Serial0.println("[SD][ERROR] 2. CS, SCLK, MOSI and MISO pins");
        Serial0.println("[SD][ERROR] 3. Card adapter power and logic levels");
        Serial0.println("[SD][ERROR] 4. SPI bus initialization");
        Serial0.println("[SD][ERROR] 5. Card format and physical condition");

        Serial0.printf("[SD][ERROR] Mounted state: %s\n",
                       _mounted ? "true" : "false");

        Serial0.println("[SD][BEGIN] Initialization aborted");
        Serial0.println("================================================");

        return false;
    }

    // --------------------------------------------------------
    // MOUNT SUCCESS
    // --------------------------------------------------------

    _mounted = true;

    Serial0.println();
    Serial0.println("[SD][BEGIN] Card mounted successfully");
    Serial0.printf("[SD][BEGIN] Mounted state: %s\n",
                   _mounted ? "true" : "false");

    // --------------------------------------------------------
    // CARD TYPE
    // --------------------------------------------------------

    Serial0.println("[SD][BEGIN] Reading card type...");

    const uint8_t cardType = SD.cardType();

    Serial0.printf("[SD][BEGIN] Raw card type value: %u\n",
                   cardType);

    switch (cardType)
    {
        case CARD_NONE:
            Serial0.println("[SD][BEGIN] Card type: NONE");
            break;

        case CARD_MMC:
            Serial0.println("[SD][BEGIN] Card type: MMC");
            break;

        case CARD_SD:
            Serial0.println("[SD][BEGIN] Card type: SDSC");
            break;

        case CARD_SDHC:
            Serial0.println("[SD][BEGIN] Card type: SDHC / SDXC");
            break;

        default:
            Serial0.println("[SD][BEGIN] Card type: UNKNOWN");
            break;
    }

    // --------------------------------------------------------
    // CARD CAPACITY
    // --------------------------------------------------------

    Serial0.println("[SD][BEGIN] Reading card capacity...");

    const uint64_t cardSize = SD.cardSize();

    Serial0.printf("[SD][BEGIN] Card size: %llu bytes\n",
                   static_cast<unsigned long long>(cardSize));

    Serial0.printf("[SD][BEGIN] Card size: %.2f MB\n",
                   static_cast<double>(cardSize) / (1024.0 * 1024.0));

    Serial0.printf("[SD][BEGIN] Card size: %.2f GB\n",
                   static_cast<double>(cardSize) /
                       (1024.0 * 1024.0 * 1024.0));

    if (cardSize == 0)
    {
        Serial0.println("[SD][WARNING] Card size returned zero");
    }

    // --------------------------------------------------------
    // FILE SYSTEM CHECK
    // --------------------------------------------------------

    Serial0.println("[SD][BEGIN] Reading filesystem statistics...");

    const uint64_t totalBytes = SD.totalBytes();
    const uint64_t usedBytes = SD.usedBytes();

    Serial0.printf("[SD][BEGIN] Filesystem total: %llu bytes\n",
                   static_cast<unsigned long long>(totalBytes));

    Serial0.printf("[SD][BEGIN] Filesystem used: %llu bytes\n",
                   static_cast<unsigned long long>(usedBytes));

    if (totalBytes > 0)
    {
        Serial0.printf("[SD][BEGIN] Filesystem usage: %.2f%%\n",
                       (static_cast<double>(usedBytes) /
                        static_cast<double>(totalBytes)) * 100.0);
    }
    else
    {
        Serial0.println(
            "[SD][WARNING] Filesystem total size returned zero");
    }

    if (usedBytes > totalBytes && totalBytes > 0)
    {
        Serial0.println(
            "[SD][WARNING] Used space exceeds total filesystem size");
    }

    // --------------------------------------------------------
    // FILESYSTEM ROOT CHECK
    // --------------------------------------------------------

    Serial0.println("[SD][BEGIN] Checking root directory...");

    const bool rootExists = SD.exists("/");

    Serial0.printf("[SD][BEGIN] Root directory exists: %s\n",
                   rootExists ? "true" : "false");

    if (!rootExists)
    {
        Serial0.println(
            "[SD][WARNING] Root directory check failed");
    }

    Serial0.println("[SD][BEGIN] Initialization diagnostics complete");
    Serial0.println("================================================");

    return true;
}

// ============================================================
// IS MOUNTED
// ============================================================

bool SDCard::isMounted() const
{
    Serial0.printf("[SD][IS_MOUNTED] Result: %s\n",
                   _mounted ? "true" : "false");

    return _mounted;
}

// ============================================================
// END
// ============================================================

void SDCard::end()
{
    Serial0.println();
    Serial0.println("[SD][END] Stopping SD card");

    Serial0.printf("[SD][END] Current mounted state: %s\n",
                   _mounted ? "true" : "false");

    if (!_mounted)
    {
        Serial0.println("[SD][END] Card is not mounted; nothing to stop");
        return;
    }

    Serial0.println("[SD][END] Calling SD.end()");

    SD.end();

    _mounted = false;

    Serial0.println("[SD][END] SD.end() completed");
    Serial0.printf("[SD][END] Mounted state: %s\n",
                   _mounted ? "true" : "false");
}

// ============================================================
// GET INFO
// ============================================================

SDCardInfo SDCard::getInfo() const
{
    Serial0.println();
    Serial0.println("[SD][INFO] Reading SD card information");

    SDCardInfo info;

    Serial0.printf("[SD][INFO] Mounted state: %s\n",
                   _mounted ? "true" : "false");

    if (!_mounted)
    {
        Serial0.println("[SD][INFO] Card is not mounted");
        Serial0.println("[SD][INFO] Returning empty SDCardInfo");
        return info;
    }

    // --------------------------------------------------------
    // BYTES
    // --------------------------------------------------------

    Serial0.println("[SD][INFO] Reading totalBytes()...");
    info.totalBytes = SD.totalBytes();

    Serial0.println("[SD][INFO] Reading usedBytes()...");
    info.usedBytes = SD.usedBytes();

    Serial0.printf("[SD][INFO] Total bytes: %llu\n",
                   static_cast<unsigned long long>(info.totalBytes));

    Serial0.printf("[SD][INFO] Used bytes: %llu\n",
                   static_cast<unsigned long long>(info.usedBytes));

    if (info.usedBytes >= info.totalBytes)
    {
        info.freeBytes = 0;

        Serial0.println(
            "[SD][INFO] Free bytes set to zero (used >= total)");
    }
    else
    {
        info.freeBytes = info.totalBytes - info.usedBytes;
    }

    Serial0.printf("[SD][INFO] Free bytes: %llu\n",
                   static_cast<unsigned long long>(info.freeBytes));

    // --------------------------------------------------------
    // PERCENT
    // --------------------------------------------------------

    if (info.totalBytes > 0)
    {
        info.usedPercent =
            (static_cast<float>(info.usedBytes) /
             static_cast<float>(info.totalBytes)) * 100.0f;

        info.freePercent = 100.0f - info.usedPercent;
    }
    else
    {
        Serial0.println(
            "[SD][INFO][WARNING] Total bytes is zero; percentages remain zero");
    }

    Serial0.printf("[SD][INFO] Used percentage: %.2f%%\n",
                   info.usedPercent);

    Serial0.printf("[SD][INFO] Free percentage: %.2f%%\n",
                   info.freePercent);

    // --------------------------------------------------------
    // FORMATTED SIZE
    // --------------------------------------------------------

    Serial0.println("[SD][INFO] Formatting storage sizes...");

    info.totalSize = formatBytes(info.totalBytes);
    info.usedSize = formatBytes(info.usedBytes);
    info.freeSize = formatBytes(info.freeBytes);

    Serial0.printf("[SD][INFO] Total size: %s\n",
                   info.totalSize.c_str());

    Serial0.printf("[SD][INFO] Used size: %s\n",
                   info.usedSize.c_str());

    Serial0.printf("[SD][INFO] Free size: %s\n",
                   info.freeSize.c_str());

    Serial0.println("[SD][INFO] Information read complete");

    return info;
}

// ============================================================
// FORMAT BYTES
// ============================================================

String SDCard::formatBytes(uint64_t bytes) const
{
    Serial0.printf("[SD][FORMAT] Input bytes: %llu\n",
                   static_cast<unsigned long long>(bytes));

    if (bytes >= 1024ULL * 1024ULL * 1024ULL)
    {
        String result =
            String(
                static_cast<float>(bytes) /
                (1024.0f * 1024.0f * 1024.0f),
                2
            ) + " GB";

        Serial0.printf("[SD][FORMAT] Result: %s\n", result.c_str());
        return result;
    }

    if (bytes >= 1024ULL * 1024ULL)
    {
        String result =
            String(
                static_cast<float>(bytes) /
                (1024.0f * 1024.0f),
                2
            ) + " MB";

        Serial0.printf("[SD][FORMAT] Result: %s\n", result.c_str());
        return result;
    }

    if (bytes >= 1024ULL)
    {
        String result =
            String(
                static_cast<float>(bytes) / 1024.0f,
                2
            ) + " KB";

        Serial0.printf("[SD][FORMAT] Result: %s\n", result.c_str());
        return result;
    }

    String result = String(bytes) + " B";

    Serial0.printf("[SD][FORMAT] Result: %s\n", result.c_str());

    return result;
}

// ============================================================
// EXISTS
// ============================================================

bool SDCard::exists(const char* path) const
{
    Serial0.println();
    Serial0.println("[SD][EXISTS] Checking path");

    if (path == nullptr)
    {
        Serial0.println("[SD][EXISTS][ERROR] Path is nullptr");
        return false;
    }

    Serial0.printf("[SD][EXISTS] Path: %s\n", path);
    Serial0.printf("[SD][EXISTS] Mounted state: %s\n",
                   _mounted ? "true" : "false");

    if (!_mounted)
    {
        Serial0.println("[SD][EXISTS][ERROR] Card is not mounted");
        return false;
    }

    const bool result = SD.exists(path);

    Serial0.printf("[SD][EXISTS] Result: %s\n",
                   result ? "true" : "false");

    return result;
}

// ============================================================
// FILE SYSTEM
// ============================================================

fs::FS& SDCard::fs()
{
    Serial0.println("[SD][FS] Returning SD filesystem reference");

    return SD;
}