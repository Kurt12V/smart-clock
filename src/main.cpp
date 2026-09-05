#include <Arduino.h>
#include "../include/Pins.h"
#include "../include/Config.h"
#include "../include/Constants.h"
#include "../include/Version.h"
#include <Wire.h>
#include "RTC.h"
#include "./managers/SensorsManager.h"
#include "./core/ClockSystem.h"
#include <WiFi.h>


// =====================================================
// Wi-Fi настройки
// =====================================================

const char* WIFI_SSID     = "tpl45";
const char* WIFI_PASSWORD = "12713714";

// =====================================================
// Глобальные объекты
// =====================================================

RTC rtc;
ClockManager clockManager(rtc);
ClockSystem clockSystem;

// =====================================================
// Setup
// =====================================================

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n=== RTC Test ===\n");

    // 1. Проверка I2C
    Serial.println("[1] Checking I2C...");
    Wire.begin(21, 22);
    Wire.beginTransmission(0x68);
    if (Wire.endTransmission() != 0) {
        Serial.println("[ERROR] DS3231 not found!");
        return;
    }
    Serial.println("[OK] DS3231 found");

    // 2. Инициализация RTC
    Serial.println("[2] Initializing RTC...");
    if (!rtc.begin()) {
        Serial.println("[ERROR] RTC begin failed!");
        return;
    }
    Serial.println("[OK] RTC initialized");

    // 3. Инициализация ClockManager
    Serial.println("[3] Initializing ClockManager...");
    if (!clockManager.begin("Europe/Kyiv")) {
        Serial.println("[ERROR] ClockManager begin failed!");
        return;
    }
    Serial.println("[OK] ClockManager initialized");

    // 4. Подключение к Wi-Fi
    Serial.println("[4] Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[OK] Wi-Fi connected");
        
        // 5. Синхронизация через NTP
        Serial.println("[5] Syncing with NTP...");
        if (clockManager.syncFromNTP()) {
            Serial.println("[OK] Time synchronized");
        } else {
            Serial.println("[ERROR] NTP sync failed");
        }
    } else {
        Serial.println("[WARN] Wi-Fi not connected");
        Serial.println("[WARN] Set time manually");
        
        // Установка времени вручную (UTC)
        clockManager.setUTCTime(2026, 9, 5, 10, 0, 0);
        Serial.println("[OK] Time set manually");
    }

    // 6. Чтение времени и даты
    Serial.println("[6] Reading time and date...");
    
    TimeData time = clockManager.getTimeData();
    DateData date = clockManager.getDateData();
    
    if (time.valid && date.valid) {
        Serial.printf("[OK] Date: %02d.%02d.%04d\n", 
            date.day, date.month, date.year);
        Serial.printf("[OK] Time: %02d:%02d:%02d\n", 
            time.hour, time.minute, time.second);
        
        // День недели
        Constants::DayOfWeek dow = clockManager.getDayOfWeek();
        const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", 
                              "Thursday", "Friday", "Saturday"};
        Serial.printf("[OK] Day: %s\n", days[(int)dow]);
    } else {
        Serial.println("[ERROR] Time or date not valid!");
    }

    Serial.println("\n[READY]\n");
    Serial.println();
    Serial.println("================================");
    Serial.println("       ClockSystem TEST");
    Serial.println("================================");

    // Инициализация часов
    if (!clockSystem.begin("Europe/Kyiv"))
    {
        Serial.println("ERROR: ClockSystem begin failed!");
        return;
    }

    Serial.println("ClockSystem initialized");

    // Часовой пояс
    Serial.print("Timezone: ");
    Serial.println(clockSystem.getTimeZone());

    // Синхронизация через NTP
    Serial.println("Synchronizing time...");

    if (clockSystem.syncFromNTP())
    {
        Serial.println("NTP synchronization OK");
    }
    else
    {
        Serial.println("NTP synchronization FAILED");
    }

    Serial.println();
    Serial.println("Clock test started");
    Serial.println();
}

// =====================================================
// Loop
// =====================================================

void loop()
{
    delay(1000);
    
    // Получаем время и дату
    TimeData time = clockManager.getTimeData();
    DateData date = clockManager.getDateData();
    
    // Вывод с проверкой валидности
    if (time.valid && date.valid) {
        // Форматированный вывод даты и времени
        Serial.printf("%02d.%02d.%04d  %02d:%02d:%02d\n",
            date.day, date.month, date.year,
            time.hour, time.minute, time.second);
    } else {
        Serial.println("--.--.----  --:--:--");
    }
    uint8_t H1 = clockSystem.getHourTens();
    uint8_t H2 = clockSystem.getHourOnes();

    uint8_t M1 = clockSystem.getMinuteTens();
    uint8_t M2 = clockSystem.getMinuteOnes();

    // ==========================================
    // Вывод HH:MM
    // ==========================================

    Serial.print("Time: ");

    Serial.print(H1);
    Serial.print(H2);

    Serial.print(":");

    Serial.print(M1);
    Serial.println(M2);
    DateData dateData = clockSystem.getDateData();

    Serial.print("Date: ");

    if (dateData.valid)
    {
        Serial.print(dateData.day);
        Serial.print(".");
        Serial.print(dateData.month);
        Serial.print(".");
        Serial.println(dateData.year);
    }
    else
    {
        Serial.println("--.--.----");
    }

    // ==========================================
    // Часовой пояс
    // ==========================================

    Serial.print("Timezone: ");
    Serial.println(clockSystem.getTimeZone());

    // ==========================================
    // Проверка валидности
    // ==========================================

    Serial.print("Time valid: ");

    if (clockSystem.isTimeValid())
        Serial.println("YES");
    else
        Serial.println("NO");

    Serial.println("-------------------------------");
}