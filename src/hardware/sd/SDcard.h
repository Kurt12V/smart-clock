#pragma once

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

class SDCard
{
public:
    SDCard();

    bool begin(uint8_t csPin);

    bool isMounted() const;

    void end();

    uint64_t getTotalBytes() const;
    uint64_t getUsedBytes() const;
    uint64_t getFreeBytes() const;

    float getUsedPercent() const;
    float getFreePercent() const;

    String getTotalSizeString() const;
    String getUsedSizeString() const;
    String getFreeSizeString() const;

    bool exists(const char* path) const;

    fs::FS& fs();

private:
    bool _mounted;
};