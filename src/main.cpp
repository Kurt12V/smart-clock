
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
// НАСТРОЙКИ ВОЛНЫ
// ============================================================

// Время перехода от одного COB к следующему
constexpr uint32_t WAVE_TIME = 1500;

// Шаг обновления
constexpr uint32_t UPDATE_TIME = 10;

// Максимальная яркость
constexpr uint8_t MAX_BRIGHTNESS = 255;

// ============================================================
// Вспомогательная функция
// ============================================================

uint8_t calculateBrightness(
    float position,
    float cobPosition
)
{
    float distance = fabs(position - cobPosition);

    // COB светится только в пределах своей зоны
    if (distance >= 1.0f)
        return 0;

    // Плавная форма волны:
    //
    // distance = 0     -> 255
    // distance = 0.5   -> ~128
    // distance = 1     -> 0
    //
    float brightness = 1.0f - distance;

    // Небольшое сглаживание
    brightness = brightness * brightness * (3.0f - 2.0f * brightness);

    return (uint8_t)(brightness * MAX_BRIGHTNESS);
}

// ============================================================
// Волна
// ============================================================

void updateWave(float position)
{
    cobManager.setBrightness(
        1,
        calculateBrightness(position, 0.0f)
    );

    cobManager.setBrightness(
        2,
        calculateBrightness(position, 1.0f)
    );

    cobManager.setBrightness(
        3,
        calculateBrightness(position, 2.0f)
    );

    cobManager.setBrightness(
        4,
        calculateBrightness(position, 3.0f)
    );
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println(" COB RUNNING WAVE TEST");
    Serial.println("================================");

    cobManager.begin();

    cobManager.off();

    delay(500);

    Serial.println("Wave started");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    static uint32_t lastUpdate = 0;

    static float position = 0.0f;

    uint32_t now = millis();

    if (now - lastUpdate < UPDATE_TIME)
        return;

    lastUpdate = now;

    // Скорость движения волны
    const float speed =
        3.0f / (float)WAVE_TIME;

    position += speed * UPDATE_TIME;

    // --------------------------------------------------------
    // Когда дошли до COB4
    // --------------------------------------------------------

    if (position >= 3.0f)
    {
        position = 3.0f;

        updateWave(position);

        delay(50);

        // Теперь идём обратно
        static bool forward = false;

        while (position > 0.0f)
        {
            uint32_t start = millis();

            while (millis() - start < UPDATE_TIME)
            {
                uint32_t current = millis();

                float delta =
                    (float)(current - start) /
                    (float)UPDATE_TIME;

                float currentPosition =
                    position -
                    speed * delta;

                if (currentPosition < 0.0f)
                    currentPosition = 0.0f;

                updateWave(currentPosition);

                delay(1);
            }

            position -= speed * UPDATE_TIME;

            if (position < 0.0f)
                position = 0.0f;
        }

        updateWave(0.0f);

        delay(50);

        return;
    }

    updateWave(position);
}