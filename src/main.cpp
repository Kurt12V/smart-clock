#include <Arduino.h>
#include <FS.h>

// ============================================================
// CORE
// ============================================================

#include "Config.h"
#include "Pins.h"
#include "Constants.h"
#include "Version.h"

// ============================================================
// SYSTEMS
// ============================================================

#include "Settings.h"

#include "./core/ClockSystem.h"
#include "./managers/SensorsManager.h"
#include "./core/DisplaySystem.h"
#include "./managers/InputManager.h"

// ============================================================
// AUDIO / STORAGE
// ============================================================

#include "./managers/SPIManager.h"
#include "./managers/I2SManager.h"
#include "./managers/SDManager.h"

// ============================================================
// CONFIG
// ============================================================

namespace
{
    constexpr uint32_t SERIAL_BAUD = 115200;

    constexpr size_t BUFFER_SAMPLES = 512;

    // Файлы, которые пробуем проиграть при старте (по порядку).
    constexpr const char* STARTUP_SOUND_FILES[] =
    {
        "/audio/system/START_SYSTEM.wav",
        // "/test.wav",
        // "/recordings/test.wav"
    };
}

// ============================================================
// MANAGERS
// ============================================================

InputManager inputManager;

Settings::Clock clockSettings;

ClockSystem clockSystem(
    clockSettings
);

SensorManager sensorManager;

DisplaySystem displaySystem(
    clockSystem,
    sensorManager
);

SPIManager spiManager;
I2SManager i2sManager;
SDManager  sdManager;

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

// ============================================================
// WAV HELPERS
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

String findStartupSound()
{
    for (const char* path : STARTUP_SOUND_FILES)
    {
        Serial0.print("[BOOT-AUDIO] Checking: ");
        Serial0.println(path);

        if (sdManager.card().fs().exists(path))
        {
            Serial0.print("[BOOT-AUDIO] FOUND: ");
            Serial0.println(path);

            return String(path);
        }
    }

    return String();
}

// ============================================================
// PARSE WAV
// ============================================================

bool parseWav(File& file, WavInfo& info)
{
    char riff[5];
    char wave[5];

    if (!readFourCC(file, riff))
        return false;

    if (strcmp(riff, "RIFF") != 0)
    {
        Serial0.println("[WAV] ERROR: Not RIFF");
        return false;
    }

    readLE32(file); // RIFF chunk size

    if (!readFourCC(file, wave))
        return false;

    if (strcmp(wave, "WAVE") != 0)
    {
        Serial0.println("[WAV] ERROR: Not WAVE");
        return false;
    }

    bool foundFmt  = false;
    bool foundData = false;

    while (file.available())
    {
        char chunkId[5];

        if (!readFourCC(file, chunkId))
            break;

        uint32_t chunkSize = readLE32(file);

        // ----------------------------------------------------
        // fmt
        // ----------------------------------------------------

        if (strcmp(chunkId, "fmt ") == 0)
        {
            if (chunkSize < 16)
            {
                Serial0.println("[WAV] ERROR: invalid fmt chunk");
                return false;
            }

            info.audioFormat   = readLE16(file);
            info.channels      = readLE16(file);
            info.sampleRate    = readLE32(file);
            info.byteRate      = readLE32(file);
            info.blockAlign    = readLE16(file);
            info.bitsPerSample = readLE16(file);

            if (chunkSize > 16)
            {
                file.seek(file.position() + (chunkSize - 16));
            }

            foundFmt = true;
        }

        // ----------------------------------------------------
        // data
        // ----------------------------------------------------

        else if (strcmp(chunkId, "data") == 0)
        {
            info.dataOffset = file.position();
            info.dataSize   = chunkSize;

            foundData = true;
            break;
        }

        // ----------------------------------------------------
        // Other chunk
        // ----------------------------------------------------

        else
        {
            file.seek(file.position() + chunkSize);
        }

        if (chunkSize & 1)
        {
            file.seek(file.position() + 1);
        }
    }

    if (!foundFmt || !foundData)
    {
        Serial0.println("[WAV] ERROR: missing fmt/data chunk");
        return false;
    }

    if (info.audioFormat != 1)
    {
        Serial0.println("[WAV] ERROR: only PCM supported");
        return false;
    }

    if (info.bitsPerSample != 16)
    {
        Serial0.println("[WAV] ERROR: only 16-bit supported");
        return false;
    }

    return true;
}

// ============================================================
// PLAY WAV
// ============================================================

bool playWav(const char* path)
{
    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("PLAY WAV");
    Serial0.println("========================================");

    Serial0.print("[PLAY] File: ");
    Serial0.println(path);

    File file = sdManager.card().fs().open(path, FILE_READ);

    if (!file)
    {
        Serial0.println("[PLAY] ERROR: cannot open file");
        return false;
    }

    WavInfo info;

    if (!parseWav(file, info))
    {
        Serial0.println("[PLAY] ERROR: invalid WAV");
        file.close();
        return false;
    }

    Serial0.print("[PLAY] Sample rate: ");
    Serial0.println(info.sampleRate);

    Serial0.print("[PLAY] Data size: ");
    Serial0.println(info.dataSize);

    uint32_t durationMs =
        static_cast<uint32_t>(
            (static_cast<uint64_t>(info.dataSize) * 1000ULL) /
            (static_cast<uint64_t>(info.sampleRate) * 2ULL)
        );

    Serial0.print("[PLAY] Duration: ");
    Serial0.print(durationMs);
    Serial0.println(" ms");

    // --------------------------------------------------------
    // Переконфигурируем динамик под sample rate WAV
    // --------------------------------------------------------

    i2sManager.endSpeaker();

    if (!i2sManager.beginSpeaker(info.sampleRate))
    {
        Serial0.println("[PLAY] ERROR: beginSpeaker() failed");
        file.close();
        return false;
    }

    i2sManager.clearSpeaker();

    if (!i2sManager.startSpeaker())
    {
        Serial0.println("[PLAY] ERROR: startSpeaker() failed");
        file.close();
        return false;
    }

    if (!file.seek(info.dataOffset))
    {
        Serial0.println("[PLAY] ERROR: seek failed");
        i2sManager.stopSpeaker();
        file.close();
        return false;
    }

    int16_t  buffer[BUFFER_SAMPLES];
    uint32_t remaining = info.dataSize;
    uint32_t totalSent = 0;

    while (remaining > 0)
    {
        size_t bytesToRead = min(
            remaining,
            static_cast<uint32_t>(sizeof(buffer))
        );

        size_t bytesRead = file.read(
            reinterpret_cast<uint8_t*>(buffer),
            bytesToRead
        );

        if (bytesRead == 0)
        {
            Serial0.println("[PLAY] ERROR: SD read returned 0");
            break;
        }

        size_t bytesWritten = 0;

        if (!i2sManager.writeSpeaker(
                reinterpret_cast<const uint8_t*>(buffer),
                bytesRead,
                bytesWritten,
                100))
        {
            Serial0.println("[PLAY] ERROR: I2S write failed");
            break;
        }

        if (bytesWritten == 0)
        {
            Serial0.println("[PLAY] ERROR: I2S wrote 0 bytes");
            break;
        }

        remaining -= bytesWritten;
        totalSent += bytesWritten;

        yield();
    }

    delay(300); // дать DMA доиграть

    i2sManager.stopSpeaker();
    file.close();

    Serial0.print("[PLAY] Bytes sent: ");
    Serial0.println(totalSent);

    Serial0.println("========================================");

    return totalSent == info.dataSize;
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    // ========================================================
    // SERIAL
    // ========================================================

    Serial0.begin(SERIAL_BAUD);
    delay(1000);

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("        ESP32-S3 SMART CLOCK");
    Serial0.println("========================================");

    // ========================================================
    // SPI  (нужен для SD-карты)
    // ========================================================

    Serial0.println("[MAIN] Initializing SPI...");

    if (!spiManager.begin())
    {
        Serial0.println("[MAIN] SPI ERROR");
    }
    else
    {
        Serial0.println("[MAIN] SPI OK");
    }

    // ========================================================
    // I2S  (нужен для динамика)
    // ========================================================

    Serial0.println("[MAIN] Initializing I2SManager...");

    if (!i2sManager.begin())
    {
        Serial0.println("[MAIN] I2S ERROR");
    }
    else
    {
        Serial0.println("[MAIN] I2S OK");
    }

    // ========================================================
    // SD
    // ========================================================

    Serial0.println("[MAIN] Initializing SD...");

    if (!sdManager.begin(PIN_SD_CS))
    {
        Serial0.println("[MAIN] SD ERROR");
    }
    else
    {
        Serial0.println("[MAIN] SD OK");
    }

    // ========================================================
    // CLOCK
    // ========================================================

    Serial0.println("[MAIN] Initializing ClockSystem...");

    if (!clockSystem.begin())
    {
        Serial0.println("[MAIN] ClockSystem ERROR");
    }
    else
    {
        Serial0.println("[MAIN] ClockSystem OK");
    }

    // ========================================================
    // SENSORS
    // ========================================================

    Serial0.println("[MAIN] Initializing SensorManager...");

    if (!sensorManager.begin())
    {
        Serial0.println("[MAIN] SensorManager ERROR");
    }
    else
    {
        Serial0.println("[MAIN] SensorManager OK");
    }

    // ========================================================
    // DISPLAY
    // ========================================================

    Serial0.println("[MAIN] Initializing DisplaySystem...");

    if (!displaySystem.begin())
    {
        Serial0.println("[MAIN] DisplaySystem ERROR");

        while (true)
        {
            delay(1000);
            Serial0.println("[MAIN] DisplaySystem is not available");
        }
    }

    Serial0.println("[MAIN] DisplaySystem OK");

    // ========================================================
    // INPUT
    // ========================================================

    Serial0.println("[MAIN] Initializing InputManager...");

    if (!inputManager.begin())
    {
        Serial0.println("[MAIN] InputManager ERROR");
    }
    else
    {
        Serial0.println("[MAIN] InputManager OK");
    }

    // ========================================================
    // BOOT AUDIO  (музыка при старте с SD-карты)
    // ========================================================

    String startupSound = findStartupSound();

    if (startupSound.length() > 0)
    {
        Serial0.println();
        Serial0.println("[MAIN] Playing startup sound...");
        playWav(startupSound.c_str());
    }
    else
    {
        Serial0.println();
        Serial0.println("[MAIN] No startup WAV on SD card");
    }

    // ========================================================
    // SYSTEM READY
    // ========================================================

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("          SYSTEM READY");
    Serial0.println("========================================");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // ========================================================
    // CLOCK
    // ========================================================

    clockSystem.update();

    // ========================================================
    // SENSORS
    // ========================================================

    sensorManager.update();

    // ========================================================
    // DISPLAY
    // ========================================================

    displaySystem.update();

    // ========================================================
    // INPUT
    // ========================================================
    delay(1);
}
















// #include <Adafruit_NeoPixel.h>

// #define LED_PIN     14
// #define MATRIX_W    16
// #define MATRIX_H    16
// #define NUM_LEDS    (MATRIX_W * MATRIX_H)
// #define BRIGHTNESS  20

// #define COOLING     55
// #define SPARKING    120
// #define FPS         60

// // Направление огня:
// //   0 = снизу вверх   (по умолчанию)
// //   1 = слева направо (поворот на 90° по часовой)
// //   2 = справа налево (поворот на 90° против часовой)
// //   3 = сверху вниз   (переворот на 180°)
// #define FIRE_DIR    2

// Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// uint16_t XY(uint8_t x, uint8_t y) {
//   if (y & 1) return y * MATRIX_W + (MATRIX_W - 1 - x);
//   return y * MATRIX_W + x;
// }

// uint8_t scale8_video(uint8_t i, uint8_t scale) {
//   uint8_t j = ((uint16_t)i * (uint16_t)scale) >> 8;
//   return j + (i && scale && !j);
// }

// uint32_t fireColor(uint8_t temperature) {
//   uint8_t t192 = scale8_video(temperature, 191);
//   uint8_t heatramp = t192 & 0x3F;
//   heatramp <<= 2;
//   uint8_t r, g, b;
//   if (t192 & 0x80)      { r = 255; g = 255; b = heatramp; }
//   else if (t192 & 0x40) { r = 255; g = heatramp; b = 0; }
//   else                  { r = heatramp; g = 0; b = 0; }
//   return strip.Color(r, g, b);
// }

// // heat[a][b]: b=0 — источник огня, b растёт в сторону распространения
// static uint8_t heat[MATRIX_W][MATRIX_H];

// void setup() {
//   strip.begin();
//   strip.setBrightness(BRIGHTNESS);
//   strip.clear();
//   strip.show();
//   memset(heat, 0, sizeof(heat));
// }

// void loop() {
//   // 1) Остывание
//   for (uint8_t a = 0; a < MATRIX_W; a++) {
//     for (uint8_t b = 0; b < MATRIX_H; b++) {
//       uint8_t cool = random(0, ((COOLING * 10) / MATRIX_H) + 2);
//       heat[a][b] = (heat[a][b] > cool) ? heat[a][b] - cool : 0;
//     }
//   }

//   // 2) Подъём тепла от b к b+1 с боковым дрожанием по a
//   for (uint8_t a = 0; a < MATRIX_W; a++) {
//     for (int8_t b = MATRIX_H - 1; b >= 2; b--) {
//       int8_t da = (int8_t)random(-1, 2);
//       int8_t na = (int8_t)a + da;
//       if (na < 0) na = 0;
//       if (na >= MATRIX_W) na = MATRIX_W - 1;

//       uint8_t below  = heat[a][b - 1];
//       uint8_t below2 = heat[a][b - 2];
//       heat[na][b] = (below + below2) / 2;
//     }
//     heat[a][1] = heat[a][0];
//   }

//   // 3) Вспышки в источнике (b=0)
//   for (uint8_t a = 0; a < MATRIX_W; a++) {
//     if (random(0, 255) < SPARKING) {
//       uint8_t spark = random(160, 255);
//       heat[a][0] = spark;
//       heat[a][1] = spark;
//     }
//   }

//   // 4) Вывод с учётом направления
//   for (uint8_t a = 0; a < MATRIX_W; a++) {
//     for (uint8_t b = 0; b < MATRIX_H; b++) {
//       uint8_t x, y;
//       switch (FIRE_DIR) {
//         case 0:  x = a;                  y = MATRIX_H - 1 - b; break; // снизу вверх
//         case 1:  x = b;                  y = a;               break; // слева направо (90° CW)
//         case 2:  x = MATRIX_W - 1 - b;   y = MATRIX_H - 1 - a; break; // справа налево (90° CCW)
//         case 3:  x = MATRIX_W - 1 - a;   y = b;               break; // сверху вниз (180°)
//         default: x = a;                  y = MATRIX_H - 1 - b; break;
//       }
//       strip.setPixelColor(XY(x, y), fireColor(heat[a][b]));
//     }
//   }
//   strip.show();
//   delay(1000 / FPS);
// }