
// #include "WeatherManager.h"

// // ============================================================
// // CONSTRUCTOR
// // ============================================================

// WeatherManager::WeatherManager()
//     : _latitude(0.0f)
//     , _longitude(0.0f)
//     , _useCoordinates(false)
//     , _useCelsius(true)
//     , _updateInterval(DEFAULT_UPDATE_INTERVAL)
//     , _lastUpdate(0)
//     , _initialized(false)
// {
// }

// // ============================================================
// // BEGIN
// // ============================================================

// bool WeatherManager::begin()
// {
//     if (_initialized)
//     {
//         Serial0.println("[WEATHER] Already initialized");
//         return true;
//     }

//     Serial0.println("[WEATHER] Initializing Weather Manager...");

//     if (_apiKey.isEmpty())
//     {
//         Serial0.println("[WEATHER] WARNING: API key not set");
//     }

//     _initialized = true;

//     Serial0.println("[WEATHER] Weather Manager initialized");

//     return true;
// }

// // ============================================================
// // UPDATE
// // ============================================================

// void WeatherManager::update()
// {
//     if (!_initialized)
//         return;

//     if (_apiKey.isEmpty())
//         return;

//     const uint32_t now = millis();

//     // Handle millis() overflow
//     if (now < _lastUpdate)
//     {
//         _lastUpdate = now;
//     }

//     if (now - _lastUpdate >= _updateInterval)
//     {
//         _lastUpdate = now;

//         Serial0.println("[WEATHER] Updating weather data...");

//         if (fetchCurrentWeather())
//         {
//             Serial0.println("[WEATHER] Current weather updated");
//         }
//         else
//         {
//             Serial0.println("[WEATHER] ERROR: Failed to fetch current weather");
//         }

//         if (fetchForecast())
//         {
//             Serial0.println("[WEATHER] Forecast updated");
//         }
//         else
//         {
//             Serial0.println("[WEATHER] WARNING: Failed to fetch forecast");
//         }
//     }
// }

// // ============================================================
// // CONFIGURATION
// // ============================================================

// void WeatherManager::setApiKey(const String& apiKey)
// {
//     _apiKey = apiKey;
// }

// void WeatherManager::setLocation(const String& city, const String& countryCode)
// {
//     _city = city;
//     _countryCode = countryCode;
//     _useCoordinates = false;

//     Serial0.printf("[WEATHER] Location set: %s, %s\n", city.c_str(), countryCode.c_str());
// }

// void WeatherManager::setLocation(float latitude, float longitude)
// {
//     _latitude = latitude;
//     _longitude = longitude;
//     _useCoordinates = true;

//     Serial0.printf("[WEATHER] Coordinates set: %.4f, %.4f\n", latitude, longitude);
// }

// void WeatherManager::setUpdateInterval(uint32_t intervalMs)
// {
//     if (intervalMs < MIN_UPDATE_INTERVAL)
//     {
//         intervalMs = MIN_UPDATE_INTERVAL;
//     }

//     _updateInterval = intervalMs;
// }

// void WeatherManager::setUnits(bool useCelsius)
// {
//     _useCelsius = useCelsius;
// }

// // ============================================================
// // DATA ACCESS
// // ============================================================

// const WeatherData& WeatherManager::getCurrentWeather() const
// {
//     return _currentWeather;
// }

// const WeatherForecast& WeatherManager::getForecast() const
// {
//     return _forecast;
// }

// bool WeatherManager::isDataValid() const
// {
//     return _currentWeather.valid;
// }

// bool WeatherManager::isDataFresh() const
// {
//     return _currentWeather.isDataFresh();
// }

// time_t WeatherManager::getLastUpdate() const
// {
//     return _currentWeather.lastUpdate;
// }

// // ============================================================
// // MANUAL UPDATE
// // ============================================================

// bool WeatherManager::updateNow()
// {
//     if (!_initialized || _apiKey.isEmpty())
//         return false;

//     _lastUpdate = millis();
//     return fetchCurrentWeather();
// }

// bool WeatherManager::updateForecast()
// {
//     if (!_initialized || _apiKey.isEmpty())
//         return false;

//     return fetchForecast();
// }

// // ============================================================
// // API CALLS
// // ============================================================

// bool WeatherManager::fetchCurrentWeather()
// {
//     const String url = buildWeatherUrl();

//     if (!_http.begin(url))
//     {
//         Serial0.println("[WEATHER] ERROR: Failed to initialize HTTP client");
//         return false;
//     }

//     _http.setTimeout(10000); // 10 second timeout
//     const int httpCode = _http.GET();

//     if (httpCode != HTTP_CODE_OK)
//     {
//         Serial0.printf("[WEATHER] HTTP error: %d\n", httpCode);

//         if (httpCode > 0)
//         {
//             // Server responded with an error
//             const String errorPayload = _http.getString();
//             Serial0.printf("[WEATHER] Error response: %s\n", errorPayload.c_str());
//         }
//         else
//         {
//             // Connection failed
//             Serial0.println("[WEATHER] Connection failed");
//         }

//         _http.end();
//         _currentWeather.invalidate();
//         return false;
//     }

//     const String payload = _http.getString();
//     _http.end();

//     if (payload.isEmpty())
//     {
//         Serial0.println("[WEATHER] ERROR: Empty response");
//         _currentWeather.invalidate();
//         return false;
//     }

//     return parseCurrentWeather(payload);
// }

// bool WeatherManager::fetchForecast()
// {
//     const String url = buildForecastUrl();

//     if (!_http.begin(url))
//     {
//         Serial0.println("[WEATHER] ERROR: Failed to initialize HTTP client");
//         return false;
//     }

//     _http.setTimeout(10000); // 10 second timeout
//     const int httpCode = _http.GET();

//     if (httpCode != HTTP_CODE_OK)
//     {
//         Serial0.printf("[WEATHER] HTTP error: %d\n", httpCode);

//         if (httpCode > 0)
//         {
//             const String errorPayload = _http.getString();
//             Serial0.printf("[WEATHER] Error response: %s\n", errorPayload.c_str());
//         }
//         else
//         {
//             Serial0.println("[WEATHER] Connection failed");
//         }

//         _http.end();
//         _forecast.valid = false;
//         return false;
//     }

//     const String payload = _http.getString();
//     _http.end();

//     if (payload.isEmpty())
//     {
//         Serial0.println("[WEATHER] ERROR: Empty response");
//         _forecast.valid = false;
//         return false;
//     }

//     return parseForecast(payload);
// }

// bool WeatherManager::parseCurrentWeather(const String& json)
// {
//     JsonDocument doc;
//     const DeserializationError error = deserializeJson(doc, json);

//     if (error)
//     {
//         Serial0.printf("[WEATHER] JSON parse error: %s\n", error.c_str());
//         _currentWeather.invalidate();
//         return false;
//     }

//     // Validate required fields exist
//     if (!doc.containsKey("main") || !doc.containsKey("weather"))
//     {
//         Serial0.println("[WEATHER] ERROR: Invalid JSON structure");
//         _currentWeather.invalidate();
//         return false;
//     }

//     // Parse temperature data
//     _currentWeather.temperature = doc["main"]["temp"].as<float>();
//     _currentWeather.feelsLike = doc["main"]["feels_like"].as<float>();
//     _currentWeather.humidity = doc["main"]["humidity"].as<float>();
//     _currentWeather.pressure = doc["main"]["pressure"].as<float>();

//     // Parse wind data (optional)
//     if (doc.containsKey("wind"))
//     {
//         _currentWeather.windSpeed = doc["wind"]["speed"].as<float>();
//         _currentWeather.windDirection = doc["wind"]["deg"].as<float>();
//     }

//     // Parse visibility (optional)
//     if (doc.containsKey("visibility"))
//     {
//         _currentWeather.visibility = doc["visibility"].as<float>() / 1000.0f; // m to km
//     }

//     // Parse clouds (optional)
//     if (doc.containsKey("clouds"))
//     {
//         _currentWeather.cloudiness = doc["clouds"]["all"].as<uint8_t>();
//     }

//     // Parse weather condition
//     if (doc["weather"].is<JsonArray>() && doc["weather"].size() > 0)
//     {
//         const int conditionCode = doc["weather"][0]["id"].as<int>();
//         _currentWeather.condition.type = mapConditionCode(conditionCode);
//         _currentWeather.condition.description = doc["weather"][0]["description"].as<String>();
//     }

//     // Parse sunrise/sunset (optional)
//     if (doc.containsKey("sys"))
//     {
//         _currentWeather.sunrise = doc["sys"]["sunrise"].as<time_t>();
//         _currentWeather.sunset = doc["sys"]["sunset"].as<time_t>();
//         _currentWeather.country = doc["sys"]["country"].as<String>();
//     }

//     // Parse city name
//     if (doc.containsKey("name"))
//     {
//         _currentWeather.city = doc["name"].as<String>();
//     }

//     const time_t now = time(nullptr);
//     if (now > 0)
//     {
//         _currentWeather.lastUpdate = now;
//     }

//     _currentWeather.valid = true;

//     Serial0.printf("[WEATHER] Parsed: %s, %.1f°C\n",
//                    _currentWeather.city.c_str(),
//                    _currentWeather.temperature);

//     return true;
// }

// bool WeatherManager::parseForecast(const String& json)
// {
//     JsonDocument doc;
//     const DeserializationError error = deserializeJson(doc, json);

//     if (error)
//     {
//         Serial0.printf("[WEATHER] JSON parse error: %s\n", error.c_str());
//         _forecast.valid = false;
//         return false;
//     }

//     // Validate JSON structure
//     if (!doc.containsKey("list") || !doc["list"].is<JsonArray>())
//     {
//         Serial0.println("[WEATHER] ERROR: Invalid forecast JSON structure");
//         _forecast.valid = false;
//         return false;
//     }

//     _forecast.clear();

//     JsonArray list = doc["list"].as<JsonArray>();

//     if (list.size() == 0)
//     {
//         Serial0.println("[WEATHER] WARNING: Empty forecast list");
//         _forecast.valid = false;
//         return false;
//     }

//     for (JsonObject item : list)
//     {
//         if (_forecast.count >= WeatherForecast::MAX_FORECAST_DAYS)
//             break;

//         // Validate required fields
//         if (!item.containsKey("dt") || !item.containsKey("main"))
//             continue;

//         ForecastDay& day = _forecast.days[_forecast.count];

//         day.date = item["dt"].as<time_t>();

//         if (day.date == 0)
//             continue;

//         day.tempMin = item["main"]["temp_min"].as<float>();
//         day.tempMax = item["main"]["temp_max"].as<float>();

//         // Parse precipitation (optional)
//         if (item.containsKey("rain") && item["rain"].containsKey("3h"))
//         {
//             day.precipitationAmount = item["rain"]["3h"].as<float>();
//         }
//         else if (item.containsKey("snow") && item["snow"].containsKey("3h"))
//         {
//             day.precipitationAmount = item["snow"]["3h"].as<float>();
//         }

//         // Parse precipitation probability (optional)
//         if (item.containsKey("pop"))
//         {
//             day.precipitationChance = item["pop"].as<float>() * 100.0f;
//         }

//         // Parse weather condition
//         if (item.containsKey("weather") &&
//             item["weather"].is<JsonArray>() &&
//             item["weather"].size() > 0)
//         {
//             const int conditionCode = item["weather"][0]["id"].as<int>();
//             day.condition.type = mapConditionCode(conditionCode);
//             day.condition.description = item["weather"][0]["description"].as<String>();
//         }

//         day.valid = true;
//         _forecast.count++;
//     }

//     const time_t now = time(nullptr);
//     if (now > 0)
//     {
//         _forecast.lastUpdate = now;
//     }

//     _forecast.valid = (_forecast.count > 0);

//     Serial0.printf("[WEATHER] Parsed forecast: %d days\n", _forecast.count);

//     return _forecast.valid;
// }

// // ============================================================
// // HELPERS
// // ============================================================

// WeatherCondition::Type WeatherManager::mapConditionCode(int code) const
// {
//     if (code >= 200 && code < 300)
//         return WeatherCondition::Type::THUNDERSTORM;
//     if (code >= 300 && code < 600)
//         return WeatherCondition::Type::RAIN;
//     if (code >= 600 && code < 700)
//         return WeatherCondition::Type::SNOW;
//     if (code >= 700 && code < 800)
//         return WeatherCondition::Type::FOG;
//     if (code == 800)
//         return WeatherCondition::Type::CLEAR;
//     if (code == 801 || code == 802)
//         return WeatherCondition::Type::PARTLY_CLOUDY;
//     if (code >= 803)
//         return WeatherCondition::Type::CLOUDY;

//     return WeatherCondition::Type::UNKNOWN;
// }

// String WeatherManager::buildWeatherUrl() const
// {
//     String url = String(API_BASE_URL) + "/weather?";

//     if (_useCoordinates)
//     {
//         url += "lat=" + String(_latitude, 4);
//         url += "&lon=" + String(_longitude, 4);
//     }
//     else
//     {
//         url += "q=" + _city;
//         if (!_countryCode.isEmpty())
//         {
//             url += "," + _countryCode;
//         }
//     }

//     url += "&units=" + String(_useCelsius ? "metric" : "imperial");
//     url += "&appid=" + _apiKey;

//     return url;
// }

// String WeatherManager::buildForecastUrl() const
// {
//     String url = String(API_BASE_URL) + "/forecast?";

//     if (_useCoordinates)
//     {
//         url += "lat=" + String(_latitude, 4);
//         url += "&lon=" + String(_longitude, 4);
//     }
//     else
//     {
//         url += "q=" + _city;
//         if (!_countryCode.isEmpty())
//         {
//             url += "," + _countryCode;
//         }
//     }

//     url += "&units=" + String(_useCelsius ? "metric" : "imperial");
//     url += "&cnt=40"; // 5 days
//     url += "&appid=" + _apiKey;

//     return url;
// }
