#include <Arduino.h>
#include <driver/i2s.h>

#include "Pins.h"
#include "./managers/SPIManager.h"
#include "./managers/SDManager.h"

// ============================================================
// MANAGERS
// ============================================================

SPIManager spiManager;
SDManager sdManager;

// ============================================================
// AUDIO
// ============================================================

#define I2S_PORT I2S_NUM_0

static constexpr const char* TEST_WAV = "/audio/alarms/alarm_1.wav";


// ============================================================
// WAV INFO
// ============================================================

struct WavInfo
{
    uint16_t audioFormat = 0;
    uint16_t channels = 0;
    uint32_t sampleRate = 0;
    uint16_t bitsPerSample = 0;

    uint32_t dataOffset = 0;
    uint32_t dataSize = 0;

    bool valid = false;
};


// ============================================================
// READ LITTLE ENDIAN
// ============================================================

uint16_t readLE16(File& file)
{
    uint8_t b[2];

    if (file.read(b, 2) != 2)
        return 0;

    return
        static_cast<uint16_t>(b[0]) |
        (static_cast<uint16_t>(b[1]) << 8);
}


uint32_t readLE32(File& file)
{
    uint8_t b[4];

    if (file.read(b, 4) != 4)
        return 0;

    return
        static_cast<uint32_t>(b[0]) |
        (static_cast<uint32_t>(b[1]) << 8) |
        (static_cast<uint32_t>(b[2]) << 16) |
        (static_cast<uint32_t>(b[3]) << 24);
}


// ============================================================
// READ CHUNK ID
// ============================================================

bool readChunkId(File& file, char* id)
{
    if (file.read(
            reinterpret_cast<uint8_t*>(id),
            4
        ) != 4)
    {
        return false;
    }

    id[4] = '\0';

    return true;
}


// ============================================================
// PARSE WAV
// ============================================================

bool parseWav(
    File& file,
    WavInfo& wav
)
{
    wav = WavInfo();

    // --------------------------------------------------------
    // RIFF
    // --------------------------------------------------------

    char riff[5];

    if (!readChunkId(file, riff))
        return false;

    if (strcmp(riff, "RIFF") != 0)
    {
        Serial.println("[WAV] Not RIFF");
        return false;
    }

    // File size
    readLE32(file);

    // WAVE
    char wave[5];

    if (!readChunkId(file, wave))
        return false;

    if (strcmp(wave, "WAVE") != 0)
    {
        Serial.println("[WAV] Not WAVE");
        return false;
    }

    bool foundFmt = false;
    bool foundData = false;

    // --------------------------------------------------------
    // CHUNKS
    // --------------------------------------------------------

    while (file.available())
    {
        char chunkId[5];

        if (!readChunkId(file, chunkId))
            break;

        uint32_t chunkSize =
            readLE32(file);

        uint32_t chunkStart =
            file.position();

        // ----------------------------------------------------
        // FORMAT
        // ----------------------------------------------------

        if (strcmp(chunkId, "fmt ") == 0)
        {
            wav.audioFormat =
                readLE16(file);

            wav.channels =
                readLE16(file);

            wav.sampleRate =
                readLE32(file);

            // Byte rate
            readLE32(file);

            // Block align
            readLE16(file);

            wav.bitsPerSample =
                readLE16(file);

            foundFmt = true;
        }

        // ----------------------------------------------------
        // DATA
        // ----------------------------------------------------

        else if (strcmp(chunkId, "data") == 0)
        {
            wav.dataOffset =
                file.position();

            wav.dataSize =
                chunkSize;

            foundData = true;

            break;
        }

        // ----------------------------------------------------
        // UNKNOWN CHUNK
        // ----------------------------------------------------

        else
        {
            file.seek(
                file.position() + chunkSize
            );
        }

        // ----------------------------------------------------
        // RIFF chunks are padded to even size
        // ----------------------------------------------------

        if (chunkSize & 1)
        {
            file.seek(
                file.position() + 1
            );
        }

        // Safety against malformed WAV
        if (file.position() < chunkStart)
            return false;
    }

    // --------------------------------------------------------
    // VALIDATE
    // --------------------------------------------------------

    if (!foundFmt)
    {
        Serial.println("[WAV] fmt chunk not found");
        return false;
    }

    if (!foundData)
    {
        Serial.println("[WAV] data chunk not found");
        return false;
    }

    // PCM
    if (wav.audioFormat != 1)
    {
        Serial.printf(
            "[WAV] Unsupported format: %u\n",
            wav.audioFormat
        );

        return false;
    }

    // Stereo
    if (wav.channels != 2)
    {
        Serial.printf(
            "[WAV] Need stereo, got %u channels\n",
            wav.channels
        );

        return false;
    }

    // 16 bit
    if (wav.bitsPerSample != 16)
    {
        Serial.printf(
            "[WAV] Need 16-bit, got %u-bit\n",
            wav.bitsPerSample
        );

        return false;
    }

    if (wav.sampleRate == 0)
    {
        Serial.println("[WAV] Invalid sample rate");
        return false;
    }

    wav.valid = true;

    return true;
}


// ============================================================
// I2S START
// ============================================================

bool startI2S(
    uint32_t sampleRate
)
{
    Serial.println();
    Serial.println("[I2S] Starting...");

    // --------------------------------------------------------
    // CONFIG
    // --------------------------------------------------------

    i2s_config_t config =
    {
        .mode =
            static_cast<i2s_mode_t>(
                I2S_MODE_MASTER |
                I2S_MODE_TX
            ),

        .sample_rate = sampleRate,

        .bits_per_sample =
            I2S_BITS_PER_SAMPLE_16BIT,

        .channel_format =
            I2S_CHANNEL_FMT_RIGHT_LEFT,

        .communication_format =
            I2S_COMM_FORMAT_STAND_I2S,

        .intr_alloc_flags = 0,

        .dma_buf_count = 8,

        .dma_buf_len = 512,

        .use_apll = false,

        .tx_desc_auto_clear = true,

        .fixed_mclk = 0
    };

    // --------------------------------------------------------
    // PINS
    // --------------------------------------------------------

    i2s_pin_config_t pins =
    {
        .bck_io_num = PIN_I2S_BCLK,

        .ws_io_num = PIN_I2S_LRCLK,

        .data_out_num = PIN_I2S_DIN,

        .data_in_num = I2S_PIN_NO_CHANGE
    };

    // --------------------------------------------------------
    // INSTALL
    // --------------------------------------------------------

    esp_err_t result =
        i2s_driver_install(
            I2S_PORT,
            &config,
            0,
            nullptr
        );

    if (result != ESP_OK)
    {
        Serial.printf(
            "[I2S] Driver install failed: %s\n",
            esp_err_to_name(result)
        );

        return false;
    }

    // --------------------------------------------------------
    // SET PINS
    // --------------------------------------------------------

    result =
        i2s_set_pin(
            I2S_PORT,
            &pins
        );

    if (result != ESP_OK)
    {
        Serial.printf(
            "[I2S] Pin configuration failed: %s\n",
            esp_err_to_name(result)
        );

        i2s_driver_uninstall(I2S_PORT);

        return false;
    }

    // --------------------------------------------------------
    // CLEAR BUFFER
    // --------------------------------------------------------

    i2s_zero_dma_buffer(I2S_PORT);

    Serial.println("[I2S] Started");

    Serial.printf(
        "[I2S] Sample rate: %lu Hz\n",
        static_cast<unsigned long>(sampleRate)
    );

    Serial.println("[I2S] Bits: 16");
    Serial.println("[I2S] Channels: stereo");

    Serial.printf(
        "[I2S] BCLK: GPIO%d\n",
        PIN_I2S_BCLK
    );

    Serial.printf(
        "[I2S] LRCLK: GPIO%d\n",
        PIN_I2S_LRCLK
    );

    Serial.printf(
        "[I2S] DIN: GPIO%d\n",
        PIN_I2S_DIN
    );

    return true;
}


// ============================================================
// I2S STOP
// ============================================================

void stopI2S()
{
    Serial.println("[I2S] Stopping...");

    i2s_driver_uninstall(I2S_PORT);

    Serial.println("[I2S] Stopped");
}


// ============================================================
// PLAY WAV
// ============================================================

bool playWav(
    const char* path
)
{
    // --------------------------------------------------------
    // SD CHECK
    // --------------------------------------------------------

    if (!sdManager.isReady())
    {
        Serial.println(
            "[AUDIO] SD card is not ready"
        );

        return false;
    }

    // --------------------------------------------------------
    // FILE SYSTEM
    // --------------------------------------------------------

    fs::FS& fs =
        sdManager.card().fs();

    // --------------------------------------------------------
    // CHECK FILE
    // --------------------------------------------------------

    if (!fs.exists(path))
    {
        Serial.printf(
            "[AUDIO] File not found: %s\n",
            path
        );

        return false;
    }

    // --------------------------------------------------------
    // OPEN
    // --------------------------------------------------------

    File file =
        fs.open(
            path,
            FILE_READ
        );

    if (!file)
    {
        Serial.printf(
            "[AUDIO] Failed to open: %s\n",
            path
        );

        return false;
    }

    Serial.println();
    Serial.println("==============================");
    Serial.println(" WAV AUDIO TEST");
    Serial.println("==============================");

    Serial.printf(
        "File: %s\n",
        path
    );

    Serial.printf(
        "Size: %lu bytes\n",
        static_cast<unsigned long>(
            file.size()
        )
    );

    // --------------------------------------------------------
    // PARSE WAV
    // --------------------------------------------------------

    WavInfo wav;

    if (!parseWav(file, wav))
    {
        Serial.println(
            "[AUDIO] Invalid WAV file"
        );

        file.close();

        return false;
    }

    // --------------------------------------------------------
    // PRINT INFO
    // --------------------------------------------------------

    Serial.println();
    Serial.println("[WAV]");
    
    Serial.printf(
        "Format: %u\n",
        wav.audioFormat
    );

    Serial.printf(
        "Channels: %u\n",
        wav.channels
    );

    Serial.printf(
        "Sample rate: %lu Hz\n",
        static_cast<unsigned long>(
            wav.sampleRate
        )
    );

    Serial.printf(
        "Bits: %u\n",
        wav.bitsPerSample
    );

    Serial.printf(
        "Data offset: %lu\n",
        static_cast<unsigned long>(
            wav.dataOffset
        )
    );

    Serial.printf(
        "Data size: %lu bytes\n",
        static_cast<unsigned long>(
            wav.dataSize
        )
    );

    // --------------------------------------------------------
    // START I2S
    // --------------------------------------------------------

    if (!startI2S(wav.sampleRate))
    {
        file.close();

        return false;
    }

    // --------------------------------------------------------
    // SEEK TO AUDIO DATA
    // --------------------------------------------------------

    if (!file.seek(wav.dataOffset))
    {
        Serial.println(
            "[AUDIO] Failed to seek to data"
        );

        stopI2S();
        file.close();

        return false;
    }

    // --------------------------------------------------------
    // AUDIO BUFFER
    // --------------------------------------------------------

    static uint8_t buffer[2048];

    uint32_t remaining =
        wav.dataSize;

    uint32_t totalPlayed = 0;

    // --------------------------------------------------------
    // STREAM
    // --------------------------------------------------------

    Serial.println();
    Serial.println("[AUDIO] Playing...");

    while (remaining > 0)
    {
        size_t toRead =
            remaining > sizeof(buffer)
                ? sizeof(buffer)
                : remaining;

        size_t bytesRead =
            file.read(
                buffer,
                toRead
            );

        if (bytesRead == 0)
        {
            Serial.println(
                "[AUDIO] Read error"
            );

            break;
        }

        size_t bytesWritten = 0;

        esp_err_t result =
            i2s_write(
                I2S_PORT,
                buffer,
                bytesRead,
                &bytesWritten,
                portMAX_DELAY
            );

        if (result != ESP_OK)
        {
            Serial.printf(
                "[AUDIO] I2S write error: %s\n",
                esp_err_to_name(result)
            );

            break;
        }

        remaining -= bytesRead;

        totalPlayed += bytesWritten;
    }

    // --------------------------------------------------------
    // STOP
    // --------------------------------------------------------

    // Дадим DMA закончить передачу
    delay(100);

    stopI2S();

    file.close();

    // --------------------------------------------------------
    // RESULT
    // --------------------------------------------------------

    Serial.println();

    Serial.printf(
        "[AUDIO] Played: %lu bytes\n",
        static_cast<unsigned long>(
            totalPlayed
        )
    );

    if (remaining == 0)
    {
        Serial.println(
            "[AUDIO] Playback finished"
        );

        Serial.println(
            "=============================="
        );

        return true;
    }

    Serial.println(
        "[AUDIO] Playback failed"
    );

    Serial.println(
        "=============================="
    );

    return false;
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println(" ESP32-S3 AUDIO TEST");
    Serial.println(" MAX98357A STEREO");
    Serial.println("================================");

    // --------------------------------------------------------
    // SPI
    // --------------------------------------------------------

    Serial.println();
    Serial.println("[SYSTEM] Starting SPI...");

    if (!spiManager.begin())
    {
        Serial.println(
            "[SYSTEM] SPI initialization failed"
        );

        return;
    }

    // --------------------------------------------------------
    // SD
    // --------------------------------------------------------

    Serial.println();
    Serial.println("[SYSTEM] Starting SD...");

    if (!sdManager.begin(PIN_SD_CS))
    {
        Serial.println(
            "[SYSTEM] SD initialization failed"
        );

        return;
    }

    // --------------------------------------------------------
    // PLAY
    // --------------------------------------------------------

    playWav(TEST_WAV);
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
}