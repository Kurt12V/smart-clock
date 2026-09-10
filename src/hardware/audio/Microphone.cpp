#include "Microphone.h"
#include "Pins.h"

#include <math.h>


// =====================================================
// CONSTRUCTOR
// =====================================================

Microphone::Microphone()
    : initialized(false),
      enabled(true),
      listening(false),
      audioChunkSize(0),
      audioChunkId(0),
      newAudio(false),
      rms(0.0f),
      level(0.0f),
      peak(0)
{
}


// =====================================================
// BEGIN
// =====================================================

bool Microphone::begin()
{
    Serial.println();
    Serial.println("================================");
    Serial.println("          MICROPHONE");
    Serial.println("================================");


    i2s_config_t config = {};


    config.mode =
        (i2s_mode_t)(
            I2S_MODE_MASTER |
            I2S_MODE_RX
        );


    config.sample_rate =
        SAMPLE_RATE;


    /*
     * INMP441 передаёт 24-bit audio
     * в 32-bit I2S контейнере.
     */

    config.bits_per_sample =
        I2S_BITS_PER_SAMPLE_32BIT;


    config.channel_format =
        I2S_CHANNEL_FMT_ONLY_LEFT;


    config.communication_format =
        I2S_COMM_FORMAT_I2S;


    config.intr_alloc_flags =
        ESP_INTR_FLAG_LEVEL1;


    config.dma_buf_count = 8;

    config.dma_buf_len = 256;


    config.use_apll = false;

    config.tx_desc_auto_clear = false;

    config.fixed_mclk = 0;


    esp_err_t result =
        i2s_driver_install(
            I2S_PORT,
            &config,
            0,
            nullptr
        );


    if (result != ESP_OK)
    {
        Serial.printf(
            "[Microphone] I2S driver FAILED: %d\n",
            result
        );

        return false;
    }


    // =================================================
    // PINS
    // =================================================

    i2s_pin_config_t pins = {};


    pins.bck_io_num =
        PIN_MIC_SCK;


    pins.ws_io_num =
        PIN_MIC_WS;


    pins.data_out_num =
        I2S_PIN_NO_CHANGE;


    pins.data_in_num =
        PIN_MIC_SD;


    result =
        i2s_set_pin(
            I2S_PORT,
            &pins
        );


    if (result != ESP_OK)
    {
        Serial.printf(
            "[Microphone] I2S pins FAILED: %d\n",
            result
        );


        i2s_driver_uninstall(
            I2S_PORT
        );


        return false;
    }


    i2s_zero_dma_buffer(
        I2S_PORT
    );


    initialized = true;


    Serial.println(
        "[Microphone] READY"
    );


    Serial.printf(
        "[Microphone] Sample rate: %lu Hz\n",
        SAMPLE_RATE
    );


    Serial.println(
        "[Microphone] PCM: 16-bit / Mono"
    );


    return true;
}


// =====================================================
// START LISTENING
// =====================================================

bool Microphone::startListening()
{
    if (!initialized)
    {
        Serial.println(
            "[Microphone] Not initialized"
        );

        return false;
    }


    if (!enabled)
    {
        Serial.println(
            "[Microphone] Disabled"
        );

        return false;
    }


    i2s_zero_dma_buffer(
        I2S_PORT
    );


    audioChunkSize = 0;

    audioChunkId = 0;

    newAudio = false;


    listening = true;


    Serial.println(
        "[Microphone] Listening STARTED"
    );


    return true;
}


// =====================================================
// STOP LISTENING
// =====================================================

void Microphone::stopListening()
{
    listening = false;

    newAudio = false;


    Serial.println(
        "[Microphone] Listening STOPPED"
    );
}


// =====================================================
// UPDATE AUDIO
// =====================================================

bool Microphone::updateAudio()
{
    if (!initialized)
        return false;


    if (!enabled)
        return false;


    if (!listening)
        return false;


    /*
     * ВАЖНО:
     *
     * i2s_read() вызывается ТОЛЬКО здесь.
     */

    int32_t rawBuffer[
        BUFFER_SAMPLES
    ];


    size_t bytesRead = 0;


    esp_err_t result =
        i2s_read(
            I2S_PORT,
            rawBuffer,
            sizeof(rawBuffer),
            &bytesRead,
            0
        );


    if (result != ESP_OK)
    {
        return false;
    }


    size_t samples =
        bytesRead /
        sizeof(int32_t);


    if (samples == 0)
    {
        return false;
    }


    if (samples > BUFFER_SAMPLES)
    {
        samples = BUFFER_SAMPLES;
    }


    // =================================================
    // CONVERT 32-BIT → 16-BIT
    // =================================================

    for (size_t i = 0; i < samples; i++)
    {
        audioBuffer[i] =
            (int16_t)(
                rawBuffer[i] >> 14
            );
    }


    audioChunkSize =
        samples;


    audioChunkId++;


    newAudio = true;


    // =================================================
    // ANALYSIS
    // =================================================

    analyzeAudio(
        audioBuffer,
        audioChunkSize
    );


    return true;
}


// =====================================================
// HAS NEW AUDIO
// =====================================================

bool Microphone::hasNewAudio() const
{
    return newAudio;
}


// =====================================================
// GET AUDIO CHUNK
// =====================================================

size_t Microphone::getAudioChunk(
    const int16_t*& data
) const
{
    data = audioBuffer;

    return audioChunkSize;
}


// =====================================================
// GET CHUNK SIZE
// =====================================================

size_t Microphone::getAudioChunkSize() const
{
    return audioChunkSize;
}


// =====================================================
// GET CHUNK ID
// =====================================================

uint32_t Microphone::getAudioChunkId() const
{
    return audioChunkId;
}


// =====================================================
// ANALYZE AUDIO
// =====================================================

void Microphone::analyzeAudio(
    const int16_t* samples,
    size_t count
)
{
    if (samples == nullptr)
        return;


    if (count == 0)
        return;


    double sumSquares = 0.0;

    int32_t maxPeak = 0;


    for (size_t i = 0; i < count; i++)
    {
        int32_t sample =
            samples[i];


        int32_t absolute =
            abs(sample);


        if (absolute > maxPeak)
        {
            maxPeak = absolute;
        }


        sumSquares +=
            (double)sample *
            (double)sample;
    }


    // =================================================
    // RMS
    // =================================================

    rms =
        sqrt(
            sumSquares /
            (double)count
        );


    // =================================================
    // PEAK
    // =================================================

    if (maxPeak > 32767)
    {
        maxPeak = 32767;
    }


    peak =
        (int16_t)maxPeak;


    // =================================================
    // LEVEL
    // =================================================

    level =
        rms / 8000.0f;


    if (level > 1.0f)
    {
        level = 1.0f;
    }


    if (level < 0.0f)
    {
        level = 0.0f;
    }
}


// =====================================================
// ENABLE
// =====================================================

void Microphone::setEnabled(
    bool value
)
{
    enabled = value;


    if (!enabled)
    {
        listening = false;

        newAudio = false;
    }
}


// =====================================================
// STATUS
// =====================================================

bool Microphone::isInitialized() const
{
    return initialized;
}


bool Microphone::isEnabled() const
{
    return enabled;
}


bool Microphone::isListening() const
{
    return listening;
}


// =====================================================
// ANALYSIS DATA
// =====================================================

float Microphone::getRMS() const
{
    return rms;
}


float Microphone::getLevel() const
{
    return level;
}


int16_t Microphone::getPeak() const
{
    return peak;
}


// =====================================================
// NAME
// =====================================================

const char* Microphone::getName() const
{
    return "INMP441";
}