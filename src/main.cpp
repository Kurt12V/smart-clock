#include <Arduino.h>

#include "Settings.h"

#include "./managers/SPIManager.h"
#include "./managers/SDManager.h"
#include "./managers/SoundManager.h"

// ============================================================
// SETTINGS
// ============================================================

Settings::Data settings;

// ============================================================
// HARDWARE
// ============================================================

SPIManager spiManager;
SDManager sdManager;

// ============================================================
// SOUND
// ============================================================

SoundManager soundManager(
    sdManager,
    settings.audio
);

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(115200);

    delay(1000);

    Serial0.println();
    Serial0.println("================================");
    Serial0.println(" SmartClock - Audio Test");
    Serial0.println("================================");

    // ========================================================
    // AUDIO SETTINGS
    // ========================================================

    Serial0.println();
    Serial0.println("Audio settings:");

    Serial0.print("Enabled: ");
    Serial0.println(
        settings.audio.enabled
            ? "YES"
            : "NO"
    );

    Serial0.print("Global volume: ");
    Serial0.print(
        settings.audio.volume
    );
    Serial0.println("%");

    Serial0.print("Sample rate: ");
    Serial0.print(
        settings.audio.sampleRate
    );
    Serial0.println(" kHz");

    // ========================================================
    // SPI
    // ========================================================

    Serial0.println();
    Serial0.println("[1] Starting SPI...");

    if (!spiManager.begin())
    {
        Serial0.println(
            "[ERROR] SPI initialization failed!"
        );

        return;
    }

    Serial0.println(
        "[OK] SPI initialized"
    );

    // ========================================================
    // SD
    // ========================================================

    Serial0.println();
    Serial0.println("[2] Starting SD...");

    // SD CS = GPIO35
    if (!sdManager.begin(35))
    {
        Serial0.println(
            "[ERROR] SD initialization failed!"
        );

        return;
    }

    Serial0.println(
        "[OK] SD initialized"
    );

    // ========================================================
    // ALARM FILE
    // ========================================================

    const char* alarmPath =
        "/audio/alarms/alarm_1.wav";

    Serial0.println();
    Serial0.print(
        "[3] Checking alarm file: "
    );

    Serial0.println(alarmPath);

    if (!sdManager.card().exists(alarmPath))
    {
        Serial0.println(
            "[ERROR] Alarm file not found!"
        );

        Serial0.println();
        Serial0.println(
            "Expected path on SD:"
        );

        Serial0.println(
            "/audio/alarms/alarm_1.wav"
        );

        return;
    }

    Serial0.println(
        "[OK] Alarm file found"
    );

    // ========================================================
    // SOUND MANAGER
    // ========================================================

    Serial0.println();
    Serial0.println(
        "[4] Starting SoundManager..."
    );

    if (!soundManager.begin())
    {
        Serial0.println(
            "[ERROR] SoundManager initialization failed!"
        );

        return;
    }

    Serial0.println(
        "[OK] SoundManager initialized"
    );

    // ========================================================
    // SHOW VOLUME
    // ========================================================

    Serial0.println();
    Serial0.println(
        "================================"
    );

    Serial0.print(
        "Global volume: "
    );

    Serial0.print(
        soundManager.getGlobalVolume()
    );

    Serial0.println("%");

    Serial0.println(
        "Alarm volume: 100% LOCAL"
    );

    Serial0.println(
        "Global volume does NOT affect alarm."
    );

    Serial0.println(
        "================================"
    );

    // ========================================================
    // PLAY ALARM
    // ========================================================
    //
    // Local volume = 100%
    //
    // Fade-in = 30 seconds
    //
    // Fade-out = 0
    //
    // Curve = Exponential
    //
    // Settings.audio.volume = 20%
    // DOES NOT affect this alarm.
    //
    // ========================================================

    Serial0.println();
    Serial0.println(
        "Starting alarm..."
    );

    bool result =
        soundManager.playAlarm(
            alarmPath,

            100,    // local volume

            30000,  // fade-in 30 sec

            0,      // no fade-out

            SoundManager::FadeCurve::Exponential
        );

    if (!result)
    {
        Serial0.println(
            "[ERROR] Failed to start alarm!"
        );

        return;
    }

    Serial0.println(
        "[OK] Alarm started"
    );

    Serial0.println();
    Serial0.println(
        "Commands:"
    );

    Serial0.println(
        "  p - pause"
    );

    Serial0.println(
        "  r - resume"
    );

    Serial0.println(
        "  s - stop"
    );

    Serial0.println(
        "  f - fade-out 3 sec"
    );

    Serial0.println(
        "  + - local volume +10%"
    );

    Serial0.println(
        "  - - local volume -10%"
    );

    Serial0.println();
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    soundManager.update();

    // ========================================================
    // SERIAL COMMANDS
    // ========================================================

    if (Serial0.available())
    {
        char command =
            Serial0.read();

        switch (command)
        {
            // ------------------------------------------------
            // PAUSE
            // ------------------------------------------------

            case 'p':
            case 'P':
            {
                Serial0.println(
                    "[CMD] Pause"
                );

                soundManager.pause();

                break;
            }

            // ------------------------------------------------
            // RESUME
            // ------------------------------------------------

            case 'r':
            case 'R':
            {
                Serial0.println(
                    "[CMD] Resume"
                );

                soundManager.resume();

                break;
            }

            // ------------------------------------------------
            // STOP
            // ------------------------------------------------

            case 's':
            case 'S':
            {
                Serial0.println(
                    "[CMD] Stop"
                );

                soundManager.stop();

                break;
            }

            // ------------------------------------------------
            // FADE OUT
            // ------------------------------------------------

            case 'f':
            case 'F':
            {
                Serial0.println(
                    "[CMD] Fade-out 3 sec"
                );

                soundManager.fadeOut(
                    3000,
                    SoundManager::FadeCurve::Exponential
                );

                break;
            }

            // ------------------------------------------------
            // VOLUME +
            // ------------------------------------------------

            case '+':
            {
                uint8_t volume =
                    soundManager.getLocalVolume();

                if (volume <= 90)
                    volume += 10;
                else
                    volume = 100;

                soundManager.setLocalVolume(
                    volume
                );

                Serial0.print(
                    "[CMD] Local volume: "
                );

                Serial0.print(
                    volume
                );

                Serial0.println("%");

                break;
            }

            // ------------------------------------------------
            // VOLUME -
            // ------------------------------------------------

            case '-':
            {
                uint8_t volume =
                    soundManager.getLocalVolume();

                if (volume >= 10)
                    volume -= 10;
                else
                    volume = 0;

                soundManager.setLocalVolume(
                    volume
                );

                Serial0.print(
                    "[CMD] Local volume: "
                );

                Serial0.print(
                    volume
                );

                Serial0.println("%");

                break;
            }

            default:
                break;
        }
    }

    delay(1);
}
