#pragma once

#include <Arduino.h>

// ============================================================
// USER SETTINGS
//
// Settings.h содержит только параметры, которые пользователь
// может изменять во время работы устройства.
//
// Config.h содержит аппаратные и системные параметры.
// ============================================================

namespace Settings
{

// ============================================================
// DISPLAY
// ============================================================

struct Display
{
    // Включён ли дисплей
    bool enabled = true;

    // Яркость 0-100%
    uint8_t brightness = 80;

    // Автоматическая яркость
    bool autoBrightness = false;

    // Показывать секунды
    bool showSeconds = true;

    // Показывать дату
    bool showDate = true;

    // Показывать день недели
    bool showDayOfWeek = true;

    // Формат времени
    bool use24HourFormat = true;

    // Отключать экран ночью
    bool nightMode = false;

    // Начало ночного режима
    uint8_t nightStartHour = 23;

    // Конец ночного режима
    uint8_t nightEndHour = 7;

    // Яркость в ночном режиме
    uint8_t nightBrightness = 20;

    // Интервал обновления
    uint16_t refreshRate = 20;
};


// ============================================================
// LED MATRIX
// ============================================================

struct Matrix
{
    // Включена ли матрица
    bool enabled = true;

    // Яркость 0-100%
    uint8_t brightness = 30;

    // Автоматическая яркость
    bool autoBrightness = false;

    // Яркость ночью
    uint8_t nightBrightness = 10;

    // Ночной режим
    bool nightMode = true;

    // Показывать матрицу во время ожидания
    bool idleEnabled = true;

    // Анимация по умолчанию
    uint8_t defaultAnimation = 0;

    // Скорость анимации
    uint8_t animationSpeed = 50;

    // Цвет матрицы
    uint8_t colorR = 255;
    uint8_t colorG = 255;
    uint8_t colorB = 255;

    // Использовать автоматический цвет
    bool autoColor = false;

    // Показывать часы на матрице
    bool showClock = false;

    // Показывать уведомления
    bool showNotifications = true;
};


// ============================================================
// COB LED
// ============================================================

struct CobLed
{
    // Общий флаг
    bool enabled = true;

    // Основная яркость
    uint8_t brightness = 50;

    // Левая/первая лампа
    bool led1Enabled = true;
    uint8_t led1Brightness = 50;

    // Правая/вторая лампа
    bool led2Enabled = true;
    uint8_t led2Brightness = 50;

    // Автоматическая яркость
    bool autoBrightness = false;

    // Ночная яркость
    uint8_t nightBrightness = 10;

    // Плавное включение
    bool fadeEnabled = true;

    // Время плавного изменения, мс
    uint16_t fadeTime = 500;
};


// ============================================================
// AUDIO
// ============================================================

struct Audio
{
    // Звук включён
    bool enabled = true;

    // Громкость 0-100
    uint8_t volume = 60;

    // Максимальная громкость
    uint8_t maxVolume = 80;

    // Звуки интерфейса
    bool interfaceSounds = true;

    // Звук уведомлений
    bool notificationSounds = true;

    // Звук будильника
    bool alarmSound = true;

    // Звук таймера
    bool timerSound = true;

    // Звук запуска
    bool startupSound = true;

    // Баланс
    int8_t balance = 0;

    // Эквалайзер
    bool equalizerEnabled = false;

    // Бас
    int8_t bass = 0;

    // Средние
    int8_t mid = 0;

    // Высокие
    int8_t treble = 0;
};


// ============================================================
// MICROPHONE
// ============================================================

struct Microphone
{
    // Микрофон включён
    bool enabled = true;

    // Автоматическая регулировка усиления
    bool autoGain = true;

    // Усиление
    uint8_t gain = 50;

    // Чувствительность
    uint8_t sensitivity = 50;

    // Подавление шума
    bool noiseReduction = true;

    // Обнаружение голоса
    bool voiceDetection = true;

    // Wake word
    bool wakeWordEnabled = false;
};


// ============================================================
// TEMPERATURE / HUMIDITY
// ============================================================

struct Environment
{
    // Температура
    bool temperatureEnabled = true;

    // Влажность
    bool humidityEnabled = true;

    // Использовать °C
    bool celsius = true;

    // Коррекция температуры
    float temperatureOffset = 0.0f;

    // Коррекция влажности
    float humidityOffset = 0.0f;

    // Показывать температуру
    bool showTemperature = true;

    // Показывать влажность
    bool showHumidity = true;
};


// ============================================================
// LIGHT SENSOR
// ============================================================

struct LightSensor
{
    // Датчик освещённости
    bool enabled = true;

    // Использовать для автояркости
    bool autoBrightness = true;

    // Минимальная яркость
    uint8_t minBrightness = 10;

    // Максимальная яркость
    uint8_t maxBrightness = 80;

    // Порог темноты
    uint16_t darkThreshold = 20;

    // Порог яркого света
    uint16_t brightThreshold = 500;
};


// ============================================================
// DISTANCE SENSOR
// ============================================================

struct DistanceSensor
{
    // VL53L8CX включён
    bool enabled = true;

    // Использовать для пробуждения
    bool wakeOnPresence = true;

    // Использовать для управления интерфейсом
    bool interactionEnabled = true;

    // Расстояние присутствия, мм
    uint16_t presenceDistance = 1000;

    // Время до засыпания после ухода
    uint16_t absenceTimeout = 30;
};


// ============================================================
// CLOCK
// ============================================================

struct Clock
{
    // Автоматическая синхронизация
    bool autoSync = true;

    // Использовать RTC
    bool useRTC = true;

    // Использовать NTP
    bool useNTP = true;

    // 24 часа
    bool use24HourFormat = true;

    // Показывать секунды
    bool showSeconds = true;

    // Показывать дату
    bool showDate = true;

    // Показывать день недели
    bool showDayOfWeek = true;

    // Показывать год
    bool showYear = true;

    // Часовой пояс
    char timezone[32] = "Europe/Moscow";
};


// ============================================================
// NOTIFICATIONS
// ============================================================

struct Notifications
{
    // Уведомления включены
    bool enabled = true;

    // Показывать уведомления на дисплее
    bool displayEnabled = true;

    // Показывать уведомления на матрице
    bool matrixEnabled = true;

    // Звуковые уведомления
    bool soundEnabled = true;

    // Показывать системные уведомления
    bool systemNotifications = true;

    // Показывать уведомления сенсоров
    bool sensorNotifications = true;

    // Показывать уведомления Wi-Fi
    bool wifiNotifications = true;

    // Продолжительность уведомления
    uint16_t duration = 3000;
};


// ============================================================
// ALARM
// ============================================================

struct Alarm
{
    // Будильник включён
    bool enabled = false;

    // Час
    uint8_t hour = 7;

    // Минута
    uint8_t minute = 0;

    // Дни недели
    //
    // bit 0 = Sunday
    // bit 1 = Monday
    // bit 2 = Tuesday
    // bit 3 = Wednesday
    // bit 4 = Thursday
    // bit 5 = Friday
    // bit 6 = Saturday
    //
    uint8_t days = 0b0111110;

    // Громкость будильника
    uint8_t volume = 60;

    // Постепенное увеличение громкости
    bool fadeIn = true;

    // Длительность плавного увеличения
    uint16_t fadeTime = 30000;

    // Использовать свет
    bool useLight = true;

    // Использовать матрицу
    bool useMatrix = true;

    // Использовать звук
    bool useSound = true;

    // Повтор будильника
    bool snoozeEnabled = true;

    // Интервал snooze
    uint8_t snoozeMinutes = 5;
};


// ============================================================
// TIMER
// ============================================================

struct Timer
{
    bool soundEnabled = true;

    bool displayEnabled = true;

    bool matrixEnabled = true;

    uint8_t volume = 60;

    // Автоматически запускать свет
    bool lightEnabled = false;
};


// ============================================================
// WIFI
// ============================================================

struct WiFi
{
    // Wi-Fi включён
    bool enabled = true;

    // Автоматически подключаться
    bool autoConnect = true;

    // Синхронизировать время
    bool syncTime = true;

    // Подключаться после загрузки
    bool connectOnBoot = true;

    // Показывать статус на дисплее
    bool showStatus = true;
};


// ============================================================
// POWER / SLEEP
// ============================================================

struct Power
{
    // Энергосбережение
    bool sleepEnabled = false;

    // Время бездействия до сна
    uint16_t sleepMinutes = 10;

    // Выключать дисплеи
    bool displaySleep = true;

    // Выключать матрицу
    bool matrixSleep = true;

    // Выключать COB
    bool cobSleep = true;

    // Выключать аудио
    bool audioSleep = true;

    // Просыпаться при движении
    bool wakeOnMotion = true;

    // Просыпаться при нажатии энкодера
    bool wakeOnButton = true;

    // Ночная яркость
    bool nightMode = true;
};


// ============================================================
// UI
// ============================================================

struct UI
{
    // Тема
    uint8_t theme = 0;

    // Анимации интерфейса
    bool animations = true;

    // Скорость анимаций
    uint8_t animationSpeed = 50;

    // Время показа меню
    uint16_t menuTimeout = 10000;

    // Показывать подсказки
    bool hints = true;

    // Звуки интерфейса
    bool sounds = true;

    // Вибрация
    bool vibration = false;
};


// ============================================================
// ENCODER
// ============================================================

struct Encoder
{
    // Инвертировать направление
    bool inverted = false;

    // Шаг изменения
    uint8_t step = 1;

    // Быстрое изменение
    bool acceleration = true;

    // Скорость ускорения
    uint8_t accelerationSpeed = 50;

    // Нажатие энкодера
    bool buttonEnabled = true;
};


// ============================================================
// STORAGE
// ============================================================

struct Storage
{
    // Автоматически сохранять настройки
    bool autoSave = true;

    // Интервал автосохранения
    uint32_t saveInterval = 60000;

    // Создавать резервную копию
    bool backupEnabled = true;
};


// ============================================================
// SYSTEM
// ============================================================

struct System
{
    // Автоматический запуск
    bool autoStart = true;

    // Показывать splash screen
    bool splashEnabled = true;

    // Длительность splash
    uint16_t splashDuration = 2000;

    // Логирование
    bool loggingEnabled = true;

    // Отладка
    bool debugEnabled = false;

    // Язык интерфейса
    uint8_t language = 0;

    // 0 = русский
    // 1 = English
};


// ============================================================
// COMPLETE SETTINGS
// ============================================================

struct Data
{
    Display display;
    Matrix matrix;
    CobLed cobLed;

    Audio audio;
    Microphone microphone;

    Environment environment;
    LightSensor lightSensor;
    DistanceSensor distanceSensor;

    Clock clock;

    Notifications notifications;

    Alarm alarm;
    Timer timer;

    WiFi wifi;

    Power power;

    UI ui;
    Encoder encoder;

    Storage storage;

    System system;
};

} // namespace Settings
