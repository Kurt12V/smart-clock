#include "SDManager.h"
#include "./utils/Logger.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

SDManager::SDManager()
    : _initialized(false)
{
}

// ============================================================
// BEGIN
// ============================================================

bool SDManager::begin(uint8_t csPin)
{
    if (_initialized)
        return true;

    Logger::info(
        "SD",
        "Initializing SD card..."
    );

    // --------------------------------------------------------
    // Проверяем SD
    // --------------------------------------------------------

    if (!_card.begin(csPin))
    {
        Logger::error(
            "SD",
            "Failed to initialize SD card"
        );

        _initialized = false;

        return false;
    }

    _initialized = true;

    // --------------------------------------------------------
    // Информация о карте
    // --------------------------------------------------------

    SDCardInfo info = _card.getInfo();

    Logger::info(
        "SD",
        "SD card initialized"
    );

    Logger::info(
        "SD",
        "Total: %s",
        info.totalSize.c_str()
    );

    Logger::info(
        "SD",
        "Used: %s (%.1f%%)",
        info.usedSize.c_str(),
        info.usedPercent
    );

    Logger::info(
        "SD",
        "Free: %s (%.1f%%)",
        info.freeSize.c_str(),
        info.freePercent
    );

    return true;
}

// ============================================================
// END
// ============================================================

void SDManager::end()
{
    if (!_initialized)
        return;

    _card.end();

    _initialized = false;

    Logger::info(
        "SD",
        "SD card unmounted"
    );
}

// ============================================================
// IS READY
// ============================================================

bool SDManager::isReady() const
{
    return _initialized &&
           _card.isMounted();
}

// ============================================================
// CARD
// ============================================================

SDCard& SDManager::card()
{
    return _card;
}

const SDCard& SDManager::card() const
{
    return _card;
}

// ============================================================
// GET INFO
// ============================================================

SDCardInfo SDManager::getInfo() const
{
    return _card.getInfo();
}