#include "I2SManager.h"

#include <Arduino.h>

#include "Pins.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

I2SManager::I2SManager()
    : initialized(false),
      microphoneInitialized(false),
      speakerInitialized(false)
{
}


// ============================================================
// BEGIN
// ============================================================

bool I2SManager::begin()
{
    if (initialized)
        return true;

    initialized = true;

    Serial.println(
        "[I2SManager] READY"
    );

    return true;
}


// ============================================================
// END
// ============================================================

void I2SManager::end()
{
    endMicrophone();
    endSpeaker();

    initialized = false;

    Serial.println(
        "[I2SManager] STOPPED"
    );
}


// ============================================================
// MICROPHONE
// ============================================================

bool I2SManager::beginMicrophone(
    uint32_t sampleRate
)
{
    if (!initialized)
    {
        Serial.println(
            "[I2SManager] ERROR: manager not initialized"
        );

        return false;
    }


    if (microphoneInitialized)
    {
        Serial.println(
            "[I2SManager] Microphone already initialized"
        );

        return true;
    }


    if (
        !installMicrophoneDriver(
            sampleRate
        )
    )
    {
        return false;
    }


    microphoneInitialized = true;


    Serial.println(
        "[I2SManager] Microphone I2S READY"
    );


    return true;
}


void I2SManager::endMicrophone()
{
    if (!microphoneInitialized)
        return;


    i2s_stop(
        MICROPHONE_PORT
    );


    i2s_driver_uninstall(
        MICROPHONE_PORT
    );


    microphoneInitialized = false;


    Serial.println(
        "[I2SManager] Microphone I2S STOPPED"
    );
}


bool I2SManager::isMicrophoneInitialized() const
{
    return microphoneInitialized;
}


// ============================================================
// SPEAKER
// ============================================================

bool I2SManager::beginSpeaker(
    uint32_t sampleRate
)
{
    if (!initialized)
    {
        Serial.println(
            "[I2SManager] ERROR: manager not initialized"
        );

        return false;
    }


    if (speakerInitialized)
    {
        Serial.println(
            "[I2SManager] Speaker already initialized"
        );

        return true;
    }


    if (
        !installSpeakerDriver(
            sampleRate
        )
    )
    {
        return false;
    }


    speakerInitialized = true;


    Serial.println(
        "[I2SManager] Speaker I2S READY"
    );


    return true;
}


void I2SManager::endSpeaker()
{
    if (!speakerInitialized)
        return;


    i2s_stop(
        SPEAKER_PORT
    );


    i2s_driver_uninstall(
        SPEAKER_PORT
    );


    speakerInitialized = false;


    Serial.println(
        "[I2SManager] Speaker I2S STOPPED"
    );
}


bool I2SManager::isSpeakerInitialized() const
{
    return speakerInitialized;
}


// ============================================================
// INSTALL MICROPHONE
// ============================================================

bool I2SManager::installMicrophoneDriver(
    uint32_t sampleRate
)
{
    i2s_config_t config =
    {
        .mode =
            (i2s_mode_t)(
                I2S_MODE_MASTER |
                I2S_MODE_RX
            ),

        .sample_rate =
            (int)sampleRate,

        .bits_per_sample =
            I2S_BITS_PER_SAMPLE_32BIT,

        .channel_format =
            I2S_CHANNEL_FMT_ONLY_LEFT,

        .communication_format =
            I2S_COMM_FORMAT_I2S,

        .intr_alloc_flags =
            ESP_INTR_FLAG_LEVEL1,

        .dma_buf_count =
            8,

        .dma_buf_len =
            256,

        .use_apll =
            false,

        .tx_desc_auto_clear =
            false,

        .fixed_mclk =
            0
    };


    esp_err_t result =
        i2s_driver_install(
            MICROPHONE_PORT,
            &config,
            0,
            nullptr
        );


    if (result != ESP_OK)
    {
        Serial.print(
            "[I2SManager] ERROR: "
            "microphone driver install failed: "
        );

        Serial.println(
            esp_err_to_name(result)
        );

        return false;
    }


    i2s_pin_config_t pins =
    {
        .bck_io_num =
            PIN_INMP_SCK,

        .ws_io_num =
            PIN_INMP_WS,

        .data_out_num =
            I2S_PIN_NO_CHANGE,

        .data_in_num =
            PIN_INMP_SD
    };


    result =
        i2s_set_pin(
            MICROPHONE_PORT,
            &pins
        );


    if (result != ESP_OK)
    {
        Serial.print(
            "[I2SManager] ERROR: "
            "microphone pin configuration failed: "
        );

        Serial.println(
            esp_err_to_name(result)
        );


        i2s_driver_uninstall(
            MICROPHONE_PORT
        );


        return false;
    }


    i2s_zero_dma_buffer(
        MICROPHONE_PORT
    );


    return true;
}


// ============================================================
// INSTALL SPEAKER
// ============================================================

bool I2SManager::installSpeakerDriver(
    uint32_t sampleRate
)
{
    i2s_config_t config =
    {
        .mode =
            (i2s_mode_t)(
                I2S_MODE_MASTER |
                I2S_MODE_TX
            ),

        .sample_rate =
            (int)sampleRate,

        .bits_per_sample =
            I2S_BITS_PER_SAMPLE_16BIT,

        .channel_format =
            I2S_CHANNEL_FMT_ONLY_LEFT,

        .communication_format =
            I2S_COMM_FORMAT_I2S,

        .intr_alloc_flags =
            ESP_INTR_FLAG_LEVEL1,

        .dma_buf_count =
            8,

        .dma_buf_len =
            256,

        .use_apll =
            false,

        .tx_desc_auto_clear =
            true,

        .fixed_mclk =
            0
    };


    esp_err_t result =
        i2s_driver_install(
            SPEAKER_PORT,
            &config,
            0,
            nullptr
        );


    if (result != ESP_OK)
    {
        Serial.print(
            "[I2SManager] ERROR: "
            "speaker driver install failed: "
        );

        Serial.println(
            esp_err_to_name(result)
        );

        return false;
    }


    i2s_pin_config_t pins =
    {
        .bck_io_num =
            PIN_I2S_BCLK,

        .ws_io_num =
            PIN_I2S_LRCLK,

        .data_out_num =
            PIN_I2S_DIN,

        .data_in_num =
            I2S_PIN_NO_CHANGE
    };


    result =
        i2s_set_pin(
            SPEAKER_PORT,
            &pins
        );


    if (result != ESP_OK)
    {
        Serial.print(
            "[I2SManager] ERROR: "
            "speaker pin configuration failed: "
        );

        Serial.println(
            esp_err_to_name(result)
        );


        i2s_driver_uninstall(
            SPEAKER_PORT
        );


        return false;
    }


    i2s_zero_dma_buffer(
        SPEAKER_PORT
    );


    return true;
}


// ============================================================
// STATUS
// ============================================================

bool I2SManager::isInitialized() const
{
    return initialized;
}


bool I2SManager::isPortInstalled(
    i2s_port_t port
) const
{
    if (
        port ==
        MICROPHONE_PORT
    )
    {
        return microphoneInitialized;
    }


    if (
        port ==
        SPEAKER_PORT
    )
    {
        return speakerInitialized;
    }


    return false;
}


// ============================================================
// ACCESS
// ============================================================

i2s_port_t I2SManager::microphonePort() const
{
    return MICROPHONE_PORT;
}


i2s_port_t I2SManager::speakerPort() const
{
    return SPEAKER_PORT;
}
