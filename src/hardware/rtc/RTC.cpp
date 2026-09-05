#include "RTC.h"
#include "../include/Pins.h"
#include <Wire.h>

bool RTC::begin() {

Serial.println("[RTC] Initializing...");

Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

if (!_rtc.begin()) {

    Serial.println("[RTC] ERROR: DS3231 not found");

    return false;
}

Serial.println("[RTC] DS3231 found");

return true;


}

bool RTC::isRunning() {


return !_rtc.lostPower();


}

DateTime RTC::getDateTime() {

return _rtc.now();

}

void RTC::setDateTime(
uint16_t year,
uint8_t month,
uint8_t day,
uint8_t hour,
uint8_t minute,
uint8_t second
) {


_rtc.adjust(
    DateTime(
        year,
        month,
        day,
        hour,
        minute,
        second
    )
);


}

void RTC::setDateTime(time_t timestamp) {


_rtc.adjust(DateTime(timestamp));


}

time_t RTC::getUnixTime() {


return _rtc.now().unixtime();


}
