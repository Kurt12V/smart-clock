#include "System.h"
#include "Pins.h"

// ============================================================
// CONSTRUCTOR
// ============================================================

System::System()
    : _ready(false)
{
}

// ============================================================
// BEGIN
// ============================================================

bool System::begin()
{
    Serial.begin(115200);
    delay(300);

    Serial.println();
    Serial.println("========================================");
    Serial.println("             SMART CLOCK");
    Serial.println("          SYSTEM INITIALIZE");
    Serial.println("========================================");

    // ========================================================
    // SPI
    // ========================================================

    // Serial.println("[SYSTEM] SPI...");

    // if (!_spi.begin())
    // {
    //     Serial.println("[SYSTEM] SPI FAILED");
    //     return false;
    // }

    // Serial.println("[SYSTEM] SPI OK");


    // ========================================================
    // SD
    // ========================================================

    // Serial.println("[SYSTEM] SD...");

    // if (!_sd.begin(PIN_SD_CS))
    // {
    //     Serial.println("[SYSTEM] SD FAILED");
    // }
    // else
    // {
    //     Serial.println("[SYSTEM] SD OK");
    // }


    // ========================================================
    // DISPLAY
    // ========================================================

    // Serial.println("[SYSTEM] Display...");

    // if (!_display.begin())
    // {
    //     Serial.println("[SYSTEM] Display FAILED");
    //     return false;
    // }

    // Serial.println("[SYSTEM] Display OK");


    // ========================================================
    // SENSORS
    // ========================================================

    Serial.println("[SYSTEM] Sensors...");

    if (!_sensors.begin())
    {
        Serial.println("[SYSTEM] Sensors FAILED");
    }
    else
    {
        Serial.println("[SYSTEM] Sensors OK");
    }


    // ========================================================
    // CLOCK
    // ========================================================

 Serial.println("[SYSTEM] Clock...");

if (!_clock.begin("Europe/Moscow"))
{
    Serial.println("[SYSTEM] Clock FAILED");
}
else
{
    Serial.println("[SYSTEM] Clock OK");
}


    // ========================================================
    // AUDIO
    // ========================================================

    // Serial.println("[SYSTEM] Audio...");

    // if (!_audio.begin())
    // {
    //     Serial.println("[SYSTEM] Audio FAILED");
    // }
    // else
    // {
    //     Serial.println("[SYSTEM] Audio OK");
    // }


    // ========================================================
    // LED
    // ========================================================

    // Serial.println("[SYSTEM] LED...");

    // if (!_led.begin())
    // {
    //     Serial.println("[SYSTEM] LED FAILED");
    // }
    // else
    // {
    //     Serial.println("[SYSTEM] LED OK");
    // }


    // ========================================================
    // ENCODER
    // ========================================================

    // Serial.println("[SYSTEM] Encoder...");

    // if (!_encoder.begin())
    // {
    //     Serial.println("[SYSTEM] Encoder FAILED");
    // }
    // else
    // {
    //     Serial.println("[SYSTEM] Encoder OK");
    // }


    // ========================================================
    // READY
    // ========================================================

    _ready = true;

    Serial.println();
    Serial.println("========================================");
    Serial.println("             SYSTEM READY");
    Serial.println("========================================");

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void System::update()
{
    if (!_ready)
        return;

    static uint32_t lastUpdate = 0;

    if (millis() - lastUpdate < 1000)
        return;

    lastUpdate = millis();


    // ========================================================
    // SENSORS
    // ========================================================

    _sensors.update();

    Serial.println();
    Serial.println("================================");
    Serial.println("           SENSOR DATA");
    Serial.println("================================");

    // ========================================================
    // SHT45 / VEML7700
    // ========================================================

    Serial.print("Temperature: ");
    Serial.println(
        _sensors.getTemperatureC()
    );

    Serial.print("Humidity:    ");
    Serial.println(
        _sensors.getHumidity()
    );

    Serial.print("Light:       ");
    Serial.println(
        _sensors.getLight()
    );


    // ========================================================
    // VL53L8CX
    // ========================================================

    Serial.println();
    Serial.println("---------- VL53L8CX ----------");

    if (!_sensors.isVL53L8CXInitialized())
    {
        Serial.println("Status: NOT INITIALIZED");
    }
    else
    {
        const int16_t* distances =
            _sensors.getVL53L8CXDistances();

        const uint8_t* targets =
            _sensors.getVL53L8CXTargets();

        // ====================================================
        // DISTANCE 8 × 8
        // ====================================================

        Serial.println("Distances:");

        for (uint8_t y = 0; y < 8; y++)
        {
            for (uint8_t x = 0; x < 8; x++)
            {
                uint8_t zone = y * 8 + x;

                Serial.printf(
                    "%5d ",
                    distances[zone]
                );
            }

            Serial.println();
        }

        // ====================================================
        // TARGETS 8 × 8
        // ====================================================

        Serial.println();
        Serial.println("Targets:");

        for (uint8_t y = 0; y < 8; y++)
        {
            for (uint8_t x = 0; x < 8; x++)
            {
                uint8_t zone = y * 8 + x;

                Serial.printf(
                    "%3d ",
                    targets[zone]
                );
            }

            Serial.println();
        }
    }


    // ========================================================
    // CLOCK
    // ========================================================

    Serial.println();
    Serial.println("================================");
    Serial.println("             CLOCK");
    Serial.println("================================");

    if (!_clock.isTimeValid())
    {
        Serial.println("Status: INVALID");
    }
    else
    {
        // ====================================================
        // INDIVIDUAL DIGITS
        // ====================================================
        uint8_t hourTens =
            _clock.getHourTens();

        uint8_t hourOnes =
            _clock.getHourOnes();

        uint8_t minuteTens =
            _clock.getMinuteTens();

        uint8_t minuteOnes =
            _clock.getMinuteOnes();


        Serial.print("Hour tens:    ");
        Serial.println(hourTens);

        Serial.print("Hour ones:    ");
        Serial.println(hourOnes);

        Serial.print("Minute tens:  ");
        Serial.println(minuteTens);

        Serial.print("Minute ones:  ");
        Serial.println(minuteOnes);


        // ====================================================
        // FULL TIME
        // ====================================================

        Serial.printf(
            "Time: %d%d:%d%d\n",
            hourTens,
            hourOnes,
            minuteTens,
            minuteOnes
        );


        // ====================================================
        // DATE
        // ====================================================

        DateData date =
            _clock.getDateData();

        if (date.valid)
        {
            Serial.printf(
                "Date: %02d.%02d.%04d\n",
                date.day,
                date.month,
                date.year
            );

            Serial.print("Day: ");

            Serial.println(
                static_cast<int>(
                    date.dayOfWeek
                )
            );
        }
        else
        {
            Serial.println("Date: --");
        }


        // ====================================================
        // TIMEZONE
        // ====================================================

        Serial.print("Timezone: ");

        Serial.println(
            _clock.getTimeZone()
        );
    }
    Serial.println("-------------------------------");




    // ========================================================
    // ENCODER
    // ========================================================

    // _encoder.update();


    // ========================================================
    // AUDIO
    // ========================================================

    // _audio.update();


    // ========================================================
    // LED
    // ========================================================

    // _led.update();


    // ========================================================
    // DISPLAY
    // ========================================================

    // _display.update();
}

// ============================================================
// STATUS
// ============================================================

bool System::isReady() const
{
    return _ready;
}

// ============================================================
// ACCESS
// ============================================================

// SPIManager& System::spi()
// {
//     return _spi;
// }


// SDManager& System::sd()
// {
//     return _sd;
// }


// DisplaySystem& System::display()
// {
//     return _display;
// }


SensorManager& System::sensors()
{
    return _sensors;
}


ClockSystem& System::clock()
{
    return _clock;
}


// AudioSystem& System::audio()
// {
//     return _audio;
// }


// LEDManager& System::led()
// {
//     return _led;
// }


// EncoderManager& System::encoder()
// {
//     return _encoder;
// }