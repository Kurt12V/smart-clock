// #pragma once

// #include <Arduino.h>

// #include "../settings/Settings.h"
// #include "../settings/SettingsRepository.h"

// #include "../storage/SDManager.h"

// #include "../clock/ClockSystem.h"

// #include "../sensors/SensorManager.h"

// #include "../display/DisplayManager.h"

// #include "../ai/AIManager.h"


// class System
// {
// public:

//     System();

//     // Инициализация всей системы
//     bool begin();

//     // Основной цикл
//     void update();


//     // =========================
//     // SETTINGS
//     // =========================

//     Settings& getSettings();

//     bool saveSettings();

//     bool resetSettings();


//     // =========================
//     // STORAGE
//     // =========================

//     SDManager& getSDManager();

//     bool isSDReady() const;


//     // =========================
//     // CLOCK
//     // =========================

//     ClockSystem& getClockSystem();

//     bool isClockReady() const;


//     // =========================
//     // SENSORS
//     // =========================

//     SensorManager& getSensorManager();

//     bool areSensorsReady() const;


//     // =========================
//     // DISPLAY
//     // =========================

//     DisplayManager& getDisplayManager();

//     bool isDisplayReady() const;


//     // =========================
//     // AI
//     // =========================

//     AIManager& getAIManager();

//     bool isAIReady() const;


//     // =========================
//     // SYSTEM STATUS
//     // =========================

//     bool isInitialized() const;

// private:

//     // =========================
//     // INITIALIZATION
//     // =========================

//     bool initializeLogger();

//     bool initializeSettings();

//     bool initializeStorage();

//     bool initializeClock();

//     bool initializeSensors();

//     bool initializeDisplays();

//     bool initializeAI();


//     // =========================
//     // UPDATE
//     // =========================

//     void updateClock();

//     void updateSensors();

//     void updateDisplays();

//     void updateAI();


// private:

//     // =========================
//     // SYSTEM STATE
//     // =========================

//     bool _initialized;

//     bool _loggerReady;
//     bool _settingsReady;
//     bool _storageReady;
//     bool _clockReady;
//     bool _sensorsReady;
//     bool _displayReady;
//     bool _aiReady;


//     // =========================
//     // SETTINGS
//     // =========================

//     Settings _settings;

//     SettingsRepository _settingsRepository;


//     // =========================
//     // STORAGE
//     // =========================

//     SDManager _sdManager;


//     // =========================
//     // CLOCK
//     // =========================

//     ClockSystem _clockSystem;


//     // =========================
//     // SENSORS
//     // =========================

//     SensorManager _sensorManager;


//     // =========================
//     // DISPLAY
//     // =========================

//     DisplayManager _displayManager;


//     // =========================
//     // AI
//     // =========================

//     AIManager _aiManager;


//     // =========================
//     // UPDATE TIMERS
//     // =========================

//     uint32_t _lastClockUpdate;
//     uint32_t _lastSensorUpdate;
//     uint32_t _lastDisplayUpdate;
//     uint32_t _lastAIUpdate;
// };