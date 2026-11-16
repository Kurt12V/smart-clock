
// #include "EventManager.h"
// #include <ctime>

// // ============================================================
// // CONSTRUCTOR
// // ============================================================

// EventManager::EventManager()
//     : _count(0)
//     , _reminderCallback(nullptr)
//     , _lastReminderCheck(0)
//     , _initialized(false)
// {
//     for (uint8_t i = 0; i < MAX_EVENTS; i++)
//     {
//         _used[i] = false;
//     }
// }

// // ============================================================
// // BEGIN
// // ============================================================

// bool EventManager::begin()
// {
//     if (_initialized)
//     {
//         Serial0.println("[EVENT] Already initialized");
//         return true;
//     }

//     Serial0.println("[EVENT] Initializing Event Manager...");

//     _lastReminderCheck = millis();

//     _initialized = true;

//     Serial0.println("[EVENT] Event Manager initialized");

//     return true;
// }

// // ============================================================
// // UPDATE
// // ============================================================

// void EventManager::update()
// {
//     if (!_initialized)
//         return;

//     const uint32_t now = millis();

//     // Handle millis() overflow (every ~49.7 days)
//     if (now < _lastReminderCheck)
//     {
//         _lastReminderCheck = now;
//     }

//     // Check reminders every 10 seconds
//     if (now - _lastReminderCheck >= 10000)
//     {
//         _lastReminderCheck = now;
//         checkReminders();
//     }
// }

// // ============================================================
// // EVENT MANAGEMENT
// // ============================================================

// bool EventManager::add(const Event& event)
// {
//     const int8_t index = findFreeSlot();
//     if (index < 0)
//     {
//         Serial0.println("[EVENT] ERROR: No free slots available");
//         return false;
//     }

//     // Validate event data
//     if (event.title.isEmpty())
//     {
//         Serial0.println("[EVENT] ERROR: Event title is required");
//         return false;
//     }

//     if (event.startTime == 0)
//     {
//         Serial0.println("[EVENT] ERROR: Invalid start time");
//         return false;
//     }

//     if (event.endTime > 0 && event.endTime <= event.startTime)
//     {
//         Serial0.println("[EVENT] ERROR: End time must be after start time");
//         return false;
//     }

//     _events[index] = event;

//     if (_events[index].id.isEmpty())
//     {
//         _events[index].id = generateId();
//     }

//     _used[index] = true;
//     _count++;

//     Serial0.printf("[EVENT] Event added: %s (ID: %s)\n",
//                    _events[index].title.c_str(),
//                    _events[index].id.c_str());

//     return true;
// }

// bool EventManager::remove(const String& id)
// {
//     const int8_t index = findIndex(id);
//     if (index < 0)
//         return false;

//     _used[index] = false;
//     _count--;

//     Serial0.printf("[EVENT] Event removed: %s\n", id.c_str());

//     return true;
// }

// bool EventManager::update(const Event& event)
// {
//     // Validate event ID
//     if (event.id.isEmpty())
//     {
//         Serial0.println("[EVENT] ERROR: Cannot update event with empty ID");
//         return false;
//     }

//     const int8_t index = findIndex(event.id);
//     if (index < 0)
//     {
//         Serial0.printf("[EVENT] ERROR: Event not found: %s\n", event.id.c_str());
//         return false;
//     }

//     // Validate timestamps
//     if (event.startTime == 0)
//     {
//         Serial0.println("[EVENT] ERROR: Invalid start time");
//         return false;
//     }

//     if (event.endTime > 0 && event.endTime <= event.startTime)
//     {
//         Serial0.println("[EVENT] ERROR: End time must be after start time");
//         return false;
//     }

//     _events[index] = event;

//     Serial0.printf("[EVENT] Event updated: %s\n", event.id.c_str());

//     return true;
// }

// void EventManager::clear()
// {
//     for (uint8_t i = 0; i < MAX_EVENTS; i++)
//     {
//         _used[i] = false;
//     }
//     _count = 0;

//     Serial0.println("[EVENT] All events cleared");
// }

// // ============================================================
// // GETTERS
// // ============================================================

// const Event* EventManager::get(const String& id) const
// {
//     const int8_t index = findIndex(id);
//     if (index < 0)
//         return nullptr;

//     return &_events[index];
// }

// const Event* EventManager::getAt(uint8_t index) const
// {
//     if (index >= MAX_EVENTS || !_used[index])
//         return nullptr;

//     return &_events[index];
// }

// uint8_t EventManager::count() const
// {
//     return _count;
// }

// uint8_t EventManager::getTodayCount() const
// {
//     const time_t now = time(nullptr);
//     uint8_t count = 0;

//     for (uint8_t i = 0; i < MAX_EVENTS; i++)
//     {
//         if (_used[i] && isSameDay(_events[i].startTime, now))
//         {
//             count++;
//         }
//     }

//     return count;
// }

// uint8_t EventManager::getUpcomingCount(uint32_t minutesAhead) const
// {
//     uint8_t count = 0;

//     for (uint8_t i = 0; i < MAX_EVENTS; i++)
//     {
//         if (_used[i] && _events[i].isUpcoming(minutesAhead))
//         {
//             count++;
//         }
//     }

//     return count;
// }

// // ============================================================
// // QUERIES
// // ============================================================

// bool EventManager::getEventsForDay(time_t date, Event* outEvents, uint8_t maxCount, uint8_t& outCount) const
// {
//     outCount = 0;

//     for (uint8_t i = 0; i < MAX_EVENTS && outCount < maxCount; i++)
//     {
//         if (_used[i] && isSameDay(_events[i].startTime, date))
//         {
//             outEvents[outCount++] = _events[i];
//         }
//     }

//     return outCount > 0;
// }

// bool EventManager::getUpcomingEvents(Event* outEvents, uint8_t maxCount, uint8_t& outCount, uint32_t minutesAhead) const
// {
//     outCount = 0;

//     for (uint8_t i = 0; i < MAX_EVENTS && outCount < maxCount; i++)
//     {
//         if (_used[i] && _events[i].isUpcoming(minutesAhead))
//         {
//             outEvents[outCount++] = _events[i];
//         }
//     }

//     return outCount > 0;
// }

// bool EventManager::getActiveEvents(Event* outEvents, uint8_t maxCount, uint8_t& outCount) const
// {
//     outCount = 0;

//     for (uint8_t i = 0; i < MAX_EVENTS && outCount < maxCount; i++)
//     {
//         if (_used[i] && _events[i].isActive())
//         {
//             outEvents[outCount++] = _events[i];
//         }
//     }

//     return outCount > 0;
// }

// const Event* EventManager::getNextEvent() const
// {
//     const time_t now = time(nullptr);
//     const Event* next = nullptr;
//     time_t minDiff = LONG_MAX;

//     for (uint8_t i = 0; i < MAX_EVENTS; i++)
//     {
//         if (_used[i] && _events[i].startTime > now)
//         {
//             const time_t diff = _events[i].startTime - now;
//             if (diff < minDiff)
//             {
//                 minDiff = diff;
//                 next = &_events[i];
//             }
//         }
//     }

//     return next;
// }

// const Event* EventManager::getCurrentEvent() const
// {
//     for (uint8_t i = 0; i < MAX_EVENTS; i++)
//     {
//         if (_used[i] && _events[i].isActive())
//         {
//             return &_events[i];
//         }
//     }

//     return nullptr;
// }

// // ============================================================
// // CALLBACKS
// // ============================================================

// void EventManager::setReminderCallback(ReminderCallback callback)
// {
//     _reminderCallback = callback;
// }

// // ============================================================
// // HELPERS
// // ============================================================

// int8_t EventManager::findIndex(const String& id) const
// {
//     for (uint8_t i = 0; i < MAX_EVENTS; i++)
//     {
//         if (_used[i] && _events[i].id == id)
//             return i;
//     }
//     return -1;
// }

// int8_t EventManager::findFreeSlot() const
// {
//     for (uint8_t i = 0; i < MAX_EVENTS; i++)
//     {
//         if (!_used[i])
//             return i;
//     }
//     return -1;
// }

// void EventManager::checkReminders()
// {
//     for (uint8_t i = 0; i < MAX_EVENTS; i++)
//     {
//         if (_used[i] && _events[i].needsReminder())
//         {
//             _events[i].reminderTriggered = true;

//             if (_reminderCallback)
//             {
//                 _reminderCallback(_events[i]);
//             }

//             Serial0.printf("[EVENT] Reminder triggered: %s\n", _events[i].title.c_str());
//         }
//     }
// }

// bool EventManager::isSameDay(time_t t1, time_t t2) const
// {
//     struct tm tm1, tm2;

//     if (gmtime_r(&t1, &tm1) == nullptr)
//         return false;

//     if (gmtime_r(&t2, &tm2) == nullptr)
//         return false;

//     return (tm1.tm_year == tm2.tm_year &&
//             tm1.tm_mon == tm2.tm_mon &&
//             tm1.tm_mday == tm2.tm_mday);
// }

// String EventManager::generateId() const
// {
//     const uint32_t timestamp = millis();
//     const uint32_t random = esp_random();

//     char buffer[32];
//     snprintf(buffer, sizeof(buffer), "evt_%lu_%lu", timestamp, random);

//     return String(buffer);
// }
