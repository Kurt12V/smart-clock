#include <Arduino.h>

// 4 канала: GPIO4..7 -> входы EN четырёх драйверов LD1500SB
#define COB1 40
#define COB2 41
#define COB3 20
#define COB4 21

const uint8_t pins[4] = { COB1, COB2, COB3, COB4 };

// КРИТИЧНО для LD1500SB: частота ШИМ < 2 кГц
#define PWM_FREQ 1000    // 1 кГц
#define PWM_RES  8       // 0..255

// Гамма-таблица для плавности, воспринимаемой глазом
uint8_t gammaTable[256];

void buildGamma()
{
    for (int i = 0; i < 256; i++)
    {
        float x = i / 255.0f;
        gammaTable[i] = (uint8_t)(powf(x, 2.2f) * 255.0f + 0.5f);
    }
}

// Плавное изменение яркости канала: from -> to за durationMs (0..255)
void fade(uint8_t ch, int from, int to, uint16_t durationMs)
{
    const int steps = 100;
    uint16_t stepDelay = durationMs / steps;

    for (int i = 0; i <= steps; i++)
    {
        int b = from + (to - from) * i / steps;
        ledcWrite(ch, gammaTable[b]);
        delay(stepDelay);
    }
    ledcWrite(ch, gammaTable[to]);
}

void setup()
{
    Serial0.begin(115200);
    delay(1500);

    buildGamma();

    for (uint8_t i = 0; i < 4; i++)
    {
        if (ledcSetup(i, PWM_FREQ, PWM_RES) == 0) {
            Serial0.printf("ledcSetup FAILED on channel %d\n", i);
            while (1) delay(1000);
        }
        ledcAttachPin(pins[i], i);
        ledcWrite(i, 0);   // старт с выключенного
    }

    Serial0.println("LD1500SB 4-channel PWM ready (1 kHz)");
}

void loop()
{
    // Каждый канал: плавно разжечь -> пауза -> плавно погасить -> пауза
    for (uint8_t i = 0; i < 4; i++)
    {
        Serial0.printf("CH%d: Fade IN\n", i + 1);
        fade(i, 0, 255, 2000);
        delay(1000);

        Serial0.printf("CH%d: Fade OUT\n", i + 1);
        fade(i, 255, 0, 2000);
        delay(1000);
    }

    // Гасим всё перед новым циклом
    Serial0.println("ALL -> 0%");
    for (uint8_t i = 0; i < 4; i++)
        fade(i, 255, 0, 1500);

    delay(2000);
}