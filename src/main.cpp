#include <Arduino.h>

#include "Pins.h"
#include "./managers/SPIManager.h"
#include "./managers/SDManager.h"

// ============================================================
// MANAGERS
// ============================================================

SPIManager spiManager;
SDManager sdManager;

// ============================================================
// PRINT DIRECTORY
// ============================================================

void printDirectory(
    fs::FS& fs,
    const char* dirname,
    uint8_t level = 0
)
{
    File root = fs.open(dirname);

    if (!root)
    {
        Serial.printf(
            "ERROR: Cannot open directory: %s\n",
            dirname
        );

        return;
    }

    if (!root.isDirectory())
    {
        Serial.printf(
            "ERROR: Not a directory: %s\n",
            dirname
        );

        root.close();
        return;
    }

    File file = root.openNextFile();

    while (file)
    {
        // ----------------------------------------------------
        // Отступ
        // ----------------------------------------------------

        for (uint8_t i = 0; i < level; i++)
        {
            Serial.print("    ");
        }

        // ----------------------------------------------------
        // DIRECTORY
        // ----------------------------------------------------

        if (file.isDirectory())
        {
            Serial.print("[DIR]  ");
            Serial.println(file.name());

            // Рекурсивно выводим содержимое
            printDirectory(
                fs,
                file.path(),
                level + 1
            );
        }

        // ----------------------------------------------------
        // FILE
        // ----------------------------------------------------

        else
        {
            Serial.print("[FILE] ");
            Serial.print(file.name());

            Serial.print("  (");
            Serial.print(file.size());
            Serial.println(" bytes)");
        }

        file.close();

        file = root.openNextFile();
    }

    root.close();
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("       ESP32-S3 SD CARD TEST");
    Serial.println("========================================");

    // ========================================================
    // SPI
    // ========================================================

    Serial.println();
    Serial.println("[1] Initializing SPI...");

    if (!spiManager.begin())
    {
        Serial.println("ERROR: SPI initialization failed!");
        return;
    }

    Serial.println("SPI OK");

    // ========================================================
    // SD
    // ========================================================

    Serial.println();
    Serial.println("[2] Initializing SD card...");

    if (!sdManager.begin(PIN_SD_CS))
    {
        Serial.println("ERROR: SD initialization failed!");
        return;
    }

    Serial.println("SD OK");

    // ========================================================
    // SD INFO
    // ========================================================

    Serial.println();
    Serial.println("========================================");
    Serial.println("              SD CARD INFO");
    Serial.println("========================================");

    SDCardInfo info = sdManager.getInfo();

    Serial.print("Total: ");
    Serial.println(info.totalSize);

    Serial.print("Used:  ");
    Serial.print(info.usedSize);
    Serial.print(" (");
    Serial.print(info.usedPercent, 1);
    Serial.println("%)");

    Serial.print("Free:  ");
    Serial.print(info.freeSize);
    Serial.print(" (");
    Serial.print(info.freePercent, 1);
    Serial.println("%)");

    // ========================================================
    // DIRECTORY TREE
    // ========================================================

    Serial.println();
    Serial.println("========================================");
    Serial.println("          SD CARD DIRECTORY TREE");
    Serial.println("========================================");

    printDirectory(
        sdManager.card().fs(),
        "/"
    );

    Serial.println();
    Serial.println("========================================");
    Serial.println("              TEST DONE");
    Serial.println("========================================");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // Ничего не делаем
}
// ============================================================
// ============================================================
// ============================================================
// ============================================================
// ============================================================
// ============================================================
// ============================================================
// ============================================================
// ============================================================
// ============================================================
// ============================================================
// ============================================================













// #include <Arduino.h>
// #include <driver/i2s.h>

// #include "Pins.h"

// #include "./managers/SPIManager.h"
// #include "./managers/SDManager.h"

// // ============================================================
// // MANAGERS
// // ============================================================

// SPIManager spiManager;
// SDManager sdManager;

// // ============================================================
// // I2S
// // ============================================================

// #define I2S_PORT I2S_NUM_0

// // ============================================================
// // WAV
// // ============================================================

// struct WavInfo
// {
//     uint32_t sampleRate;
//     uint16_t channels;
//     uint16_t bitsPerSample;
//     uint32_t dataSize;
// };

// // ============================================================
// // READ LITTLE ENDIAN
// // ============================================================

// uint16_t readLE16(File& file)
// {
//     uint8_t b[2];

//     if (file.read(b, 2) != 2)
//         return 0;

//     return
//         static_cast<uint16_t>(b[0]) |
//         (static_cast<uint16_t>(b[1]) << 8);
// }

// uint32_t readLE32(File& file)
// {
//     uint8_t b[4];

//     if (file.read(b, 4) != 4)
//         return 0;

//     return
//         static_cast<uint32_t>(b[0]) |
//         (static_cast<uint32_t>(b[1]) << 8) |
//         (static_cast<uint32_t>(b[2]) << 16) |
//         (static_cast<uint32_t>(b[3]) << 24);
// }

// // ============================================================
// // WAV HEADER
// // ============================================================

// bool readWavHeader(
//     File& file,
//     WavInfo& wav
// )
// {
//     char riff[4];
//     char wave[4];

//     // --------------------------------------------------------
//     // RIFF
//     // --------------------------------------------------------

//     if (file.readBytes(riff, 4) != 4)
//         return false;

//     if (memcmp(riff, "RIFF", 4) != 0)
//     {
//         Serial.println("ERROR: Not RIFF");
//         return false;
//     }

//     // RIFF size
//     readLE32(file);

//     // WAVE
//     if (file.readBytes(wave, 4) != 4)
//         return false;

//     if (memcmp(wave, "WAVE", 4) != 0)
//     {
//         Serial.println("ERROR: Not WAVE");
//         return false;
//     }

//     bool fmtFound = false;
//     bool dataFound = false;

//     wav.sampleRate = 0;
//     wav.channels = 0;
//     wav.bitsPerSample = 0;
//     wav.dataSize = 0;

//     // --------------------------------------------------------
//     // CHUNKS
//     // --------------------------------------------------------

//     while (file.available())
//     {
//         char chunkId[4];

//         if (file.readBytes(chunkId, 4) != 4)
//             return false;

//         uint32_t chunkSize =
//             readLE32(file);

//         // ----------------------------------------------------
//         // FORMAT
//         // ----------------------------------------------------

//         if (memcmp(chunkId, "fmt ", 4) == 0)
//         {
//             uint16_t audioFormat =
//                 readLE16(file);

//             wav.channels =
//                 readLE16(file);

//             wav.sampleRate =
//                 readLE32(file);

//             // byte rate
//             readLE32(file);

//             // block align
//             readLE16(file);

//             wav.bitsPerSample =
//                 readLE16(file);

//             // Дополнительные данные fmt
//             if (chunkSize > 16)
//             {
//                 file.seek(
//                     file.position() +
//                     (chunkSize - 16)
//                 );
//             }

//             // PCM
//             if (audioFormat != 1)
//             {
//                 Serial.println(
//                     "ERROR: WAV is not PCM"
//                 );

//                 return false;
//             }

//             fmtFound = true;
//         }

//         // ----------------------------------------------------
//         // DATA
//         // ----------------------------------------------------

//         else if (memcmp(chunkId, "data", 4) == 0)
//         {
//             wav.dataSize = chunkSize;

//             dataFound = true;

//             break;
//         }

//         // ----------------------------------------------------
//         // OTHER CHUNK
//         // ----------------------------------------------------

//         else
//         {
//             file.seek(
//                 file.position() +
//                 chunkSize
//             );
//         }

//         // WAV chunks выравниваются по 2 байта
//         if (chunkSize & 1)
//         {
//             file.seek(
//                 file.position() + 1
//             );
//         }
//     }

//     if (!fmtFound)
//     {
//         Serial.println(
//             "ERROR: fmt chunk not found"
//         );

//         return false;
//     }

//     if (!dataFound)
//     {
//         Serial.println(
//             "ERROR: data chunk not found"
//         );

//         return false;
//     }

//     return true;
// }

// // ============================================================
// // I2S BEGIN
// // ============================================================

// bool setupI2S(
//     uint32_t sampleRate
// )
// {
//     Serial.println(
//         "Initializing I2S..."
//     );

//     i2s_config_t config = {};

//     config.mode =
//         static_cast<i2s_mode_t>(
//             I2S_MODE_MASTER |
//             I2S_MODE_TX
//         );

//     config.sample_rate =
//         sampleRate;

//     config.bits_per_sample =
//         I2S_BITS_PER_SAMPLE_16BIT;

//     config.channel_format =
//         I2S_CHANNEL_FMT_RIGHT_LEFT;

//     config.communication_format =
//         I2S_COMM_FORMAT_STAND_I2S;

//     config.intr_alloc_flags =
//         ESP_INTR_FLAG_LEVEL1;

//     config.dma_buf_count = 8;
//     config.dma_buf_len = 512;

//     config.use_apll = false;

//     config.tx_desc_auto_clear = true;

//     config.fixed_mclk = 0;

//     esp_err_t result =
//         i2s_driver_install(
//             I2S_PORT,
//             &config,
//             0,
//             nullptr
//         );

//     if (result != ESP_OK)
//     {
//         Serial.printf(
//             "I2S driver error: %d\n",
//             result
//         );

//         return false;
//     }

//     // --------------------------------------------------------
//     // I2S PINS
//     // --------------------------------------------------------

//     i2s_pin_config_t pins = {};

//     pins.bck_io_num =
//         PIN_I2S_BCLK;

//     pins.ws_io_num =
//         PIN_I2S_LRCLK;

//     pins.data_out_num =
//         PIN_I2S_DIN;

//     pins.data_in_num =
//         I2S_PIN_NO_CHANGE;

//     result =
//         i2s_set_pin(
//             I2S_PORT,
//             &pins
//         );

//     if (result != ESP_OK)
//     {
//         Serial.printf(
//             "I2S pin error: %d\n",
//             result
//         );

//         i2s_driver_uninstall(
//             I2S_PORT
//         );

//         return false;
//     }

//     i2s_zero_dma_buffer(
//         I2S_PORT
//     );

//     Serial.println(
//         "I2S initialized"
//     );

//     return true;
// }

// // ============================================================
// // PLAY WAV
// // ============================================================

// bool playWav(
//     const char* path
// )
// {
//     if (!sdManager.isReady())
//     {
//         Serial.println(
//             "ERROR: SD is not ready"
//         );

//         return false;
//     }

//     Serial.printf(
//         "Opening: %s\n",
//         path
//     );

//     // --------------------------------------------------------
//     // Открываем через SDManager
//     // --------------------------------------------------------

//     File file =
//         sdManager.card().open(
//             path,
//             FILE_READ
//         );

//     if (!file)
//     {
//         Serial.println(
//             "ERROR: Cannot open WAV"
//         );

//         return false;
//     }

//     Serial.printf(
//         "File size: %u bytes\n",
//         file.size()
//     );

//     // --------------------------------------------------------
//     // WAV HEADER
//     // --------------------------------------------------------

//     WavInfo wav;

//     if (!readWavHeader(
//             file,
//             wav
//         ))
//     {
//         Serial.println(
//             "ERROR: Invalid WAV"
//         );

//         file.close();

//         return false;
//     }

//     // --------------------------------------------------------
//     // INFO
//     // --------------------------------------------------------

//     Serial.println();
//     Serial.println(
//         "========== WAV =========="
//     );

//     Serial.printf(
//         "Sample rate: %lu Hz\n",
//         wav.sampleRate
//     );

//     Serial.printf(
//         "Channels: %u\n",
//         wav.channels
//     );

//     Serial.printf(
//         "Bits: %u\n",
//         wav.bitsPerSample
//     );

//     Serial.printf(
//         "Data size: %lu bytes\n",
//         wav.dataSize
//     );

//     Serial.println(
//         "=========================="
//     );

//     // --------------------------------------------------------
//     // CHECK FORMAT
//     // --------------------------------------------------------

//     if (wav.channels != 2)
//     {
//         Serial.println(
//             "ERROR: WAV must be stereo"
//         );

//         file.close();

//         return false;
//     }

//     if (wav.bitsPerSample != 16)
//     {
//         Serial.println(
//             "ERROR: WAV must be 16-bit"
//         );

//         file.close();

//         return false;
//     }

//     // --------------------------------------------------------
//     // I2S
//     // --------------------------------------------------------

//     if (!setupI2S(
//             wav.sampleRate
//         ))
//     {
//         file.close();

//         return false;
//     }

//     // --------------------------------------------------------
//     // PLAY
//     // --------------------------------------------------------

//     Serial.println();
//     Serial.println(
//         "========== PLAY =========="
//     );

//     uint8_t buffer[1024];

//     uint32_t remaining =
//         wav.dataSize;

//     while (
//         remaining > 0 &&
//         file.available()
//     )
//     {
//         size_t bytesToRead =
//             min(
//                 static_cast<uint32_t>(
//                     sizeof(buffer)
//                 ),
//                 remaining
//             );

//         size_t bytesRead =
//             file.read(
//                 buffer,
//                 bytesToRead
//             );

//         if (bytesRead == 0)
//         {
//             Serial.println(
//                 "ERROR: SD read failed"
//             );

//             break;
//         }

//         size_t bytesWritten = 0;

//         esp_err_t result =
//             i2s_write(
//                 I2S_PORT,
//                 buffer,
//                 bytesRead,
//                 &bytesWritten,
//                 portMAX_DELAY
//             );

//         if (result != ESP_OK)
//         {
//             Serial.printf(
//                 "ERROR: I2S write: %d\n",
//                 result
//             );

//             break;
//         }

//         remaining -= bytesRead;
//     }

//     // --------------------------------------------------------
//     // WAIT FOR DMA
//     // --------------------------------------------------------

//     delay(100);

//     i2s_zero_dma_buffer(
//         I2S_PORT
//     );

//     file.close();

//     i2s_driver_uninstall(
//         I2S_PORT
//     );

//     Serial.println(
//         "Playback finished"
//     );

//     return true;
// }

// // ============================================================
// // SETUP
// // ============================================================

// void setup()
// {
//     Serial.begin(115200);

//     delay(1000);

//     Serial.println();
//     Serial.println(
//         "=============================="
//     );
//     Serial.println(
//         " ESP32-S3 AUDIO TEST"
//     );
//     Serial.println(
//         "=============================="
//     );

//     // ========================================================
//     // SPI
//     // ========================================================

//     if (!spiManager.begin())
//     {
//         Serial.println(
//             "SPI initialization FAILED"
//         );

//         return;
//     }

//     Serial.println(
//         "SPI OK"
//     );

//     // ========================================================
//     // SD
//     // ========================================================

//     if (!sdManager.begin(PIN_SD_CS))
//     {
//         Serial.println(
//             "SD initialization FAILED"
//         );

//         return;
//     }

//     Serial.println(
//         "SD OK"
//     );

//     // ========================================================
//     // PLAY
//     // ========================================================

//     playWav(
//         "audio\alarms/alarm_1.wav"
//     );
// }

// // ============================================================
// // LOOP
// // ============================================================

// void loop()
// {
//     // Для теста ничего не делаем
// }