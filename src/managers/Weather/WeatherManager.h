#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "../models/Weather.h"

// ============================================================
// WEATHER MANAGER
// ============================================================

class WeatherManager
{
public:

    WeatherManager();

    bool begin();
    void update();

    // ========================================================
    // CONFIGURATION
    // ========================================================

    void setApiKey(const String& apiKey);
    void setLocation(const String& city, const String& countryCode = "");
    void setLocation(float latitude, float longitude);
    void setUpdateInterval(uint32_t intervalMs);
    void setUnits(bool useCelsius = true);

    // ========================================================
    // DATA ACCESS
    // ========================================================

    const WeatherData& getCurrentWeather() const;
    const WeatherForecast& getForecast() const;

    bool isDataValid() const;
    bool isDataFresh() const;

    time_t getLastUpdate() const;

    // ========================================================
    // MANUAL UPDATE
    // ========================================================

    bool updateNow();
    bool updateForecast();

private:

    // ========================================================
    // API CALLS
    // ========================================================

    bool fetchCurrentWeather();
    bool fetchForecast();

    bool parseCurrentWeather(const String& json);
    bool parseForecast(const String& json);

    // ========================================================
    // HELPERS
    // ========================================================

    WeatherCondition::Type mapConditionCode(int code) const;
    String buildWeatherUrl() const;
    String buildForecastUrl() const;

    // ========================================================
    // DATA
    // ========================================================

    String _apiKey;
    String _city;
    String _countryCode;

    float _latitude;
    float _longitude;
    bool _useCoordinates;

    bool _useCelsius;

    uint32_t _updateInterval;
    uint32_t _lastUpdate;

    WeatherData _currentWeather;
    WeatherForecast _forecast;

    HTTPClient _http;

    bool _initialized;

    // ========================================================
    // CONSTANTS
    // ========================================================

    static constexpr uint32_t DEFAULT_UPDATE_INTERVAL = 1800000; // 30 min
    static constexpr uint32_t MIN_UPDATE_INTERVAL = 300000;      // 5 min
    static constexpr const char* API_BASE_URL = "https://api.openweathermap.org/data/2.5";
};
