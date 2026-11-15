#pragma once

#include <Arduino.h>
#include <functional>
#include "./models/Timer.h"

class TimerManager
{
public:
    using StartCallback = std::function<void(const Timer&)>;
    using TickCallback = std::function<void(
        const Timer& timer,
        uint8_t percent,
        uint32_t remainingMs
    )>;
    using FinishCallback = std::function<void(const Timer&)>;
    using StopCallback = std::function<void(const String& id)>;

    static constexpr uint8_t MAX_TIMERS = 8;

    TimerManager();

    bool begin();
    void update();

    bool add(Timer& timer);
    bool remove(const String& id);
    void clear();

    bool start(const String& id);
    bool start(Timer& timer);

    bool pause(const String& id);
    bool resume(const String& id);

    void cancel(const String& id);
    bool dismiss(const String& id);

    bool isRunning(const String& id) const;
    bool isPaused(const String& id) const;
    bool isFinished(const String& id) const;
    bool isActive(const String& id) const;

    uint8_t progress(const String& id) const;
    uint32_t remainingMs(const String& id) const;
    uint32_t elapsedMs(const String& id) const;

    const Timer* get(const String& id) const;

    // Returns the timer occupying the specified internal slot, or nullptr.
    // Used by management interfaces to enumerate the timer pool.
    const Timer* getAt(uint8_t index) const;

    const Timer* nextToFinish() const;

    uint8_t count() const;
    uint8_t runningCount() const;

    void setStartCallback(StartCallback callback);
    void setTickCallback(TickCallback callback);
    void setFinishCallback(FinishCallback callback);
    void setStopCallback(StopCallback callback);

private:
    struct Slot
    {
        Timer timer;
        bool used = false;
        bool running = false;
        bool paused = false;
        bool finished = false;
        uint32_t startMs = 0;
        uint32_t elapsedBeforePause = 0;
        uint8_t lastProgress = 0;
    };

    int8_t findIndex(const String& id) const;
    int8_t findFreeSlot() const;
    void resetSlot(Slot& slot);
    void resetRuntime(Slot& slot);
    uint32_t computeElapsedMs(const Slot& slot) const;
    uint32_t computeRemainingMs(const Slot& slot) const;
    uint8_t computeProgress(const Slot& slot) const;
    String generateId() const;
    void notifyStart(const Timer& timer);
    void notifyTick(const Timer& timer, uint8_t percent, uint32_t remainingMs);
    void notifyFinish(const Timer& timer);
    void notifyStop(const String& id);

    Slot _slots[MAX_TIMERS];
    bool _begun = false;
    StartCallback _startCallback;
    TickCallback _tickCallback;
    FinishCallback _finishCallback;
    StopCallback _stopCallback;
};
