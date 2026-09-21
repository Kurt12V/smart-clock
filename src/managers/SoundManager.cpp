
#include "SoundManager.h"

#include <cmath>
#include <cstring>

// ============================================================
// Constructor
// ============================================================

SoundManager::SoundManager(
    SDManager& sdManager,
    Settings::Audio& settings,
    I2SManager& i2sManager
)
    : sdManager(sdManager),
      settings(settings),
      i2sManager(i2sManager),

      initialized(false),
      playing(false),
      paused(false),
      alarmMode(false),

      file(),
      wav{},

      currentPath{},

      dataRead(0),
      positionSamples(0),

      playbackStartMs(0),
      pausedAtMs(0),

      localVolume(100),

      fadeInDuration(0),
      fadeOutDuration(0),

      fadeInStart(0),
      fadeOutStart(0),

      fadeInCurve(FadeCurve::Linear),
      fadeOutCurve(FadeCurve::Linear),

      fadingOut(false),

      sampleBuffer{}
{
}

// ============================================================
// Begin
// ============================================================

bool SoundManager::begin()
{
    if (initialized)
    {
        return true;
    }

    if (!settings.enabled)
    {
        Serial.println(
            "[SoundManager] Disabled by settings"
        );

        return false;
    }

    // --------------------------------------------------------
    // Initialize speaker I2S through I2SManager.
    //
    // IMPORTANT:
    // This is I2S_NUM_1.
    // --------------------------------------------------------

    if (!i2sManager.beginSpeaker(44100))
    {
        Serial.println(
            "[SoundManager] ERROR: speaker I2S init failed"
        );

        return false;
    }

    initialized = true;

    Serial.println("[SoundManager] READY");
    Serial.println("[SoundManager] Speaker: I2S_NUM_1");

    return true;
}

// ============================================================
// End
// ============================================================

void SoundManager::end()
{
    stop();

    if (i2sManager.isSpeakerInitialized())
    {
        i2sManager.endSpeaker();
    }

    initialized = false;

    Serial.println("[SoundManager] STOPPED");
}

// ============================================================
// Update
// ============================================================

void SoundManager::update()
{
    if (!initialized)
    {
        return;
    }

    if (!playing)
    {
        return;
    }

    if (paused)
    {
        return;
    }

    processPlayback();
}

// ============================================================
// Is initialized
// ============================================================

bool SoundManager::isInitialized() const
{
    return initialized;
}

// ============================================================
// Is playing
// ============================================================

bool SoundManager::isPlaying() const
{
    return playing;
}

// ============================================================
// Is paused
// ============================================================

bool SoundManager::isPaused() const
{
    return paused;
}

// ============================================================
// Volume
// ============================================================

void SoundManager::setVolume(uint8_t volume)
{
    volume = constrain(volume, 0, 100);

    settings.volume = volume;
}

uint8_t SoundManager::getVolume() const
{
    return settings.volume;
}

// ============================================================
// Play WAV
// ============================================================

bool SoundManager::playWav(const char* path)
{
    if (!initialized)
    {
        if (!begin())
        {
            return false;
        }
    }

    if (!settings.enabled)
    {
        return false;
    }

    // Normal sounds use global volume.
    alarmMode = false;

    localVolume = settings.volume;

    fadeInDuration = 0;
    fadeOutDuration = 0;

    fadingOut = false;

    return openWav(path);
}

// ============================================================
// Play WAV local volume
// ============================================================

bool SoundManager::playWavLocal(
    const char* path,
    uint8_t volume
)
{
    if (!initialized)
    {
        if (!begin())
        {
            return false;
        }
    }

    if (!settings.enabled)
    {
        return false;
    }

    alarmMode = false;

    localVolume = constrain(
        volume,
        0,
        100
    );

    fadeInDuration = 0;
    fadeOutDuration = 0;

    fadingOut = false;

    return openWav(path);
}

// ============================================================
// Play alarm
// ============================================================

bool SoundManager::playAlarm(
    const char* path,
    uint8_t volume,
    uint32_t fadeInMs,
    uint32_t fadeOutMs,
    FadeCurve curve
)
{
    if (!initialized)
    {
        if (!begin())
        {
            return false;
        }
    }

    if (!settings.enabled)
    {
        return false;
    }

    if (!settings.alarms)
    {
        Serial.println(
            "[SoundManager] Alarms disabled"
        );

        return false;
    }

    // --------------------------------------------------------
    // Alarm DOES NOT use global settings.volume.
    // --------------------------------------------------------

    alarmMode = true;

    localVolume = constrain(
        volume,
        0,
        100
    );

    fadeInDuration = fadeInMs;
    fadeOutDuration = fadeOutMs;

    fadeInCurve = curve;
    fadeOutCurve = curve;

    fadingOut = false;

    return openWav(path);
}

// ============================================================
// Open WAV
// ============================================================

bool SoundManager::openWav(
    const char* path
)
{
    if (path == nullptr)
    {
        Serial.println(
            "[SoundManager] ERROR: path is null"
        );

        return false;
    }

    // Stop current playback.
    stop();

    // --------------------------------------------------------
    // Open SD file.
    // --------------------------------------------------------

    file = sdManager.card().fs().open(
        path,
        FILE_READ
    );

    if (!file)
    {
        Serial.print(
            "[SoundManager] ERROR: cannot open: "
        );

        Serial.println(path);

        return false;
    }

    // --------------------------------------------------------
    // Parse WAV.
    // --------------------------------------------------------

    if (!readWavHeader(file, wav))
    {
        Serial.println(
            "[SoundManager] ERROR: invalid WAV"
        );

        file.close();

        return false;
    }

    // --------------------------------------------------------
    // Supported format.
    // --------------------------------------------------------

    if (wav.audioFormat != 1)
    {
        Serial.println(
            "[SoundManager] ERROR: WAV is not PCM"
        );

        file.close();

        return false;
    }

    if (wav.bitsPerSample != 16)
    {
        Serial.println(
            "[SoundManager] ERROR: only 16-bit WAV supported"
        );

        file.close();

        return false;
    }

    if (wav.channels != 1 && wav.channels != 2)
    {
        Serial.println(
            "[SoundManager] ERROR: only mono/stereo supported"
        );

        file.close();

        return false;
    }

    // --------------------------------------------------------
    // Current speaker configuration.
    //
    // For now the I2SManager speaker is initialized at 44.1 kHz.
    //
    // If the WAV has another rate, reject it rather than
    // playing it at the wrong speed.
    // --------------------------------------------------------

    if (wav.sampleRate != 44100)
    {
        Serial.print(
            "[SoundManager] ERROR: unsupported sample rate: "
        );

        Serial.println(wav.sampleRate);

        file.close();

        return false;
    }

    // --------------------------------------------------------
    // Store path.
    // --------------------------------------------------------

    strncpy(
        currentPath,
        path,
        sizeof(currentPath) - 1
    );

    currentPath[
        sizeof(currentPath) - 1
    ] = '\0';

    // --------------------------------------------------------
    // Reset playback.
    // --------------------------------------------------------

    dataRead = 0;
    positionSamples = 0;

    playbackStartMs = millis();
    pausedAtMs = 0;

    fadingOut = false;

    // --------------------------------------------------------
    // Seek to data.
    // --------------------------------------------------------

    if (!file.seek(wav.dataOffset))
    {
        Serial.println(
            "[SoundManager] ERROR: seek failed"
        );

        file.close();

        return false;
    }

    // --------------------------------------------------------
    // Start.
    // --------------------------------------------------------

    playing = true;
    paused = false;

    fadeInStart = millis();

    if (fadeOutDuration > 0)
    {
        fadeOutStart = 0;
    }

    Serial.print(
        "[SoundManager] PLAY: "
    );

    Serial.println(currentPath);

    Serial.print(
        "[SoundManager] Rate: "
    );

    Serial.println(wav.sampleRate);

    Serial.print(
        "[SoundManager] Channels: "
    );

    Serial.println(wav.channels);

    Serial.print(
        "[SoundManager] Bits: "
    );

    Serial.println(wav.bitsPerSample);

    Serial.print(
        "[SoundManager] Data: "
    );

    Serial.println(wav.dataSize);

    return true;
}

// ============================================================
// Read WAV header
// ============================================================

bool SoundManager::readWavHeader(
    File& wavFile,
    WavInfo& info
)
{
    if (!wavFile)
    {
        return false;
    }

    if (wavFile.size() < 12)
    {
        return false;
    }

    wavFile.seek(0);

    char riff[4];

    if (wavFile.read(
            reinterpret_cast<uint8_t*>(riff),
            4
        ) != 4)
    {
        return false;
    }

    if (memcmp(riff, "RIFF", 4) != 0)
    {
        return false;
    }

    readLE32(wavFile);

    char wave[4];

    if (wavFile.read(
            reinterpret_cast<uint8_t*>(wave),
            4
        ) != 4)
    {
        return false;
    }

    if (memcmp(wave, "WAVE", 4) != 0)
    {
        return false;
    }

    memset(
        &info,
        0,
        sizeof(info)
    );

    bool fmtFound = false;

    bool dataFound = false;

    const uint32_t fileSize =
        wavFile.size();

    while (
        wavFile.position() + 8 <= fileSize
    )
    {
        char chunkId[4];

        if (wavFile.read(
                reinterpret_cast<uint8_t*>(chunkId),
                4
            ) != 4)
        {
            break;
        }

        const uint32_t chunkSize =
            readLE32(wavFile);

        const uint32_t chunkDataPos =
            wavFile.position();

        // ----------------------------------------------------
        // fmt
        // ----------------------------------------------------

        if (memcmp(chunkId, "fmt ", 4) == 0)
        {
            if (chunkSize < 16)
            {
                return false;
            }

            info.audioFormat =
                readLE16(wavFile);

            info.channels =
                readLE16(wavFile);

            info.sampleRate =
                readLE32(wavFile);

            info.byteRate =
                readLE32(wavFile);

            info.blockAlign =
                readLE16(wavFile);

            info.bitsPerSample =
                readLE16(wavFile);

            fmtFound = true;
        }

        // ----------------------------------------------------
        // data
        // ----------------------------------------------------

        else if (memcmp(chunkId, "data", 4) == 0)
        {
            info.dataOffset =
                chunkDataPos;

            info.dataSize =
                chunkSize;

            dataFound = true;

            if (fmtFound)
            {
                break;
            }
        }

        // ----------------------------------------------------
        // Skip unknown chunk.
        // ----------------------------------------------------

        const uint32_t nextPos =
            chunkDataPos + chunkSize;

        if (nextPos > fileSize)
        {
            return false;
        }

        wavFile.seek(nextPos);

        // WAV chunks are word aligned.
        if (chunkSize & 1)
        {
            wavFile.seek(
                wavFile.position() + 1
            );
        }
    }

    return fmtFound && dataFound;
}

// ============================================================
// Read LE16
// ============================================================

uint16_t SoundManager::readLE16(File& file)
{
    uint8_t b[2];

    if (file.read(b, 2) != 2)
    {
        return 0;
    }

    return
        static_cast<uint16_t>(
            b[0] |
            (static_cast<uint16_t>(b[1]) << 8)
        );
}

// ============================================================
// Read LE32
// ============================================================

uint32_t SoundManager::readLE32(File& file)
{
    uint8_t b[4];

    if (file.read(b, 4) != 4)
    {
        return 0;
    }

    return
        static_cast<uint32_t>(b[0]) |
        (static_cast<uint32_t>(b[1]) << 8) |
        (static_cast<uint32_t>(b[2]) << 16) |
        (static_cast<uint32_t>(b[3]) << 24);
}

// ============================================================
// Process playback
// ============================================================

void SoundManager::processPlayback()
{
    if (!file)
    {
        stop();
        return;
    }

    const uint32_t remaining =
        wav.dataSize - dataRead;

    if (remaining == 0)
    {
        stop();
        return;
    }

    // --------------------------------------------------------
    // Read one buffer.
    // --------------------------------------------------------

    size_t bytesToRead =
        BUFFER_SAMPLES *
        sizeof(int16_t);

    if (bytesToRead > remaining)
    {
        bytesToRead = remaining;
    }

    // Make sure we don't read a half sample.
    bytesToRead &= ~static_cast<size_t>(1);

    if (bytesToRead == 0)
    {
        stop();
        return;
    }

    const size_t samplesRead =
        file.read(
            reinterpret_cast<uint8_t*>(sampleBuffer),
            bytesToRead
        );

    if (samplesRead == 0)
    {
        stop();
        return;
    }

    const size_t sampleCount =
        samplesRead / sizeof(int16_t);

    // --------------------------------------------------------
    // Stereo -> mono.
    //
    // MAX98357A only needs one channel here.
    // --------------------------------------------------------

    if (wav.channels == 2)
    {
        const size_t stereoSamples =
            sampleCount / 2;

        for (
            size_t i = 0;
            i < stereoSamples;
            ++i
        )
        {
            const int32_t left =
                sampleBuffer[i * 2];

            const int32_t right =
                sampleBuffer[i * 2 + 1];

            const int32_t mono =
                (left + right) / 2;

            sampleBuffer[i] =
                static_cast<int16_t>(
                    constrain(
                        mono,
                        -32768,
                        32767
                    )
                );
        }

        const size_t monoCount =
            stereoSamples;

        const uint8_t volume =
            calculatePlaybackVolume();

        applyVolume(
            sampleBuffer,
            monoCount,
            volume
        );

        if (!writeAudio(
                sampleBuffer,
                monoCount
            ))
        {
            stop();
            return;
        }

        dataRead += samplesRead;
        positionSamples += monoCount;
    }
    else
    {
        const uint8_t volume =
            calculatePlaybackVolume();

        applyVolume(
            sampleBuffer,
            sampleCount,
            volume
        );

        if (!writeAudio(
                sampleBuffer,
                sampleCount
            ))
        {
            stop();
            return;
        }

        dataRead += samplesRead;
        positionSamples += sampleCount;
    }

    // --------------------------------------------------------
    // Automatic fade-out.
    //
    // Fade-out is started when remaining playback time is
    // less than fadeOutDuration.
    // --------------------------------------------------------

    if (
        fadeOutDuration > 0 &&
        !fadingOut
    )
    {
        const uint32_t duration =
            getDurationMs();

        const uint32_t position =
            getPositionMs();

        if (
            duration > position &&
            duration - position <= fadeOutDuration
        )
        {
            fadingOut = true;

            fadeOutStart =
                millis();
        }
    }
}

// ============================================================
// Write audio
// ============================================================

bool SoundManager::writeAudio(
    const int16_t* samples,
    size_t count
)
{
    if (!samples || count == 0)
    {
        return false;
    }

    const size_t bytes =
        count * sizeof(int16_t);

    size_t written = 0;

    const esp_err_t result =
        i2s_write(
            i2sManager.speakerPort(),
            samples,
            bytes,
            &written,
            portMAX_DELAY
        );

    if (result != ESP_OK)
    {
        Serial.print(
            "[SoundManager] I2S write error: "
        );

        Serial.println(
            esp_err_to_name(result)
        );

        return false;
    }

    return written > 0;
}

// ============================================================
// Calculate volume
// ============================================================

uint8_t SoundManager::calculatePlaybackVolume()
{
    float volume =
        static_cast<float>(localVolume)
        / 100.0f;

    // --------------------------------------------------------
    // Fade-in.
    // --------------------------------------------------------

    if (fadeInDuration > 0)
    {
        const uint32_t elapsed =
            millis() - fadeInStart;

        const float fade =
            calculateFade(
                elapsed,
                fadeInDuration,
                fadeInCurve
            );

        volume *= fade;
    }

    // --------------------------------------------------------
    // Fade-out.
    // --------------------------------------------------------

    if (
        fadingOut &&
        fadeOutDuration > 0
    )
    {
        const uint32_t elapsed =
            millis() - fadeOutStart;

        const float fade =
            calculateFade(
                elapsed,
                fadeOutDuration,
                fadeOutCurve
            );

        volume *= 1.0f - fade;

        if (elapsed >= fadeOutDuration)
        {
            volume = 0.0f;
        }
    }

    volume = constrain(
        volume,
        0.0f,
        1.0f
    );

    return static_cast<uint8_t>(
        volume * 100.0f
    );
}

// ============================================================
// Calculate fade
// ============================================================

float SoundManager::calculateFade(
    uint32_t elapsed,
    uint32_t duration,
    FadeCurve curve
) const
{
    if (duration == 0)
    {
        return 1.0f;
    }

    if (elapsed >= duration)
    {
        return 1.0f;
    }

    float x =
        static_cast<float>(elapsed)
        / static_cast<float>(duration);

    x = constrain(
        x,
        0.0f,
        1.0f
    );

    switch (curve)
    {
        case FadeCurve::Linear:
            return x;

        case FadeCurve::Exponential:
        {
            // Smooth slow start and fast finish.
            //
            // y = (e^(kx)-1)/(e^k-1)
            //
            // k = 5
            constexpr float k = 5.0f;

            const float numerator =
                expf(k * x) - 1.0f;

            const float denominator =
                expf(k) - 1.0f;

            return numerator / denominator;
        }

        case FadeCurve::Logarithmic:
        {
            // Fast start, slower finish.
            //
            // log2(1+x)
            return log2f(1.0f + x);
        }
    }

    return x;
}

// ============================================================
// Apply volume
// ============================================================

void SoundManager::applyVolume(
    int16_t* samples,
    size_t count,
    uint8_t volume
)
{
    if (!samples || count == 0)
    {
        return;
    }

    if (volume >= 100)
    {
        return;
    }

    if (volume == 0)
    {
        memset(
            samples,
            0,
            count * sizeof(int16_t)
        );

        return;
    }

    const int32_t gain =
        volume;

    for (size_t i = 0; i < count; ++i)
    {
        const int32_t value =
            static_cast<int32_t>(
                samples[i]
            );

        samples[i] =
            static_cast<int16_t>(
                (value * gain) / 100
            );
    }
}

// ============================================================
// Stop
// ============================================================

void SoundManager::stop()
{
    if (!playing && !file)
    {
        return;
    }

    playing = false;
    paused = false;

    fadingOut = false;

    if (file)
    {
        file.close();
    }

    clearSpeaker();

    dataRead = 0;
    positionSamples = 0;

    Serial.println(
        "[SoundManager] STOP"
    );
}

// ============================================================
// Pause
// ============================================================

void SoundManager::pause()
{
    if (!playing || paused)
    {
        return;
    }

    paused = true;

    pausedAtMs = millis();

    i2s_stop(
        i2sManager.speakerPort()
    );

    Serial.println(
        "[SoundManager] PAUSE"
    );
}

// ============================================================
// Resume
// ============================================================

void SoundManager::resume()
{
    if (!playing || !paused)
    {
        return;
    }

    paused = false;

    const uint32_t now =
        millis();

    const uint32_t pauseDuration =
        now - pausedAtMs;

    // Shift timers so fade calculations don't include pause.
    playbackStartMs += pauseDuration;
    fadeInStart += pauseDuration;

    if (fadeOutStart != 0)
    {
        fadeOutStart += pauseDuration;
    }

    if (!i2sManager.isSpeakerInitialized())
    {
        if (!i2sManager.beginSpeaker(
                wav.sampleRate
            ))
        {
            Serial.println(
                "[SoundManager] ERROR: cannot resume speaker"
            );

            playing = false;

            if (file)
            {
                file.close();
            }

            return;
        }
    }
    else
    {
        i2s_start(
            i2sManager.speakerPort()
        );
    }

    Serial.println(
        "[SoundManager] RESUME"
    );
}

// ============================================================
// Fade in
// ============================================================

void SoundManager::setFadeIn(
    uint32_t durationMs,
    FadeCurve curve
)
{
    fadeInDuration = durationMs;

    fadeInCurve = curve;

    fadeInStart = millis();
}

// ============================================================
// Fade out
// ============================================================

void SoundManager::setFadeOut(
    uint32_t durationMs,
    FadeCurve curve
)
{
    fadeOutDuration = durationMs;

    fadeOutCurve = curve;

    fadingOut = false;
}

// ============================================================
// Position
// ============================================================

uint32_t SoundManager::getPositionMs() const
{
    if (wav.sampleRate == 0)
    {
        return 0;
    }

    return static_cast<uint32_t>(
        (
            static_cast<uint64_t>(
                positionSamples
            ) * 1000ULL
        ) /
        wav.sampleRate
    );
}

// ============================================================
// Duration
// ============================================================

uint32_t SoundManager::getDurationMs() const
{
    if (
        wav.sampleRate == 0 ||
        wav.channels == 0 ||
        wav.bitsPerSample == 0
    )
    {
        return 0;
    }

    const uint32_t bytesPerSample =
        (
            static_cast<uint32_t>(
                wav.channels
            ) *
            wav.bitsPerSample
        ) / 8;

    if (bytesPerSample == 0)
    {
        return 0;
    }

    const uint32_t samples =
        wav.dataSize / bytesPerSample;

    return static_cast<uint32_t>(
        (
            static_cast<uint64_t>(
                samples
            ) * 1000ULL
        ) /
        wav.sampleRate
    );
}

// ============================================================
// Current volume
// ============================================================

uint8_t SoundManager::getCurrentVolume() const
{
    if (!playing)
    {
        return 0;
    }

    return const_cast<SoundManager*>(this)
        ->calculatePlaybackVolume();
}

// ============================================================
// Current path
// ============================================================

const char* SoundManager::getCurrentPath() const
{
    return currentPath;
}

// ============================================================
// Clear speaker
// ============================================================

void SoundManager::clearSpeaker()
{
    if (!i2sManager.isSpeakerInitialized())
    {
        return;
    }

    i2s_zero_dma_buffer(
        i2sManager.speakerPort()
    );
}
