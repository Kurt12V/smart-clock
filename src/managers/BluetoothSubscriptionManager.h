#pragma once

#include <Arduino.h>

class BluetoothSubscriptionManager
{
public:
    static constexpr uint8_t MAX_TOPICS = 16;

    BluetoothSubscriptionManager();

    bool subscribe(const char* topic);
    bool unsubscribe(const char* topic);

    void clear();

    bool isSubscribed(const char* topic) const;

    uint8_t count() const;

private:
    String _topics[MAX_TOPICS];
    uint8_t _count;
};

