// #pragma once

// #include <Arduino.h>
// #include "../models/Notification.h"
// #include <functional>

// // ============================================================
// // NOTIFICATION MANAGER
// // ============================================================

// class NotificationManager
// {
// public:

//     using NotificationCallback = std::function<void(const Notification&)>;
//     using ActionCallback = std::function<void(const Notification&, const String& actionId)>;

//     NotificationManager();

//     bool begin();
//     void update();

//     // ========================================================
//     // NOTIFICATION MANAGEMENT
//     // ========================================================

//     bool add(const Notification& notification);
//     bool remove(const String& id);
//     void clear();
//     void clearRead();
//     void clearExpired();

//     // ========================================================
//     // ACTIONS
//     // ========================================================

//     bool markAsRead(const String& id);
//     bool dismiss(const String& id);
//     bool executeAction(const String& notificationId, const String& actionId);

//     // ========================================================
//     // GETTERS
//     // ========================================================

//     const Notification* get(const String& id) const;
//     const Notification* getAt(uint8_t index) const;

//     uint8_t count() const;
//     uint8_t unreadCount() const;
//     uint8_t getPriorityCount(Notification::Priority priority) const;

//     // ========================================================
//     // QUERIES
//     // ========================================================

//     bool getUnread(Notification* outNotifications, uint8_t maxCount, uint8_t& outCount) const;
//     bool getByType(Notification::Type type, Notification* outNotifications, uint8_t maxCount, uint8_t& outCount) const;
//     bool getByPriority(Notification::Priority priority, Notification* outNotifications, uint8_t maxCount, uint8_t& outCount) const;

//     const Notification* getLatest() const;
//     const Notification* getLatestUnread() const;

//     // ========================================================
//     // CALLBACKS
//     // ========================================================

//     void setNotificationCallback(NotificationCallback callback);
//     void setActionCallback(ActionCallback callback);

// private:

//     // ========================================================
//     // HELPERS
//     // ========================================================

//     int8_t findIndex(const String& id) const;
//     int8_t findFreeSlot() const;

//     String generateId() const;

//     // ========================================================
//     // DATA
//     // ========================================================

//     static constexpr uint8_t MAX_NOTIFICATIONS = 50;

//     Notification _notifications[MAX_NOTIFICATIONS];
//     bool _used[MAX_NOTIFICATIONS];
//     uint8_t _count;

//     NotificationCallback _notificationCallback;
//     ActionCallback _actionCallback;

//     bool _initialized;
// };
