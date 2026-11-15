
#include "TimerManager.h"


#include <esp_system.h>

// ============================================================
// CONSTRUCTOR
// ============================================================

TimerManager::TimerManager()
{
}

// ============================================================
// BEGIN
// ============================================================

bool TimerManager::begin()
{
    if (_begun)
        return true;

    for (uint8_t i = 0; i < MAX_TIMERS; ++i)
        resetSlot(_slots[i]);

    _begun = true;

    Serial0.println("[TIMER][MANAGER] READY");

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void TimerManager::update()
{
    if (!_begun)
        return;

    for (uint8_t i = 0; i < MAX_TIMERS; ++i)
    {
        Slot& slot = _slots[i];

        if (!slot.used || !slot.running)
            continue;

        const uint32_t elapsed = computeElapsedMs(slot);

        // ---- FINISH ----

        if (elapsed >= slot.timer.durationMs)
        {
            slot.running  = false;
            slot.finished = true;
            slot.lastProgress = 100;

            notifyTick(slot.timer, 100, 0);

            Serial0.printf(
                "[TIMER][MANAGER] FINISH id=%s\n",
                slot.timer.id.c_str()
            );

            notifyFinish(slot.timer);

            continue;
        }

        // ---- PROGRESS ----

        const uint8_t percent = computeProgress(slot);

        if (percent == slot.lastProgress)
            continue;

        slot.lastProgress = percent;

        notifyTick(
            slot.timer,
            percent,
            slot.timer.durationMs - elapsed
        );
    }
}

// ============================================================
// ADD
// ============================================================

bool TimerManager::add(Timer& timer)
{
    if (!_begun)
        return false;

    if (timer.durationMs == 0)
    {
        Serial0.println(
            "[TIMER][MANAGER] ADD FAILED: zero duration"
        );
        return false;
    }

    if (timer.id.isEmpty())
        timer.id = generateId();

    // Update existing idle / finished slot.
    const int8_t existing = findIndex(timer.id);

    if (existing >= 0)
    {
        Slot& slot = _slots[existing];

        if (slot.running || slot.paused)
        {
            Serial0.printf(
                "[TIMER][MANAGER] ADD FAILED: id=%s is active\n",
                timer.id.c_str()
            );
            return false;
        }

        resetRuntime(slot);

        slot.timer = timer;
        slot.used  = true;

        Serial0.printf(
            "[TIMER][MANAGER] ADD UPDATE id=%s\n",
            timer.id.c_str()
        );

        return true;
    }

    const int8_t freeSlot = findFreeSlot();

    if (freeSlot < 0)
    {
        Serial0.println(
            "[TIMER][MANAGER] ADD FAILED: pool full"
        );
        return false;
    }

    Slot& slot = _slots[freeSlot];

    resetSlot(slot);

    slot.used  = true;
    slot.timer = timer;

    Serial0.printf(
        "[TIMER][MANAGER] ADD id=%s duration=%lu effect=%s\n",
        timer.id.c_str(),
        static_cast<unsigned long>(timer.durationMs),
        timer.matrixEffect.c_str()
    );

    return true;
}

// ============================================================
// REMOVE
// ============================================================

bool TimerManager::remove(const String& id)
{
    const int8_t index = findIndex(id);

    if (index < 0)
        return false;

    Slot& slot = _slots[index];

    if (slot.running || slot.paused || slot.finished)
        notifyStop(id);

    resetSlot(slot);

    Serial0.printf(
        "[TIMER][MANAGER] REMOVE id=%s\n",
        id.c_str()
    );

    return true;
}

void TimerManager::clear()
{
    for (uint8_t i = 0; i < MAX_TIMERS; ++i)
    {
        if (_slots[i].used)
            notifyStop(_slots[i].timer.id);

        resetSlot(_slots[i]);
    }
}

// ============================================================
// START / PAUSE / RESUME
// ============================================================

bool TimerManager::start(const String& id)
{
    const int8_t index = findIndex(id);

    if (index < 0)
    {
        Serial0.printf(
            "[TIMER][MANAGER] START FAILED: unknown id=%s\n",
            id.c_str()
        );
        return false;
    }

    Slot& slot = _slots[index];

    if (slot.running || slot.paused)
    {
        Serial0.printf(
            "[TIMER][MANAGER] START IGNORED: id=%s active\n",
            id.c_str()
        );
        return false;
    }

    resetRuntime(slot);

    slot.running  = true;
    slot.paused   = false;
    slot.finished = false;
    slot.startMs  = millis();

    slot.lastProgress = 0;

    Serial0.printf(
        "[TIMER][MANAGER] START id=%s duration=%lu\n",
        slot.timer.id.c_str(),
        static_cast<unsigned long>(slot.timer.durationMs)
    );

    notifyStart(slot.timer);
    notifyTick(slot.timer, 0, slot.timer.durationMs);

    return true;
}

bool TimerManager::start(Timer& timer)
{
    if (!add(timer))
    {
        // If already running we still fall through to start()
        // only when add() failed for a non-active reason.
        if (findIndex(timer.id) < 0)
            return false;
    }

    return start(timer.id);
}

bool TimerManager::pause(const String& id)
{
    const int8_t index = findIndex(id);

    if (index < 0)
        return false;

    Slot& slot = _slots[index];

    if (!slot.running)
        return false;

    const uint32_t now = millis();
    uint32_t runningTime;

    // Handle millis() overflow
    if (now >= slot.startMs)
    {
        runningTime = now - slot.startMs;
    }
    else
    {
        // Overflow occurred
        runningTime = (0xFFFFFFFF - slot.startMs) + now + 1;
    }

    slot.elapsedBeforePause += runningTime;

    slot.running = false;
    slot.paused  = true;
    slot.startMs = 0;

    Serial0.printf(
        "[TIMER][MANAGER] PAUSE id=%s elapsed=%lu\n",
        slot.timer.id.c_str(),
        static_cast<unsigned long>(slot.elapsedBeforePause)
    );

    return true;
}

bool TimerManager::resume(const String& id)
{
    const int8_t index = findIndex(id);

    if (index < 0)
        return false;

    Slot& slot = _slots[index];

    if (!slot.paused)
        return false;

    slot.startMs = millis();
    slot.paused  = false;
    slot.running = true;

    Serial0.printf(
        "[TIMER][MANAGER] RESUME id=%s\n",
        slot.timer.id.c_str()
    );

    return true;
}

// ============================================================
// CANCEL / DISMISS
// ============================================================

void TimerManager::cancel(const String& id)
{
    const int8_t index = findIndex(id);

    if (index < 0)
        return;

    Slot& slot = _slots[index];

    if (!slot.running && !slot.paused)
        return;

    Serial0.printf(
        "[TIMER][MANAGER] CANCEL id=%s\n",
        id.c_str()
    );

    notifyStop(id);

    resetRuntime(slot);
}

bool TimerManager::dismiss(const String& id)
{
    const int8_t index = findIndex(id);

    if (index < 0)
        return false;

    Slot& slot = _slots[index];

    if (!slot.finished)
        return false;

    Serial0.printf(
        "[TIMER][MANAGER] DISMISS id=%s\n",
        id.c_str()
    );

    notifyStop(id);

    resetRuntime(slot);

    return true;
}

// ============================================================
// STATE
// ============================================================

bool TimerManager::isRunning(const String& id) const
{
    const int8_t i = findIndex(id);
    return i >= 0 && _slots[i].running;
}

bool TimerManager::isPaused(const String& id) const
{
    const int8_t i = findIndex(id);
    return i >= 0 && _slots[i].paused;
}

bool TimerManager::isFinished(const String& id) const
{
    const int8_t i = findIndex(id);
    return i >= 0 && _slots[i].finished;
}

bool TimerManager::isActive(const String& id) const
{
    const int8_t i = findIndex(id);

    if (i < 0)
        return false;

    return
        _slots[i].running ||
        _slots[i].paused  ||
        _slots[i].finished;
}

// ============================================================
// TIME / PROGRESS
// ============================================================

uint8_t TimerManager::progress(const String& id) const
{
    const int8_t i = findIndex(id);
    return i >= 0 ? computeProgress(_slots[i]) : 0;
}

uint32_t TimerManager::remainingMs(const String& id) const
{
    const int8_t i = findIndex(id);
    return i >= 0 ? computeRemainingMs(_slots[i]) : 0;
}

uint32_t TimerManager::elapsedMs(const String& id) const
{
    const int8_t i = findIndex(id);
    return i >= 0 ? computeElapsedMs(_slots[i]) : 0;
}

// ============================================================
// LOOKUP
// ============================================================

const Timer* TimerManager::get(const String& id) const
{
    const int8_t i = findIndex(id);
    return i >= 0 ? &_slots[i].timer : nullptr;
}


const Timer* TimerManager::getAt(uint8_t index) const
{
    if (index >= MAX_TIMERS || !_slots[index].used)
        return nullptr;

    return &_slots[index].timer;
}

const Timer* TimerManager::nextToFinish() const
{
    const Slot* best = nullptr;
    uint32_t bestRemaining = 0xFFFFFFFFUL;

    for (uint8_t i = 0; i < MAX_TIMERS; ++i)
    {
        const Slot& slot = _slots[i];

        if (!slot.used || !slot.running)
            continue;

        const uint32_t remaining = computeRemainingMs(slot);

        if (!best || remaining < bestRemaining)
        {
            best = &slot;
            bestRemaining = remaining;
        }
    }

    return best ? &best->timer : nullptr;
}

uint8_t TimerManager::count() const
{
    uint8_t n = 0;

    for (uint8_t i = 0; i < MAX_TIMERS; ++i)
    {
        if (_slots[i].used)
            ++n;
    }

    return n;
}

uint8_t TimerManager::runningCount() const
{
    uint8_t n = 0;

    for (uint8_t i = 0; i < MAX_TIMERS; ++i)
    {
        if (_slots[i].used && _slots[i].running)
            ++n;
    }

    return n;
}

// ============================================================
// CALLBACKS
// ============================================================

void TimerManager::setStartCallback(StartCallback callback)
{
    _startCallback = std::move(callback);
}

void TimerManager::setTickCallback(TickCallback callback)
{
    _tickCallback = std::move(callback);
}

void TimerManager::setFinishCallback(FinishCallback callback)
{
    _finishCallback = std::move(callback);
}

void TimerManager::setStopCallback(StopCallback callback)
{
    _stopCallback = std::move(callback);
}

// ============================================================
// INTERNAL HELPERS
// ============================================================

int8_t TimerManager::findIndex(const String& id) const
{
    if (id.isEmpty())
        return -1;

    for (uint8_t i = 0; i < MAX_TIMERS; ++i)
    {
        if (_slots[i].used && _slots[i].timer.id == id)
            return static_cast<int8_t>(i);
    }

    return -1;
}

int8_t TimerManager::findFreeSlot() const
{
    for (uint8_t i = 0; i < MAX_TIMERS; ++i)
    {
        if (!_slots[i].used)
            return static_cast<int8_t>(i);
    }

    return -1;
}

void TimerManager::resetSlot(Slot& slot)
{
    slot = Slot{};
}

void TimerManager::resetRuntime(Slot& slot)
{
    slot.running  = false;
    slot.paused   = false;
    slot.finished = false;

    slot.startMs            = 0;
    slot.elapsedBeforePause = 0;

    slot.lastProgress = 0;
}

uint32_t TimerManager::computeElapsedMs(const Slot& slot) const
{
    if (slot.finished)
        return slot.timer.durationMs;

    if (slot.running)
    {
        const uint32_t now = millis();
        uint32_t runningTime;

        // Handle millis() overflow
        if (now >= slot.startMs)
        {
            runningTime = now - slot.startMs;
        }
        else
        {
            // Overflow occurred
            runningTime = (0xFFFFFFFF - slot.startMs) + now + 1;
        }

        const uint32_t raw = slot.elapsedBeforePause + runningTime;

        return
            raw > slot.timer.durationMs
                ? slot.timer.durationMs
                : raw;
    }

    if (slot.paused)
    {
        return
            slot.elapsedBeforePause > slot.timer.durationMs
                ? slot.timer.durationMs
                : slot.elapsedBeforePause;
    }

    return 0;
}

uint32_t TimerManager::computeRemainingMs(const Slot& slot) const
{
    if (slot.finished)
        return 0;

    const uint32_t elapsed = computeElapsedMs(slot);

    if (elapsed >= slot.timer.durationMs)
        return 0;

    return slot.timer.durationMs - elapsed;
}

uint8_t TimerManager::computeProgress(const Slot& slot) const
{
    if (slot.finished)
        return 100;

    if (slot.timer.durationMs == 0)
        return 0;

    const uint32_t elapsed = computeElapsedMs(slot);

    if (elapsed >= slot.timer.durationMs)
        return 100;

    return static_cast<uint8_t>(
        (
            static_cast<uint64_t>(elapsed) *
            100ULL
        ) / slot.timer.durationMs
    );
}

String TimerManager::generateId() const
{
    uint8_t bytes[16];

    for (uint8_t i = 0; i < sizeof(bytes); i += 4)
    {
        const uint32_t v = esp_random();

        bytes[i]     = static_cast<uint8_t>(v);
        bytes[i + 1] = static_cast<uint8_t>(v >> 8);
        bytes[i + 2] = static_cast<uint8_t>(v >> 16);
        bytes[i + 3] = static_cast<uint8_t>(v >> 24);
    }

    bytes[6] = static_cast<uint8_t>((bytes[6] & 0x0F) | 0x40);
    bytes[8] = static_cast<uint8_t>((bytes[8] & 0x3F) | 0x80);

    char buffer[37];

    snprintf(
        buffer,
        sizeof(buffer),
        "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        bytes[0],  bytes[1],  bytes[2],  bytes[3],
        bytes[4],  bytes[5],
        bytes[6],  bytes[7],
        bytes[8],  bytes[9],
        bytes[10], bytes[11],
        bytes[12], bytes[13], bytes[14], bytes[15]
    );

    return String(buffer);
}

void TimerManager::notifyStart(const Timer& timer)
{
    if (_startCallback)
        _startCallback(timer);
}

void TimerManager::notifyTick(
    const Timer& timer,
    uint8_t percent,
    uint32_t remainingMs
)
{
    if (_tickCallback)
        _tickCallback(timer, percent, remainingMs);
}

void TimerManager::notifyFinish(const Timer& timer)
{
    if (_finishCallback)
        _finishCallback(timer);
}

void TimerManager::notifyStop(const String& id)
{
    if (_stopCallback)
        _stopCallback(id);
}