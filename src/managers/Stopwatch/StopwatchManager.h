// #pragma once

// #include <Arduino.h>
// #include "../models/Stopwatch.h"

// // ============================================================
// // STOPWATCH MANAGER
// // ============================================================

// class StopwatchManager
// {
// public:

//     StopwatchManager();

//     bool begin();
//     void update();

//     // ========================================================
//     // STOPWATCH CONTROL
//     // ========================================================

//     bool start(const String& id = "default");
//     bool pause(const String& id = "default");
//     bool stop(const String& id = "default");
//     bool reset(const String& id = "default");

//     bool addLap(const String& id = "default");

//     // ========================================================
//     // GETTERS
//     // ========================================================

//     const Stopwatch* get(const String& id = "default") const;
//     uint32_t getElapsedMs(const String& id = "default") const;
//     String getFormattedTime(const String& id = "default") const;

//     bool isRunning(const String& id = "default") const;
//     bool isPaused(const String& id = "default") const;

//     // ========================================================
//     // MULTIPLE STOPWATCHES
//     // ========================================================

//     bool create(const String& id, const String& name);
//     bool remove(const String& id);
//     void removeAll();

//     uint8_t count() const;
//     const Stopwatch* getAt(uint8_t index) const;

// private:

//     // ========================================================
//     // HELPERS
//     // ========================================================

//     int8_t findIndex(const String& id) const;
//     int8_t findFreeSlot() const;

//     String formatTime(uint32_t ms) const;

//     // ========================================================
//     // DATA
//     // ========================================================

//     static constexpr uint8_t MAX_STOPWATCHES = 8;

//     Stopwatch _stopwatches[MAX_STOPWATCHES];
//     bool _used[MAX_STOPWATCHES];
//     uint8_t _count;

//     bool _initialized;
// };
