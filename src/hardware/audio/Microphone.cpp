#include "Microphone.h"

#include <Arduino.h>
#include <driver/i2s.h>

#include "./managers/I2SManager.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

Microphone::Microphone(
    I2SManager& i2sManager
)
    : i2sManager(i2sManager),
      initialized(false),
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


// ============================================================
// BEGIN
// ============================================================

bool Microphone::begin()
{
    if (initialized)
        return true;


    if (!i2sManager.isInitialized())
    {
        Serial.println(
            "[Microphone] ERROR: "
            "I2SManager is not initialized"
        );

        return false;
    }


    if (
        !i2sManager.beginMicrophone(16000)
    )
    {
        Serial.println(
            "[Microphone] ERROR: "
            "failed to initialize I2S"
        );

        return false;
    }


    initialized = true;


    Serial.println(
        "[Microphone] READY"
    );


    return true;
}


// ============================================================
// END
// ============================================================

void Microphone::end()
{
    if (!initialized)
        return;


    stopListening();


    i2sManager.endMicrophone();


    initialized = false;


    Serial.println(
        "[Microphone] STOPPED"
    );
}


// ============================================================
// LISTENING
// ============================================================

bool Microphone::startListening()
{
    if (!initialized)
    {
        Serial.println(
            "[Microphone] ERROR: "
            "not initialized"
        );

        return false;
    }


    if (!enabled)
    {
        Serial.println(
            "[Microphone] ERROR: "
            "microphone disabled"
        );

        return false;
    }


    if (listening)
        return true;


    i2s_zero_dma_buffer(
        i2sManager.microphonePort()
    );


    audioChunkSize = 0;

    audioChunkId = 0;

    newAudio = false;

    rms = 0.0f;

    level = 0.0f;

    peak = 0;


    listening = true;


    Serial.println(
        "[Microphone] LISTENING"
    );


    return true;
}


// ============================================================
// STOP LISTENING
// ============================================================

void Microphone::stopListening()
{
    if (!listening)
        return;


    listening = false;

    newAudio = false;


    Serial.println(
        "[Microphone] LISTENING STOPPED"
    );
}


// ============================================================
// ENABLE
// ============================================================

void Microphone::setEnabled(
    bool enabled
)
{
    this->enabled = enabled;


    if (!enabled)
    {
        stopListening();
    }
}


bool Microphone::isEnabled() const
{
    return enabled;
}


// ============================================================
// STATUS
// ============================================================

bool Microphone::isInitialized() const
{
    return initialized;
}


bool Microphone::isListening() const
{
    return listening;
}


// ============================================================
// UPDATE AUDIO
// ============================================================

bool Microphone::updateAudio()
{
    if (!initialized)
        return false;


    if (!enabled)
        return false;


    if (!listening)
        return false;


    int32_t rawBuffer[
        BUFFER_SAMPLES
    ];


    size_t bytesRead = 0;


    esp_err_t result =
        i2s_read(
            i2sManager.microphonePort(),
            rawBuffer,
            sizeof(rawBuffer),
            &bytesRead,
            0
        );


    if (result != ESP_OK)
        return false;


    if (bytesRead == 0)
        return false;


    size_t samples =
        bytesRead /
        sizeof(int32_t);


    if (samples > BUFFER_SAMPLES)
        samples = BUFFER_SAMPLES;


    for (
        size_t i = 0;
        i < samples;
        ++i
    )
    {
        int32_t sample =
            rawBuffer[i] >> 14;


        if (sample > 32767)
            sample = 32767;

        if (sample < -32768)
            sample = -32768;


        audioBuffer[i] =
            (int16_t)sample;
    }


    audioChunkSize = samples;

    audioChunkId++;

    newAudio = true;


    analyzeAudio(
        audioBuffer,
        audioChunkSize
    );


    return true;
}


// ============================================================
// CHUNK
// ============================================================

bool Microphone::hasNewAudio() const
{
    return newAudio;
}


size_t Microphone::getAudioChunk(
    const int16_t*& data
) const
{
    data = audioBuffer;

    return audioChunkSize;
}


size_t Microphone::getAudioChunkSize() const
{
    return audioChunkSize;
}


uint32_t Microphone::getAudioChunkId() const
{
    return audioChunkId;
}


// ============================================================
// ANALYSIS
// ============================================================

void Microphone::analyzeAudio(
    const int16_t* samples,
    size_t count
)
{
    if (
        samples == nullptr ||
        count == 0
    )
    {
        rms = 0.0f;
        level = 0.0f;
        peak = 0;

        return;
    }


    double sumSquares = 0.0;

    int16_t maximumPeak = 0;


    for (
        size_t i = 0;
        i < count;
        ++i
    )
    {
        int32_t value =
            samples[i];


        int32_t absolute =
            abs(value);


        if (
            absolute >
            maximumPeak
        )
        {
            maximumPeak =
                (int16_t)
                min(
                    absolute,
                    32767
                );
        }


        sumSquares +=
            (double)value *
            (double)value;
    }


    rms =
        sqrt(
            sumSquares /
            (double)count
        );


    peak =
        maximumPeak;


    level =
        rms /
        8000.0f;


    if (level < 0.0f)
        level = 0.0f;


    if (level > 1.0f)
        level = 1.0f;
}


// ============================================================
// ANALYSIS GETTERS
// ============================================================

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


// ============================================================
// NAME
// ============================================================

const char* Microphone::getName() const
{
    return "INMP441";
}
