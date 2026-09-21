
#include <Arduino.h>

#define COB1 18
#define COB2 46
#define COB3 40
#define COB4 41

void setup()
{
    Serial.begin(115200);

    delay(1000);

    pinMode(COB1, OUTPUT);
    pinMode(COB2, OUTPUT);
    pinMode(COB3, OUTPUT);
    pinMode(COB4, OUTPUT);

    // Сначала LOW
    digitalWrite(COB1, LOW);
    digitalWrite(COB2, LOW);
    digitalWrite(COB3, LOW);
    digitalWrite(COB4, LOW);

    Serial.println("================================");
    Serial.println("LD1500SB GPIO TEST");
    Serial.println("================================");
}

void loop()
{
    // ========================================================
    // ВСЕ ВЫКЛ
    // ========================================================

    Serial.println("ALL LOW");

    digitalWrite(COB1, LOW);
    digitalWrite(COB2, LOW);
    digitalWrite(COB3, LOW);
    digitalWrite(COB4, LOW);

    delay(3000);


    // ========================================================
    // COB1
    // ========================================================

    Serial.println("COB1 HIGH");

    digitalWrite(COB1, HIGH);

    delay(2000);

    Serial.println("COB1 LOW");

    digitalWrite(COB1, LOW);

    delay(1000);


    // ========================================================
    // COB2
    // ========================================================

    Serial.println("COB2 HIGH");

    digitalWrite(COB2, HIGH);

    delay(2000);

    Serial.println("COB2 LOW");

    digitalWrite(COB2, LOW);

    delay(1000);


    // ========================================================
    // COB3
    // ========================================================

    Serial.println("COB3 HIGH");

    digitalWrite(COB3, HIGH);

    delay(2000);

    Serial.println("COB3 LOW");

    digitalWrite(COB3, LOW);

    delay(1000);


    // ========================================================
    // COB4
    // ========================================================

    Serial.println("COB4 HIGH");

    digitalWrite(COB4, HIGH);

    delay(2000);

    Serial.println("COB4 LOW");

    digitalWrite(COB4, LOW);

    delay(1000);
}