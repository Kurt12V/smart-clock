// #pragma once

// #include <Arduino.h>

// #include "SDCard.h"

// class SDManager
// {
// public:

//     SDManager();

//     bool begin(uint8_t csPin);

//     void end();

//     bool isReady() const;

//     SDCard& card();

//     bool createProjectDirectories();

//     float getUsedPercent() const;

//     float getFreePercent() const;

//     uint64_t getTotalBytes() const;

//     uint64_t getUsedBytes() const;

//     uint64_t getFreeBytes() const;

// private:

//     SDCard _card;

//     bool _initialized;
// };