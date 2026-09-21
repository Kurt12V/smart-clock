#include <Arduino.h>
#include <FS.h>

#include "Pins.h"

#include "./managers/SPIManager.h"
#include "./managers/I2SManager.h"
#include "./managers/SDManager.h"

// ============================================================
// CONFIG
// ============================================================

namespace
{
    constexpr uint32_t SERIAL_BAUD = 115200;

    constexpr uint32_t SPEAKER_SAMPLE_RATE = 16000;

    constexpr size_t BUFFER_SAMPLES = 512;

    constexpr const char* TEST_FILES[] =
    {
        "/test.wav",
        "/recordings/test.wav"
    };
}

// ============================================================
// MANAGERS
// ============================================================

SPIManager spiManager;
I2SManager i2sManager;
SDManager sdManager;

// ============================================================
// WAV INFO
// ============================================================

struct WavInfo
{
    uint16_t audioFormat = 0;
    uint16_t channels = 0;

    uint32_t sampleRate = 0;
    uint32_t byteRate = 0;

    uint16_t blockAlign = 0;
    uint16_t bitsPerSample = 0;

    uint32_t dataOffset = 0;
    uint32_t dataSize = 0;
};

WavInfo wavInfo;

// ============================================================
// HELPERS
// ============================================================

uint16_t readLE16(File& file)
{
    uint8_t b0 = file.read();
    uint8_t b1 = file.read();

    return
        static_cast<uint16_t>(b0) |
        (static_cast<uint16_t>(b1) << 8);
}

uint32_t readLE32(File& file)
{
    uint8_t b0 = file.read();
    uint8_t b1 = file.read();
    uint8_t b2 = file.read();
    uint8_t b3 = file.read();

    return
        static_cast<uint32_t>(b0) |
        (static_cast<uint32_t>(b1) << 8) |
        (static_cast<uint32_t>(b2) << 16) |
        (static_cast<uint32_t>(b3) << 24);
}

bool readFourCC(File& file, char* out)
{
    if (file.readBytes(out, 4) != 4)
        return false;

    out[4] = '\0';

    return true;
}

// ============================================================
// FIND WAV
// ============================================================

String findTestFile()
{
    for (const char* path : TEST_FILES)
    {
        Serial.print("[TEST] Checking: ");
        Serial.println(path);

        if (sdManager.card().fs().exists(path))
        {
            Serial.print("[TEST] FOUND: ");
            Serial.println(path);

            return String(path);
        }
    }

    return String();
}

// ============================================================
// PARSE WAV
// ============================================================

bool parseWav(
    File& file,
    WavInfo& info
)
{
    char riff[5];
    char wave[5];

    if (!readFourCC(file, riff))
        return false;

    if (strcmp(riff, "RIFF") != 0)
    {
        Serial.println("[WAV] ERROR: Not RIFF");

        return false;
    }

    // RIFF chunk size
    readLE32(file);

    if (!readFourCC(file, wave))
        return false;

    if (strcmp(wave, "WAVE") != 0)
    {
        Serial.println("[WAV] ERROR: Not WAVE");

        return false;
    }

    bool foundFmt = false;
    bool foundData = false;

    while (file.available())
    {
        char chunkId[5];

        if (!readFourCC(file, chunkId))
            break;

        uint32_t chunkSize = readLE32(file);

        Serial.print("[WAV] Chunk: ");
        Serial.print(chunkId);

        Serial.print(" size=");
        Serial.println(chunkSize);

        // ----------------------------------------------------
        // fmt
        // ----------------------------------------------------

        if (strcmp(chunkId, "fmt ") == 0)
        {
            if (chunkSize < 16)
            {
                Serial.println(
                    "[WAV] ERROR: invalid fmt chunk"
                );

                return false;
            }

            info.audioFormat = readLE16(file);
            info.channels = readLE16(file);
            info.sampleRate = readLE32(file);
            info.byteRate = readLE32(file);
            info.blockAlign = readLE16(file);
            info.bitsPerSample = readLE16(file);

            if (chunkSize > 16)
            {
                file.seek(
                    file.position() +
                    (chunkSize - 16)
                );
            }

            foundFmt = true;

            Serial.print("[WAV] Format: ");
            Serial.println(info.audioFormat);

            Serial.print("[WAV] Channels: ");
            Serial.println(info.channels);

            Serial.print("[WAV] Sample rate: ");
            Serial.println(info.sampleRate);

            Serial.print("[WAV] Bits: ");
            Serial.println(info.bitsPerSample);
        }

        // ----------------------------------------------------
        // data
        // ----------------------------------------------------

        else if (strcmp(chunkId, "data") == 0)
        {
            info.dataOffset = file.position();
            info.dataSize = chunkSize;

            foundData = true;

            Serial.print("[WAV] Data offset: ");
            Serial.println(info.dataOffset);

            Serial.print("[WAV] Data size: ");
            Serial.println(info.dataSize);

            break;
        }

        // ----------------------------------------------------
        // Other chunk
        // ----------------------------------------------------

        else
        {
            file.seek(
                file.position() +
                chunkSize
            );
        }

        // WAV chunks are aligned to 2 bytes.
        if (chunkSize & 1)
        {
            file.seek(
                file.position() + 1
            );
        }
    }

    if (!foundFmt)
    {
        Serial.println("[WAV] ERROR: fmt chunk missing");
        return false;
    }

    if (!foundData)
    {
        Serial.println("[WAV] ERROR: data chunk missing");
        return false;
    }

    if (info.audioFormat != 1)
    {
        Serial.println(
            "[WAV] ERROR: only PCM supported"
        );

        return false;
    }

    if (info.bitsPerSample != 16)
    {
        Serial.println(
            "[WAV] ERROR: only 16-bit supported"
        );

        return false;
    }

    if (info.channels != 1)
    {
        Serial.println(
            "[WAV] ERROR: test expects MONO WAV"
        );

        return false;
    }

    return true;
}

// ============================================================
// PLAY WAV
// ============================================================

bool playWav(const char* path)
{
    Serial.println();
    Serial.println("========================================");
    Serial.println("PLAY WAV");
    Serial.println("========================================");

    Serial.print("[PLAY] File: ");
    Serial.println(path);

    File file = sdManager.card().fs().open(
        path,
        FILE_READ
    );

    if (!file)
    {
        Serial.println(
            "[PLAY] ERROR: cannot open file"
        );

        return false;
    }

    Serial.print("[PLAY] File size: ");
    Serial.println(file.size());

    // --------------------------------------------------------
    // Parse WAV
    // --------------------------------------------------------

    WavInfo info;

    if (!parseWav(file, info))
    {
        Serial.println(
            "[PLAY] ERROR: invalid WAV"
        );

        file.close();

        return false;
    }

    // --------------------------------------------------------
    // Sample rate
    // --------------------------------------------------------

    Serial.println();
    Serial.println("[PLAY] WAV information:");

    Serial.print("  Format: ");
    Serial.println(info.audioFormat);

    Serial.print("  Channels: ");
    Serial.println(info.channels);

    Serial.print("  Sample rate: ");
    Serial.println(info.sampleRate);

    Serial.print("  Bits: ");
    Serial.println(info.bitsPerSample);

    Serial.print("  Data size: ");
    Serial.println(info.dataSize);

    uint32_t durationMs =
        static_cast<uint32_t>(
            (
                static_cast<uint64_t>(info.dataSize) *
                1000ULL
            ) /
            (
                static_cast<uint64_t>(info.sampleRate) *
                2ULL
            )
        );

    Serial.print("  Duration: ");
    Serial.print(durationMs);
    Serial.println(" ms");

    // --------------------------------------------------------
    // Important:
    // Reconfigure speaker to WAV sample rate.
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[PLAY] Reconfiguring speaker..."
    );

    i2sManager.endSpeaker();

    if (!i2sManager.beginSpeaker(info.sampleRate))
    {
        Serial.println(
            "[PLAY] ERROR: beginSpeaker() failed"
        );

        file.close();

        return false;
    }

    Serial.print(
        "[PLAY] Speaker sample rate: "
    );

    Serial.println(info.sampleRate);

    // --------------------------------------------------------
    // Clear DMA
    // --------------------------------------------------------

    i2sManager.clearSpeaker();

    // --------------------------------------------------------
    // Start speaker
    // --------------------------------------------------------

    if (!i2sManager.startSpeaker())
    {
        Serial.println(
            "[PLAY] ERROR: startSpeaker() failed"
        );

        file.close();

        return false;
    }

    // --------------------------------------------------------
    // Seek to PCM data
    // --------------------------------------------------------

    if (!file.seek(info.dataOffset))
    {
        Serial.println(
            "[PLAY] ERROR: seek failed"
        );

        i2sManager.stopSpeaker();

        file.close();

        return false;
    }

    // --------------------------------------------------------
    // Playback buffer
    // --------------------------------------------------------

    int16_t buffer[BUFFER_SAMPLES];

    uint32_t remaining =
        info.dataSize;

    uint32_t totalSent = 0;

    uint32_t lastPrint = millis();

    Serial.println();
    Serial.println(
        "[PLAY] Playback started"
    );

    // --------------------------------------------------------
    // PLAY LOOP
    // --------------------------------------------------------

    while (remaining > 0)
    {
        size_t bytesToRead =
            min(
                remaining,
                static_cast<uint32_t>(
                    sizeof(buffer)
                )
            );

        size_t bytesRead =
            file.read(
                reinterpret_cast<uint8_t*>(buffer),
                bytesToRead
            );

        if (bytesRead == 0)
        {
            Serial.println(
                "[PLAY] ERROR: SD read returned 0"
            );

            break;
        }

        size_t bytesWritten = 0;

        if (!i2sManager.writeSpeaker(
                reinterpret_cast<const uint8_t*>(buffer),
                bytesRead,
                bytesWritten,
                100))
        {
            Serial.println(
                "[PLAY] ERROR: I2S write failed"
            );

            break;
        }

        if (bytesWritten == 0)
        {
            Serial.println(
                "[PLAY] ERROR: I2S wrote 0 bytes"
            );

            break;
        }

        remaining -= bytesWritten;
        totalSent += bytesWritten;

        // ----------------------------------------------------
        // Progress
        // ----------------------------------------------------

        if (millis() - lastPrint >= 500)
        {
            lastPrint = millis();

            uint32_t playedMs =
                static_cast<uint32_t>(
                    (
                        static_cast<uint64_t>(
                            totalSent
                        ) *
                        1000ULL
                    ) /
                    (
                        static_cast<uint64_t>(
                            info.sampleRate
                        ) *
                        2ULL
                    )
                );

            Serial.print("[PLAY] ");
            Serial.print(playedMs);
            Serial.print(" / ");
            Serial.print(durationMs);
            Serial.println(" ms");
        }

        yield();
    }

    // --------------------------------------------------------
    // Wait for DMA to drain
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[PLAY] Waiting for DMA..."
    );

    delay(300);

    i2sManager.stopSpeaker();

    file.close();

    Serial.println();
    Serial.println(
        "[PLAY] Playback finished"
    );

    Serial.print(
        "[PLAY] Bytes sent: "
    );

    Serial.println(totalSent);

    Serial.println(
        "========================================"
    );

    return totalSent == info.dataSize;
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(SERIAL_BAUD);

    delay(1000);

    Serial.println();
    Serial.println();
    Serial.println(
        "############################################"
    );
    Serial.println(
        "# ESP32-S3 DIRECT AUDIO TEST"
    );
    Serial.println(
        "# SD -> I2S_NUM_1 -> MAX98357A"
    );
    Serial.println(
        "############################################"
    );

    // --------------------------------------------------------
    // SPI
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[TEST] Initializing SPI..."
    );

    if (!spiManager.begin())
    {
        Serial.println(
            "[TEST] ERROR: SPI initialization failed"
        );

        return;
    }

    Serial.println("[TEST] SPI OK");

    // --------------------------------------------------------
    // I2S
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[TEST] Initializing I2SManager..."
    );

    if (!i2sManager.begin())
    {
        Serial.println(
            "[TEST] ERROR: I2SManager initialization failed"
        );

        return;
    }

    Serial.println(
        "[TEST] I2SManager OK"
    );

    Serial.print(
        "[TEST] Speaker port: I2S_NUM_"
    );

    Serial.println(
        i2sManager.speakerPort()
    );

    // --------------------------------------------------------
    // SD
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "[TEST] Initializing SD..."
    );

    if (!sdManager.begin(PIN_SD_CS))
    {
        Serial.println(
            "[TEST] ERROR: SD initialization failed"
        );

        return;
    }

    Serial.println("[TEST] SD OK");

    // --------------------------------------------------------
    // Find file
    // --------------------------------------------------------

    String path = findTestFile();

    if (path.length() == 0)
    {
        Serial.println();
        Serial.println(
            "[TEST] ERROR: WAV file not found"
        );

        Serial.println(
            "[TEST] Put test.wav into:"
        );

        Serial.println(
            "        /test.wav"
        );

        Serial.println(
            "or"
        );

        Serial.println(
            "        /recordings/test.wav"
        );

        return;
    }

    // --------------------------------------------------------
    // PLAY
    // --------------------------------------------------------

    delay(1000);

    if (!playWav(path.c_str()))
    {
        Serial.println();
        Serial.println(
            "############################################"
        );
        Serial.println(
            "# AUDIO TEST FAILED"
        );
        Serial.println(
            "############################################"
        );

        return;
    }

    Serial.println();
    Serial.println(
        "############################################"
    );
    Serial.println(
        "# AUDIO TEST PASSED"
    );
    Serial.println(
        "############################################"
    );
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // Ничего не делаем.
    // WAV проигрывается один раз в setup().
    delay(1000);
}