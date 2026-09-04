#pragma once

#include <Arduino.h>
#include "Constants.h"

struct TimeData
{
uint8_t hour = 0;
uint8_t minute = 0;
uint8_t second = 0;

```
bool valid = false;

uint8_t getHourTens() const
{
    return hour / 10;
}

uint8_t getHourOnes() const
{
    return hour % 10;
}

uint8_t getMinuteTens() const
{
    return minute / 10;
}

uint8_t getMinuteOnes() const
{
    return minute % 10;
}

uint8_t getSecondTens() const
{
    return second / 10;
}

uint8_t getSecondOnes() const
{
    return second % 10;
}
```

};

struct DateData
{
uint16_t year = 0;

```
uint8_t month = 0;

uint8_t day = 0;

Constants::DayOfWeek dayOfWeek =
    Constants::DayOfWeek::Sunday;

bool valid = false;
```

};
