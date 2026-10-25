#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include "Pins.h"

// ============================================================
// НАСТРОЙКИ ТЕСТА SD-КАРТЫ
// ============================================================

static constexpr uint32_t SD_SPI_FREQUENCY = 400000;

// ============================================================
// ИНИЦИАЛИЗАЦИЯ И ТЕСТ SD-КАРТЫ
// ============================================================

static void testSDCard()
{
    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("[TEST] SD CARD DIAGNOSTIC");
    Serial0.println("========================================");

    Serial0.printf("[TEST] SCLK: %d\n", PIN_SCLK);
    Serial0.printf("[TEST] MOSI: %d\n", PIN_MOSI);
    Serial0.printf("[TEST] MISO: %d\n", PIN_SD_MISO);
    Serial0.printf("[TEST] CS:   %d\n", PIN_SD_CS);
    Serial0.printf("[TEST] SPI frequency: %lu Hz\n",
                   static_cast<unsigned long>(SD_SPI_FREQUENCY));

    // --------------------------------------------------------
    // 1. Настройка CS
    // --------------------------------------------------------

    pinMode(PIN_SD_CS, OUTPUT);
    digitalWrite(PIN_SD_CS, HIGH);

    // --------------------------------------------------------
    // 2. Инициализация SPI
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println("[TEST] Initializing SPI...");

    SPI.begin(
        PIN_SCLK,
        PIN_SD_MISO,
        PIN_MOSI,
        PIN_SD_CS
    );

    delay(100);

    Serial0.println("[TEST] SPI initialized");

    // --------------------------------------------------------
    // 3. Инициализация SD-карты
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println("[TEST] Mounting SD card...");

    if (!SD.begin(PIN_SD_CS, SPI, SD_SPI_FREQUENCY))
    {
        Serial0.println("[TEST] ERROR: SD.begin() failed");
        Serial0.println("[TEST] Card was not mounted");
        Serial0.println("[TEST] Check the SPI pins, CS and power");

        SPI.end();

        while (true)
        {
            delay(1000);
        }
    }

    Serial0.println("[TEST] SD card mounted successfully!");

    // --------------------------------------------------------
    // 4. Информация о карте
    // --------------------------------------------------------

    uint8_t cardType = SD.cardType();

    Serial0.println();
    Serial0.println("----------- CARD INFO -----------");

    if (cardType == CARD_NONE)
    {
        Serial0.println("[TEST] ERROR: No SD card detected");
    }
    else
    {
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

        uint64_t cardSize = SD.cardSize();

        Serial0.printf(
            "[TEST] Card size: %llu MB\n",
            static_cast<unsigned long long>(
                cardSize / (1024ULL * 1024ULL)
            )
        );

        Serial0.printf(
            "[TEST] Total space: %llu MB\n",
            static_cast<unsigned long long>(
                SD.totalBytes() / (1024ULL * 1024ULL)
            )
        );

        Serial0.printf(
            "[TEST] Used space: %llu MB\n",
            static_cast<unsigned long long>(
                SD.usedBytes() / (1024ULL * 1024ULL)
            )
        );
    }

    // --------------------------------------------------------
    // 5. Проверка файловой системы
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println("----------- FILESYSTEM -----------");

    File root = SD.open("/");

    if (!root)
    {
        Serial0.println("[TEST] ERROR: Cannot open root directory");
    }
    else if (!root.isDirectory())
    {
        Serial0.println("[TEST] ERROR: Root is not a directory");
        root.close();
    }
    else
    {
        Serial0.println("[TEST] Root directory opened");

        File file = root.openNextFile();

        if (!file)
        {
            Serial0.println("[TEST] Directory is empty");
        }

        while (file)
        {
            Serial0.print("[TEST] ");

            if (file.isDirectory())
            {
                Serial0.print("[DIR]  ");
            }
            else
            {
                Serial0.print("[FILE] ");
            }

            Serial0.print(file.name());

            if (!file.isDirectory())
            {
                Serial0.printf(
                    " (%llu bytes)",
                    static_cast<unsigned long long>(file.size())
                );
            }

            Serial0.println();

            file.close();
            file = root.openNextFile();
        }

        root.close();
    }

    // --------------------------------------------------------
    // 6. Проверка записи и чтения тестового файла
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println("----------- WRITE / READ TEST -----------");

    const char* testPath = "/sd_test.txt";
    const char* testText = "ESP32-S3 SD card test";

    File writeFile = SD.open(testPath, FILE_WRITE);

    if (!writeFile)
    {
        Serial0.println("[TEST] ERROR: Cannot open file for writing");
    }
    else
    {
        size_t written = writeFile.println(testText);
        writeFile.close();

        if (written == 0)
        {
            Serial0.println("[TEST] ERROR: Failed to write test data");
        }
        else
        {
            Serial0.println("[TEST] Write operation completed");

            File readFile = SD.open(testPath, FILE_READ);

            if (!readFile)
            {
                Serial0.println("[TEST] ERROR: Cannot open file for reading");
            }
            else
            {
                String content = readFile.readStringUntil('\n');
                readFile.close();

                Serial0.print("[TEST] Read result: ");
                Serial0.println(content);

                if (content == testText)
                {
                    Serial0.println("[TEST] PASS: Write/read successful");
                }
                else
                {
                    Serial0.println("[TEST] ERROR: Read data mismatch");
                }
            }
        }
    }

    // --------------------------------------------------------
    // 7. Завершение теста
    // --------------------------------------------------------

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("[TEST] SD CARD DIAGNOSTIC FINISHED");
    Serial0.println("========================================");

    // Оставляем карту смонтированной до перезагрузки.
}

void setup()
{
    Serial0.begin(115200);
    delay(1500);

    Serial0.println();
    Serial0.println("[TEST] Starting standalone SD test");

    testSDCard();
}

void loop()
{
    // Другие компоненты приложения не запускаются.
    delay(1000);
}