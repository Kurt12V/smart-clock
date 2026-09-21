
#include <Arduino.h>

#include "Pins.h"
#include "./hardware/light/CobLed.h"
#include "./managers/CobLedManager.h"

// ============================================================
// COB
// ============================================================

CobLed cob1(PIN_PWM_LD1, 0, 1000, 8);
CobLed cob2(PIN_PWM_LD2, 1, 1000, 8);
CobLed cob3(PIN_PWM_LD3, 2, 1000, 8);
CobLed cob4(PIN_PWM_LD4, 3, 1000, 8);

CobLedManager cobManager(
    cob1,
    cob2,
    cob3,
    cob4
);

// ============================================================
// НАСТРОЙКИ
// ============================================================

constexpr uint32_t STEP_TIME = 10;

// Текущий COB
uint8_t currentCob = 1;

// Текущая яркость
int brightness = 0;

// Направление
bool increasing = true;

// ============================================================
// Установка яркости всех COB
// ============================================================

void setAllOff()
{
    cobManager.setBrightness(1, 0);
    cobManager.setBrightness(2, 0);
    cobManager.setBrightness(3, 0);
    cobManager.setBrightness(4, 0);

    cobManager.off();
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(500);

    Serial.println();
    Serial.println("==============================");
    Serial.println("4 COB FADE TEST");
    Serial.println("==============================");

    cobManager.begin();

    // Сначала всё выключено
    cobManager.off();

    cobManager.setBrightness(1, 0);
    cobManager.setBrightness(2, 0);
    cobManager.setBrightness(3, 0);
    cobManager.setBrightness(4, 0);

    delay(500);

    // Включаем состояние ON,
    // но яркость пока 0
    cobManager.on();

    Serial.println("START");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    static uint32_t lastUpdate = 0;

    uint32_t now = millis();

    if (now - lastUpdate < STEP_TIME)
        return;

    lastUpdate = now;

    // ========================================================
    // ПЛАВНОЕ УВЕЛИЧЕНИЕ
    // ========================================================

    if (increasing)
    {
        brightness++;

        if (brightness >= 255)
        {
            brightness = 255;
            increasing = false;

            Serial.print("COB ");
            Serial.print(currentCob);
            Serial.println(" FULL");
        }
    }

    // ========================================================
    // ПЛАВНОЕ УМЕНЬШЕНИЕ
    // ========================================================

    else
    {
        brightness--;

        if (brightness <= 0)
        {
            brightness = 0;
            increasing = true;

            Serial.print("COB ");
            Serial.print(currentCob);
            Serial.println(" OFF");

            // Следующий COB
            currentCob++;

            if (currentCob > 4)
                currentCob = 1;
        }
    }

    // ========================================================
    // Устанавливаем яркость ТОЛЬКО текущего COB
    // ========================================================

    cobManager.setBrightness(
        currentCob,
        brightness
    );

    // Остальные обязательно выключены
    for (uint8_t i = 1; i <= 4; i++)
    {
        if (i != currentCob)
            cobManager.setBrightness(i, 0);
    }
}

