#pragma once

#include <Arduino.h>

// ============================================================
// STOPWATCH MODEL
// ============================================================

struct Stopwatch
{
    String id;
    String name;

    enum class State : uint8_t
    {
        IDLE,
        RUNNING,
        PAUSED,
        STOPPED
    };

    State state = State::IDLE;

    // ========================================================
    // TIMING
    // ========================================================

    uint32_t startTimeMs = 0;
    uint32_t pauseTimeMs = 0;
    uint32_t totalPausedMs = 0;
    uint32_t elapsedMs = 0;

    // ========================================================
    // LAP TIMES
    // ========================================================

    struct Lap
    {
        uint8_t number = 0;
        uint32_t timeMs = 0;
        uint32_t splitMs = 0;  // time since last lap
    };

    static constexpr uint8_t MAX_LAPS = 100;
    Lap laps[MAX_LAPS];
    uint8_t lapCount = 0;

    // ========================================================
    // METHODS
    // ========================================================

    void start()
    {
        const uint32_t now = millis();

        if (state == State::IDLE || state == State::STOPPED)
        {
            startTimeMs = now;
            totalPausedMs = 0;
            elapsedMs = 0;
            state = State::RUNNING;
        }
        else if (state == State::PAUSED)
        {
            // Calculate pause duration
            uint32_t pauseDuration;
            if (now >= pauseTimeMs)
            {
                pauseDuration = now - pauseTimeMs;
            }
            else
            {
                // Handle millis() overflow
                pauseDuration = (0xFFFFFFFF - pauseTimeMs) + now;
            }

            totalPausedMs += pauseDuration;
            state = State::RUNNING;
        }
    }

    void pause()
    {
        if (state == State::RUNNING)
        {
            const uint32_t now = millis();
            pauseTimeMs = now;

            // Calculate elapsed time before pausing
            if (now >= startTimeMs)
            {
                elapsedMs = now - startTimeMs - totalPausedMs;
            }
            else
            {
                // Handle millis() overflow
                elapsedMs = (0xFFFFFFFF - startTimeMs) + now - totalPausedMs;
            }

            state = State::PAUSED;
        }
    }

    void stop()
    {
        if (state == State::RUNNING || state == State::PAUSED)
        {
            elapsedMs = getElapsedMs();
            state = State::STOPPED;
        }
    }

    void reset()
    {
        state = State::IDLE;
        startTimeMs = 0;
        pauseTimeMs = 0;
        totalPausedMs = 0;
        elapsedMs = 0;
        lapCount = 0;
    }

    uint32_t getElapsedMs() const
    {
        if (state == State::IDLE)
            return 0;

        if (state == State::STOPPED || state == State::PAUSED)
            return elapsedMs;

        const uint32_t now = millis();

        // Handle millis() overflow
        if (now < startTimeMs)
        {
            // Overflow occurred, calculate correctly
            return (0xFFFFFFFF - startTimeMs) + now - totalPausedMs;
        }

        return now - startTimeMs - totalPausedMs;
    }

    bool addLap()
    {
        if (state != State::RUNNING)
            return false;

        if (lapCount >= MAX_LAPS)
        {
            // Cannot add more laps
            return false;
        }

        const uint32_t currentTime = getElapsedMs();

        laps[lapCount].number = lapCount + 1;
        laps[lapCount].timeMs = currentTime;

        if (lapCount > 0)
        {
            // Calculate split time
            const uint32_t prevTime = laps[lapCount - 1].timeMs;
            if (currentTime >= prevTime)
            {
                laps[lapCount].splitMs = currentTime - prevTime;
            }
            else
            {
                // Should not happen, but handle it
                laps[lapCount].splitMs = 0;
            }
        }
        else
        {
            // First lap - split time is the same as total time
            laps[lapCount].splitMs = currentTime;
        }

        lapCount++;
        return true;
    }

    const Lap* getLastLap() const
    {
        if (lapCount == 0)
            return nullptr;
        return &laps[lapCount - 1];
    }

    const Lap* getFastestLap() const
    {
        if (lapCount == 0)
            return nullptr;

        const Lap* fastest = &laps[0];
        for (uint8_t i = 1; i < lapCount; i++)
        {
            if (laps[i].splitMs < fastest->splitMs)
            {
                fastest = &laps[i];
            }
        }
        return fastest;
    }

    const Lap* getSlowestLap() const
    {
        if (lapCount == 0)
            return nullptr;

        const Lap* slowest = &laps[0];
        for (uint8_t i = 1; i < lapCount; i++)
        {
            if (laps[i].splitMs > slowest->splitMs)
            {
                slowest = &laps[i];
            }
        }
        return slowest;
    }
};
