// #include "SettingsManager.h"

// #include <Preferences.h>
// #include <cstring>

// namespace
// {
//     Preferences preferences;
// }


// // ============================================================
// // CONSTRUCTOR
// // ============================================================

// SettingsManager::SettingsManager()
// {
//     reset();
// }


// // ============================================================
// // BEGIN
// // ============================================================

// void SettingsManager::begin()
// {
//     _initialized = true;

//     if (!load())
//     {
//         reset();
//         save();
//     }

//     validate();
// }


// // ============================================================
// // LOAD
// // ============================================================

// bool SettingsManager::load()
// {
//     if (!preferences.begin(NAMESPACE, true))
//         return false;

//     // --------------------------------------------------------
//     // DISPLAY
//     // --------------------------------------------------------

//     _settings.display.enabled =
//         preferences.getBool(
//             "display_enabled",
//             _settings.display.enabled
//         );

//     _settings.display.brightness =
//         preferences.getUChar(
//             "display_bright",
//             _settings.display.brightness
//         );

//     _settings.display.autoBrightness =
//         preferences.getBool(
//             "display_auto",
//             _settings.display.autoBrightness
//         );

//     _settings.display.showSeconds =
//         preferences.getBool(
//             "show_seconds",
//             _settings.display.showSeconds
//         );

//     _settings.display.showDate =
//         preferences.getBool(
//             "show_date",
//             _settings.display.showDate
//         );

//     _settings.display.showDayOfWeek =
//         preferences.getBool(
//             "show_dow",
//             _settings.display.showDayOfWeek
//         );

//     _settings.display.use24HourFormat =
//         preferences.getBool(
//             "clock_24h",
//             _settings.display.use24HourFormat
//         );

//     _settings.display.nightMode =
//         preferences.getBool(
//             "display_night",
//             _settings.display.nightMode
//         );

//     _settings.display.nightStartHour =
//         preferences.getUChar(
//             "night_start",
//             _settings.display.nightStartHour
//         );

//     _settings.display.nightEndHour =
//         preferences.getUChar(
//             "night_end",
//             _settings.display.nightEndHour
//         );

//     _settings.display.nightBrightness =
//         preferences.getUChar(
//             "night_bright",
//             _settings.display.nightBrightness
//         );

//     _settings.display.refreshRate =
//         preferences.getUShort(
//             "refresh_rate",
//             _settings.display.refreshRate
//         );


//     // --------------------------------------------------------
//     // MATRIX
//     // --------------------------------------------------------

//     _settings.matrix.enabled =
//         preferences.getBool(
//             "matrix_enabled",
//             _settings.matrix.enabled
//         );

//     _settings.matrix.brightness =
//         preferences.getUChar(
//             "matrix_bright",
//             _settings.matrix.brightness
//         );

//     _settings.matrix.autoBrightness =
//         preferences.getBool(
//             "matrix_auto",
//             _settings.matrix.autoBrightness
//         );

//     _settings.matrix.nightBrightness =
//         preferences.getUChar(
//             "matrix_night",
//             _settings.matrix.nightBrightness
//         );

//     _settings.matrix.nightMode =
//         preferences.getBool(
//             "matrix_nightmode",
//             _settings.matrix.nightMode
//         );

//     _settings.matrix.idleEnabled =
//         preferences.getBool(
//             "matrix_idle",
//             _settings.matrix.idleEnabled
//         );

//     _settings.matrix.defaultAnimation =
//         preferences.getUChar(
//             "matrix_anim",
//             _settings.matrix.defaultAnimation
//         );

//     _settings.matrix.animationSpeed =
//         preferences.getUChar(
//             "matrix_speed",
//             _settings.matrix.animationSpeed
//         );

//     _settings.matrix.colorR =
//         preferences.getUChar(
//             "matrix_r",
//             _settings.matrix.colorR
//         );

//     _settings.matrix.colorG =
//         preferences.getUChar(
//             "matrix_g",
//             _settings.matrix.colorG
//         );

//     _settings.matrix.colorB =
//         preferences.getUChar(
//             "matrix_b",
//             _settings.matrix.colorB
//         );

//     _settings.matrix.autoColor =
//         preferences.getBool(
//             "matrix_autocolor",
//             _settings.matrix.autoColor
//         );

//     _settings.matrix.showClock =
//         preferences.getBool(
//             "matrix_clock",
//             _settings.matrix.showClock
//         );

//     _settings.matrix.showNotifications =
//         preferences.getBool(
//             "matrix_notify",
//             _settings.matrix.showNotifications
//         );


//     // --------------------------------------------------------
//     // COB LED
//     // --------------------------------------------------------

//     _settings.cobLed.enabled =
//         preferences.getBool(
//             "cob_enabled",
//             _settings.cobLed.enabled
//         );

//     _settings.cobLed.brightness =
//         preferences.getUChar(
//             "cob_bright",
//             _settings.cobLed.brightness
//         );

//     _settings.cobLed.led1Enabled =
//         preferences.getBool(
//             "cob1_enabled",
//             _settings.cobLed.led1Enabled
//         );

//     _settings.cobLed.led1Brightness =
//         preferences.getUChar(
//             "cob1_bright",
//             _settings.cobLed.led1Brightness
//         );

//     _settings.cobLed.led2Enabled =
//         preferences.getBool(
//             "cob2_enabled",
//             _settings.cobLed.led2Enabled
//         );

//     _settings.cobLed.led2Brightness =
//         preferences.getUChar(
//             "cob2_bright",
//             _settings.cobLed.led2Brightness
//         );

//     _settings.cobLed.autoBrightness =
//         preferences.getBool(
//             "cob_auto",
//             _settings.cobLed.autoBrightness
//         );

//     _settings.cobLed.nightBrightness =
//         preferences.getUChar(
//             "cob_night",
//             _settings.cobLed.nightBrightness
//         );

//     _settings.cobLed.fadeEnabled =
//         preferences.getBool(
//             "cob_fade",
//             _settings.cobLed.fadeEnabled
//         );

//     _settings.cobLed.fadeTime =
//         preferences.getUShort(
//             "cob_fadetime",
//             _settings.cobLed.fadeTime
//         );


//     // --------------------------------------------------------
//     // AUDIO
//     // --------------------------------------------------------

//     _settings.audio.enabled =
//         preferences.getBool(
//             "audio_enabled",
//             _settings.audio.enabled
//         );

//     _settings.audio.volume =
//         preferences.getUChar(
//             "volume",
//             _settings.audio.volume
//         );

//     _settings.audio.maxVolume =
//         preferences.getUChar(
//             "max_volume",
//             _settings.audio.maxVolume
//         );

//     _settings.audio.interfaceSounds =
//         preferences.getBool(
//             "ui_sounds",
//             _settings.audio.interfaceSounds
//         );

//     _settings.audio.notificationSounds =
//         preferences.getBool(
//             "notify_sound",
//             _settings.audio.notificationSounds
//         );

//     _settings.audio.alarmSound =
//         preferences.getBool(
//             "alarm_sound",
//             _settings.audio.alarmSound
//         );

//     _settings.audio.timerSound =
//         preferences.getBool(
//             "timer_sound",
//             _settings.audio.timerSound
//         );

//     _settings.audio.startupSound =
//         preferences.getBool(
//             "startup_sound",
//             _settings.audio.startupSound
//         );

//     _settings.audio.balance =
//         preferences.getChar(
//             "balance",
//             _settings.audio.balance
//         );

//     _settings.audio.equalizerEnabled =
//         preferences.getBool(
//             "eq_enabled",
//             _settings.audio.equalizerEnabled
//         );

//     _settings.audio.bass =
//         preferences.getChar(
//             "bass",
//             _settings.audio.bass
//         );

//     _settings.audio.mid =
//         preferences.getChar(
//             "mid",
//             _settings.audio.mid
//         );

//     _settings.audio.treble =
//         preferences.getChar(
//             "treble",
//             _settings.audio.treble
//         );


//     // --------------------------------------------------------
//     // MICROPHONE
//     // --------------------------------------------------------

//     _settings.microphone.enabled =
//         preferences.getBool(
//             "mic_enabled",
//             _settings.microphone.enabled
//         );

//     _settings.microphone.autoGain =
//         preferences.getBool(
//             "mic_autogain",
//             _settings.microphone.autoGain
//         );

//     _settings.microphone.gain =
//         preferences.getUChar(
//             "mic_gain",
//             _settings.microphone.gain
//         );

//     _settings.microphone.sensitivity =
//         preferences.getUChar(
//             "mic_sens",
//             _settings.microphone.sensitivity
//         );

//     _settings.microphone.noiseReduction =
//         preferences.getBool(
//             "mic_noise",
//             _settings.microphone.noiseReduction
//         );

//     _settings.microphone.voiceDetection =
//         preferences.getBool(
//             "voice_detect",
//             _settings.microphone.voiceDetection
//         );

//     _settings.microphone.wakeWordEnabled =
//         preferences.getBool(
//             "wake_word",
//             _settings.microphone.wakeWordEnabled
//         );


//     // --------------------------------------------------------
//     // ENVIRONMENT
//     // --------------------------------------------------------

//     _settings.environment.temperatureEnabled =
//         preferences.getBool(
//             "temp_enabled",
//             _settings.environment.temperatureEnabled
//         );

//     _settings.environment.humidityEnabled =
//         preferences.getBool(
//             "hum_enabled",
//             _settings.environment.humidityEnabled
//         );

//     _settings.environment.celsius =
//         preferences.getBool(
//             "celsius",
//             _settings.environment.celsius
//         );

//     _settings.environment.temperatureOffset =
//         preferences.getFloat(
//             "temp_offset",
//             _settings.environment.temperatureOffset
//         );

//     _settings.environment.humidityOffset =
//         preferences.getFloat(
//             "hum_offset",
//             _settings.environment.humidityOffset
//         );

//     _settings.environment.showTemperature =
//         preferences.getBool(
//             "show_temp",
//             _settings.environment.showTemperature
//         );

//     _settings.environment.showHumidity =
//         preferences.getBool(
//             "show_hum",
//             _settings.environment.showHumidity
//         );


//     // --------------------------------------------------------
//     // LIGHT SENSOR
//     // --------------------------------------------------------

//     _settings.lightSensor.enabled =
//         preferences.getBool(
//             "light_enabled",
//             _settings.lightSensor.enabled
//         );

//     _settings.lightSensor.autoBrightness =
//         preferences.getBool(
//             "light_auto",
//             _settings.lightSensor.autoBrightness
//         );

//     _settings.lightSensor.minBrightness =
//         preferences.getUChar(
//             "light_min",
//             _settings.lightSensor.minBrightness
//         );

//     _settings.lightSensor.maxBrightness =
//         preferences.getUChar(
//             "light_max",
//             _settings.lightSensor.maxBrightness
//         );

//     _settings.lightSensor.darkThreshold =
//         preferences.getUShort(
//             "light_dark",
//             _settings.lightSensor.darkThreshold
//         );

//     _settings.lightSensor.brightThreshold =
//         preferences.getUShort(
//             "light_bright",
//             _settings.lightSensor.brightThreshold
//         );


//     // --------------------------------------------------------
//     // DISTANCE
//     // --------------------------------------------------------

//     _settings.distanceSensor.enabled =
//         preferences.getBool(
//             "dist_enabled",
//             _settings.distanceSensor.enabled
//         );

//     _settings.distanceSensor.wakeOnPresence =
//         preferences.getBool(
//             "wake_presence",
//             _settings.distanceSensor.wakeOnPresence
//         );

//     _settings.distanceSensor.interactionEnabled =
//         preferences.getBool(
//             "dist_interaction",
//             _settings.distanceSensor.interactionEnabled
//         );

//     _settings.distanceSensor.presenceDistance =
//         preferences.getUShort(
//             "presence_dist",
//             _settings.distanceSensor.presenceDistance
//         );

//     _settings.distanceSensor.absenceTimeout =
//         preferences.getUShort(
//             "absence_time",
//             _settings.distanceSensor.absenceTimeout
//         );


//     // --------------------------------------------------------
//     // CLOCK
//     // --------------------------------------------------------

//     _settings.clock.autoSync =
//         preferences.getBool(
//             "clock_sync",
//             _settings.clock.autoSync
//         );

//     _settings.clock.useRTC =
//         preferences.getBool(
//             "clock_rtc",
//             _settings.clock.useRTC
//         );

//     _settings.clock.useNTP =
//         preferences.getBool(
//             "clock_ntp",
//             _settings.clock.useNTP
//         );

//     _settings.clock.use24HourFormat =
//         preferences.getBool(
//             "clock_24h2",
//             _settings.clock.use24HourFormat
//         );

//     _settings.clock.showSeconds =
//         preferences.getBool(
//             "clock_seconds",
//             _settings.clock.showSeconds
//         );

//     _settings.clock.showDate =
//         preferences.getBool(
//             "clock_date",
//             _settings.clock.showDate
//         );

//     _settings.clock.showDayOfWeek =
//         preferences.getBool(
//             "clock_dow",
//             _settings.clock.showDayOfWeek
//         );

//     _settings.clock.showYear =
//         preferences.getBool(
//             "clock_year",
//             _settings.clock.showYear
//         );

//     String timezone =
//         preferences.getString(
//             "timezone",
//             _settings.clock.timezone
//         );

//     strncpy(
//         _settings.clock.timezone,
//         timezone.c_str(),
//         sizeof(_settings.clock.timezone) - 1
//     );

//     _settings.clock.timezone[
//         sizeof(_settings.clock.timezone) - 1
//     ] = '\0';


//     // --------------------------------------------------------
//     // NOTIFICATIONS
//     // --------------------------------------------------------

//     _settings.notifications.enabled =
//         preferences.getBool(
//             "notify_enabled",
//             _settings.notifications.enabled
//         );

//     _settings.notifications.displayEnabled =
//         preferences.getBool(
//             "notify_display",
//             _settings.notifications.displayEnabled
//         );

//     _settings.notifications.matrixEnabled =
//         preferences.getBool(
//             "notify_matrix",
//             _settings.notifications.matrixEnabled
//         );

//     _settings.notifications.soundEnabled =
//         preferences.getBool(
//             "notify_sound2",
//             _settings.notifications.soundEnabled
//         );

//     _settings.notifications.systemNotifications =
//         preferences.getBool(
//             "notify_system",
//             _settings.notifications.systemNotifications
//         );

//     _settings.notifications.sensorNotifications =
//         preferences.getBool(
//             "notify_sensor",
//             _settings.notifications.sensorNotifications
//         );

//     _settings.notifications.wifiNotifications =
//         preferences.getBool(
//             "notify_wifi",
//             _settings.notifications.wifiNotifications
//         );

//     _settings.notifications.duration =
//         preferences.getUShort(
//             "notify_duration",
//             _settings.notifications.duration
//         );


//     // --------------------------------------------------------
//     // ALARM
//     // --------------------------------------------------------

//     _settings.alarm.enabled =
//         preferences.getBool(
//             "alarm_enabled",
//             _settings.alarm.enabled
//         );

//     _settings.alarm.hour =
//         preferences.getUChar(
//             "alarm_hour",
//             _settings.alarm.hour
//         );

//     _settings.alarm.minute =
//         preferences.getUChar(
//             "alarm_min",
//             _settings.alarm.minute
//         );

//     _settings.alarm.days =
//         preferences.getUChar(
//             "alarm_days",
//             _settings.alarm.days
//         );

//     _settings.alarm.volume =
//         preferences.getUChar(
//             "alarm_volume",
//             _settings.alarm.volume
//         );

//     _settings.alarm.fadeIn =
//         preferences.getBool(
//             "alarm_fade",
//             _settings.alarm.fadeIn
//         );

//     _settings.alarm.fadeTime =
//         preferences.getUShort(
//             "alarm_fadetime",
//             _settings.alarm.fadeTime
//         );

//     _settings.alarm.useLight =
//         preferences.getBool(
//             "alarm_light",
//             _settings.alarm.useLight
//         );

//     _settings.alarm.useMatrix =
//         preferences.getBool(
//             "alarm_matrix",
//             _settings.alarm.useMatrix
//         );

//     _settings.alarm.useSound =
//         preferences.getBool(
//             "alarm_sound2",
//             _settings.alarm.useSound
//         );

//     _settings.alarm.snoozeEnabled =
//         preferences.getBool(
//             "snooze",
//             _settings.alarm.snoozeEnabled
//         );

//     _settings.alarm.snoozeMinutes =
//         preferences.getUChar(
//             "snooze_min",
//             _settings.alarm.snoozeMinutes
//         );


//     // --------------------------------------------------------
//     // TIMER
//     // --------------------------------------------------------

//     _settings.timer.soundEnabled =
//         preferences.getBool(
//             "timer_sound2",
//             _settings.timer.soundEnabled
//         );

//     _settings.timer.displayEnabled =
//         preferences.getBool(
//             "timer_display",
//             _settings.timer.displayEnabled
//         );

//     _settings.timer.matrixEnabled =
//         preferences.getBool(
//             "timer_matrix",
//             _settings.timer.matrixEnabled
//         );

//     _settings.timer.volume =
//         preferences.getUChar(
//             "timer_volume",
//             _settings.timer.volume
//         );

//     _settings.timer.lightEnabled =
//         preferences.getBool(
//             "timer_light",
//             _settings.timer.lightEnabled
//         );


//     // --------------------------------------------------------
//     // WIFI
//     // --------------------------------------------------------

//     _settings.wifi.enabled =
//         preferences.getBool(
//             "wifi_enabled",
//             _settings.wifi.enabled
//         );

//     _settings.wifi.autoConnect =
//         preferences.getBool(
//             "wifi_auto",
//             _settings.wifi.autoConnect
//         );

//     _settings.wifi.syncTime =
//         preferences.getBool(
//             "wifi_sync",
//             _settings.wifi.syncTime
//         );

//     _settings.wifi.connectOnBoot =
//         preferences.getBool(
//             "wifi_boot",
//             _settings.wifi.connectOnBoot
//         );

//     _settings.wifi.showStatus =
//         preferences.getBool(
//             "wifi_status",
//             _settings.wifi.showStatus
//         );


//     // --------------------------------------------------------
//     // POWER
//     // --------------------------------------------------------

//     _settings.power.sleepEnabled =
//         preferences.getBool(
//             "sleep_enabled",
//             _settings.power.sleepEnabled
//         );

//     _settings.power.sleepMinutes =
//         preferences.getUShort(
//             "sleep_minutes",
//             _settings.power.sleepMinutes
//         );

//     _settings.power.displaySleep =
//         preferences.getBool(
//             "sleep_display",
//             _settings.power.displaySleep
//         );

//     _settings.power.matrixSleep =
//         preferences.getBool(
//             "sleep_matrix",
//             _settings.power.matrixSleep
//         );

//     _settings.power.cobSleep =
//         preferences.getBool(
//             "sleep_cob",
//             _settings.power.cobSleep
//         );

//     _settings.power.audioSleep =
//         preferences.getBool(
//             "sleep_audio",
//             _settings.power.audioSleep
//         );

//     _settings.power.wakeOnMotion =
//         preferences.getBool(
//             "wake_motion",
//             _settings.power.wakeOnMotion
//         );

//     _settings.power.wakeOnButton =
//         preferences.getBool(
//             "wake_button",
//             _settings.power.wakeOnButton
//         );

//     _settings.power.nightMode =
//         preferences.getBool(
//             "power_night",
//             _settings.power.nightMode
//         );


//     // --------------------------------------------------------
//     // UI
//     // --------------------------------------------------------

//     _settings.ui.theme =
//         preferences.getUChar(
//             "theme",
//             _settings.ui.theme
//         );

//     _settings.ui.animations =
//         preferences.getBool(
//             "animations",
//             _settings.ui.animations
//         );

//     _settings.ui.animationSpeed =
//         preferences.getUChar(
//             "ui_anim_speed",
//             _settings.ui.animationSpeed
//         );

//     _settings.ui.menuTimeout =
//         preferences.getUShort(
//             "menu_timeout",
//             _settings.ui.menuTimeout
//         );

//     _settings.ui.hints =
//         preferences.getBool(
//             "hints",
//             _settings.ui.hints
//         );

//     _settings.ui.sounds =
//         preferences.getBool(
//             "ui_sound2",
//             _settings.ui.sounds
//         );

//     _settings.ui.vibration =
//         preferences.getBool(
//             "vibration",
//             _settings.ui.vibration
//         );


//     // --------------------------------------------------------
//     // ENCODER
//     // --------------------------------------------------------

//     _settings.encoder.inverted =
//         preferences.getBool(
//             "enc_inverted",
//             _settings.encoder.inverted
//         );

//     _settings.encoder.step =
//         preferences.getUChar(
//             "enc_step",
//             _settings.encoder.step
//         );

//     _settings.encoder.acceleration =
//         preferences.getBool(
//             "enc_accel",
//             _settings.encoder.acceleration
//         );

//     _settings.encoder.accelerationSpeed =
//         preferences.getUChar(
//             "enc_accel_speed",
//             _settings.encoder.accelerationSpeed
//         );

//     _settings.encoder.buttonEnabled =
//         preferences.getBool(
//             "enc_button",
//             _settings.encoder.buttonEnabled
//         );


//     // --------------------------------------------------------
//     // STORAGE
//     // --------------------------------------------------------

//     _settings.storage.autoSave =
//         preferences.getBool(
//             "autosave",
//             _settings.storage.autoSave
//         );

//     _settings.storage.saveInterval =
//         preferences.getULong(
//             "save_interval",
//             _settings.storage.saveInterval
//         );

//     _settings.storage.backupEnabled =
//         preferences.getBool(
//             "backup",
//             _settings.storage.backupEnabled
//         );


//     // --------------------------------------------------------
//     // SYSTEM
//     // --------------------------------------------------------

//     _settings.system.autoStart =
//         preferences.getBool(
//             "autostart",
//             _settings.system.autoStart
//         );

//     _settings.system.splashEnabled =
//         preferences.getBool(
//             "splash",
//             _settings.system.splashEnabled
//         );

//     _settings.system.splashDuration =
//         preferences.getUShort(
//             "splash_time",
//             _settings.system.splashDuration
//         );

//     _settings.system.loggingEnabled =
//         preferences.getBool(
//             "logging",
//             _settings.system.loggingEnabled
//         );

//     _settings.system.debugEnabled =
//         preferences.getBool(
//             "debug",
//             _settings.system.debugEnabled
//         );

//     _settings.system.language =
//         preferences.getUChar(
//             "language",
//             _settings.system.language
//         );

//     preferences.end();

//     validate();

//     return true;
// }


// // ============================================================
// // SAVE
// // ============================================================

// bool SettingsManager::save()
// {
//     validate();

//     if (!preferences.begin(NAMESPACE, false))
//         return false;


//     // DISPLAY
//     preferences.putBool(
//         "display_enabled",
//         _settings.display.enabled
//     );

//     preferences.putUChar(
//         "display_bright",
//         _settings.display.brightness
//     );

//     preferences.putBool(
//         "display_auto",
//         _settings.display.autoBrightness
//     );

//     preferences.putBool(
//         "show_seconds",
//         _settings.display.showSeconds
//     );

//     preferences.putBool(
//         "show_date",
//         _settings.display.showDate
//     );

//     preferences.putBool(
//         "show_dow",
//         _settings.display.showDayOfWeek
//     );

//     preferences.putBool(
//         "clock_24h",
//         _settings.display.use24HourFormat
//     );

//     preferences.putBool(
//         "display_night",
//         _settings.display.nightMode
//     );

//     preferences.putUChar(
//         "night_start",
//         _settings.display.nightStartHour
//     );

//     preferences.putUChar(
//         "night_end",
//         _settings.display.nightEndHour
//     );

//     preferences.putUChar(
//         "night_bright",
//         _settings.display.nightBrightness
//     );

//     preferences.putUShort(
//         "refresh_rate",
//         _settings.display.refreshRate
//     );


//     // MATRIX
//     preferences.putBool(
//         "matrix_enabled",
//         _settings.matrix.enabled
//     );

//     preferences.putUChar(
//         "matrix_bright",
//         _settings.matrix.brightness
//     );

//     preferences.putBool(
//         "matrix_auto",
//         _settings.matrix.autoBrightness
//     );

//     preferences.putUChar(
//         "matrix_night",
//         _settings.matrix.nightBrightness
//     );

//     preferences.putBool(
//         "matrix_nightmode",
//         _settings.matrix.nightMode
//     );

//     preferences.putBool(
//         "matrix_idle",
//         _settings.matrix.idleEnabled
//     );

//     preferences.putUChar(
//         "matrix_anim",
//         _settings.matrix.defaultAnimation
//     );

//     preferences.putUChar(
//         "matrix_speed",
//         _settings.matrix.animationSpeed
//     );

//     preferences.putUChar(
//         "matrix_r",
//         _settings.matrix.colorR
//     );

//     preferences.putUChar(
//         "matrix_g",
//         _settings.matrix.colorG
//     );

//     preferences.putUChar(
//         "matrix_b",
//         _settings.matrix.colorB
//     );

//     preferences.putBool(
//         "matrix_autocolor",
//         _settings.matrix.autoColor
//     );

//     preferences.putBool(
//         "matrix_clock",
//         _settings.matrix.showClock
//     );

//     preferences.putBool(
//         "matrix_notify",
//         _settings.matrix.showNotifications
//     );


//     // COB
//     preferences.putBool(
//         "cob_enabled",
//         _settings.cobLed.enabled
//     );

//     preferences.putUChar(
//         "cob_bright",
//         _settings.cobLed.brightness
//     );

//     preferences.putBool(
//         "cob1_enabled",
//         _settings.cobLed.led1Enabled
//     );

//     preferences.putUChar(
//         "cob1_bright",
//         _settings.cobLed.led1Brightness
//     );

//     preferences.putBool(
//         "cob2_enabled",
//         _settings.cobLed.led2Enabled
//     );

//     preferences.putUChar(
//         "cob2_bright",
//         _settings.cobLed.led2Brightness
//     );

//     preferences.putBool(
//         "cob_auto",
//         _settings.cobLed.autoBrightness
//     );

//     preferences.putUChar(
//         "cob_night",
//         _settings.cobLed.nightBrightness
//     );

//     preferences.putBool(
//         "cob_fade",
//         _settings.cobLed.fadeEnabled
//     );

//     preferences.putUShort(
//         "cob_fadetime",
//         _settings.cobLed.fadeTime
//     );


//     // AUDIO
//     preferences.putBool(
//         "audio_enabled",
//         _settings.audio.enabled
//     );

//     preferences.putUChar(
//         "volume",
//         _settings.audio.volume
//     );

//     preferences.putUChar(
//         "max_volume",
//         _settings.audio.maxVolume
//     );

//     preferences.putBool(
//         "ui_sounds",
//         _settings.audio.interfaceSounds
//     );

//     preferences.putBool(
//         "notify_sound",
//         _settings.audio.notificationSounds
//     );

//     preferences.putBool(
//         "alarm_sound",
//         _settings.audio.alarmSound
//     );

//     preferences.putBool(
//         "timer_sound",
//         _settings.audio.timerSound
//     );

//     preferences.putBool(
//         "startup_sound",
//         _settings.audio.startupSound
//     );

//     preferences.putChar(
//         "balance",
//         _settings.audio.balance
//     );

//     preferences.putBool(
//         "eq_enabled",
//         _settings.audio.equalizerEnabled
//     );

//     preferences.putChar(
//         "bass",
//         _settings.audio.bass
//     );

//     preferences.putChar(
//         "mid",
//         _settings.audio.mid
//     );

//     preferences.putChar(
//         "treble",
//         _settings.audio.treble
//     );


//     // MICROPHONE
//     preferences.putBool(
//         "mic_enabled",
//         _settings.microphone.enabled
//     );

//     preferences.putBool(
//         "mic_autogain",
//         _settings.microphone.autoGain
//     );

//     preferences.putUChar(
//         "mic_gain",
//         _settings.microphone.gain
//     );

//     preferences.putUChar(
//         "mic_sens",
//         _settings.microphone.sensitivity
//     );

//     preferences.putBool(
//         "mic_noise",
//         _settings.microphone.noiseReduction
//     );

//     preferences.putBool(
//         "voice_detect",
//         _settings.microphone.voiceDetection
//     );

//     preferences.putBool(
//         "wake_word",
//         _settings.microphone.wakeWordEnabled
//     );


//     // ENVIRONMENT
//     preferences.putBool(
//         "temp_enabled",
//         _settings.environment.temperatureEnabled
//     );

//     preferences.putBool(
//         "hum_enabled",
//         _settings.environment.humidityEnabled
//     );

//     preferences.putBool(
//         "celsius",
//         _settings.environment.celsius
//     );

//     preferences.putFloat(
//         "temp_offset",
//         _settings.environment.temperatureOffset
//     );

//     preferences.putFloat(
//         "hum_offset",
//         _settings.environment.humidityOffset
//     );

//     preferences.putBool(
//         "show_temp",
//         _settings.environment.showTemperature
//     );

//     preferences.putBool(
//         "show_hum",
//         _settings.environment.showHumidity
//     );


//     // LIGHT SENSOR
//     preferences.putBool(
//         "light_enabled",
//         _settings.lightSensor.enabled
//     );

//     preferences.putBool(
//         "light_auto",
//         _settings.lightSensor.autoBrightness
//     );

//     preferences.putUChar(
//         "light_min",
//         _settings.lightSensor.minBrightness
//     );

//     preferences.putUChar(
//         "light_max",
//         _settings.lightSensor.maxBrightness
//     );

//     preferences.putUShort(
//         "light_dark",
//         _settings.lightSensor.darkThreshold
//     );

//     preferences.putUShort(
//         "light_bright",
//         _settings.lightSensor.brightThreshold
//     );


//     // DISTANCE
//     preferences.putBool(
//         "dist_enabled",
//         _settings.distanceSensor.enabled
//     );

//     preferences.putBool(
//         "wake_presence",
//         _settings.distanceSensor.wakeOnPresence
//     );

//     preferences.putBool(
//         "dist_interaction",
//         _settings.distanceSensor.interactionEnabled
//     );

//     preferences.putUShort(
//         "presence_dist",
//         _settings.distanceSensor.presenceDistance
//     );

//     preferences.putUShort(
//         "absence_time",
//         _settings.distanceSensor.absenceTimeout
//     );


//     // CLOCK
//     preferences.putBool(
//         "clock_sync",
//         _settings.clock.autoSync
//     );

//     preferences.putBool(
//         "clock_rtc",
//         _settings.clock.useRTC
//     );

//     preferences.putBool(
//         "clock_ntp",
//         _settings.clock.useNTP
//     );

//     preferences.putBool(
//         "clock_24h2",
//         _settings.clock.use24HourFormat
//     );

//     preferences.putBool(
//         "clock_seconds",
//         _settings.clock.showSeconds
//     );

//     preferences.putBool(
//         "clock_date",
//         _settings.clock.showDate
//     );

//     preferences.putBool(
//         "clock_dow",
//         _settings.clock.showDayOfWeek
//     );

//     preferences.putBool(
//         "clock_year",
//         _settings.clock.showYear
//     );

//     preferences.putString(
//         "timezone",
//         _settings.clock.timezone
//     );


//     // NOTIFICATIONS
//     preferences.putBool(
//         "notify_enabled",
//         _settings.notifications.enabled
//     );

//     preferences.putBool(
//         "notify_display",
//         _settings.notifications.displayEnabled
//     );

//     preferences.putBool(
//         "notify_matrix",
//         _settings.notifications.matrixEnabled
//     );

//     preferences.putBool(
//         "notify_sound2",
//         _settings.notifications.soundEnabled
//     );

//     preferences.putBool(
//         "notify_system",
//         _settings.notifications.systemNotifications
//     );

//     preferences.putBool(
//         "notify_sensor",
//         _settings.notifications.sensorNotifications
//     );

//     preferences.putBool(
//         "notify_wifi",
//         _settings.notifications.wifiNotifications
//     );

//     preferences.putUShort(
//         "notify_duration",
//         _settings.notifications.duration
//     );


//     // ALARM
//     preferences.putBool(
//         "alarm_enabled",
//         _settings.alarm.enabled
//     );

//     preferences.putUChar(
//         "alarm_hour",
//         _settings.alarm.hour
//     );

//     preferences.putUChar(
//         "alarm_min",
//         _settings.alarm.minute
//     );

//     preferences.putUChar(
//         "alarm_days",
//         _settings.alarm.days
//     );

//     preferences.putUChar(
//         "alarm_volume",
//         _settings.alarm.volume
//     );

//     preferences.putBool(
//         "alarm_fade",
//         _settings.alarm.fadeIn
//     );

//     preferences.putUShort(
//         "alarm_fadetime",
//         _settings.alarm.fadeTime
//     );

//     preferences.putBool(
//         "alarm_light",
//         _settings.alarm.useLight
//     );

//     preferences.putBool(
//         "alarm_matrix",
//         _settings.alarm.useMatrix
//     );

//     preferences.putBool(
//         "alarm_sound2",
//         _settings.alarm.useSound
//     );

//     preferences.putBool(
//         "snooze",
//         _settings.alarm.snoozeEnabled
//     );

//     preferences.putUChar(
//         "snooze_min",
//         _settings.alarm.snoozeMinutes
//     );


//     // TIMER
//     preferences.putBool(
//         "timer_sound2",
//         _settings.timer.soundEnabled
//     );

//     preferences.putBool(
//         "timer_display",
//         _settings.timer.displayEnabled
//     );

//     preferences.putBool(
//         "timer_matrix",
//         _settings.timer.matrixEnabled
//     );

//     preferences.putUChar(
//         "timer_volume",
//         _settings.timer.volume
//     );

//     preferences.putBool(
//         "timer_light",
//         _settings.timer.lightEnabled
//     );


//     // WIFI
//     preferences.putBool(
//         "wifi_enabled",
//         _settings.wifi.enabled
//     );

//     preferences.putBool(
//         "wifi_auto",
//         _settings.wifi.autoConnect
//     );

//     preferences.putBool(
//         "wifi_sync",
//         _settings.wifi.syncTime
//     );

//     preferences.putBool(
//         "wifi_boot",
//         _settings.wifi.connectOnBoot
//     );

//     preferences.putBool(
//         "wifi_status",
//         _settings.wifi.showStatus
//     );


//     // POWER
//     preferences.putBool(
//         "sleep_enabled",
//         _settings.power.sleepEnabled
//     );

//     preferences.putUShort(
//         "sleep_minutes",
//         _settings.power.sleepMinutes
//     );

//     preferences.putBool(
//         "sleep_display",
//         _settings.power.displaySleep
//     );

//     preferences.putBool(
//         "sleep_matrix",
//         _settings.power.matrixSleep
//     );

//     preferences.putBool(
//         "sleep_cob",
//         _settings.power.cobSleep
//     );

//     preferences.putBool(
//         "sleep_audio",
//         _settings.power.audioSleep
//     );

//     preferences.putBool(
//         "wake_motion",
//         _settings.power.wakeOnMotion
//     );

//     preferences.putBool(
//         "wake_button",
//         _settings.power.wakeOnButton
//     );

//     preferences.putBool(
//         "power_night",
//         _settings.power.nightMode
//     );


//     // UI
//     preferences.putUChar(
//         "theme",
//         _settings.ui.theme
//     );

//     preferences.putBool(
//         "animations",
//         _settings.ui.animations
//     );

//     preferences.putUChar(
//         "ui_anim_speed",
//         _settings.ui.animationSpeed
//     );

//     preferences.putUShort(
//         "menu_timeout",
//         _settings.ui.menuTimeout
//     );

//     preferences.putBool(
//         "hints",
//         _settings.ui.hints
//     );

//     preferences.putBool(
//         "ui_sound2",
//         _settings.ui.sounds
//     );

//     preferences.putBool(
//         "vibration",
//         _settings.ui.vibration
//     );


//     // ENCODER
//     preferences.putBool(
//         "enc_inverted",
//         _settings.encoder.inverted
//     );

//     preferences.putUChar(
//         "enc_step",
//         _settings.encoder.step
//     );

//     preferences.putBool(
//         "enc_accel",
//         _settings.encoder.acceleration
//     );

//     preferences.putUChar(
//         "enc_accel_speed",
//         _settings.encoder.accelerationSpeed
//     );

//     preferences.putBool(
//         "enc_button",
//         _settings.encoder.buttonEnabled
//     );


//     // STORAGE
//     preferences.putBool(
//         "autosave",
//         _settings.storage.autoSave
//     );

//     preferences.putULong(
//         "save_interval",
//         _settings.storage.saveInterval
//     );

//     preferences.putBool(
//         "backup",
//         _settings.storage.backupEnabled
//     );


//     // SYSTEM
//     preferences.putBool(
//         "autostart",
//         _settings.system.autoStart
//     );

//     preferences.putBool(
//         "splash",
//         _settings.system.splashEnabled
//     );

//     preferences.putUShort(
//         "splash_time",
//         _settings.system.splashDuration
//     );

//     preferences.putBool(
//         "logging",
//         _settings.system.loggingEnabled
//     );

//     preferences.putBool(
//         "debug",
//         _settings.system.debugEnabled
//     );

//     preferences.putUChar(
//         "language",
//         _settings.system.language
//     );


//     preferences.end();

//     return true;
// }


// // ============================================================
// // RESET
// // ============================================================

// void SettingsManager::reset()
// {
//     _settings = Settings::Data{};
// }


// // ============================================================
// // DATA
// // ============================================================

// Settings::Data& SettingsManager::data()
// {
//     return _settings;
// }


// const Settings::Data& SettingsManager::data() const
// {
//     return _settings;
// }


// // ============================================================
// // DISPLAY
// // ============================================================

// void SettingsManager::setDisplayEnabled(bool enabled)
// {
//     _settings.display.enabled = enabled;
// }

// bool SettingsManager::displayEnabled() const
// {
//     return _settings.display.enabled;
// }

// void SettingsManager::setDisplayBrightness(uint8_t brightness)
// {
//     _settings.display.brightness =
//         clampBrightness(brightness);
// }

// uint8_t SettingsManager::displayBrightness() const
// {
//     return _settings.display.brightness;
// }

// void SettingsManager::setDisplayAutoBrightness(bool enabled)
// {
//     _settings.display.autoBrightness = enabled;
// }

// bool SettingsManager::displayAutoBrightness() const
// {
//     return _settings.display.autoBrightness;
// }

// void SettingsManager::setShowSeconds(bool enabled)
// {
//     _settings.display.showSeconds = enabled;
// }

// bool SettingsManager::showSeconds() const
// {
//     return _settings.display.showSeconds;
// }

// void SettingsManager::setShowDate(bool enabled)
// {
//     _settings.display.showDate = enabled;
// }

// bool SettingsManager::showDate() const
// {
//     return _settings.display.showDate;
// }

// void SettingsManager::setShowDayOfWeek(bool enabled)
// {
//     _settings.display.showDayOfWeek = enabled;
// }

// bool SettingsManager::showDayOfWeek() const
// {
//     return _settings.display.showDayOfWeek;
// }

// void SettingsManager::set24HourFormat(bool enabled)
// {
//     _settings.display.use24HourFormat = enabled;
// }

// bool SettingsManager::is24HourFormat() const
// {
//     return _settings.display.use24HourFormat;
// }

// void SettingsManager::setNightMode(bool enabled)
// {
//     _settings.display.nightMode = enabled;
// }

// bool SettingsManager::nightMode() const
// {
//     return _settings.display.nightMode;
// }

// void SettingsManager::setNightBrightness(uint8_t brightness)
// {
//     _settings.display.nightBrightness =
//         clampBrightness(brightness);
// }

// uint8_t SettingsManager::nightBrightness() const
// {
//     return _settings.display.nightBrightness;
// }

// void SettingsManager::setNightHours(
//     uint8_t startHour,
//     uint8_t endHour
// )
// {
//     _settings.display.nightStartHour =
//         startHour > 23 ? 23 : startHour;

//     _settings.display.nightEndHour =
//         endHour > 23 ? 23 : endHour;
// }


// // ============================================================
// // MATRIX
// // ============================================================

// void SettingsManager::setMatrixEnabled(bool enabled)
// {
//     _settings.matrix.enabled = enabled;
// }

// bool SettingsManager::matrixEnabled() const
// {
//     return _settings.matrix.enabled;
// }

// void SettingsManager::setMatrixBrightness(uint8_t brightness)
// {
//     _settings.matrix.brightness =
//         clampBrightness(brightness);
// }

// uint8_t SettingsManager::matrixBrightness() const
// {
//     return _settings.matrix.brightness;
// }

// void SettingsManager::setMatrixAutoBrightness(bool enabled)
// {
//     _settings.matrix.autoBrightness = enabled;
// }

// bool SettingsManager::matrixAutoBrightness() const
// {
//     return _settings.matrix.autoBrightness;
// }

// void SettingsManager::setMatrixNightMode(bool enabled)
// {
//     _settings.matrix.nightMode = enabled;
// }

// bool SettingsManager::matrixNightMode() const
// {
//     return _settings.matrix.nightMode;
// }

// void SettingsManager::setMatrixNightBrightness(uint8_t brightness)
// {
//     _settings.matrix.nightBrightness =
//         clampBrightness(brightness);
// }

// uint8_t SettingsManager::matrixNightBrightness() const
// {
//     return _settings.matrix.nightBrightness;
// }

// void SettingsManager::setMatrixAnimation(uint8_t animation)
// {
//     _settings.matrix.defaultAnimation = animation;
// }

// uint8_t SettingsManager::matrixAnimation() const
// {
//     return _settings.matrix.defaultAnimation;
// }

// void SettingsManager::setMatrixAnimationSpeed(uint8_t speed)
// {
//     _settings.matrix.animationSpeed =
//         clampPercent(speed);
// }

// uint8_t SettingsManager::matrixAnimationSpeed() const
// {
//     return _settings.matrix.animationSpeed;
// }

// void SettingsManager::setMatrixColor(
//     uint8_t r,
//     uint8_t g,
//     uint8_t b
// )
// {
//     _settings.matrix.colorR = r;
//     _settings.matrix.colorG = g;
//     _settings.matrix.colorB = b;
// }

// void SettingsManager::setMatrixClock(bool enabled)
// {
//     _settings.matrix.showClock = enabled;
// }

// bool SettingsManager::matrixClock() const
// {
//     return _settings.matrix.showClock;
// }

// void SettingsManager::setMatrixNotifications(bool enabled)
// {
//     _settings.matrix.showNotifications = enabled;
// }

// bool SettingsManager::matrixNotifications() const
// {
//     return _settings.matrix.showNotifications;
// }


// // ============================================================
// // COB LED
// // ============================================================

// void SettingsManager::setCobEnabled(bool enabled)
// {
//     _settings.cobLed.enabled = enabled;
// }

// bool SettingsManager::cobEnabled() const
// {
//     return _settings.cobLed.enabled;
// }

// void SettingsManager::setCobBrightness(uint8_t brightness)
// {
//     _settings.cobLed.brightness =
//         clampPercent(brightness);
// }

// uint8_t SettingsManager::cobBrightness() const
// {
//     return _settings.cobLed.brightness;
// }

// void SettingsManager::setCobLedEnabled(
//     uint8_t led,
//     bool enabled
// )
// {
//     if (led == 1)
//         _settings.cobLed.led1Enabled = enabled;

//     else if (led == 2)
//         _settings.cobLed.led2Enabled = enabled;
// }

// bool SettingsManager::cobLedEnabled(uint8_t led) const
// {
//     if (led == 1)
//         return _settings.cobLed.led1Enabled;

//     if (led == 2)
//         return _settings.cobLed.led2Enabled;

//     return false;
// }

// void SettingsManager::setCobLedBrightness(
//     uint8_t led,
//     uint8_t brightness
// )
// {
//     brightness =
//         clampPercent(brightness);

//     if (led == 1)
//         _settings.cobLed.led1Brightness = brightness;

//     else if (led == 2)
//         _settings.cobLed.led2Brightness = brightness;
// }

// uint8_t SettingsManager::cobLedBrightness(uint8_t led) const
// {
//     if (led == 1)
//         return _settings.cobLed.led1Brightness;

//     if (led == 2)
//         return _settings.cobLed.led2Brightness;

//     return 0;
// }

// void SettingsManager::setCobAutoBrightness(bool enabled)
// {
//     _settings.cobLed.autoBrightness = enabled;
// }

// bool SettingsManager::cobAutoBrightness() const
// {
//     return _settings.cobLed.autoBrightness;
// }

// void SettingsManager::setCobNightBrightness(uint8_t brightness)
// {
//     _settings.cobLed.nightBrightness =
//         clampPercent(brightness);
// }

// uint8_t SettingsManager::cobNightBrightness() const
// {
//     return _settings.cobLed.nightBrightness;
// }

// void SettingsManager::setCobFade(
//     bool enabled,
//     uint16_t time
// )
// {
//     _settings.cobLed.fadeEnabled = enabled;
//     _settings.cobLed.fadeTime = time;
// }


// // ============================================================
// // AUDIO
// // ============================================================

// void SettingsManager::setAudioEnabled(bool enabled)
// {
//     _settings.audio.enabled = enabled;
// }

// bool SettingsManager::audioEnabled() const
// {
//     return _settings.audio.enabled;
// }

// void SettingsManager::setVolume(uint8_t volume)
// {
//     volume = clampVolume(volume);

//     if (volume > _settings.audio.maxVolume)
//         volume = _settings.audio.maxVolume;

//     _settings.audio.volume = volume;
// }

// uint8_t SettingsManager::volume() const
// {
//     return _settings.audio.volume;
// }

// void SettingsManager::setMaxVolume(uint8_t volume)
// {
//     _settings.audio.maxVolume =
//         clampVolume(volume);

//     if (_settings.audio.volume >
//         _settings.audio.maxVolume)
//     {
//         _settings.audio.volume =
//             _settings.audio.maxVolume;
//     }
// }

// uint8_t SettingsManager::maxVolume() const
// {
//     return _settings.audio.maxVolume;
// }

// void SettingsManager::setInterfaceSounds(bool enabled)
// {
//     _settings.audio.interfaceSounds = enabled;
// }

// bool SettingsManager::interfaceSounds() const
// {
//     return _settings.audio.interfaceSounds;
// }

// void SettingsManager::setNotificationSounds(bool enabled)
// {
//     _settings.audio.notificationSounds = enabled;
// }

// bool SettingsManager::notificationSounds() const
// {
//     return _settings.audio.notificationSounds;
// }

// void SettingsManager::setAlarmSound(bool enabled)
// {
//     _settings.audio.alarmSound = enabled;
// }

// bool SettingsManager::alarmSound() const
// {
//     return _settings.audio.alarmSound;
// }

// void SettingsManager::setTimerSound(bool enabled)
// {
//     _settings.audio.timerSound = enabled;
// }

// bool SettingsManager::timerSound() const
// {
//     return _settings.audio.timerSound;
// }

// void SettingsManager::setStartupSound(bool enabled)
// {
//     _settings.audio.startupSound = enabled;
// }

// bool SettingsManager::startupSound() const
// {
//     return _settings.audio.startupSound;
// }

// void SettingsManager::setBalance(int8_t balance)
// {
//     _settings.audio.balance =
//         constrain(balance, -100, 100);
// }

// int8_t SettingsManager::balance() const
// {
//     return _settings.audio.balance;
// }

// void SettingsManager::setEqualizerEnabled(bool enabled)
// {
//     _settings.audio.equalizerEnabled = enabled;
// }

// bool SettingsManager::equalizerEnabled() const
// {
//     return _settings.audio.equalizerEnabled;
// }

// void SettingsManager::setBass(int8_t value)
// {
//     _settings.audio.bass =
//         clampAudioValue(value);
// }

// int8_t SettingsManager::bass() const
// {
//     return _settings.audio.bass;
// }

// void SettingsManager::setMid(int8_t value)
// {
//     _settings.audio.mid =
//         clampAudioValue(value);
// }

// int8_t SettingsManager::mid() const
// {
//     return _settings.audio.mid;
// }

// void SettingsManager::setTreble(int8_t value)
// {
//     _settings.audio.treble =
//         clampAudioValue(value);
// }

// int8_t SettingsManager::treble() const
// {
//     return _settings.audio.treble;
// }


// // ============================================================
// // MICROPHONE
// // ============================================================

// void SettingsManager::setMicrophoneEnabled(bool enabled)
// {
//     _settings.microphone.enabled = enabled;
// }

// bool SettingsManager::microphoneEnabled() const
// {
//     return _settings.microphone.enabled;
// }

// void SettingsManager::setMicrophoneAutoGain(bool enabled)
// {
//     _settings.microphone.autoGain = enabled;
// }

// bool SettingsManager::microphoneAutoGain() const
// {
//     return _settings.microphone.autoGain;
// }

// void SettingsManager::setMicrophoneGain(uint8_t gain)
// {
//     _settings.microphone.gain =
//         clampPercent(gain);
// }

// uint8_t SettingsManager::microphoneGain() const
// {
//     return _settings.microphone.gain;
// }

// void SettingsManager::setMicrophoneSensitivity(
//     uint8_t sensitivity
// )
// {
//     _settings.microphone.sensitivity =
//         clampPercent(sensitivity);
// }

// uint8_t SettingsManager::microphoneSensitivity() const
// {
//     return _settings.microphone.sensitivity;
// }

// void SettingsManager::setNoiseReduction(bool enabled)
// {
//     _settings.microphone.noiseReduction = enabled;
// }

// bool SettingsManager::noiseReduction() const
// {
//     return _settings.microphone.noiseReduction;
// }

// void SettingsManager::setVoiceDetection(bool enabled)
// {
//     _settings.microphone.voiceDetection = enabled;
// }

// bool SettingsManager::voiceDetection() const
// {
//     return _settings.microphone.voiceDetection;
// }

// void SettingsManager::setWakeWordEnabled(bool enabled)
// {
//     _settings.microphone.wakeWordEnabled = enabled;
// }

// bool SettingsManager::wakeWordEnabled() const
// {
//     return _settings.microphone.wakeWordEnabled;
// }


// // ============================================================
// // ENVIRONMENT
// // ============================================================

// void SettingsManager::setTemperatureEnabled(bool enabled)
// {
//     _settings.environment.temperatureEnabled = enabled;
// }

// bool SettingsManager::temperatureEnabled() const
// {
//     return _settings.environment.temperatureEnabled;
// }

// void SettingsManager::setHumidityEnabled(bool enabled)
// {
//     _settings.environment.humidityEnabled = enabled;
// }

// bool SettingsManager::humidityEnabled() const
// {
//     return _settings.environment.humidityEnabled;
// }

// void SettingsManager::setCelsius(bool enabled)
// {
//     _settings.environment.celsius = enabled;
// }

// bool SettingsManager::isCelsius() const
// {
//     return _settings.environment.celsius;
// }

// void SettingsManager::setTemperatureOffset(float offset)
// {
//     _settings.environment.temperatureOffset = offset;
// }

// float SettingsManager::temperatureOffset() const
// {
//     return _settings.environment.temperatureOffset;
// }

// void SettingsManager::setHumidityOffset(float offset)
// {
//     _settings.environment.humidityOffset = offset;
// }

// float SettingsManager::humidityOffset() const
// {
//     return _settings.environment.humidityOffset;
// }

// void SettingsManager::setShowTemperature(bool enabled)
// {
//     _settings.environment.showTemperature = enabled;
// }

// bool SettingsManager::showTemperature() const
// {
//     return _settings.environment.showTemperature;
// }

// void SettingsManager::setShowHumidity(bool enabled)
// {
//     _settings.environment.showHumidity = enabled;
// }

// bool SettingsManager::showHumidity() const
// {
//     return _settings.environment.showHumidity;
// }


// // ============================================================
// // LIGHT SENSOR
// // ============================================================

// void SettingsManager::setLightEnabled(bool enabled)
// {
//     _settings.lightSensor.enabled = enabled;
// }

// bool SettingsManager::lightEnabled() const
// {
//     return _settings.lightSensor.enabled;
// }

// void SettingsManager::setLightAutoBrightness(bool enabled)
// {
//     _settings.lightSensor.autoBrightness = enabled;
// }

// bool SettingsManager::lightAutoBrightness() const
// {
//     return _settings.lightSensor.autoBrightness;
// }

// void SettingsManager::setLightBrightnessRange(
//     uint8_t minBrightness,
//     uint8_t maxBrightness
// )
// {
//     minBrightness =
//         clampPercent(minBrightness);

//     maxBrightness =
//         clampPercent(maxBrightness);

//     if (minBrightness > maxBrightness)
//         minBrightness = maxBrightness;

//     _settings.lightSensor.minBrightness =
//         minBrightness;

//     _settings.lightSensor.maxBrightness =
//         maxBrightness;
// }

// uint8_t SettingsManager::lightMinBrightness() const
// {
//     return _settings.lightSensor.minBrightness;
// }

// uint8_t SettingsManager::lightMaxBrightness() const
// {
//     return _settings.lightSensor.maxBrightness;
// }

// void SettingsManager::setDarkThreshold(uint16_t value)
// {
//     _settings.lightSensor.darkThreshold = value;
// }

// uint16_t SettingsManager::darkThreshold() const
// {
//     return _settings.lightSensor.darkThreshold;
// }

// void SettingsManager::setBrightThreshold(uint16_t value)
// {
//     _settings.lightSensor.brightThreshold = value;
// }

// uint16_t SettingsManager::brightThreshold() const
// {
//     return _settings.lightSensor.brightThreshold;
// }


// // ============================================================
// // DISTANCE
// // ============================================================

// void SettingsManager::setDistanceEnabled(bool enabled)
// {
//     _settings.distanceSensor.enabled = enabled;
// }

// bool SettingsManager::distanceEnabled() const
// {
//     return _settings.distanceSensor.enabled;
// }

// void SettingsManager::setWakeOnPresence(bool enabled)
// {
//     _settings.distanceSensor.wakeOnPresence = enabled;
// }

// bool SettingsManager::wakeOnPresence() const
// {
//     return _settings.distanceSensor.wakeOnPresence;
// }

// void SettingsManager::setInteractionEnabled(bool enabled)
// {
//     _settings.distanceSensor.interactionEnabled = enabled;
// }

// bool SettingsManager::interactionEnabled() const
// {
//     return _settings.distanceSensor.interactionEnabled;
// }

// void SettingsManager::setPresenceDistance(uint16_t distance)
// {
//     _settings.distanceSensor.presenceDistance =
//         distance;
// }

// uint16_t SettingsManager::presenceDistance() const
// {
//     return _settings.distanceSensor.presenceDistance;
// }

// void SettingsManager::setAbsenceTimeout(uint16_t seconds)
// {
//     _settings.distanceSensor.absenceTimeout =
//         seconds;
// }

// uint16_t SettingsManager::absenceTimeout() const
// {
//     return _settings.distanceSensor.absenceTimeout;
// }


// // ============================================================
// // CLOCK
// // ============================================================

// void SettingsManager::setClockAutoSync(bool enabled)
// {
//     _settings.clock.autoSync = enabled;
// }

// bool SettingsManager::clockAutoSync() const
// {
//     return _settings.clock.autoSync;
// }

// void SettingsManager::setUseRTC(bool enabled)
// {
//     _settings.clock.useRTC = enabled;
// }

// bool SettingsManager::useRTC() const
// {
//     return _settings.clock.useRTC;
// }

// void SettingsManager::setUseNTP(bool enabled)
// {
//     _settings.clock.useNTP = enabled;
// }

// bool SettingsManager::useNTP() const
// {
//     return _settings.clock.useNTP;
// }

// void SettingsManager::setTimezone(const char* timezone)
// {
//     if (!timezone)
//         return;

//     strncpy(
//         _settings.clock.timezone,
//         timezone,
//         sizeof(_settings.clock.timezone) - 1
//     );

//     _settings.clock.timezone[
//         sizeof(_settings.clock.timezone) - 1
//     ] = '\0';
// }

// const char* SettingsManager::timezone() const
// {
//     return _settings.clock.timezone;
// }


// // ============================================================
// // NOTIFICATIONS
// // ============================================================

// void SettingsManager::setNotificationsEnabled(bool enabled)
// {
//     _settings.notifications.enabled = enabled;
// }

// bool SettingsManager::notificationsEnabled() const
// {
//     return _settings.notifications.enabled;
// }

// void SettingsManager::setNotificationDisplay(bool enabled)
// {
//     _settings.notifications.displayEnabled = enabled;
// }

// bool SettingsManager::notificationDisplay() const
// {
//     return _settings.notifications.displayEnabled;
// }

// void SettingsManager::setNotificationMatrix(bool enabled)
// {
//     _settings.notifications.matrixEnabled = enabled;
// }

// bool SettingsManager::notificationMatrix() const
// {
//     return _settings.notifications.matrixEnabled;
// }

// void SettingsManager::setNotificationSound(bool enabled)
// {
//     _settings.notifications.soundEnabled = enabled;
// }

// bool SettingsManager::notificationSound() const
// {
//     return _settings.notifications.soundEnabled;
// }

// void SettingsManager::setNotificationDuration(uint16_t duration)
// {
//     _settings.notifications.duration = duration;
// }

// uint16_t SettingsManager::notificationDuration() const
// {
//     return _settings.notifications.duration;
// }


// // ============================================================
// // ALARM
// // ============================================================

// void SettingsManager::setAlarmEnabled(bool enabled)
// {
//     _settings.alarm.enabled = enabled;
// }

// bool SettingsManager::alarmEnabled() const
// {
//     return _settings.alarm.enabled;
// }

// void SettingsManager::setAlarmTime(
//     uint8_t hour,
//     uint8_t minute
// )
// {
//     _settings.alarm.hour =
//         hour > 23 ? 23 : hour;

//     _settings.alarm.minute =
//         minute > 59 ? 59 : minute;
// }

// uint8_t SettingsManager::alarmHour() const
// {
//     return _settings.alarm.hour;
// }

// uint8_t SettingsManager::alarmMinute() const
// {
//     return _settings.alarm.minute;
// }

// void SettingsManager::setAlarmDays(uint8_t days)
// {
//     _settings.alarm.days = days;
// }

// uint8_t SettingsManager::alarmDays() const
// {
//     return _settings.alarm.days;
// }

// void SettingsManager::setAlarmVolume(uint8_t volume)
// {
//     _settings.alarm.volume =
//         clampVolume(volume);
// }

// uint8_t SettingsManager::alarmVolume() const
// {
//     return _settings.alarm.volume;
// }

// void SettingsManager::setAlarmFade(
//     bool enabled,
//     uint16_t time
// )
// {
//     _settings.alarm.fadeIn = enabled;
//     _settings.alarm.fadeTime = time;
// }

// void SettingsManager::setAlarmLight(bool enabled)
// {
//     _settings.alarm.useLight = enabled;
// }

// bool SettingsManager::alarmLight() const
// {
//     return _settings.alarm.useLight;
// }

// void SettingsManager::setAlarmMatrix(bool enabled)
// {
//     _settings.alarm.useMatrix = enabled;
// }

// bool SettingsManager::alarmMatrix() const
// {
//     return _settings.alarm.useMatrix;
// }

// void SettingsManager::setAlarmSound(bool enabled)
// {
//     _settings.alarm.useSound = enabled;
// }

// bool SettingsManager::alarmSoundEnabled() const
// {
//     return _settings.alarm.useSound;
// }

// void SettingsManager::setSnooze(
//     bool enabled,
//     uint8_t minutes
// )
// {
//     _settings.alarm.snoozeEnabled = enabled;
//     _settings.alarm.snoozeMinutes = minutes;
// }


// // ============================================================
// // TIMER
// // ============================================================

// void SettingsManager::setTimerSound(bool enabled)
// {
//     _settings.timer.soundEnabled = enabled;
// }

// bool SettingsManager::timerSoundEnabled() const
// {
//     return _settings.timer.soundEnabled;
// }

// void SettingsManager::setTimerDisplay(bool enabled)
// {
//     _settings.timer.displayEnabled = enabled;
// }

// bool SettingsManager::timerDisplayEnabled() const
// {
//     return _settings.timer.displayEnabled;
// }

// void SettingsManager::setTimerMatrix(bool enabled)
// {
//     _settings.timer.matrixEnabled = enabled;
// }

// bool SettingsManager::timerMatrixEnabled() const
// {
//     return _settings.timer.matrixEnabled;
// }

// void SettingsManager::setTimerVolume(uint8_t volume)
// {
//     _settings.timer.volume =
//         clampVolume(volume);
// }

// uint8_t SettingsManager::timerVolume() const
// {
//     return _settings.timer.volume;
// }

// void SettingsManager::setTimerLight(bool enabled)
// {
//     _settings.timer.lightEnabled = enabled;
// }

// bool SettingsManager::timerLightEnabled() const
// {
//     return _settings.timer.lightEnabled;
// }


// // ============================================================
// // WIFI
// // ============================================================

// void SettingsManager::setWiFiEnabled(bool enabled)
// {
//     _settings.wifi.enabled = enabled;
// }

// bool SettingsManager::wifiEnabled() const
// {
//     return _settings.wifi.enabled;
// }

// void SettingsManager::setWiFiAutoConnect(bool enabled)
// {
//     _settings.wifi.autoConnect = enabled;
// }

// bool SettingsManager::wifiAutoConnect() const
// {
//     return _settings.wifi.autoConnect;
// }

// void SettingsManager::setWiFiSyncTime(bool enabled)
// {
//     _settings.wifi.syncTime = enabled;
// }

// bool SettingsManager::wifiSyncTime() const
// {
//     return _settings.wifi.syncTime;
// }

// void SettingsManager::setWiFiConnectOnBoot(bool enabled)
// {
//     _settings.wifi.connectOnBoot = enabled;
// }

// bool SettingsManager::wifiConnectOnBoot() const
// {
//     return _settings.wifi.connectOnBoot;
// }

// void SettingsManager::setWiFiShowStatus(bool enabled)
// {
//     _settings.wifi.showStatus = enabled;
// }

// bool SettingsManager::wifiShowStatus() const
// {
//     return _settings.wifi.showStatus;
// }


// // ============================================================
// // POWER
// // ============================================================

// void SettingsManager::setSleepEnabled(bool enabled)
// {
//     _settings.power.sleepEnabled = enabled;
// }

// bool SettingsManager::sleepEnabled() const
// {
//     return _settings.power.sleepEnabled;
// }

// void SettingsManager::setSleepMinutes(uint16_t minutes)
// {
//     _settings.power.sleepMinutes = minutes;
// }

// uint16_t SettingsManager::sleepMinutes() const
// {
//     return _settings.power.sleepMinutes;
// }

// void SettingsManager::setDisplaySleep(bool enabled)
// {
//     _settings.power.displaySleep = enabled;
// }

// bool SettingsManager::displaySleep() const
// {
//     return _settings.power.displaySleep;
// }

// void SettingsManager::setMatrixSleep(bool enabled)
// {
//     _settings.power.matrixSleep = enabled;
// }

// bool SettingsManager::matrixSleep() const
// {
//     return _settings.power.matrixSleep;
// }

// void SettingsManager::setCobSleep(bool enabled)
// {
//     _settings.power.cobSleep = enabled;
// }

// bool SettingsManager::cobSleep() const
// {
//     return _settings.power.cobSleep;
// }

// void SettingsManager::setAudioSleep(bool enabled)
// {
//     _settings.power.audioSleep = enabled;
// }

// bool SettingsManager::audioSleep() const
// {
//     return _settings.power.audioSleep;
// }

// void SettingsManager::setWakeOnMotion(bool enabled)
// {
//     _settings.power.wakeOnMotion = enabled;
// }

// bool SettingsManager::wakeOnMotion() const
// {
//     return _settings.power.wakeOnMotion;
// }

// void SettingsManager::setWakeOnButton(bool enabled)
// {
//     _settings.power.wakeOnButton = enabled;
// }

// bool SettingsManager::wakeOnButton() const
// {
//     return _settings.power.wakeOnButton;
// }

// void SettingsManager::setPowerNightMode(bool enabled)
// {
//     _settings.power.nightMode = enabled;
// }

// bool SettingsManager::powerNightMode() const
// {
//     return _settings.power.nightMode;
// }


// // ============================================================
// // UI
// // ============================================================

// void SettingsManager::setTheme(uint8_t theme)
// {
//     _settings.ui.theme = theme;
// }

// uint8_t SettingsManager::theme() const
// {
//     return _settings.ui.theme;
// }

// void SettingsManager::setAnimations(bool enabled)
// {
//     _settings.ui.animations = enabled;
// }

// bool SettingsManager::animations() const
// {
//     return _settings.ui.animations;
// }

// void SettingsManager::setAnimationSpeed(uint8_t speed)
// {
//     _settings.ui.animationSpeed =
//         clampPercent(speed);
// }

// uint8_t SettingsManager::animationSpeed() const
// {
//     return _settings.ui.animationSpeed;
// }

// void SettingsManager::setMenuTimeout(uint16_t timeout)
// {
//     _settings.ui.menuTimeout = timeout;
// }

// uint16_t SettingsManager::menuTimeout() const
// {
//     return _settings.ui.menuTimeout;
// }

// void SettingsManager::setHints(bool enabled)
// {
//     _settings.ui.hints = enabled;
// }

// bool SettingsManager::hints() const
// {
//     return _settings.ui.hints;
// }

// void SettingsManager::setUISounds(bool enabled)
// {
//     _settings.ui.sounds = enabled;
// }

// bool SettingsManager::uiSounds() const
// {
//     return _settings.ui.sounds;
// }


// // ============================================================
// // ENCODER
// // ============================================================

// void SettingsManager::setEncoderInverted(bool inverted)
// {
//     _settings.encoder.inverted = inverted;
// }

// bool SettingsManager::encoderInverted() const
// {
//     return _settings.encoder.inverted;
// }

// void SettingsManager::setEncoderStep(uint8_t step)
// {
//     if (step == 0)
//         step = 1;

//     _settings.encoder.step = step;
// }

// uint8_t SettingsManager::encoderStep() const
// {
//     return _settings.encoder.step;
// }

// void SettingsManager::setEncoderAcceleration(bool enabled)
// {
//     _settings.encoder.acceleration = enabled;
// }

// bool SettingsManager::encoderAcceleration() const
// {
//     return _settings.encoder.acceleration;
// }

// void SettingsManager::setEncoderAccelerationSpeed(uint8_t speed)
// {
//     _settings.encoder.accelerationSpeed =
//         clampPercent(speed);
// }

// uint8_t SettingsManager::encoderAccelerationSpeed() const
// {
//     return _settings.encoder.accelerationSpeed;
// }

// void SettingsManager::setEncoderButtonEnabled(bool enabled)
// {
//     _settings.encoder.buttonEnabled = enabled;
// }

// bool SettingsManager::encoderButtonEnabled() const
// {
//     return _settings.encoder.buttonEnabled;
// }


// // ============================================================
// // STORAGE
// // ============================================================

// void SettingsManager::setAutoSave(bool enabled)
// {
//     _settings.storage.autoSave = enabled;
// }

// bool SettingsManager::autoSave() const
// {
//     return _settings.storage.autoSave;
// }

// void SettingsManager::setSaveInterval(uint32_t interval)
// {
//     _settings.storage.saveInterval = interval;
// }

// uint32_t SettingsManager::saveInterval() const
// {
//     return _settings.storage.saveInterval;
// }

// void SettingsManager::setBackupEnabled(bool enabled)
// {
//     _settings.storage.backupEnabled = enabled;
// }

// bool SettingsManager::backupEnabled() const
// {
//     return _settings.storage.backupEnabled;
// }


// // ============================================================
// // SYSTEM
// // ============================================================

// void SettingsManager::setAutoStart(bool enabled)
// {
//     _settings.system.autoStart = enabled;
// }

// bool SettingsManager::autoStart() const
// {
//     return _settings.system.autoStart;
// }

// void SettingsManager::setSplashEnabled(bool enabled)
// {
//     _settings.system.splashEnabled = enabled;
// }

// bool SettingsManager::splashEnabled() const
// {
//     return _settings.system.splashEnabled;
// }

// void SettingsManager::setSplashDuration(uint16_t duration)
// {
//     _settings.system.splashDuration = duration;
// }

// uint16_t SettingsManager::splashDuration() const
// {
//     return _settings.system.splashDuration;
// }

// void SettingsManager::setLoggingEnabled(bool enabled)
// {
//     _settings.system.loggingEnabled = enabled;
// }

// bool SettingsManager::loggingEnabled() const
// {
//     return _settings.system.loggingEnabled;
// }

// void SettingsManager::setDebugEnabled(bool enabled)
// {
//     _settings.system.debugEnabled = enabled;
// }

// bool SettingsManager::debugEnabled() const
// {
//     return _settings.system.debugEnabled;
// }

// void SettingsManager::setLanguage(uint8_t language)
// {
//     _settings.system.language =
//         language > 1 ? 1 : language;
// }

// uint8_t SettingsManager::language() const
// {
//     return _settings.system.language;
// }


// // ============================================================
// // VALIDATION
// // ============================================================

// void SettingsManager::validate()
// {
//     _settings.display.brightness =
//         clampBrightness(
//             _settings.display.brightness
//         );

//     _settings.display.nightBrightness =
//         clampBrightness(
//             _settings.display.nightBrightness
//         );


//     _settings.matrix.brightness =
//         clampBrightness(
//             _settings.matrix.brightness
//         );

//     _settings.matrix.nightBrightness =
//         clampBrightness(
//             _settings.matrix.nightBrightness
//         );

//     _settings.matrix.animationSpeed =
//         clampPercent(
//             _settings.matrix.animationSpeed
//         );


//     _settings.cobLed.brightness =
//         clampPercent(
//             _settings.cobLed.brightness
//         );

//     _settings.cobLed.led1Brightness =
//         clampPercent(
//             _settings.cobLed.led1Brightness
//         );

//     _settings.cobLed.led2Brightness =
//         clampPercent(
//             _settings.cobLed.led2Brightness
//         );

//     _settings.cobLed.nightBrightness =
//         clampPercent(
//             _settings.cobLed.nightBrightness
//         );


//     _settings.audio.maxVolume =
//         clampVolume(
//             _settings.audio.maxVolume
//         );

//     _settings.audio.volume =
//         clampVolume(
//             _settings.audio.volume
//         );

//     if (_settings.audio.volume >
//         _settings.audio.maxVolume)
//     {
//         _settings.audio.volume =
//             _settings.audio.maxVolume;
//     }


//     _settings.microphone.gain =
//         clampPercent(
//             _settings.microphone.gain
//         );

//     _settings.microphone.sensitivity =
//         clampPercent(
//             _settings.microphone.sensitivity
//         );


//     _settings.lightSensor.minBrightness =
//         clampPercent(
//             _settings.lightSensor.minBrightness
//         );

//     _settings.lightSensor.maxBrightness =
//         clampPercent(
//             _settings.lightSensor.maxBrightness
//         );

//     if (_settings.lightSensor.minBrightness >
//         _settings.lightSensor.maxBrightness)
//     {
//         _settings.lightSensor.minBrightness =
//             _settings.lightSensor.maxBrightness;
//     }


//     if (_settings.alarm.hour > 23)
//         _settings.alarm.hour = 23;

//     if (_settings.alarm.minute > 59)
//         _settings.alarm.minute = 59;

//     _settings.alarm.volume =
//         clampVolume(
//             _settings.alarm.volume
//         );


//     _settings.timer.volume =
//         clampVolume(
//             _settings.timer.volume
//         );


//     _settings.ui.animationSpeed =
//         clampPercent(
//             _settings.ui.animationSpeed
//         );


//     _settings.encoder.accelerationSpeed =
//         clampPercent(
//             _settings.encoder.accelerationSpeed
//         );


//     if (_settings.system.language > 1)
//         _settings.system.language = 0;


//     _settings.clock.timezone[
//         sizeof(_settings.clock.timezone) - 1
//     ] = '\0';
// }


// // ============================================================
// // CLAMP
// // ============================================================

// uint8_t SettingsManager::clampBrightness(
//     uint8_t value
// ) const
// {
//     if (value > 100)
//         return 100;

//     return value;
// }

// uint8_t SettingsManager::clampVolume(
//     uint8_t value
// ) const
// {
//     if (value > 100)
//         return 100;

//     return value;
// }

// uint8_t SettingsManager::clampPercent(
//     uint8_t value
// ) const
// {
//     if (value > 100)
//         return 100;

//     return value;
// }

// int8_t SettingsManager::clampAudioValue(
//     int8_t value
// ) const
// {
//     if (value < -12)
//         return -12;

//     if (value > 12)
//         return 12;

//     return value;
// }