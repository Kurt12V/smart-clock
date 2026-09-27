#pragma once

#include <Arduino.h>

class BluetoothSubscriptionManager
{
public:
    static constexpr uint8_t MAX_SUBSCRIPTIONS = 16;

    BluetoothSubscriptionManager();

    bool subscribe(const char* topic);
    bool unsubscribe(const char* topic);
    bool isSubscribed(const char* topic) const;
    void clear();

    uint8_t count() const;

    bool getTopic(
        uint8_t index,
        char* buffer,
        size_t bufferSize
    ) const;

private:
    struct Subscription
    {
        bool active;
        char topic[32];
    };

    Subscription _subscriptions[MAX_SUBSCRIPTIONS];
    uint8_t _count;

    int findTopic(const char* topic) const;
};
