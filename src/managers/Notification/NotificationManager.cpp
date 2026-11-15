
#include "NotificationManager.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

NotificationManager::NotificationManager()
    : _count(0)
    , _notificationCallback(nullptr)
    , _actionCallback(nullptr)
    , _initialized(false)
{
    for (uint8_t i = 0; i < MAX_NOTIFICATIONS; i++)
    {
        _used[i] = false;
    }
}

// ============================================================
// BEGIN
// ============================================================

bool NotificationManager::begin()
{
    if (_initialized)
    {
        Serial0.println("[NOTIFICATION] Already initialized");
        return true;
    }

    Serial0.println("[NOTIFICATION] Initializing Notification Manager...");

    _initialized = true;

    Serial0.println("[NOTIFICATION] Notification Manager initialized");

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void NotificationManager::update()
{
    if (!_initialized)
        return;

    // Периодически очищаем устаревшие уведомления
    static uint32_t lastCleanup = 0;
    const uint32_t now = millis();

    // Handle millis() overflow
    if (now < lastCleanup)
    {
        lastCleanup = now;
    }

    if (now - lastCleanup >= 60000) // каждую минуту
    {
        lastCleanup = now;
        clearExpired();
    }
}

// ============================================================
// NOTIFICATION MANAGEMENT
// ============================================================

bool NotificationManager::add(const Notification& notification)
{
    const int8_t index = findFreeSlot();
    if (index < 0)
    {
        Serial0.println("[NOTIFICATION] ERROR: No free slots available");
        return false;
    }

    // Validate notification data
    if (notification.title.isEmpty())
    {
        Serial0.println("[NOTIFICATION] ERROR: Notification title is required");
        return false;
    }

    _notifications[index] = notification;

    if (_notifications[index].id.isEmpty())
    {
        _notifications[index].id = generateId();
    }

    if (_notifications[index].timestamp == 0)
    {
        const time_t now = time(nullptr);
        if (now > 0)
        {
            _notifications[index].timestamp = now;
        }
        else
        {
            Serial0.println("[NOTIFICATION] WARNING: System time not set");
        }
    }

    _used[index] = true;
    _count++;

    // Вызываем callback
    if (_notificationCallback)
    {
        _notificationCallback(_notifications[index]);
    }

    Serial0.printf("[NOTIFICATION] Added: %s (ID: %s)\n",
                   _notifications[index].title.c_str(),
                   _notifications[index].id.c_str());

    return true;
}

bool NotificationManager::remove(const String& id)
{
    const int8_t index = findIndex(id);
    if (index < 0)
        return false;

    _used[index] = false;
    _count--;

    Serial0.printf("[NOTIFICATION] Removed: %s\n", id.c_str());

    return true;
}

void NotificationManager::clear()
{
    for (uint8_t i = 0; i < MAX_NOTIFICATIONS; i++)
    {
        _used[i] = false;
    }
    _count = 0;

    Serial0.println("[NOTIFICATION] All notifications cleared");
}

void NotificationManager::clearRead()
{
    for (uint8_t i = 0; i < MAX_NOTIFICATIONS; i++)
    {
        if (_used[i] && _notifications[i].read && !_notifications[i].persistent)
        {
            _used[i] = false;
            _count--;
        }
    }

    Serial0.println("[NOTIFICATION] Read notifications cleared");
}

void NotificationManager::clearExpired()
{
    for (uint8_t i = 0; i < MAX_NOTIFICATIONS; i++)
    {
        if (_used[i] && _notifications[i].isExpired())
        {
            _used[i] = false;
            _count--;
        }
    }
}

// ============================================================
// ACTIONS
// ============================================================

bool NotificationManager::markAsRead(const String& id)
{
    const int8_t index = findIndex(id);
    if (index < 0)
        return false;

    _notifications[index].markAsRead();

    return true;
}

bool NotificationManager::dismiss(const String& id)
{
    const int8_t index = findIndex(id);
    if (index < 0)
        return false;

    _notifications[index].dismiss();

    return true;
}

bool NotificationManager::executeAction(const String& notificationId, const String& actionId)
{
    const int8_t index = findIndex(notificationId);
    if (index < 0)
        return false;

    if (_actionCallback)
    {
        _actionCallback(_notifications[index], actionId);
    }

    return true;
}

// ============================================================
// GETTERS
// ============================================================

const Notification* NotificationManager::get(const String& id) const
{
    const int8_t index = findIndex(id);
    if (index < 0)
        return nullptr;

    return &_notifications[index];
}

const Notification* NotificationManager::getAt(uint8_t index) const
{
    if (index >= MAX_NOTIFICATIONS || !_used[index])
        return nullptr;

    return &_notifications[index];
}

uint8_t NotificationManager::count() const
{
    return _count;
}

uint8_t NotificationManager::unreadCount() const
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < MAX_NOTIFICATIONS; i++)
    {
        if (_used[i] && _notifications[i].isUnread())
        {
            count++;
        }
    }

    return count;
}

uint8_t NotificationManager::getPriorityCount(Notification::Priority priority) const
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < MAX_NOTIFICATIONS; i++)
    {
        if (_used[i] && _notifications[i].priority == priority)
        {
            count++;
        }
    }

    return count;
}

// ============================================================
// QUERIES
// ============================================================

bool NotificationManager::getUnread(Notification* outNotifications, uint8_t maxCount, uint8_t& outCount) const
{
    outCount = 0;

    for (uint8_t i = 0; i < MAX_NOTIFICATIONS && outCount < maxCount; i++)
    {
        if (_used[i] && _notifications[i].isUnread())
        {
            outNotifications[outCount++] = _notifications[i];
        }
    }

    return outCount > 0;
}

bool NotificationManager::getByType(Notification::Type type, Notification* outNotifications, uint8_t maxCount, uint8_t& outCount) const
{
    outCount = 0;

    for (uint8_t i = 0; i < MAX_NOTIFICATIONS && outCount < maxCount; i++)
    {
        if (_used[i] && _notifications[i].type == type)
        {
            outNotifications[outCount++] = _notifications[i];
        }
    }

    return outCount > 0;
}

bool NotificationManager::getByPriority(Notification::Priority priority, Notification* outNotifications, uint8_t maxCount, uint8_t& outCount) const
{
    outCount = 0;

    for (uint8_t i = 0; i < MAX_NOTIFICATIONS && outCount < maxCount; i++)
    {
        if (_used[i] && _notifications[i].priority == priority)
        {
            outNotifications[outCount++] = _notifications[i];
        }
    }

    return outCount > 0;
}

const Notification* NotificationManager::getLatest() const
{
    const Notification* latest = nullptr;
    time_t latestTime = 0;

    for (uint8_t i = 0; i < MAX_NOTIFICATIONS; i++)
    {
        if (_used[i] && _notifications[i].timestamp > latestTime)
        {
            latestTime = _notifications[i].timestamp;
            latest = &_notifications[i];
        }
    }

    return latest;
}

const Notification* NotificationManager::getLatestUnread() const
{
    const Notification* latest = nullptr;
    time_t latestTime = 0;

    for (uint8_t i = 0; i < MAX_NOTIFICATIONS; i++)
    {
        if (_used[i] && _notifications[i].isUnread() && _notifications[i].timestamp > latestTime)
        {
            latestTime = _notifications[i].timestamp;
            latest = &_notifications[i];
        }
    }

    return latest;
}

// ============================================================
// CALLBACKS
// ============================================================

void NotificationManager::setNotificationCallback(NotificationCallback callback)
{
    _notificationCallback = callback;
}

void NotificationManager::setActionCallback(ActionCallback callback)
{
    _actionCallback = callback;
}

// ============================================================
// HELPERS
// ============================================================

int8_t NotificationManager::findIndex(const String& id) const
{
    for (uint8_t i = 0; i < MAX_NOTIFICATIONS; i++)
    {
        if (_used[i] && _notifications[i].id == id)
            return i;
    }
    return -1;
}

int8_t NotificationManager::findFreeSlot() const
{
    for (uint8_t i = 0; i < MAX_NOTIFICATIONS; i++)
    {
        if (!_used[i])
            return i;
    }
    return -1;
}

String NotificationManager::generateId() const
{
    const uint32_t timestamp = millis();
    const uint32_t random = esp_random();

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "notif_%lu_%lu", timestamp, random);

    return String(buffer);
}
