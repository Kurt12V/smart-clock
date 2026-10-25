#include <Arduino.h>

#include "Pins.h"
#include "./managers/SPI/SPIManager.h"
#include "./managers/SD/SDManager.h"

// ============================================================
// OBJECTS
// ============================================================

static SPIManager g_spiManager;
static SDManager  g_sdManager;

// ============================================================
// TEST CONFIGURATION
// ============================================================

// static constexpr uint32_t SD_SPI_FREQUENCY = 400000;

static const String TEST_FILE_PATH = "/sd_test.txt";
static const String TEST_FILE_CONTENT = "ESP32-S3 SD card test";

// ============================================================
// TEST: CARD INFORMATION
// ============================================================

static void printCardInfo()
{
    Serial0.println();
    Serial0.println("----------- CARD INFO -----------");

    const SDCardInfo info = g_sdManager.getInfo();

    Serial0.printf(
        "[TEST] Total space: %s\n",
        info.totalSize.c_str()
    );

    Serial0.printf(
        "[TEST] Used space: %s\n",
        info.usedSize.c_str()
    );

    Serial0.printf(
        "[TEST] Free space: %s\n",
        info.freeSize.c_str()
    );

    Serial0.printf(
        "[TEST] Used: %.2f%%\n",
        info.usedPercent
    );

    Serial0.printf(
        "[TEST] Free: %.2f%%\n",
        info.freePercent
    );
}

// ============================================================
// TEST: FILE LIST
// ============================================================

static void printFileList()
{
    Serial0.println();
    Serial0.println("----------- FILESYSTEM -----------");

    constexpr size_t MAX_FILES = 32;

    SDFileEntry entries[MAX_FILES];

    const size_t count = g_sdManager.listFiles(
        entries,
        MAX_FILES,
        3,
        "/"
    );

    if (count == 0)
    {
        Serial0.println("[TEST] No entries found");
        return;
    }

    for (size_t i = 0; i < count; ++i)
    {
        Serial0.printf(
            "[TEST] %s %s",
            entries[i].isDir ? "[DIR] " : "[FILE]",
            entries[i].path.c_str()
        );

        if (!entries[i].isDir)
        {
            Serial0.printf(
                " (%llu bytes)",
                static_cast<unsigned long long>(
                    entries[i].size
                )
            );
        }

        Serial0.println();
    }

    Serial0.printf(
        "[TEST] Entries returned: %u\n",
        static_cast<unsigned>(count)
    );
}

// ============================================================
// TEST: WRITE FILE
// ============================================================

static bool testWriteFile()
{
    Serial0.println();
    Serial0.println("----------- WRITE TEST -----------");

    // Удаляем файл предыдущего теста, если он существует.
    if (g_sdManager.fileExists(TEST_FILE_PATH))
    {
        Serial0.println("[TEST] Removing previous test file");

        if (!g_sdManager.deleteFile(TEST_FILE_PATH))
        {
            Serial0.println(
                "[TEST] ERROR: Cannot remove previous test file"
            );

            return false;
        }
    }

    if (!g_sdManager.writeFile(
            TEST_FILE_PATH,
            TEST_FILE_CONTENT))
    {
        Serial0.println("[TEST] ERROR: writeFile() failed");
        return false;
    }

    Serial0.println("[TEST] writeFile() returned true");

    if (!g_sdManager.fileExists(TEST_FILE_PATH))
    {
        Serial0.println(
            "[TEST] ERROR: File does not exist after writing"
        );

        return false;
    }

    Serial0.println("[TEST] File exists");

    return true;
}

// ============================================================
// TEST: READ FILE
// ============================================================

static bool testReadFile()
{
    Serial0.println();
    Serial0.println("----------- READ TEST -----------");

    String content;

    if (!g_sdManager.readFile(
            TEST_FILE_PATH,
            content))
    {
        Serial0.println("[TEST] ERROR: readFile() failed");
        return false;
    }

    Serial0.printf(
        "[TEST] Expected length: %u\n",
        static_cast<unsigned>(TEST_FILE_CONTENT.length())
    );

    Serial0.printf(
        "[TEST] Actual length: %u\n",
        static_cast<unsigned>(content.length())
    );

    Serial0.print("[TEST] Content: [");
    Serial0.print(content);
    Serial0.println("]");

    if (content != TEST_FILE_CONTENT)
    {
        Serial0.println("[TEST] FAIL: Content mismatch");
        return false;
    }

    Serial0.println("[TEST] PASS: Content matches");

    return true;
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);
    delay(1000);

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println(" ESP32-S3 SD CARD TEST");
    Serial0.println(" Existing managers only");
    Serial0.println("========================================");

    // Serial0.printf(
    //     "[TEST] SPI frequency requested: %lu Hz\n",
    //     static_cast<unsigned long>(SD_SPI_FREQUENCY)
    // );

    // --------------------------------------------------------
    // 1. SPI MANAGER
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println("[1] Initializing SPIManager...");

    if (!g_spiManager.begin())
    {
        Serial0.println("[TEST] ERROR: SPIManager failed");
        return;
    }

    if (!g_spiManager.isReady())
    {
        Serial0.println("[TEST] ERROR: SPI bus is not ready");
        return;
    }

    Serial0.println("[TEST] SPIManager ready");

    // --------------------------------------------------------
    // 2. SD MANAGER
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println("[2] Initializing SDManager...");

    if (!g_sdManager.begin(PIN_SD_CS))
    {
        Serial0.println("[TEST] ERROR: SDManager::begin() failed");
        return;
    }

    if (!g_sdManager.isReady())
    {
        Serial0.println("[TEST] ERROR: SDManager is not ready");
        return;
    }

    Serial0.println("[TEST] SDManager ready");

    // --------------------------------------------------------
    // 3. CARD INFORMATION
    // --------------------------------------------------------

    printCardInfo();

    // --------------------------------------------------------
    // 4. FILE LIST
    // --------------------------------------------------------

    printFileList();

    // --------------------------------------------------------
    // 5. WRITE TEST
    // --------------------------------------------------------

    const bool writeOK = testWriteFile();

    // --------------------------------------------------------
    // 6. READ TEST
    // --------------------------------------------------------

    bool readOK = false;

    if (writeOK)
    {
        readOK = testReadFile();
    }
    else
    {
        Serial0.println("[TEST] Read test skipped");
    }

    // --------------------------------------------------------
    // 7. FINAL RESULT
    // --------------------------------------------------------

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
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    delay(1000);
}