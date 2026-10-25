#include "SPIManager.h"
#include "Pins.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

SPIManager::SPIManager()
    : _initialized(false)
{
    Serial0.println();
    Serial0.println("[SPI][CONSTRUCTOR] SPIManager created");
    Serial0.printf("[SPI][CONSTRUCTOR] Initialized: %s\n",
                   _initialized ? "true" : "false");
}

// ============================================================
// BEGIN
// ============================================================

bool SPIManager::begin()
{
    Serial0.println();
    Serial0.println("================================================");
    Serial0.println("[SPI][BEGIN] Initializing SPI bus");
    Serial0.println("================================================");

    Serial0.printf("[SPI][BEGIN] Previous state: %s\n",
                   _initialized ? "initialized" : "not initialized");

    if (_initialized)
    {
        Serial0.println("[SPI][BEGIN] SPI already initialized");
        Serial0.println("[SPI][BEGIN] Skipping initialization");
        return true;
    }

    // --------------------------------------------------------
    // PIN CONFIGURATION
    // --------------------------------------------------------

    Serial0.println("[SPI][BEGIN] Configuring SPI pins");

    Serial0.printf("[SPI][PINS] SCLK: GPIO %d\n", PIN_SCLK);
    Serial0.printf("[SPI][PINS] MOSI: GPIO %d\n", PIN_MOSI);
    Serial0.printf("[SPI][PINS] MISO: GPIO %d\n", PIN_SD_MISO);

    Serial0.printf("[SPI][PINS] TFT CS1: GPIO %d\n", PIN_TFT_CS1);
    Serial0.printf("[SPI][PINS] TFT CS2: GPIO %d\n", PIN_TFT_CS2);
    Serial0.printf("[SPI][PINS] TFT CS3: GPIO %d\n", PIN_TFT_CS3);
    Serial0.printf("[SPI][PINS] TFT CS4: GPIO %d\n", PIN_TFT_CS4);
    Serial0.printf("[SPI][PINS] SD CS: GPIO %d\n", PIN_SD_CS);

    // --------------------------------------------------------
    // ALL CS PINS HIGH BEFORE STARTING DEVICES
    // --------------------------------------------------------

    Serial0.println("[SPI][BEGIN] Setting all CS pins to OUTPUT");

    pinMode(PIN_TFT_CS1, OUTPUT);
    Serial0.printf("[SPI][PIN] GPIO %d configured as OUTPUT\n",
                   PIN_TFT_CS1);

    pinMode(PIN_TFT_CS2, OUTPUT);
    Serial0.printf("[SPI][PIN] GPIO %d configured as OUTPUT\n",
                   PIN_TFT_CS2);

    pinMode(PIN_TFT_CS3, OUTPUT);
    Serial0.printf("[SPI][PIN] GPIO %d configured as OUTPUT\n",
                   PIN_TFT_CS3);

    pinMode(PIN_TFT_CS4, OUTPUT);
    Serial0.printf("[SPI][PIN] GPIO %d configured as OUTPUT\n",
                   PIN_TFT_CS4);

    pinMode(PIN_SD_CS, OUTPUT);
    Serial0.printf("[SPI][PIN] GPIO %d configured as OUTPUT\n",
                   PIN_SD_CS);

    Serial0.println("[SPI][BEGIN] Setting all CS pins HIGH");

    digitalWrite(PIN_TFT_CS1, HIGH);
    digitalWrite(PIN_TFT_CS2, HIGH);
    digitalWrite(PIN_TFT_CS3, HIGH);
    digitalWrite(PIN_TFT_CS4, HIGH);
    digitalWrite(PIN_SD_CS, HIGH);

    Serial0.printf("[SPI][CS] TFT CS1 GPIO %d level: %d\n",
                   PIN_TFT_CS1, digitalRead(PIN_TFT_CS1));

    Serial0.printf("[SPI][CS] TFT CS2 GPIO %d level: %d\n",
                   PIN_TFT_CS2, digitalRead(PIN_TFT_CS2));

    Serial0.printf("[SPI][CS] TFT CS3 GPIO %d level: %d\n",
                   PIN_TFT_CS3, digitalRead(PIN_TFT_CS3));

    Serial0.printf("[SPI][CS] TFT CS4 GPIO %d level: %d\n",
                   PIN_TFT_CS4, digitalRead(PIN_TFT_CS4));

    Serial0.printf("[SPI][CS] SD CS GPIO %d level: %d\n",
                   PIN_SD_CS, digitalRead(PIN_SD_CS));

    // --------------------------------------------------------
    // INITIALIZE SHARED SPI BUS
    // --------------------------------------------------------

    Serial0.println("[SPI][BEGIN] Starting shared SPI bus");

    Serial0.printf("[SPI][BEGIN] SPI.begin(SCLK=%d, MISO=%d, MOSI=%d)\n",
                   PIN_SCLK,
                   PIN_SD_MISO,
                   PIN_MOSI);

    const unsigned long startTime = millis();

    SPI.begin(
        PIN_SCLK,
        PIN_SD_MISO,
        PIN_MOSI
    );

    const unsigned long elapsedTime = millis() - startTime;

    Serial0.printf("[SPI][BEGIN] SPI.begin() returned after %lu ms\n",
                   elapsedTime);

    // --------------------------------------------------------
    // UPDATE STATE
    // --------------------------------------------------------

    _initialized = true;

    Serial0.println("[SPI][BEGIN] SPI bus initialization completed");

    Serial0.printf("[SPI][BEGIN] Initialized: %s\n",
                   _initialized ? "true" : "false");

    Serial0.printf("[SPI][BEGIN] SCLK pin: GPIO %d\n", PIN_SCLK);
    Serial0.printf("[SPI][BEGIN] MOSI pin: GPIO %d\n", PIN_MOSI);
    Serial0.printf("[SPI][BEGIN] MISO pin: GPIO %d\n", PIN_SD_MISO);

    Serial0.println("================================================");

    return true;
}

// ============================================================
// END
// ============================================================

void SPIManager::end()
{
    Serial0.println();
    Serial0.println("[SPI][END] Stopping SPI bus");

    Serial0.printf("[SPI][END] Initialized: %s\n",
                   _initialized ? "true" : "false");

    if (!_initialized)
    {
        Serial0.println("[SPI][END] SPI is not initialized");
        Serial0.println("[SPI][END] Nothing to stop");
        return;
    }

    Serial0.println("[SPI][END] Calling SPI.end()");

    SPI.end();

    _initialized = false;

    Serial0.println("[SPI][END] SPI.end() completed");

    Serial0.printf("[SPI][END] Initialized: %s\n",
                   _initialized ? "true" : "false");
}

// ============================================================
// IS READY
// ============================================================

bool SPIManager::isReady() const
{
    Serial0.printf("[SPI][IS_READY] Result: %s\n",
                   _initialized ? "true" : "false");

    return _initialized;
}

// ============================================================
// BUS
// ============================================================

SPIClass& SPIManager::bus()
{
    Serial0.println("[SPI][BUS] Returning SPI bus reference");

    return SPI;
}

// ============================================================
// SELECT
// ============================================================

void SPIManager::select(uint8_t csPin)
{
    Serial0.printf("[SPI][SELECT] Selecting device on GPIO %u\n",
                   csPin);

    digitalWrite(csPin, LOW);

    Serial0.printf("[SPI][SELECT] GPIO %u level after select: %d\n",
                   csPin, digitalRead(csPin));
}

// ============================================================
// DESELECT
// ============================================================

void SPIManager::deselect(uint8_t csPin)
{
    Serial0.printf("[SPI][DESELECT] Deselecting device on GPIO %u\n",
                   csPin);

    digitalWrite(csPin, HIGH);

    Serial0.printf("[SPI][DESELECT] GPIO %u level after deselect: %d\n",
                   csPin, digitalRead(csPin));
}