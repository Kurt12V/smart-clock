#include <Adafruit_NeoPixel.h>

#define LED_PIN     14
#define MATRIX_W    16
#define MATRIX_H    16
#define NUM_LEDS    (MATRIX_W * MATRIX_H)
#define BRIGHTNESS  20

#define COOLING     55
#define SPARKING    120
#define FPS         60

// Направление огня:
//   0 = снизу вверх   (по умолчанию)
//   1 = слева направо (поворот на 90° по часовой)
//   2 = справа налево (поворот на 90° против часовой)
//   3 = сверху вниз   (переворот на 180°)
#define FIRE_DIR    2

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

uint16_t XY(uint8_t x, uint8_t y) {
  if (y & 1) return y * MATRIX_W + (MATRIX_W - 1 - x);
  return y * MATRIX_W + x;
}

uint8_t scale8_video(uint8_t i, uint8_t scale) {
  uint8_t j = ((uint16_t)i * (uint16_t)scale) >> 8;
  return j + (i && scale && !j);
}

uint32_t fireColor(uint8_t temperature) {
  uint8_t t192 = scale8_video(temperature, 191);
  uint8_t heatramp = t192 & 0x3F;
  heatramp <<= 2;
  uint8_t r, g, b;
  if (t192 & 0x80)      { r = 255; g = 255; b = heatramp; }
  else if (t192 & 0x40) { r = 255; g = heatramp; b = 0; }
  else                  { r = heatramp; g = 0; b = 0; }
  return strip.Color(r, g, b);
}

// heat[a][b]: b=0 — источник огня, b растёт в сторону распространения
static uint8_t heat[MATRIX_W][MATRIX_H];

void setup() {
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.clear();
  strip.show();
  memset(heat, 0, sizeof(heat));
}

void loop() {
  // 1) Остывание
  for (uint8_t a = 0; a < MATRIX_W; a++) {
    for (uint8_t b = 0; b < MATRIX_H; b++) {
      uint8_t cool = random(0, ((COOLING * 10) / MATRIX_H) + 2);
      heat[a][b] = (heat[a][b] > cool) ? heat[a][b] - cool : 0;
    }
  }

  // 2) Подъём тепла от b к b+1 с боковым дрожанием по a
  for (uint8_t a = 0; a < MATRIX_W; a++) {
    for (int8_t b = MATRIX_H - 1; b >= 2; b--) {
      int8_t da = (int8_t)random(-1, 2);
      int8_t na = (int8_t)a + da;
      if (na < 0) na = 0;
      if (na >= MATRIX_W) na = MATRIX_W - 1;

      uint8_t below  = heat[a][b - 1];
      uint8_t below2 = heat[a][b - 2];
      heat[na][b] = (below + below2) / 2;
    }
    heat[a][1] = heat[a][0];
  }

  // 3) Вспышки в источнике (b=0)
  for (uint8_t a = 0; a < MATRIX_W; a++) {
    if (random(0, 255) < SPARKING) {
      uint8_t spark = random(160, 255);
      heat[a][0] = spark;
      heat[a][1] = spark;
    }
  }

  // 4) Вывод с учётом направления
  for (uint8_t a = 0; a < MATRIX_W; a++) {
    for (uint8_t b = 0; b < MATRIX_H; b++) {
      uint8_t x, y;
      switch (FIRE_DIR) {
        case 0:  x = a;                  y = MATRIX_H - 1 - b; break; // снизу вверх
        case 1:  x = b;                  y = a;               break; // слева направо (90° CW)
        case 2:  x = MATRIX_W - 1 - b;   y = MATRIX_H - 1 - a; break; // справа налево (90° CCW)
        case 3:  x = MATRIX_W - 1 - a;   y = b;               break; // сверху вниз (180°)
        default: x = a;                  y = MATRIX_H - 1 - b; break;
      }
      strip.setPixelColor(XY(x, y), fireColor(heat[a][b]));
    }
  }
  strip.show();
  delay(1000 / FPS);
}