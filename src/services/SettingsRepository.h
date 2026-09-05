// #pragma once

// #include <Arduino.h>
// #include <Preferences.h>

// #include "./models/Settings.h"

// class SettingsRepository
// {
// public:

//     SettingsRepository();

//     bool begin();

//     void end();

//     bool load(Settings& settings);

//     bool save(const Settings& settings);

//     bool reset();

//     bool isInitialized() const;

// private:

//     Preferences _preferences;

//     bool _initialized;

//     static constexpr const char* NAMESPACE = "settings";
// };