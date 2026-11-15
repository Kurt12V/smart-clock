#pragma once

#include <Arduino.h>

// ============================================================
// NOTIFICATION MODEL
// ============================================================

struct Notification
{
    String id;
    String title;
    String message;
    String source;      // app/service that sent notification

    time_t timestamp = 0;

    enum class Type : uint8_t
    {
        INFO,
        WARNING,
        ERROR,
        SUCCESS,
        EVENT,
        ALARM,
        TIMER,
        MESSAGE,
        WEATHER_ALERT,
        SYSTEM
    };

    Type type = Type::INFO;

    enum class Priority : uint8_t
    {
        LOW,
        NORMAL,
        HIGH,
        URGENT
    };

    Priority priority = Priority::NORMAL;

    // ========================================================
    // STATE
    // ========================================================

    bool read = false;
    bool dismissed = false;
    bool persistent = false;  // stays until dismissed

    // ========================================================
    // ACTIONS
    // ========================================================

    struct Action
    {
        String id;
        String label;
        String data;  // action-specific data
    };

    static constexpr uint8_t MAX_ACTIONS = 3;
    Action actions[MAX_ACTIONS];
    uint8_t actionCount = 0;

    // ========================================================
    // HELPERS
    // ========================================================

    bool isUnread() const
    {
        return !read && !dismissed;
    }

    bool isExpired(uint32_t maxAgeSeconds = 86400) const  // 24h default
    {
        if (persistent)
            return false;

        const time_t now = time(nullptr);

        // Check for invalid timestamp or future timestamp
        if (timestamp == 0 || now < timestamp)
            return false;

        return (now - timestamp) > maxAgeSeconds;
    }

    void markAsRead()
    {
        read = true;
    }

    void dismiss()
    {
        dismissed = true;
    }

    bool addAction(const String& actionId, const String& label, const String& data = "")
    {
        if (actionCount >= MAX_ACTIONS)
            return false;

        actions[actionCount].id = actionId;
        actions[actionCount].label = label;
        actions[actionCount].data = data;
        actionCount++;

        return true;
    }
};
