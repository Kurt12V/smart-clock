
#include "Microphone.h"

#include <Arduino.h>
#include <cstring>
#include <cmath>

#include "./managers/I2S/I2SManager.h"

Microphone::Microphone(I2SManager& i2sManager)
    : i2sManager(i2sManager),
      initialized(false),
      listening(false),
      enabled(true),
      audioChunkId(0),
      audioChunkSize(0),
      rms(0.0f),
      level(0.0f),
      peak(0)
{
    memset(audioBuffer, 0, sizeof(audioBuffer));
}

bool Microphone::begin()
{
    if (initialized)
        return true;

    if (!i2sManager.beginMicrophone(DEFAULT_SAMPLE_RATE))
        return false;

    initialized = true;

    Serial0.println("[MIC] Initialized");
    Serial0.printf(
        "[MIC] Sample rate: %lu Hz\n",
        static_cast<unsigned long>(DEFAULT_SAMPLE_RATE)
    );

    return true;
}

void Microphone::end()
{
    if (!initialized)
        return;

    stopListening();

    i2sManager.endMicrophone();

    initialized = false;

    Serial0.println("[MIC] Stopped");
}

bool Microphone::startListening()
{
    if (!initialized || !enabled)
        return false;

    if (listening)
        return true;

    if (!i2sManager.clearMicrophone())
        return false;

    portENTER_CRITICAL(&bufferMux);

    audioChunkSize = 0;
    rms = 0.0f;
    level = 0.0f;
    peak = 0;

    portEXIT_CRITICAL(&bufferMux);

    listening = true;

    Serial0.println("[MIC] Listening started");

    return true;
}

void Microphone::stopListening()
{
    listening = false;

    portENTER_CRITICAL(&bufferMux);

    audioChunkSize = 0;

    portEXIT_CRITICAL(&bufferMux);
}

bool Microphone::updateAudio()
{
    if (!initialized || !listening || !enabled)
        return false;

    int32_t rawBuffer[AUDIO_SAMPLES] = {};

    size_t bytesRead = 0;

    if (!i2sManager.readMicrophone(
            rawBuffer,
            sizeof(rawBuffer),
            bytesRead,
            20
        ))
    {
        return false;
    }

    const size_t samplesRead =
        min(bytesRead / sizeof(int32_t), AUDIO_SAMPLES);

    if (samplesRead == 0)
        return false;

    int16_t converted[AUDIO_SAMPLES] = {};

    double sumSquares = 0.0;
    int32_t maxPeak = 0;

    for (size_t i = 0; i < samplesRead; ++i)
    {
        int32_t sample = rawBuffer[i] >> 14;

        sample = constrain(
            sample,
            -32768,
            32767
        );

        converted[i] = static_cast<int16_t>(sample);

        sumSquares +=
            static_cast<double>(sample) * sample;

        const int32_t absolute =
            sample < 0 ? -sample : sample;

        if (absolute > maxPeak)
            maxPeak = absolute;
    }

    const float newRms = sqrtf(
        static_cast<float>(
            sumSquares / samplesRead
        )
    );

    const float newLevel = constrain(
        newRms / 32768.0f,
        0.0f,
        1.0f
    );

    portENTER_CRITICAL(&bufferMux);

    memcpy(
        audioBuffer,
        converted,
        samplesRead * sizeof(int16_t)
    );

    audioChunkSize = samplesRead;

    ++audioChunkId;

    rms = newRms;
    level = newLevel;

    peak = static_cast<int16_t>(
        min(maxPeak, 32767)
    );

    portEXIT_CRITICAL(&bufferMux);

    return true;
}

bool Microphone::hasNewAudio() const
{
    portENTER_CRITICAL(&bufferMux);

    const bool result = audioChunkSize > 0;

    portEXIT_CRITICAL(&bufferMux);

    return result;
}

uint32_t Microphone::getAudioChunkId() const
{
    portENTER_CRITICAL(&bufferMux);

    const uint32_t result = audioChunkId;

    portEXIT_CRITICAL(&bufferMux);

    return result;
}

size_t Microphone::getAudioChunk(
    const int16_t*& samples
) const
{
    samples = audioBuffer;

    portENTER_CRITICAL(&bufferMux);

    const size_t result = audioChunkSize;

    portEXIT_CRITICAL(&bufferMux);

    return result;
}

size_t Microphone::copyAudioChunk(
    int16_t* destination,
    size_t capacity,
    uint32_t& chunkId
) const
{
    chunkId = 0;

    if (destination == nullptr || capacity == 0)
        return 0;

    portENTER_CRITICAL(&bufferMux);

    const size_t count = min(
        audioChunkSize,
        capacity
    );

    memcpy(
        destination,
        audioBuffer,
        count * sizeof(int16_t)
    );

    chunkId = audioChunkId;

    portEXIT_CRITICAL(&bufferMux);

    return count;
}

float Microphone::getRMS() const
{
    portENTER_CRITICAL(&bufferMux);

    const float result = rms;

    portEXIT_CRITICAL(&bufferMux);

    return result;
}

float Microphone::getLevel() const
{
    portENTER_CRITICAL(&bufferMux);

    const float result = level;

    portEXIT_CRITICAL(&bufferMux);

    return result;
}

int16_t Microphone::getPeak() const
{
    portENTER_CRITICAL(&bufferMux);

    const int16_t result = peak;

    portEXIT_CRITICAL(&bufferMux);

    return result;
}

bool Microphone::isInitialized() const
{
    return initialized;
}

bool Microphone::isListening() const
{
    return listening;
}

bool Microphone::isEnabled() const
{
    return enabled;
}

void Microphone::setEnabled(bool enabled)
{
    this->enabled = enabled;

    if (!enabled)
        stopListening();
}

const char* Microphone::getName() const
{
    return "INMP441";
}
