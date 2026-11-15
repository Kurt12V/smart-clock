
#include "StopwatchManager.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

StopwatchManager::StopwatchManager()
    : _count(0)
    , _initialized(false)
{
    for (uint8_t i = 0; i < MAX_STOPWATCHES; i++)
    {
        _used[i] = false;
    }
}

// ============================================================
// BEGIN
// ============================================================

bool StopwatchManager::begin()
{
    if (_initialized)
    {
        Serial0.println("[STOPWATCH] Already initialized");
        return true;
    }

    Serial0.println("[STOPWATCH] Initializing...");

    // Create default stopwatch
    create("default", "Default");

    _initialized = true;

    Serial0.println("[STOPWATCH] Initialized successfully");

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void StopwatchManager::update()
{
    if (!_initialized)
        return;

    // Stopwatches update themselves based on millis()
    // No periodic update needed
}

// ============================================================
// CONTROL
// ============================================================

bool StopwatchManager::start(const String& id)
{
    const int8_t index = findIndex(id);
    if (index < 0)
        return false;

    _stopwatches[index].start();
    return true;
}

bool StopwatchManager::pause(const String& id)
{
    const int8_t index = findIndex(id);
    if (index < 0)
        return false;

    _stopwatches[index].pause();
    return true;
}

bool StopwatchManager::stop(const String& id)
{
    const int8_t index = findIndex(id);
    if (index < 0)
        return false;

    _stopwatches[index].stop();
    return true;
}

bool StopwatchManager::reset(const String& id)
{
    const int8_t index = findIndex(id);
    if (index < 0)
        return false;

    _stopwatches[index].reset();
    return true;
}

bool StopwatchManager::addLap(const String& id)
{
    const int8_t index = findIndex(id);
    if (index < 0)
        return false;

    return _stopwatches[index].addLap();
}

// ============================================================
// GETTERS
// ============================================================

const Stopwatch* StopwatchManager::get(const String& id) const
{
    const int8_t index = findIndex(id);
    if (index < 0)
        return nullptr;

    return &_stopwatches[index];
}

uint32_t StopwatchManager::getElapsedMs(const String& id) const
{
    const Stopwatch* sw = get(id);
    if (!sw)
        return 0;

    return sw->getElapsedMs();
}

String StopwatchManager::getFormattedTime(const String& id) const
{
    const uint32_t ms = getElapsedMs(id);
    return formatTime(ms);
}

bool StopwatchManager::isRunning(const String& id) const
{
    const Stopwatch* sw = get(id);
    if (!sw)
        return false;

    return sw->state == Stopwatch::State::RUNNING;
}

bool StopwatchManager::isPaused(const String& id) const
{
    const Stopwatch* sw = get(id);
    if (!sw)
        return false;

    return sw->state == Stopwatch::State::PAUSED;
}

// ============================================================
// MULTIPLE STOPWATCHES
// ============================================================

bool StopwatchManager::create(const String& id, const String& name)
{
    // Check if already exists
    if (findIndex(id) >= 0)
        return false;

    // Find free slot
    const int8_t index = findFreeSlot();
    if (index < 0)
        return false;

    _stopwatches[index].id = id;
    _stopwatches[index].name = name;
    _stopwatches[index].reset();

    _used[index] = true;
    _count++;

    return true;
}

bool StopwatchManager::remove(const String& id)
{
    const int8_t index = findIndex(id);
    if (index < 0)
        return false;

    _used[index] = false;
    _count--;

    return true;
}

void StopwatchManager::removeAll()
{
    for (uint8_t i = 0; i < MAX_STOPWATCHES; i++)
    {
        _used[i] = false;
    }
    _count = 0;
}

uint8_t StopwatchManager::count() const
{
    return _count;
}

const Stopwatch* StopwatchManager::getAt(uint8_t index) const
{
    if (index >= MAX_STOPWATCHES || !_used[index])
        return nullptr;

    return &_stopwatches[index];
}

// ============================================================
// HELPERS
// ============================================================

int8_t StopwatchManager::findIndex(const String& id) const
{
    for (uint8_t i = 0; i < MAX_STOPWATCHES; i++)
    {
        if (_used[i] && _stopwatches[i].id == id)
            return i;
    }
    return -1;
}

int8_t StopwatchManager::findFreeSlot() const
{
    for (uint8_t i = 0; i < MAX_STOPWATCHES; i++)
    {
        if (!_used[i])
            return i;
    }
    return -1;
}

String StopwatchManager::formatTime(uint32_t ms) const
{
    const uint32_t totalSeconds = ms / 1000;
    const uint32_t milliseconds = ms % 1000;

    const uint32_t hours = totalSeconds / 3600;
    const uint32_t minutes = (totalSeconds % 3600) / 60;
    const uint32_t seconds = totalSeconds % 60;

    char buffer[16];

    if (hours > 0)
    {
        snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu.%03lu",
                 hours, minutes, seconds, milliseconds);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "%02lu:%02lu.%03lu",
                 minutes, seconds, milliseconds);
    }

    return String(buffer);
}
