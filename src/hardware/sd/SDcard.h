#pragma once

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

// ============================================================
// SD CARD INFO
// ============================================================

struct SDCardInfo
{
    uint64_t totalBytes = 0;
    uint64_t usedBytes  = 0;
    uint64_t freeBytes  = 0;

    float usedPercent = 0.0f;
    float freePercent = 0.0f;

    String totalSize;
    String usedSize;
    String freeSize;
};

// ============================================================
// SD CARD
// ============================================================

class SDCard
{
public:

    SDCard();

    bool begin(uint8_t csPin);

    bool isMounted() const;

    void end();

    SDCardInfo getInfo() const;

    bool exists(const char* path) const;

    fs::FS& fs();

private:

    bool _mounted;
    uint8_t _csPin;

    String formatBytes(uint64_t bytes) const;
};