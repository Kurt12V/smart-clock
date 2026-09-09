#include <Arduino.h>
#include "Config.h"
#include "core/ClockSystem.h"
#include "core/DisplaySystem.h"
#include "managers/SensorsManager.h"

DisplaySystem displaySystem;
ClockSystem clockSystem;
SensorManager sensorManager;

namespace
{
    constexpr uint32_t UI_UPDATE_INTERVAL_MS = 1000;

    bool sensorsReady = false;
    uint32_t lastUiUpdate = 0;

    const char* dayName(Constants::DayOfWeek day)
    {
        switch (day)
        {
            case Constants::DayOfWeek::Monday: return "MONDAY";
            case Constants::DayOfWeek::Tuesday: return "TUESDAY";
            case Constants::DayOfWeek::Wednesday: return "WEDNESDAY";
            case Constants::DayOfWeek::Thursday: return "THURSDAY";
            case Constants::DayOfWeek::Friday: return "FRIDAY";
            case Constants::DayOfWeek::Saturday: return "SATURDAY";
            case Constants::DayOfWeek::Sunday: return "SUNDAY";
        }

        return "--";
    }

    void updateClockScreen()
    {
        ClockScreen& clock = displaySystem.clock();
        const TimeData time = clockSystem.getTimeData();
        const DateData date = clockSystem.getDateData();

        if (time.valid)
        {
            clock.setTime(
                time.getHourTens(),
                time.getHourOnes(),
                time.getMinuteTens(),
                time.getMinuteOnes()
            );
        }

        if (date.valid)
        {
            clock.setDate(
                dayName(date.dayOfWeek),
                date.day,
                date.month,
                date.year
            );
        }

        clock.setSensors(
            sensorManager.getTemperatureC().c_str(),
            sensorManager.getHumidity().c_str(),
            sensorManager.getLight().c_str(),
            "--"
        );

        displaySystem.showClock();
    }
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    displaySystem.begin();

    // =========================
    // ВРЕМЯ
    // 01:23
    // =========================
displaySystem.showSplash();
}

void loop()
{
    if (sensorsReady)
    {
        sensorManager.update();
    }

    if (displaySystem.updateSplash())
    {
        return;
    }

    const uint32_t now = millis();

    if (now - lastUiUpdate >= UI_UPDATE_INTERVAL_MS)
    {
        lastUiUpdate = now;
        updateClockScreen();
    }
}
