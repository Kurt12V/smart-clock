// #pragma once

// #include <Arduino.h>
// #include "Settings.h"

// class SettingsManager
// {
// public:

//     SettingsManager();

//     // ========================================================
//     // LIFECYCLE
//     // ========================================================

//     void begin();

//     bool load();
//     bool save();

//     void reset();

//     // ========================================================
//     // DISPLAY
//     // ========================================================

//     void setDisplayEnabled(bool enabled);
//     bool displayEnabled() const;

//     void setDisplayBrightness(uint8_t brightness);
//     uint8_t displayBrightness() const;

//     void setDisplayAutoBrightness(bool enabled);
//     bool displayAutoBrightness() const;

//     void setShowSeconds(bool enabled);
//     bool showSeconds() const;

//     void setShowDate(bool enabled);
//     bool showDate() const;

//     void setShowDayOfWeek(bool enabled);
//     bool showDayOfWeek() const;

//     void set24HourFormat(bool enabled);
//     bool is24HourFormat() const;

//     void setNightMode(bool enabled);
//     bool nightMode() const;

//     void setNightBrightness(uint8_t brightness);
//     uint8_t nightBrightness() const;

//     void setNightHours(
//         uint8_t startHour,
//         uint8_t endHour
//     );

//     // ========================================================
//     // LED MATRIX
//     // ========================================================

//     void setMatrixEnabled(bool enabled);
//     bool matrixEnabled() const;

//     void setMatrixBrightness(uint8_t brightness);
//     uint8_t matrixBrightness() const;

//     void setMatrixAutoBrightness(bool enabled);
//     bool matrixAutoBrightness() const;

//     void setMatrixNightMode(bool enabled);
//     bool matrixNightMode() const;

//     void setMatrixNightBrightness(uint8_t brightness);
//     uint8_t matrixNightBrightness() const;

//     void setMatrixAnimation(uint8_t animation);
//     uint8_t matrixAnimation() const;

//     void setMatrixAnimationSpeed(uint8_t speed);
//     uint8_t matrixAnimationSpeed() const;

//     void setMatrixColor(
//         uint8_t r,
//         uint8_t g,
//         uint8_t b
//     );

//     void setMatrixClock(bool enabled);
//     bool matrixClock() const;

//     void setMatrixNotifications(bool enabled);
//     bool matrixNotifications() const;

//     // ========================================================
//     // COB LED
//     // ========================================================

//     void setCobEnabled(bool enabled);
//     bool cobEnabled() const;

//     void setCobBrightness(uint8_t brightness);
//     uint8_t cobBrightness() const;

//     void setCobLedEnabled(
//         uint8_t led,
//         bool enabled
//     );

//     bool cobLedEnabled(uint8_t led) const;

//     void setCobLedBrightness(
//         uint8_t led,
//         uint8_t brightness
//     );

//     uint8_t cobLedBrightness(uint8_t led) const;

//     void setCobAutoBrightness(bool enabled);
//     bool cobAutoBrightness() const;

//     void setCobNightBrightness(uint8_t brightness);
//     uint8_t cobNightBrightness() const;

//     void setCobFade(
//         bool enabled,
//         uint16_t time
//     );

//     // ========================================================
//     // AUDIO
//     // ========================================================

//     void setAudioEnabled(bool enabled);
//     bool audioEnabled() const;

//     void setVolume(uint8_t volume);
//     uint8_t volume() const;

//     void setMaxVolume(uint8_t volume);
//     uint8_t maxVolume() const;

//     void setInterfaceSounds(bool enabled);
//     bool interfaceSounds() const;

//     void setNotificationSounds(bool enabled);
//     bool notificationSounds() const;

//     void setAlarmSound(bool enabled);
//     bool alarmSound() const;

//     void setTimerSound(bool enabled);
//     bool timerSound() const;

//     void setStartupSound(bool enabled);
//     bool startupSound() const;

//     void setBalance(int8_t balance);
//     int8_t balance() const;

//     void setEqualizerEnabled(bool enabled);
//     bool equalizerEnabled() const;

//     void setBass(int8_t value);
//     int8_t bass() const;

//     void setMid(int8_t value);
//     int8_t mid() const;

//     void setTreble(int8_t value);
//     int8_t treble() const;

//     // ========================================================
//     // MICROPHONE
//     // ========================================================

//     void setMicrophoneEnabled(bool enabled);
//     bool microphoneEnabled() const;

//     void setMicrophoneAutoGain(bool enabled);
//     bool microphoneAutoGain() const;

//     void setMicrophoneGain(uint8_t gain);
//     uint8_t microphoneGain() const;

//     void setMicrophoneSensitivity(uint8_t sensitivity);
//     uint8_t microphoneSensitivity() const;

//     void setNoiseReduction(bool enabled);
//     bool noiseReduction() const;

//     void setVoiceDetection(bool enabled);
//     bool voiceDetection() const;

//     void setWakeWordEnabled(bool enabled);
//     bool wakeWordEnabled() const;

//     // ========================================================
//     // ENVIRONMENT
//     // ========================================================

//     void setTemperatureEnabled(bool enabled);
//     bool temperatureEnabled() const;

//     void setHumidityEnabled(bool enabled);
//     bool humidityEnabled() const;

//     void setCelsius(bool enabled);
//     bool isCelsius() const;

//     void setTemperatureOffset(float offset);
//     float temperatureOffset() const;

//     void setHumidityOffset(float offset);
//     float humidityOffset() const;

//     void setShowTemperature(bool enabled);
//     bool showTemperature() const;

//     void setShowHumidity(bool enabled);
//     bool showHumidity() const;

//     // ========================================================
//     // LIGHT SENSOR
//     // ========================================================

//     void setLightEnabled(bool enabled);
//     bool lightEnabled() const;

//     void setLightAutoBrightness(bool enabled);
//     bool lightAutoBrightness() const;

//     void setLightBrightnessRange(
//         uint8_t minBrightness,
//         uint8_t maxBrightness
//     );

//     uint8_t lightMinBrightness() const;
//     uint8_t lightMaxBrightness() const;

//     void setDarkThreshold(uint16_t value);
//     uint16_t darkThreshold() const;

//     void setBrightThreshold(uint16_t value);
//     uint16_t brightThreshold() const;

//     // ========================================================
//     // DISTANCE SENSOR
//     // ========================================================

//     void setDistanceEnabled(bool enabled);
//     bool distanceEnabled() const;

//     void setWakeOnPresence(bool enabled);
//     bool wakeOnPresence() const;

//     void setInteractionEnabled(bool enabled);
//     bool interactionEnabled() const;

//     void setPresenceDistance(uint16_t distance);
//     uint16_t presenceDistance() const;

//     void setAbsenceTimeout(uint16_t seconds);
//     uint16_t absenceTimeout() const;

//     // ========================================================
//     // CLOCK
//     // ========================================================

//     void setClockAutoSync(bool enabled);
//     bool clockAutoSync() const;

//     void setUseRTC(bool enabled);
//     bool useRTC() const;

//     void setUseNTP(bool enabled);
//     bool useNTP() const;

//     void setTimezone(const char* timezone);
//     const char* timezone() const;

//     // ========================================================
//     // NOTIFICATIONS
//     // ========================================================

//     void setNotificationsEnabled(bool enabled);
//     bool notificationsEnabled() const;

//     void setNotificationDisplay(bool enabled);
//     bool notificationDisplay() const;

//     void setNotificationMatrix(bool enabled);
//     bool notificationMatrix() const;

//     void setNotificationSound(bool enabled);
//     bool notificationSound() const;

//     void setNotificationDuration(uint16_t duration);
//     uint16_t notificationDuration() const;

//     // ========================================================
//     // ALARM
//     // ========================================================

//     void setAlarmEnabled(bool enabled);
//     bool alarmEnabled() const;

//     void setAlarmTime(
//         uint8_t hour,
//         uint8_t minute
//     );

//     uint8_t alarmHour() const;
//     uint8_t alarmMinute() const;

//     void setAlarmDays(uint8_t days);
//     uint8_t alarmDays() const;

//     void setAlarmVolume(uint8_t volume);
//     uint8_t alarmVolume() const;

//     void setAlarmFade(
//         bool enabled,
//         uint16_t time
//     );

//     void setAlarmLight(bool enabled);
//     bool alarmLight() const;

//     void setAlarmMatrix(bool enabled);
//     bool alarmMatrix() const;

//     void setAlarmSound(bool enabled);
//     bool alarmSoundEnabled() const;

//     void setSnooze(
//         bool enabled,
//         uint8_t minutes
//     );

//     // ========================================================
//     // TIMER
//     // ========================================================

//     void setTimerSound(bool enabled);
//     bool timerSoundEnabled() const;

//     void setTimerDisplay(bool enabled);
//     bool timerDisplayEnabled() const;

//     void setTimerMatrix(bool enabled);
//     bool timerMatrixEnabled() const;

//     void setTimerVolume(uint8_t volume);
//     uint8_t timerVolume() const;

//     void setTimerLight(bool enabled);
//     bool timerLightEnabled() const;

//     // ========================================================
//     // WIFI
//     // ========================================================

//     void setWiFiEnabled(bool enabled);
//     bool wifiEnabled() const;

//     void setWiFiAutoConnect(bool enabled);
//     bool wifiAutoConnect() const;

//     void setWiFiSyncTime(bool enabled);
//     bool wifiSyncTime() const;

//     void setWiFiConnectOnBoot(bool enabled);
//     bool wifiConnectOnBoot() const;

//     void setWiFiShowStatus(bool enabled);
//     bool wifiShowStatus() const;

//     // ========================================================
//     // POWER
//     // ========================================================

//     void setSleepEnabled(bool enabled);
//     bool sleepEnabled() const;

//     void setSleepMinutes(uint16_t minutes);
//     uint16_t sleepMinutes() const;

//     void setDisplaySleep(bool enabled);
//     bool displaySleep() const;

//     void setMatrixSleep(bool enabled);
//     bool matrixSleep() const;

//     void setCobSleep(bool enabled);
//     bool cobSleep() const;

//     void setAudioSleep(bool enabled);
//     bool audioSleep() const;

//     void setWakeOnMotion(bool enabled);
//     bool wakeOnMotion() const;

//     void setWakeOnButton(bool enabled);
//     bool wakeOnButton() const;

//     void setPowerNightMode(bool enabled);
//     bool powerNightMode() const;

//     // ========================================================
//     // UI
//     // ========================================================

//     void setTheme(uint8_t theme);
//     uint8_t theme() const;

//     void setAnimations(bool enabled);
//     bool animations() const;

//     void setAnimationSpeed(uint8_t speed);
//     uint8_t animationSpeed() const;

//     void setMenuTimeout(uint16_t timeout);
//     uint16_t menuTimeout() const;

//     void setHints(bool enabled);
//     bool hints() const;

//     void setUISounds(bool enabled);
//     bool uiSounds() const;

//     // ========================================================
//     // ENCODER
//     // ========================================================

//     void setEncoderInverted(bool inverted);
//     bool encoderInverted() const;

//     void setEncoderStep(uint8_t step);
//     uint8_t encoderStep() const;

//     void setEncoderAcceleration(bool enabled);
//     bool encoderAcceleration() const;

//     void setEncoderAccelerationSpeed(uint8_t speed);
//     uint8_t encoderAccelerationSpeed() const;

//     void setEncoderButtonEnabled(bool enabled);
//     bool encoderButtonEnabled() const;

//     // ========================================================
//     // STORAGE
//     // ========================================================

//     void setAutoSave(bool enabled);
//     bool autoSave() const;

//     void setSaveInterval(uint32_t interval);
//     uint32_t saveInterval() const;

//     void setBackupEnabled(bool enabled);
//     bool backupEnabled() const;

//     // ========================================================
//     // SYSTEM
//     // ========================================================

//     void setAutoStart(bool enabled);
//     bool autoStart() const;

//     void setSplashEnabled(bool enabled);
//     bool splashEnabled() const;

//     void setSplashDuration(uint16_t duration);
//     uint16_t splashDuration() const;

//     void setLoggingEnabled(bool enabled);
//     bool loggingEnabled() const;

//     void setDebugEnabled(bool enabled);
//     bool debugEnabled() const;

//     void setLanguage(uint8_t language);
//     uint8_t language() const;

//     // ========================================================
//     // DATA ACCESS
//     // ========================================================

//     Settings::Data& data();
//     const Settings::Data& data() const;

// private:

//     Settings::Data _settings;

//     bool _initialized = false;

//     static constexpr const char* NAMESPACE = "smartclock";

//     // --------------------------------------------------------
//     // Validation
//     // --------------------------------------------------------

//     void validate();

//     uint8_t clampBrightness(uint8_t value) const;
//     uint8_t clampVolume(uint8_t value) const;
//     uint8_t clampPercent(uint8_t value) const;
//     int8_t clampAudioValue(int8_t value) const;
// };