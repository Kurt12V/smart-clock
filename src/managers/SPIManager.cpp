#include "SPIManager.h"
#include "Pins.h"
#include "./utils/Logger.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

SPIManager::SPIManager()
    : _initialized(false)
{
}

// ============================================================
// BEGIN
// ============================================================

bool SPIManager::begin()
{
    if (_initialized)
        return true;

    Logger::info(
        "SPI",
        "Initializing SPI bus..."
    );

    // --------------------------------------------------------
    // Все CS должны быть HIGH до запуска устройств
    // --------------------------------------------------------

    pinMode(PIN_TFT_CS1, OUTPUT);
    pinMode(PIN_TFT_CS2, OUTPUT);
    pinMode(PIN_TFT_CS3, OUTPUT);
    pinMode(PIN_TFT_CS4, OUTPUT);

    pinMode(PIN_SD_CS, OUTPUT);

    digitalWrite(PIN_TFT_CS1, HIGH);
    digitalWrite(PIN_TFT_CS2, HIGH);
    digitalWrite(PIN_TFT_CS3, HIGH);
    digitalWrite(PIN_TFT_CS4, HIGH);

    digitalWrite(PIN_SD_CS, HIGH);

    // --------------------------------------------------------
    // Запуск общей SPI-шины
    // --------------------------------------------------------

    SPI.begin(
        PIN_SCLK,
        PIN_SD_MISO,
        PIN_MOSI
    );

    _initialized = true;

    Logger::info(
        "SPI",
        "SPI bus initialized"
    );

    return true;
}

// ============================================================
// END
// ============================================================

void SPIManager::end()
{
    if (!_initialized)
        return;

    SPI.end();

    _initialized = false;

    Logger::info(
        "SPI",
        "SPI bus stopped"
    );
}

// ============================================================
// IS READY
// ============================================================

bool SPIManager::isReady() const
{
    return _initialized;
}

// ============================================================
// BUS
// ============================================================

SPIClass& SPIManager::bus()
{
    return SPI;
}

// ============================================================
// SELECT
// ============================================================

void SPIManager::select(uint8_t csPin)
{
    digitalWrite(csPin, LOW);
}

// ============================================================
// DESELECT
// ============================================================

void SPIManager::deselect(uint8_t csPin)
{
    digitalWrite(csPin, HIGH);
}