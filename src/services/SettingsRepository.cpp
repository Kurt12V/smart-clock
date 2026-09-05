// #include "SettingsRepository.h"

// SettingsRepository::SettingsRepository()
//     : _initialized(false)
// {
// }

// bool SettingsRepository::begin()
// {
//     if (_initialized)
//         return true;

//     if (!_preferences.begin(NAMESPACE, false))
//         return false;

//     _initialized = true;

//     return true;
// }

// void SettingsRepository::end()
// {
//     if (!_initialized)
//         return;

//     _preferences.end();

//     _initialized = false;
// }

// bool SettingsRepository::load(Settings& settings)
// {
//     if (!_initialized)
//         return false;

//     // =========================================================
//     // SYSTEM
//     // =========================================================

//     settings.timeZone =
//         _preferences.getString(
//             "timezone",
//             settings.timeZone
//         );

//     settings.use24Hour =
//         _preferences.getBool(
//             "24hour",
//             settings.use24Hour
//         );

//     // =========================================================
//     // DISPLAY
//     // =========================================================

//     settings.brightness =
//         _preferences.getUChar(
//             "brightness",
//             settings.brightness
//         );

//     settings.autoBrightness =
//         _preferences.getBool(
//             "autoBright",
//             settings.autoBrightness
//         );

//     // =========================================================
//     // AUDIO
//     // =========================================================

//     settings.volume =
//         _preferences.getUChar(
//             "volume",
//             settings.volume
//         );

//     settings.soundEnabled =
//         _preferences.getBool(
//             "sound",
//             settings.soundEnabled
//         );

//     settings.voiceEnabled =
//         _preferences.getBool(
//             "voice",
//             settings.voiceEnabled
//         );

//     // =========================================================
//     // AI
//     // =========================================================

//     settings.aiEnabled =
//         _preferences.getBool(
//             "aiEnabled",
//             settings.aiEnabled
//         );

//     settings.llmEnabled =
//         _preferences.getBool(
//             "llmEnabled",
//             settings.llmEnabled
//         );

//     settings.ttsEnabled =
//         _preferences.getBool(
//             "ttsEnabled",
//             settings.ttsEnabled
//         );

//     settings.mlEnabled =
//         _preferences.getBool(
//             "mlEnabled",
//             settings.mlEnabled
//         );

//     settings.llmModel =
//         _preferences.getString(
//             "llmModel",
//             settings.llmModel
//         );

//     settings.ttsModel =
//         _preferences.getString(
//             "ttsModel",
//             settings.ttsModel
//         );

//     // =========================================================
//     // SENSORS
//     // =========================================================

//     settings.sensorsEnabled =
//         _preferences.getBool(
//             "sensors",
//             settings.sensorsEnabled
//         );

//     settings.environmentSensorsEnabled =
//         _preferences.getBool(
//             "envSensors",
//             settings.environmentSensorsEnabled
//         );

//     settings.motionSensorsEnabled =
//         _preferences.getBool(
//             "motionSensors",
//             settings.motionSensorsEnabled
//         );

//     // =========================================================
//     // STORAGE
//     // =========================================================

//     settings.loggingEnabled =
//         _preferences.getBool(
//             "logging",
//             settings.loggingEnabled
//         );

//     settings.sensorLoggingEnabled =
//         _preferences.getBool(
//             "sensorLogs",
//             settings.sensorLoggingEnabled
//         );

//     settings.aiLoggingEnabled =
//         _preferences.getBool(
//             "aiLogs",
//             settings.aiLoggingEnabled
//         );

//     return true;
// }

// bool SettingsRepository::save(const Settings& settings)
// {
//     if (!_initialized)
//         return false;

//     bool success = true;

//     // =========================================================
//     // SYSTEM
//     // =========================================================

//     if (_preferences.putString(
//             "timezone",
//             settings.timeZone
//         ) == 0)
//     {
//         success = false;
//     }

//     if (!_preferences.putBool(
//             "24hour",
//             settings.use24Hour
//         ))
//     {
//         success = false;
//     }

//     // =========================================================
//     // DISPLAY
//     // =========================================================

//     if (!_preferences.putUChar(
//             "brightness",
//             settings.brightness
//         ))
//     {
//         success = false;
//     }

//     if (!_preferences.putBool(
//             "autoBright",
//             settings.autoBrightness
//         ))
//     {
//         success = false;
//     }

//     // =========================================================
//     // AUDIO
//     // =========================================================

//     if (!_preferences.putUChar(
//             "volume",
//             settings.volume
//         ))
//     {
//         success = false;
//     }

//     if (!_preferences.putBool(
//             "sound",
//             settings.soundEnabled
//         ))
//     {
//         success = false;
//     }

//     if (!_preferences.putBool(
//             "voice",
//             settings.voiceEnabled
//         ))
//     {
//         success = false;
//     }

//     // =========================================================
//     // AI
//     // =========================================================

//     if (!_preferences.putBool(
//             "aiEnabled",
//             settings.aiEnabled
//         ))
//     {
//         success = false;
//     }

//     if (!_preferences.putBool(
//             "llmEnabled",
//             settings.llmEnabled
//         ))
//     {
//         success = false;
//     }

//     if (!_preferences.putBool(
//             "ttsEnabled",
//             settings.ttsEnabled
//         ))
//     {
//         success = false;
//     }

//     if (!_preferences.putBool(
//             "mlEnabled",
//             settings.mlEnabled
//         ))
//     {
//         success = false;
//     }

//     if (_preferences.putString(
//             "llmModel",
//             settings.llmModel
//         ) == 0)
//     {
//         success = false;
//     }

//     if (_preferences.putString(
//             "ttsModel",
//             settings.ttsModel
//         ) == 0)
//     {
//         success = false;
//     }

//     // =========================================================
//     // SENSORS
//     // =========================================================

//     if (!_preferences.putBool(
//             "sensors",
//             settings.sensorsEnabled
//         ))
//     {
//         success = false;
//     }

//     if (!_preferences.putBool(
//             "envSensors",
//             settings.environmentSensorsEnabled
//         ))
//     {
//         success = false;
//     }

//     if (!_preferences.putBool(
//             "motionSensors",
//             settings.motionSensorsEnabled
//         ))
//     {
//         success = false;
//     }

//     // =========================================================
//     // STORAGE
//     // =========================================================

//     if (!_preferences.putBool(
//             "logging",
//             settings.loggingEnabled
//         ))
//     {
//         success = false;
//     }

//     if (!_preferences.putBool(
//             "sensorLogs",
//             settings.sensorLoggingEnabled
//         ))
//     {
//         success = false;
//     }

//     if (!_preferences.putBool(
//             "aiLogs",
//             settings.aiLoggingEnabled
//         ))
//     {
//         success = false;
//     }

//     return success;
// }

// bool SettingsRepository::reset()
// {
//     if (!_initialized)
//         return false;

//     return _preferences.clear();
// }

// bool SettingsRepository::isInitialized() const
// {
//     return _initialized;
// }