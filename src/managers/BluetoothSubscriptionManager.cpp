#include "BluetoothSubscriptionManager.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

BluetoothSubscriptionManager::BluetoothSubscriptionManager()
    : _count(0)
{
}

// ============================================================
// SUBSCRIBE
// ============================================================

bool BluetoothSubscriptionManager::subscribe(
    const char* topic
)
{
    if (topic == nullptr ||
        topic[0] == '\0')
    {
        return false;
    }

    // Already subscribed
    if (isSubscribed(topic))
    {
        return true;
    }

    if (_count >= MAX_TOPICS)
    {
        return false;
    }

    _topics[_count] =
        topic;

    _count++;

    return true;
}

// ============================================================
// UNSUBSCRIBE
// ============================================================

bool BluetoothSubscriptionManager::unsubscribe(
    const char* topic
)
{
    if (topic == nullptr)
    {
        return false;
    }

    for (
        uint8_t i = 0;
        i < _count;
        ++i
    )
    {
        if (_topics[i] == topic)
        {
            for (
                uint8_t j = i;
                j < _count - 1;
                ++j
            )
            {
                _topics[j] =
                    _topics[j + 1];
            }

            _topics[_count - 1] =
                "";

            _count--;

            return true;
        }
    }

    return false;
}

// ============================================================
// CLEAR
// ============================================================

void BluetoothSubscriptionManager::clear()
{
    for (
        uint8_t i = 0;
        i < MAX_TOPICS;
        ++i
    )
    {
        _topics[i] =
            "";
    }

    _count = 0;
}

// ============================================================
// IS SUBSCRIBED
// ============================================================

bool BluetoothSubscriptionManager::isSubscribed(
    const char* topic
) const
{
    if (topic == nullptr)
    {
        return false;
    }

    for (
        uint8_t i = 0;
        i < _count;
        ++i
    )
    {
        if (_topics[i] == topic)
        {
            return true;
        }
    }

    return false;
}

// ============================================================
// COUNT
// ============================================================

uint8_t BluetoothSubscriptionManager::count() const
{
    return _count;
}