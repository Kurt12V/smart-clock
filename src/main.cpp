#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>


// Forward declarations
void drawStaticScreen(Adafruit_ST7789 &display, uint8_t number);
void drawTime(Adafruit_ST7789 &display, uint32_t seconds);

constexpr int TFT_SCLK = 12;
constexpr int TFT_MOSI = 11;
constexpr int TFT_DC = 2;
constexpr int TFT_BL = 1;
constexpr int TFT_CS_1 = 4;
constexpr int TFT_CS_2 = 5;
constexpr int TFT_CS_3 = 6;
constexpr int TFT_CS_4 = 7;
constexpr int TFT_RST_1 = 3;
constexpr int TFT_RST_2 = 10;
constexpr int TFT_RST_3 = 13;
constexpr int TFT_RST_4 = 42;

Adafruit_ST7789 display1(TFT_CS_1, TFT_DC, TFT_RST_1);
Adafruit_ST7789 display2(TFT_CS_2, TFT_DC, TFT_RST_2);
Adafruit_ST7789 display3(TFT_CS_3, TFT_DC, TFT_RST_3);
Adafruit_ST7789 display4(TFT_CS_4, TFT_DC, TFT_RST_4);
Adafruit_ST7789 *displays[] = {&display1, &display2, &display3, &display4};

void drawStaticScreen(Adafruit_ST7789 &display, uint8_t number) {
  display.fillScreen(ST77XX_BLACK);
  display.setTextWrap(false);
  display.setTextColor(ST77XX_CYAN);
  display.setTextSize(2);
  display.setCursor(18, 48);
  display.print("SCREEN ");
  display.println(number);

  display.setTextColor(ST77XX_WHITE);
  display.setTextSize(1);
  display.setCursor(22, 86);
  display.println("ESP32-S3 / ST7789");
  display.drawRoundRect(14, 150, 144, 48, 8, ST77XX_GREEN);
  display.setTextColor(ST77XX_GREEN);
  display.setTextSize(2);
  display.setCursor(40, 165);
  display.println("READY");
}

void drawTime(Adafruit_ST7789 &display, uint32_t seconds) {
  display.fillRect(14, 215, 144, 30, ST77XX_BLACK);
  display.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
  display.setTextSize(2);
  display.setCursor(24, 222);
  display.print("Time: ");
  display.print(seconds);
  display.print("s");
}

void setup() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, -1);
  const int chipSelectPins[] = {TFT_CS_1, TFT_CS_2, TFT_CS_3, TFT_CS_4};
  for (uint8_t i = 0; i < 4; ++i) {
    pinMode(chipSelectPins[i], OUTPUT);
    digitalWrite(chipSelectPins[i], HIGH);
  }

  for (uint8_t i = 0; i < 4; ++i) {
    displays[i]->init(172, 320);
    displays[i]->setRotation(0);
    drawStaticScreen(*displays[i], i + 1);
  }
}

void loop() {
  static uint32_t lastUpdateMs = 0;
  const uint32_t now = millis();
  if (now - lastUpdateMs < 1000) {
    return;
  }
  lastUpdateMs = now;

  const uint32_t seconds = now / 1000;
  for (uint8_t i = 0; i < 4; ++i) {
    drawTime(*displays[i], seconds);
  }
}