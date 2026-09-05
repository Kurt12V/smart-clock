// #include "SDManager.h"

// #include "FileUtils.h"
// #include "Logger.h"

// SDManager::SDManager()
//     : _initialized(false)
// {
// }

// bool SDManager::begin(uint8_t csPin)
// {
//     if (_initialized)
//         return true;

//     Logger::info("SD", "Initializing SD card...");

//     if (!_card.begin(csPin))
//     {
//         Logger::error(
//             "SD",
//             "Failed to initialize SD card"
//         );

//         _initialized = false;

//         return false;
//     }

//     _initialized = true;

//     Logger::info(
//         "SD",
//         "SD card initialized"
//     );

//     Logger::info(
//         "SD",
//         "Total: %s",
//         _card.getTotalSizeString().c_str()
//     );

//     Logger::info(
//         "SD",
//         "Used: %s (%.1f%%)",
//         _card.getUsedSizeString().c_str(),
//         _card.getUsedPercent()
//     );

//     Logger::info(
//         "SD",
//         "Free: %s (%.1f%%)",
//         _card.getFreeSizeString().c_str(),
//         _card.getFreePercent()
//     );

//     createProjectDirectories();

//     return true;
// }

// void SDManager::end()
// {
//     if (!_initialized)
//         return;

//     _card.end();

//     _initialized = false;

//     Logger::info(
//         "SD",
//         "SD card unmounted"
//     );
// }

// bool SDManager::isReady() const
// {
//     return _initialized &&
//            _card.isMounted();
// }

// SDCard& SDManager::card()
// {
//     return _card;
// }

// bool SDManager::createProjectDirectories()
// {
//     if (!isReady())
//         return false;

//     bool success = true;

//     const char* directories[] =
//     {
//         "/config",

//         "/timezone",

//         "/ai",
//         "/ai/llm",
//         "/ai/ml",
//         "/ai/tts",

//         "/audio",
//         "/audio/notifications",
//         "/audio/sounds",
//         "/audio/cache",

//         "/data",
//         "/data/users",
//         "/data/routines",
//         "/data/sleep",
//         "/data/activity",
//         "/data/sensors",

//         "/logs",
//         "/logs/system",
//         "/logs/sensors",
//         "/logs/ai"
//     };

//     constexpr size_t directoryCount =
//         sizeof(directories) /
//         sizeof(directories[0]);

//     for (size_t i = 0; i < directoryCount; ++i)
//     {
//         if (!FileUtils::createDir(
//                 _card.fs(),
//                 directories[i]
//             ))
//         {
//             Logger::warning(
//                 "SD",
//                 "Failed to create directory: %s",
//                 directories[i]
//             );

//             success = false;
//         }
//     }

//     return success;
// }

// float SDManager::getUsedPercent() const
// {
//     return _card.getUsedPercent();
// }

// float SDManager::getFreePercent() const
// {
//     return _card.getFreePercent();
// }

// uint64_t SDManager::getTotalBytes() const
// {
//     return _card.getTotalBytes();
// }

// uint64_t SDManager::getUsedBytes() const
// {
//     return _card.getUsedBytes();
// }

// uint64_t SDManager::getFreeBytes() const
// {
//     return _card.getFreeBytes();
// }