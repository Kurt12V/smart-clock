#pragma once

#include <Arduino.h>
#include "../models/Event.h"
#include <functional>

// ============================================================
// EVENT MANAGER
// ============================================================

class EventManager
{
public:

    using ReminderCallback = std::function<void(const Event&)>;

    EventManager();

    bool begin();
    void update();

    // ========================================================
    // EVENT MANAGEMENT
    // ========================================================

    bool add(const Event& event);
    bool remove(const String& id);
    bool update(const Event& event);

    void clear();

    // ========================================================
    // GETTERS
    // ========================================================

    const Event* get(const String& id) const;
    const Event* getAt(uint8_t index) const;

    uint8_t count() const;
    uint8_t getTodayCount() const;
    uint8_t getUpcomingCount(uint32_t minutesAhead = 60) const;

    // ========================================================
    // QUERIES
    // ========================================================

    bool getEventsForDay(time_t date, Event* outEvents, uint8_t maxCount, uint8_t& outCount) const;
    bool getUpcomingEvents(Event* outEvents, uint8_t maxCount, uint8_t& outCount, uint32_t minutesAhead = 60) const;
    bool getActiveEvents(Event* outEvents, uint8_t maxCount, uint8_t& outCount) const;

    const Event* getNextEvent() const;
    const Event* getCurrentEvent() const;

    // ========================================================
    // CALLBACKS
    // ========================================================

    void setReminderCallback(ReminderCallback callback);

private:

    // ========================================================
    // HELPERS
    // ========================================================

    int8_t findIndex(const String& id) const;
    int8_t findFreeSlot() const;

    void checkReminders();

    bool isSameDay(time_t t1, time_t t2) const;

    String generateId() const;

    // ========================================================
    // DATA
    // ========================================================

    static constexpr uint8_t MAX_EVENTS = 100;

    Event _events[MAX_EVENTS];
    bool _used[MAX_EVENTS];
    uint8_t _count;

    ReminderCallback _reminderCallback;

    uint32_t _lastReminderCheck;

    bool _initialized;
};
