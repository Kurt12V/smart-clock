#include <Arduino.h>

#include "Pins.h"

#include "./hardware/light/CobLed.h"
#include "./managers/CobLedManager.h"


// ============================================================
// COB
// ============================================================

CobLed cob1(PIN_PWM_LD1, 0);
CobLed cob2(PIN_PWM_LD2, 1);
CobLed cob3(PIN_PWM_LD3, 2);
CobLed cob4(PIN_PWM_LD4, 3);


CobLedManager cobManager(
    cob1,
    cob2,
    cob3,
    cob4
);


// ============================================================
// WAVE
// ============================================================

const uint32_t WAVE_PERIOD = 4000;

const uint32_t WAVE_UPDATE = 10;

const uint8_t WAVE_MAX = 255;

uint32_t lastWaveUpdate = 0;


// ============================================================
// Wave brightness
// ============================================================

uint8_t waveBrightness(
    float position
)
{
    while (position >= 1.0f)
        position -= 1.0f;

    while (position < 0.0f)
        position += 1.0f;


    float value =
        sinf(
            position *
            2.0f *
            PI
        );


    // -1..1 -> 0..1

    value =
        (value + 1.0f) * 0.5f;


    // Делаем мягкую волну

    value =
        value * value;


    return (uint8_t)(
        value *
        WAVE_MAX
    );
}


// ============================================================
// Update wave
// ============================================================

void updateWave()
{
    uint32_t now =
        millis();


    if (
        now - lastWaveUpdate <
        WAVE_UPDATE
    )
    {
        return;
    }


    lastWaveUpdate =
        now;


    float position =
        (float)(
            now % WAVE_PERIOD
        )
        /
        (float)WAVE_PERIOD;


    const float phase =
        0.25f;


    // ========================================================
    // COB 1
    // ========================================================

    cobManager.set(
        1,
        waveBrightness(
            position
        )
    );


    // ========================================================
    // COB 2
    // ========================================================

    cobManager.set(
        2,
        waveBrightness(
            position + phase
        )
    );


    // ========================================================
    // COB 3
    // ========================================================

    cobManager.set(
        3,
        waveBrightness(
            position + phase * 2.0f
        )
    );


    // ========================================================
    // COB 4
    // ========================================================

    cobManager.set(
        4,
        waveBrightness(
            position + phase * 3.0f
        )
    );
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println(
        "=============================="
    );

    Serial.println(
        "LD1500SB COB WAVE TEST"
    );

    Serial.println(
        "=============================="
    );


    Serial.printf(
        "COB1 GPIO: %d\n",
        PIN_PWM_LD1
    );

    Serial.printf(
        "COB2 GPIO: %d\n",
        PIN_PWM_LD2
    );

    Serial.printf(
        "COB3 GPIO: %d\n",
        PIN_PWM_LD3
    );

    Serial.printf(
        "COB4 GPIO: %d\n",
        PIN_PWM_LD4
    );


    // ========================================================
    // Инициализация
    // ========================================================

    cobManager.begin();


    // ========================================================
    // Начальное состояние
    // ========================================================

    cobManager.setAll(0);


    Serial.println(
        "PWM initialized"
    );

    Serial.println(
        "Wave started"
    );
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    updateWave();

    cobManager.update();

    delay(1);
}
