#pragma once

#include <Arduino.h>

// ============================================================
// WEATHER DATA MODEL
// ============================================================

struct WeatherCondition
{
    enum class Type : uint8_t
    {
        UNKNOWN,
        CLEAR,
        CLOUDY,
        PARTLY_CLOUDY,
        RAIN,
        SNOW,
        THUNDERSTORM,
        FOG,
        WIND
    };

    Type type = Type::UNKNOWN;
    String description;
};

struct WeatherData
{
    // ========================================================
    // CURRENT CONDITIONS
    // ========================================================

    float temperature = 0.0f;           // °C
    float feelsLike = 0.0f;             // °C
    float humidity = 0.0f;              // %
    float pressure = 0.0f;              // hPa
    float windSpeed = 0.0f;             // m/s
    float windDirection = 0.0f;         // degrees
    float visibility = 0.0f;            // km
    uint8_t cloudiness = 0;             // %
    uint8_t uvIndex = 0;                // 0-11+

    WeatherCondition condition;

    // ========================================================
    // TIME
    // ========================================================

    time_t sunrise = 0;
    time_t sunset = 0;
    time_t lastUpdate = 0;

    // ========================================================
    // LOCATION
    // ========================================================

    String city;
    String country;

    // ========================================================
    // VALIDITY
    // ========================================================

    bool valid = false;

    // ========================================================
    // HELPERS
    // ========================================================

    bool isDataFresh(uint32_t maxAgeSeconds = 1800) const
    {
        if (!valid)
            return false;

        const time_t now = time(nullptr);

        // Check for invalid timestamp
        if (lastUpdate == 0 || now < lastUpdate)
            return false;

        return (now - lastUpdate) < maxAgeSeconds;
    }

    void invalidate()
    {
        valid = false;
    }
};

// ============================================================
// FORECAST DATA
// ============================================================

struct ForecastDay
{
    time_t date = 0;

    float tempMin = 0.0f;               // °C
    float tempMax = 0.0f;               // °C
    float precipitationChance = 0.0f;   // %
    float precipitationAmount = 0.0f;   // mm

    WeatherCondition condition;

    bool valid = false;
};

struct WeatherForecast
{
    static constexpr uint8_t MAX_FORECAST_DAYS = 5;

    ForecastDay days[MAX_FORECAST_DAYS];
    uint8_t count = 0;

    time_t lastUpdate = 0;
    bool valid = false;

    void clear()
    {
        count = 0;
        valid = false;
        for (auto& day : days)
        {
            day.valid = false;
        }
    }
};
