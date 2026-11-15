#pragma once

#include <Arduino.h>

// ============================================================
// EVENT MODEL
// ============================================================

struct Event
{
    String id;
    String title;
    String description;
    String location;

    time_t startTime = 0;
    time_t endTime = 0;

    enum class Priority : uint8_t
    {
        LOW,
        NORMAL,
        HIGH,
        URGENT
    };

    Priority priority = Priority::NORMAL;

    enum class Status : uint8_t
    {
        SCHEDULED,
        IN_PROGRESS,
        COMPLETED,
        CANCELLED
    };

    Status status = Status::SCHEDULED;

    // ========================================================
    // REMINDER
    // ========================================================

    bool hasReminder = false;
    uint32_t reminderMinutesBefore = 15;  // minutes before event
    bool reminderTriggered = false;

    // ========================================================
    // REPEAT
    // ========================================================

    enum class RepeatType : uint8_t
    {
        NONE,
        DAILY,
        WEEKLY,
        MONTHLY,
        YEARLY
    };

    RepeatType repeatType = RepeatType::NONE;

    // ========================================================
    // HELPERS
    // ========================================================

    bool isActive() const
    {
        if (status == Status::CANCELLED || status == Status::COMPLETED)
            return false;

        const time_t now = time(nullptr);
        return now >= startTime && now <= endTime;
    }

    bool isUpcoming(uint32_t minutesAhead = 60) const
    {
        if (status == Status::CANCELLED || status == Status::COMPLETED)
            return false;

        const time_t now = time(nullptr);

        // Validate timestamps
        if (startTime == 0 || startTime < now)
            return false;

        const time_t threshold = now + (minutesAhead * 60);
        return startTime <= threshold;
    }

    bool isPast() const
    {
        const time_t now = time(nullptr);

        // Check for invalid timestamp
        if (endTime == 0)
            return false;

        return endTime < now;
    }

    bool needsReminder() const
    {
        if (!hasReminder || reminderTriggered)
            return false;

        if (status == Status::CANCELLED || status == Status::COMPLETED)
            return false;

        const time_t now = time(nullptr);

        // Validate timestamp
        if (startTime == 0 || startTime <= now)
            return false;

        const time_t reminderTime = startTime - (reminderMinutesBefore * 60);

        // Check if reminder time has passed but event hasn't started
        return now >= reminderTime && now < startTime;
    }

    uint32_t getDurationMinutes() const
    {
        // Validate timestamps
        if (startTime == 0 || endTime == 0 || endTime <= startTime)
            return 0;

        return (endTime - startTime) / 60;
    }
};
