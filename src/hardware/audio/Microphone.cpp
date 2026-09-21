
#include "Microphone.h"

#include <Arduino.h>
#include <cmath>
#include <cstring>

#include "./managers/I2SManager.h"

// ============================================================
// CONFIG
// ============================================================

namespace
{
    constexpr uint32_t READ_TIMEOUT_MS = 100;

    constexpr float MAX_LEVEL = 1.0f;
}

// ============================================================
// CONSTRUCTOR
// ============================================================

Microphone::Microphone(I2SManager& manager)
    : i2sManager(manager),
      initialized(false),
      listening(false),
      enabled(true),
      audioChunkId(0),
      audioChunkSize(0),
      rms(0.0f),
      level(0.0f),
      peak(0)
{
    memset(
        audioBuffer,
        0,
        sizeof(audioBuffer)
    );
}

// ============================================================
// BEGIN
// ============================================================

bool Microphone::begin()
{
    if (initialized)
    {
        return true;
    }

    Serial.println();
    Serial.println("[MIC] ========================================");
    Serial.println("[MIC] Initializing INMP441");
    Serial.println("[MIC] ========================================");

    // --------------------------------------------------------
    // I2S
    // --------------------------------------------------------

    if (!i2sManager.beginMicrophone(
            DEFAULT_SAMPLE_RATE))
    {
        Serial.println(
            "[MIC] ERROR: I2SManager.beginMicrophone() failed"
        );

        return false;
    }

    // --------------------------------------------------------
    // RESET STATE
    // --------------------------------------------------------

    initialized = true;
    listening = false;

    audioChunkId = 0;
    audioChunkSize = 0;

    rms = 0.0f;
    level = 0.0f;
    peak = 0;

    memset(
        audioBuffer,
        0,
        sizeof(audioBuffer)
    );

    // --------------------------------------------------------
    // INFO
    // --------------------------------------------------------

    Serial.printf(
        "[MIC] Name: %s\n",
        getName()
    );

    Serial.printf(
        "[MIC] Sample rate: %lu Hz\n",
        static_cast<unsigned long>(
            DEFAULT_SAMPLE_RATE
        )
    );

    Serial.printf(
        "[MIC] Buffer samples: %u\n",
        static_cast<unsigned>(
            AUDIO_SAMPLES
        )
    );

    Serial.printf(
        "[MIC] Buffer bytes: %u\n",
        static_cast<unsigned>(
            sizeof(audioBuffer)
        )
    );

    Serial.println("[MIC] Initialized");

    return true;
}

// ============================================================
// END
// ============================================================

void Microphone::end()
{
    if (!initialized)
    {
        return;
    }

    Serial.println("[MIC] Stopping");

    stopListening();

    i2sManager.endMicrophone();

    initialized = false;

    audioChunkSize = 0;

    rms = 0.0f;
    level = 0.0f;
    peak = 0;

    memset(
        audioBuffer,
        0,
        sizeof(audioBuffer)
    );

    Serial.println("[MIC] Stopped");
}

// ============================================================
// START LISTENING
// ============================================================

bool Microphone::startListening()
{
    if (!initialized)
    {
        Serial.println(
            "[MIC] ERROR: microphone is not initialized"
        );

        return false;
    }

    if (!enabled)
    {
        Serial.println(
            "[MIC] ERROR: microphone is disabled"
        );

        return false;
    }

    if (listening)
    {
        return true;
    }

    // --------------------------------------------------------
    // CLEAR DMA
    // --------------------------------------------------------

    if (!i2sManager.clearMicrophone())
    {
        Serial.println(
            "[MIC] ERROR: clearMicrophone() failed"
        );

        return false;
    }

    // --------------------------------------------------------
    // RESET AUDIO STATE
    // --------------------------------------------------------

    audioChunkSize = 0;

    rms = 0.0f;
    level = 0.0f;
    peak = 0;

    // --------------------------------------------------------
    // START
    // --------------------------------------------------------

    listening = true;

    Serial.println("[MIC] Listening started");

    return true;
}

// ============================================================
// STOP LISTENING
// ============================================================

void Microphone::stopListening()
{
    if (!listening)
    {
        return;
    }

    listening = false;

    audioChunkSize = 0;

    Serial.println("[MIC] Listening stopped");
}

// ============================================================
// UPDATE AUDIO
// ============================================================

bool Microphone::updateAudio()
{
    if (!initialized)
    {
        return false;
    }

    if (!listening)
    {
        return false;
    }

    if (!enabled)
    {
        return false;
    }

    // ========================================================
    // TEMPORARY RAW BUFFER
    // ========================================================

    int32_t rawBuffer[AUDIO_SAMPLES];

    memset(
        rawBuffer,
        0,
        sizeof(rawBuffer)
    );

    // ========================================================
    // READ I2S
    // ========================================================

    size_t bytesRead = 0;

    if (!i2sManager.readMicrophone(
            rawBuffer,
            sizeof(rawBuffer),
            bytesRead,
            READ_TIMEOUT_MS))
    {
        return false;
    }

    if (bytesRead == 0)
    {
        return false;
    }

    // ========================================================
    // CALCULATE SAMPLE COUNT
    // ========================================================

    size_t samplesRead =
        bytesRead / sizeof(int32_t);

    if (samplesRead > AUDIO_SAMPLES)
    {
        samplesRead = AUDIO_SAMPLES;
    }

    if (samplesRead == 0)
    {
        return false;
    }

    // ========================================================
    // CONVERT 32-BIT I2S -> 16-BIT PCM
    // ========================================================

    for (size_t i = 0; i < samplesRead; ++i)
    {
        int32_t value = rawBuffer[i];

        /*
         * INMP441 delivers audio packed into
         * a 32-bit I2S word.
         *
         * We reduce it to signed 16-bit PCM.
         */

        value >>= 14;

        // ----------------------------------------------------
        // CLAMP
        // ----------------------------------------------------

        if (value > 32767)
        {
            value = 32767;
        }
        else if (value < -32768)
        {
            value = -32768;
        }

        audioBuffer[i] =
            static_cast<int16_t>(value);
    }

    // --------------------------------------------------------
    // SAVE SIZE
    // --------------------------------------------------------

    audioChunkSize = samplesRead;

    // ========================================================
    // AUDIO ANALYSIS
    // ========================================================

    double sumSquares = 0.0;

    int16_t currentPeak = 0;

    for (size_t i = 0; i < samplesRead; ++i)
    {
        const int16_t sample =
            audioBuffer[i];

        // ----------------------------------------------------
        // NORMALIZED SAMPLE
        // ----------------------------------------------------

        const float normalized =
            static_cast<float>(sample) / 32768.0f;

        // ----------------------------------------------------
        // RMS
        // ----------------------------------------------------

        sumSquares +=
            static_cast<double>(normalized) *
            static_cast<double>(normalized);

        // ----------------------------------------------------
        // PEAK
        // ----------------------------------------------------

        int32_t absoluteValue =
            static_cast<int32_t>(sample);

        if (absoluteValue < 0)
        {
            absoluteValue = -absoluteValue;
        }

        if (absoluteValue > currentPeak)
        {
            currentPeak =
                static_cast<int16_t>(
                    absoluteValue
                );
        }
    }

    // ========================================================
    // RMS
    // ========================================================

    rms = sqrtf(
        static_cast<float>(
            sumSquares /
            static_cast<double>(samplesRead)
        )
    );

    // ========================================================
    // LEVEL
    // ========================================================

    level = rms;

    if (level < 0.0f)
    {
        level = 0.0f;
    }

    if (level > MAX_LEVEL)
    {
        level = MAX_LEVEL;
    }

    // ========================================================
    // PEAK
    // ========================================================

    peak = currentPeak;

    // ========================================================
    // CHUNK ID
    // ========================================================

    ++audioChunkId;

    return true;
}

// ============================================================
// HAS NEW AUDIO
// ============================================================

bool Microphone::hasNewAudio() const
{
    return audioChunkSize > 0;
}

// ============================================================
// AUDIO CHUNK ID
// ============================================================

uint32_t Microphone::getAudioChunkId() const
{
    return audioChunkId;
}

// ============================================================
// AUDIO CHUNK
// ============================================================

size_t Microphone::getAudioChunk(
    const int16_t*& samples
) const
{
    samples = audioBuffer;

    return audioChunkSize;
}

// ============================================================
// AUDIO CHUNK SIZE
// ============================================================

size_t Microphone::getAudioChunkSize() const
{
    return audioChunkSize;
}

// ============================================================
// RMS
// ============================================================

float Microphone::getRMS() const
{
    return rms;
}

// ============================================================
// LEVEL
// ============================================================

float Microphone::getLevel() const
{
    return level;
}

// ============================================================
// PEAK
// ============================================================

int16_t Microphone::getPeak() const
{
    return peak;
}

// ============================================================
// INITIALIZED
// ============================================================

bool Microphone::isInitialized() const
{
    return initialized;
}

// ============================================================
// LISTENING
// ============================================================

bool Microphone::isListening() const
{
    return listening;
}

// ============================================================
// ENABLED
// ============================================================

bool Microphone::isEnabled() const
{
    return enabled;
}

// ============================================================
// SET ENABLED
// ============================================================

void Microphone::setEnabled(bool value)
{
    enabled = value;

    if (!enabled)
    {
        stopListening();
    }
}

// ============================================================
// NAME
// ============================================================

const char* Microphone::getName() const
{
    return "INMP441";
}
