#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include "Pins.h"

// ============================================================
// SD CARD TEST CONFIGURATION
// ============================================================

static constexpr uint32_t SD_SPI_FREQUENCY = 400000;

static constexpr char TEST_FILE_PATH[] = "/sd_test.txt";
static constexpr char TEST_FILE_CONTENT[] = "ESP32-S3 SD card test";

// ============================================================
// SPI INITIALIZATION
// ============================================================

static bool initializeSPI()
{
    Serial0.println();
    Serial0.println("[TEST] Initializing SPI...");

    pinMode(PIN_SD_CS, OUTPUT);
    digitalWrite(PIN_SD_CS, HIGH);

    SPI.begin(
        PIN_SCLK,
        PIN_SD_MISO,
        PIN_MOSI,
        PIN_SD_CS
    );

    delay(100);

    Serial0.println("[TEST] SPI initialized");

    return true;
}

// ============================================================
// SD CARD INITIALIZATION
// ============================================================

static bool initializeSD()
{
    Serial0.println();
    Serial0.println("[TEST] Mounting SD card...");

    if (!SD.begin(PIN_SD_CS, SPI, SD_SPI_FREQUENCY))
    {
        Serial0.println("[TEST] ERROR: SD.begin() failed");
        Serial0.println("[TEST] Card was not mounted");

        return false;
    }

    Serial0.println("[TEST] SD card mounted successfully!");

    return true;
}

// ============================================================
// CARD INFORMATION
// ============================================================

static void printCardInfo()
{
    Serial0.println();
    Serial0.println("----------- CARD INFO -----------");

    const uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial0.println("[TEST] ERROR: No SD card detected");
        return;
    }

    Serial0.print("[TEST] Card type: ");

    switch (cardType)
    {
        case CARD_MMC:
            Serial0.println("MMC");
            break;

        case CARD_SD:
            Serial0.println("SDSC");
            break;

        case CARD_SDHC:
            Serial0.println("SDHC / SDXC");
            break;

        default:
            Serial0.println("Unknown");
            break;
    }

    const uint64_t cardSize = SD.cardSize();
    const uint64_t totalBytes = SD.totalBytes();
    const uint64_t usedBytes = SD.usedBytes();

    Serial0.printf(
        "[TEST] Card size: %llu MB\n",
        static_cast<unsigned long long>(
            cardSize / (1024ULL * 1024ULL)
        )
    );

    Serial0.printf(
        "[TEST] Total space: %llu MB\n",
        static_cast<unsigned long long>(
            totalBytes / (1024ULL * 1024ULL)
        )
    );

    Serial0.printf(
        "[TEST] Used space: %llu MB\n",
        static_cast<unsigned long long>(
            usedBytes / (1024ULL * 1024ULL)
        )
    );
}

// ============================================================
// DIRECTORY LISTING
// ============================================================

static void listDirectory(const char* path)
{
    Serial0.println();
    Serial0.println("----------- FILESYSTEM -----------");

    File directory = SD.open(path);

    if (!directory)
    {
        Serial0.println("[TEST] ERROR: Cannot open directory");
        return;
    }

    if (!directory.isDirectory())
    {
        Serial0.println("[TEST] ERROR: Path is not a directory");
        directory.close();
        return;
    }

    Serial0.printf("[TEST] Directory: %s\n", path);

    size_t entryCount = 0;

    File entry = directory.openNextFile();

    while (entry)
    {
        Serial0.print("[TEST] ");

        if (entry.isDirectory())
        {
            Serial0.print("[DIR]  ");
        }
        else
        {
            Serial0.print("[FILE] ");
        }

        Serial0.print(entry.name());

        if (!entry.isDirectory())
        {
            Serial0.printf(
                " (%llu bytes)",
                static_cast<unsigned long long>(entry.size())
            );
        }

        Serial0.println();

        entry.close();
        ++entryCount;

        entry = directory.openNextFile();
    }

    directory.close();

    if (entryCount == 0)
    {
        Serial0.println("[TEST] Directory is empty");
    }
    else
    {
        Serial0.printf(
            "[TEST] Total entries: %u\n",
            static_cast<unsigned>(entryCount)
        );
    }
}

// ============================================================
// WRITE TEST FILE
// ============================================================

static bool writeTestFile()
{
    Serial0.println();
    Serial0.println("----------- WRITE TEST -----------");

    // Удаляем файл от предыдущего запуска,
    // чтобы результат теста не зависел от старого содержимого.
    if (SD.exists(TEST_FILE_PATH))
    {
        Serial0.println("[TEST] Removing previous test file...");

        if (!SD.remove(TEST_FILE_PATH))
        {
            Serial0.println("[TEST] ERROR: Cannot remove previous file");
            return false;
        }
    }

    File file = SD.open(TEST_FILE_PATH, FILE_WRITE);

    if (!file)
    {
        Serial0.println("[TEST] ERROR: Cannot open file for writing");
        return false;
    }

    const size_t expectedBytes = sizeof(TEST_FILE_CONTENT) - 1;

    const size_t written = file.print(TEST_FILE_CONTENT);

    file.flush();
    file.close();

    Serial0.printf(
        "[TEST] Expected bytes: %u\n",
        static_cast<unsigned>(expectedBytes)
    );

    Serial0.printf(
        "[TEST] Written bytes: %u\n",
        static_cast<unsigned>(written)
    );

    if (written != expectedBytes)
    {
        Serial0.println("[TEST] ERROR: Incomplete write");
        return false;
    }

    Serial0.println("[TEST] Write operation completed");

    return true;
}

// ============================================================
// READ TEST FILE
// ============================================================

static bool readTestFile()
{
    Serial0.println();
    Serial0.println("----------- READ TEST -----------");

    File file = SD.open(TEST_FILE_PATH, FILE_READ);

    if (!file)
    {
        Serial0.println("[TEST] ERROR: Cannot open file for reading");
        return false;
    }

    const size_t fileSize = file.size();

    Serial0.printf(
        "[TEST] File size: %u bytes\n",
        static_cast<unsigned>(fileSize)
    );

    if (fileSize == 0)
    {
        Serial0.println("[TEST] ERROR: File is empty");
        file.close();
        return false;
    }

    // Читаем ровно размер записанного файла.
    char buffer[sizeof(TEST_FILE_CONTENT)] = {};

    const size_t bytesRead = file.read(
        reinterpret_cast<uint8_t*>(buffer),
        sizeof(buffer) - 1
    );

    file.close();

    Serial0.printf(
        "[TEST] Bytes read: %u\n",
        static_cast<unsigned>(bytesRead)
    );

    Serial0.print("[TEST] Content: [");
    Serial0.write(
        reinterpret_cast<const uint8_t*>(buffer),
        bytesRead
    );
    Serial0.println("]");

    const size_t expectedBytes = sizeof(TEST_FILE_CONTENT) - 1;

    if (bytesRead != expectedBytes)
    {
        Serial0.println("[TEST] FAIL: Incorrect number of bytes read");
        return false;
    }

    if (memcmp(buffer, TEST_FILE_CONTENT, expectedBytes) != 0)
    {
        Serial0.println("[TEST] FAIL: Content mismatch");
        return false;
    }

    Serial0.println("[TEST] PASS: Write/read successful");

    return true;
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);
    delay(1500);

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("[TEST] ESP32-S3 SD CARD DIAGNOSTIC");
    Serial0.println("========================================");

    Serial0.printf("[TEST] SCLK: %d\n", PIN_SCLK);
    Serial0.printf("[TEST] MOSI: %d\n", PIN_MOSI);
    Serial0.printf("[TEST] MISO: %d\n", PIN_SD_MISO);
    Serial0.printf("[TEST] CS:   %d\n", PIN_SD_CS);

    Serial0.printf(
        "[TEST] SPI frequency: %lu Hz\n",
        static_cast<unsigned long>(SD_SPI_FREQUENCY)
    );

    initializeSPI();

    if (!initializeSD())
    {
        Serial0.println();
        Serial0.println("[TEST] RESULT: SD INITIALIZATION FAILED");
        SPI.end();
        return;
    }

    printCardInfo();

    listDirectory("/");

    const bool writeOK = writeTestFile();

    bool readOK = false;

    if (writeOK)
    {
        readOK = readTestFile();
    }
    else
    {
        Serial0.println("[TEST] Read test skipped because write failed");
    }

    Serial0.println();
    Serial0.println("========================================");

    if (writeOK && readOK)
    {
        Serial0.println("[TEST] RESULT: ALL TESTS PASSED");
    }
    else
    {
        Serial0.println("[TEST] RESULT: TEST FAILED");
    }

    Serial0.println("========================================");

    // Карту не отключаем: оставляем смонтированной.
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    delay(1000);
}