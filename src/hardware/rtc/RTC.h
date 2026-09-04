#pragma once

#include <Arduino.h>
#include <RTClib.h>

class RTC {
public:
RTC() = default;


// Инициализация RTC
bool begin(int sda, int scl);

// Проверка потери питания
bool isRunning();

// Получить дату и время
DateTime getDateTime();

// Установить UTC время
void setDateTime(
    uint16_t year,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second
);

// Установить время из Unix timestamp
void setDateTime(time_t timestamp);

// Получить Unix timestamp
time_t getUnixTime();


private:
RTC_DS3231 _rtc;
};
