
#include <Arduino.h>

// ============================================================
// COB GPIO
// ============================================================

constexpr uint8_t COB_PINS[] = {
    40, 41, 20, 21
};

constexpr uint8_t COB_COUNT = 4;

// ============================================================
// PWM CONFIGURATION - ESP32 CORE 2.x
// ============================================================

constexpr uint8_t PWM_CHANNELS[] = {
    0, 1, 2, 3
};

constexpr uint32_t PWM_FREQUENCY = 5000;
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint8_t PWM_MAX = 255;

// ============================================================
// TEST TIMING
// ============================================================

constexpr uint32_t START_DELAY_MS = 10000;
constexpr uint32_t CHANNEL_TEST_MS = 3000;
constexpr uint32_t FADE_DURATION_MS = 60000;

// ============================================================
// TEST STATE
// ============================================================

uint32_t fadeStart = 0;
uint32_t lastFadeUpdate = 0;

bool channelTestCompleted = false;
bool fadeStarted = false;
bool fadeCompleted = false;

// ============================================================
// COB CONTROL
// ============================================================

void cobOff() {

    for (uint8_t i = 0; i < COB_COUNT; i++) {
        ledcWrite(PWM_CHANNELS[i], 0);
    }

    Serial0.println("[COB] ALL OFF");
}

void cobSetChannel(uint8_t channel, uint8_t brightness) {

    if (channel >= COB_COUNT) {
        return;
    }

    ledcWrite(PWM_CHANNELS[channel], brightness);

    Serial0.print("[COB] Channel ");
    Serial0.print(channel + 1);
    Serial0.print(" PWM = ");
    Serial0.println(brightness);
}

void cobSetAll(uint8_t brightness) {

    for (uint8_t i = 0; i < COB_COUNT; i++) {
        ledcWrite(PWM_CHANNELS[i], brightness);
    }
}

// ============================================================
// SETUP
// ============================================================

void setup() {

    Serial0.begin(115200);
    delay(500);

    Serial0.println();
    Serial0.println("================================");
    Serial0.println("       COB DIRECT TEST");
    Serial0.println("       ESP32 CORE 2.x");
    Serial0.println("================================");

    // --------------------------------------------------------
    // INITIALIZE PWM
    // --------------------------------------------------------

    for (uint8_t i = 0; i < COB_COUNT; i++) {

        bool configured = ledcSetup(
            PWM_CHANNELS[i],
            PWM_FREQUENCY,
            PWM_RESOLUTION
        );

        ledcAttachPin(
            COB_PINS[i],
            PWM_CHANNELS[i]
        );

        ledcWrite(PWM_CHANNELS[i], 0);

        Serial0.print("[COB] GPIO ");
        Serial0.print(COB_PINS[i]);
        Serial0.print(" CHANNEL ");
        Serial0.print(PWM_CHANNELS[i]);
        Serial0.print(" CONFIG: ");
        Serial0.println(configured > 0 ? "OK" : "FAILED");
    }

    // --------------------------------------------------------
    // FORCE ALL CHANNELS OFF
    // --------------------------------------------------------

    cobOff();

    Serial0.println("[TEST] All COB channels OFF");
    Serial0.println("[TEST] Waiting 10 seconds...");

}

// ============================================================
// LOOP
// ============================================================

void loop() {

    const uint32_t now = millis();

    // --------------------------------------------------------
    // STEP 1: WAIT 10 SECONDS
    // --------------------------------------------------------

    if (now < START_DELAY_MS) {
        delay(10);
        return;
    }

    // --------------------------------------------------------
    // STEP 2: TEST EACH CHANNEL
    // --------------------------------------------------------

    if (!channelTestCompleted) {

        Serial0.println("[TEST] Individual channel test");

        for (uint8_t i = 0; i < COB_COUNT; i++) {

            cobOff();

            Serial0.print("[TEST] CHANNEL ");
            Serial0.println(i + 1);

            cobSetChannel(i, PWM_MAX);

            delay(CHANNEL_TEST_MS);

            cobOff();

            delay(1000);
        }

        channelTestCompleted = true;

        Serial0.println("[TEST] Individual test completed");
    }

    // --------------------------------------------------------
    // STEP 3: START FADE
    // --------------------------------------------------------

    if (!fadeStarted) {

        cobOff();

        cobSetAll(0);

        fadeStart = millis();
        lastFadeUpdate = fadeStart;

        fadeStarted = true;

        Serial0.println("[TEST] Starting fade 0-100%");
    }

    // --------------------------------------------------------
    // STEP 4: GRADUAL BRIGHTNESS
    // --------------------------------------------------------

    if (!fadeCompleted &&
        now - lastFadeUpdate >= 50) {

        lastFadeUpdate = now;

        const uint32_t elapsed = now - fadeStart;

        uint8_t brightness;

        if (elapsed >= FADE_DURATION_MS) {

            brightness = PWM_MAX;
            fadeCompleted = true;

        } else {

            brightness = static_cast<uint8_t>(
                (elapsed * PWM_MAX) / FADE_DURATION_MS
            );
        }

        cobSetAll(brightness);

        static int previousPercent = -1;

        const int percent =
            (brightness * 100) / PWM_MAX;

        if (percent != previousPercent) {

            previousPercent = percent;

            Serial0.print("[TEST] Brightness: ");
            Serial0.print(percent);
            Serial0.println("%");
        }

        if (fadeCompleted) {
            Serial0.println("[TEST] Fade completed");
        }
    }

    delay(1);
}
