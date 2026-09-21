#include "SoundManager.h"

#include <algorithm>
#include <cmath>
#include <cstring>


// ============================================================
// WAV constants
// ============================================================

static constexpr uint32_t RIFF_ID = 0x46464952; // "RIFF"
static constexpr uint32_t WAVE_ID = 0x45564157; // "WAVE"
static constexpr uint32_t FMT_ID  = 0x20746D66; // "fmt "
static constexpr uint32_t DATA_ID = 0x61746164; // "data"

// PCM
static constexpr uint16_t WAV_PCM = 1;

// ============================================================
// Math
// ============================================================

// static constexpr float TWO_PI = 6.28318530717958647692f;

// ============================================================
// Constructor
// ============================================================

SoundManager::SoundManager(
    SDManager& sd,
    const Settings::Audio& settings
)
    : _sd(sd),
      _settings(settings),

      _ready(false),
      _playing(false),
      _paused(false),

      _soundType(SoundType::NONE),
      _volumeMode(VolumeMode::GLOBAL),

      _localVolume(100),

      _fadeInMs(0),
      _fadeOutMs(0),
      _fadeCurve(FadeCurve::Linear),

      _manualFadeOut(false),
      _manualFadeOutStartMs(0),
      _manualFadeOutDurationMs(0),
      _manualFadeOutCurve(FadeCurve::Exponential),

      _audioFormat(0),
      _channels(0),
      _sampleRate(0),
      _byteRate(0),
      _blockAlign(0),
      _bitsPerSample(0),

      _dataSize(0),
      _dataPosition(0),
      _soundDurationMs(0),

      _inputSize(0),
      _outputSize(0),
      _outputPosition(0),

      _toneFrequency(0),
      _toneDurationMs(0),
      _toneElapsedMs(0),

      _sirenDurationMs(0),
      _sirenElapsedMs(0),

      _phase(0.0f),
      _sirenPhase(0.0f)
{
}

// ============================================================
// BEGIN
// ============================================================

bool SoundManager::begin()
{
    if (_ready)
        return true;

    if (!_sd.isReady())
    {

        return false;
    }

    if (!_settings.enabled)
    {

        return false;
    }

    if (!initI2S())
    {

        return false;
    }

    _ready = true;



    return true;
}

// ============================================================
// END
// ============================================================

void SoundManager::end()
{
    stop();

    if (_ready)
    {
        i2s_driver_uninstall(I2S_PORT);
    }

    _ready = false;
}

// ============================================================
// STATE
// ============================================================

bool SoundManager::isReady() const
{
    return _ready;
}

bool SoundManager::isPlaying() const
{
    return _playing;
}

bool SoundManager::isPaused() const
{
    return _paused;
}

SoundManager::SoundType SoundManager::getSoundType() const
{
    return _soundType;
}

SoundManager::VolumeMode SoundManager::getVolumeMode() const
{
    return _volumeMode;
}

// ============================================================
// GLOBAL VOLUME
// ============================================================

uint8_t SoundManager::getGlobalVolume() const
{
    return std::min<uint8_t>(
        _settings.volume,
        MAX_VOLUME
    );
}

// ============================================================
// LOCAL VOLUME
// ============================================================

void SoundManager::setLocalVolume(uint8_t volume)
{
    _localVolume = std::min<uint8_t>(
        volume,
        MAX_VOLUME
    );
}

uint8_t SoundManager::getLocalVolume() const
{
    return _localVolume;
}

void SoundManager::clearLocalVolume()
{
    _localVolume = 100;
}

// ============================================================
// TARGET VOLUME
// ============================================================

uint8_t SoundManager::getTargetVolume() const
{
    if (_volumeMode == VolumeMode::LOCAL)
    {
        return _localVolume;
    }

    return getGlobalVolume();
}

uint8_t SoundManager::getCurrentTargetVolume() const
{
    return getTargetVolume();
}

// ============================================================
// WAV - GLOBAL
// ============================================================

bool SoundManager::playWav(
    const char* path,
    uint32_t fadeInMs,
    uint32_t fadeOutMs,
    FadeCurve curve
)
{
    if (path == nullptr)
        return false;

    if (!_ready || !_sd.isReady() || !_settings.enabled)
        return false;

    stop();

    _volumeMode = VolumeMode::GLOBAL;

    _fadeInMs = fadeInMs;
    _fadeOutMs = fadeOutMs;
    _fadeCurve = curve;

    _file = _sd.card().fs().open(
        path,
        FILE_READ
    );

    if (!_file)
    {


        return false;
    }

    if (!parseWav())
    {


        _file.close();

        return false;
    }

    if (!configureI2S(
        _sampleRate,
        _channels
    ))
    {
        _file.close();
        return false;
    }

    if (_blockAlign == 0 || _sampleRate == 0)
    {
        _file.close();
        return false;
    }

    uint32_t frames =
        _dataSize / _blockAlign;

    _soundDurationMs =
        static_cast<uint32_t>(
            (static_cast<uint64_t>(frames) * 1000ULL)
            / _sampleRate
        );

    _dataPosition = 0;

    _inputSize = 0;
    _outputSize = 0;
    _outputPosition = 0;

    _soundType = SoundType::WAV;

    _playing = true;
    _paused = false;

    _manualFadeOut = false;

    i2s_start(I2S_PORT);

    return true;
}

// ============================================================
// WAV - STRING GLOBAL
// ============================================================

bool SoundManager::playWav(
    const String& path,
    uint32_t fadeInMs,
    uint32_t fadeOutMs,
    FadeCurve curve
)
{
    return playWav(
        path.c_str(),
        fadeInMs,
        fadeOutMs,
        curve
    );
}

// ============================================================
// WAV - LOCAL
// ============================================================

bool SoundManager::playWavLocal(
    const char* path,
    uint8_t localVolume,
    uint32_t fadeInMs,
    uint32_t fadeOutMs,
    FadeCurve curve
)
{
    if (path == nullptr)
        return false;

    if (!_ready || !_sd.isReady() || !_settings.enabled)
        return false;

    stop();

    _volumeMode = VolumeMode::LOCAL;

    _localVolume = std::min<uint8_t>(
        localVolume,
        MAX_VOLUME
    );

    _fadeInMs = fadeInMs;
    _fadeOutMs = fadeOutMs;
    _fadeCurve = curve;

    _file = _sd.card().fs().open(
        path,
        FILE_READ
    );

    if (!_file)
    {


        return false;
    }

    if (!parseWav())
    {
        _file.close();
        return false;
    }

    if (!configureI2S(
        _sampleRate,
        _channels
    ))
    {
        _file.close();
        return false;
    }

    uint32_t frames =
        _dataSize / _blockAlign;

    _soundDurationMs =
        static_cast<uint32_t>(
            (static_cast<uint64_t>(frames) * 1000ULL)
            / _sampleRate
        );

    _dataPosition = 0;

    _inputSize = 0;
    _outputSize = 0;
    _outputPosition = 0;

    _soundType = SoundType::WAV;

    _playing = true;
    _paused = false;

    _manualFadeOut = false;

    i2s_start(I2S_PORT);

    return true;
}

// ============================================================
// WAV - STRING LOCAL
// ============================================================

bool SoundManager::playWavLocal(
    const String& path,
    uint8_t localVolume,
    uint32_t fadeInMs,
    uint32_t fadeOutMs,
    FadeCurve curve
)
{
    return playWavLocal(
        path.c_str(),
        localVolume,
        fadeInMs,
        fadeOutMs,
        curve
    );
}

// ============================================================
// ALARM
// ============================================================

bool SoundManager::playAlarm(
    const char* path,
    uint8_t localVolume,
    uint32_t fadeInMs,
    uint32_t fadeOutMs,
    FadeCurve curve
)
{
    return playWavLocal(
        path,
        localVolume,
        fadeInMs,
        fadeOutMs,
        curve
    );
}

// ============================================================
// ALARM - STRING
// ============================================================

bool SoundManager::playAlarm(
    const String& path,
    uint8_t localVolume,
    uint32_t fadeInMs,
    uint32_t fadeOutMs,
    FadeCurve curve
)
{
    return playAlarm(
        path.c_str(),
        localVolume,
        fadeInMs,
        fadeOutMs,
        curve
    );
}

// ============================================================
// TONE - GLOBAL
// ============================================================

bool SoundManager::playTone(
    uint16_t frequency,
    uint32_t durationMs,
    uint32_t fadeInMs,
    uint32_t fadeOutMs,
    FadeCurve curve
)
{
    if (!_ready || !_settings.enabled)
        return false;

    stop();

    _volumeMode = VolumeMode::GLOBAL;

    _toneFrequency = frequency;

    _toneDurationMs = durationMs;

    _toneElapsedMs = 0;

    _fadeInMs = fadeInMs;

    _fadeOutMs = fadeOutMs;

    _fadeCurve = curve;

    _phase = 0.0f;

    _soundDurationMs = durationMs;

    if (!configureI2S(
        DEFAULT_SAMPLE_RATE,
        2
    ))
    {
        return false;
    }

    _soundType = SoundType::TONE;

    _playing = true;
    _paused = false;

    _manualFadeOut = false;

    i2s_start(I2S_PORT);

    return true;
}

// ============================================================
// TONE - LOCAL
// ============================================================

bool SoundManager::playToneLocal(
    uint16_t frequency,
    uint32_t durationMs,
    uint8_t localVolume,
    uint32_t fadeInMs,
    uint32_t fadeOutMs,
    FadeCurve curve
)
{
    if (!_ready || !_settings.enabled)
        return false;

    stop();

    _volumeMode = VolumeMode::LOCAL;

    _localVolume = std::min<uint8_t>(
        localVolume,
        MAX_VOLUME
    );

    _toneFrequency = frequency;

    _toneDurationMs = durationMs;

    _toneElapsedMs = 0;

    _fadeInMs = fadeInMs;

    _fadeOutMs = fadeOutMs;

    _fadeCurve = curve;

    _phase = 0.0f;

    _soundDurationMs = durationMs;

    if (!configureI2S(
        DEFAULT_SAMPLE_RATE,
        2
    ))
    {
        return false;
    }

    _soundType = SoundType::TONE;

    _playing = true;
    _paused = false;

    _manualFadeOut = false;

    i2s_start(I2S_PORT);

    return true;
}

// ============================================================
// BEEP
// ============================================================

bool SoundManager::playBeep(
    uint32_t fadeInMs,
    uint32_t fadeOutMs,
    FadeCurve curve
)
{
    return playTone(
        1000,
        150,
        fadeInMs,
        fadeOutMs,
        curve
    );
}

// ============================================================
// SIREN
// ============================================================

bool SoundManager::playSiren(
    uint32_t durationMs,
    uint32_t fadeInMs,
    uint32_t fadeOutMs,
    FadeCurve curve
)
{
    if (!_ready || !_settings.enabled)
        return false;

    stop();

    _volumeMode = VolumeMode::GLOBAL;

    _sirenDurationMs = durationMs;

    _sirenElapsedMs = 0;

    _fadeInMs = fadeInMs;

    _fadeOutMs = fadeOutMs;

    _fadeCurve = curve;

    _sirenPhase = 0.0f;

    _soundDurationMs = durationMs;

    if (!configureI2S(
        DEFAULT_SAMPLE_RATE,
        2
    ))
    {
        return false;
    }

    _soundType = SoundType::SIREN;

    _playing = true;
    _paused = false;

    _manualFadeOut = false;

    i2s_start(I2S_PORT);

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void SoundManager::update()
{
    if (!_ready)
        return;

    if (!_playing)
        return;

    if (_paused)
        return;

    processAudio();
}

// ============================================================
// PROCESS AUDIO
// ============================================================

void SoundManager::processAudio()
{
    switch (_soundType)
    {
        case SoundType::WAV:
            processWav();
            break;

        case SoundType::TONE:
            processTone();
            break;

        case SoundType::SIREN:
            processSiren();
            break;

        case SoundType::NONE:
        default:
            break;
    }
}

// ============================================================
// WAV PROCESS
// ============================================================

bool SoundManager::processWav()
{
    // --------------------------------------------------------
    // First write data already waiting in output buffer.
    // --------------------------------------------------------

    if (_outputPosition < _outputSize)
    {
        if (!writeOutput())
            return true;

        if (_outputPosition < _outputSize)
            return true;

        _outputSize = 0;
        _outputPosition = 0;
    }

    // --------------------------------------------------------
    // End of file
    // --------------------------------------------------------

    if (_dataPosition >= _dataSize)
    {
        stop();
        return false;
    }

    // --------------------------------------------------------
    // Read a chunk
    // --------------------------------------------------------

    uint32_t remaining =
        _dataSize - _dataPosition;

    size_t bytesToRead =
        std::min<size_t>(
            INPUT_BUFFER_SIZE,
            remaining
        );

    size_t readBytes =
        _file.read(
            _inputBuffer,
            bytesToRead
        );

    if (readBytes == 0)
    {
        stop();
        return false;
    }

    _dataPosition += readBytes;

    // --------------------------------------------------------
    // Only complete samples
    // --------------------------------------------------------

    if (_channels == 1)
    {
        size_t samples =
            readBytes / sizeof(int16_t);

        processWavMono(
            reinterpret_cast<const int16_t*>(
                _inputBuffer
            ),
            samples
        );
    }
    else if (_channels == 2)
    {
        size_t samples =
            readBytes / sizeof(int16_t);

        processWavStereo(
            reinterpret_cast<const int16_t*>(
                _inputBuffer
            ),
            samples
        );
    }
    else
    {
        stop();
        return false;
    }

    return true;
}

// ============================================================
// WAV MONO
// ============================================================

size_t SoundManager::processWavMono(
    const int16_t* input,
    size_t samples
)
{
    size_t frames = samples;

    size_t maxFrames =
        OUTPUT_BUFFER_SIZE / (sizeof(int16_t) * 2);

    frames = std::min(
        frames,
        maxFrames
    );

    int16_t* output =
        reinterpret_cast<int16_t*>(
            _outputBuffer
        );

    for (size_t i = 0; i < frames; ++i)
    {
        int16_t sample =
            applyVolume(input[i]);

        output[i * 2]     = sample;
        output[i * 2 + 1] = sample;
    }

    _outputSize =
        frames * sizeof(int16_t) * 2;

    _outputPosition = 0;

    return frames;
}

// ============================================================
// WAV STEREO
// ============================================================

size_t SoundManager::processWavStereo(
    const int16_t* input,
    size_t samples
)
{
    size_t frames = samples / 2;

    size_t maxFrames =
        OUTPUT_BUFFER_SIZE / (sizeof(int16_t) * 2);

    frames = std::min(
        frames,
        maxFrames
    );

    int16_t* output =
        reinterpret_cast<int16_t*>(
            _outputBuffer
        );

    for (size_t i = 0; i < frames; ++i)
    {
        output[i * 2] =
            applyVolume(input[i * 2]);

        output[i * 2 + 1] =
            applyVolume(input[i * 2 + 1]);
    }

    _outputSize =
        frames * sizeof(int16_t) * 2;

    _outputPosition = 0;

    return frames;
}

// ============================================================
// TONE PROCESS
// ============================================================

bool SoundManager::processTone()
{
    if (_outputPosition < _outputSize)
    {
        if (!writeOutput())
            return true;

        if (_outputPosition < _outputSize)
            return true;

        _outputSize = 0;
        _outputPosition = 0;
    }

    if (_toneElapsedMs >= _toneDurationMs)
    {
        stop();
        return false;
    }

    constexpr size_t FRAMES = 512;

    int16_t* output =
        reinterpret_cast<int16_t*>(
            _outputBuffer
        );

    size_t remainingFrames =
        static_cast<size_t>(
            (
                static_cast<uint64_t>(
                    _toneDurationMs -
                    _toneElapsedMs
                )
                * DEFAULT_SAMPLE_RATE
            ) / 1000ULL
        );

    size_t frames =
        std::min(
            FRAMES,
            remainingFrames
        );

    if (frames == 0)
    {
        stop();
        return false;
    }

    for (size_t i = 0; i < frames; ++i)
    {
        float sample =
            sinf(_phase) * 0.8f;

        int16_t pcm =
            static_cast<int16_t>(
                sample * 32767.0f
            );

        pcm = applyVolume(pcm);

        output[i * 2]     = pcm;
        output[i * 2 + 1] = pcm;

        _phase +=
            TWO_PI *
            static_cast<float>(_toneFrequency) /
            static_cast<float>(DEFAULT_SAMPLE_RATE);

        if (_phase >= TWO_PI)
            _phase -= TWO_PI;
    }

    uint32_t elapsedIncrement =
        static_cast<uint32_t>(
            (
                static_cast<uint64_t>(frames) *
                1000ULL
            ) /
            DEFAULT_SAMPLE_RATE
        );

    _toneElapsedMs += elapsedIncrement;

    _outputSize =
        frames *
        sizeof(int16_t) *
        2;

    _outputPosition = 0;

    return true;
}

// ============================================================
// SIREN PROCESS
// ============================================================

bool SoundManager::processSiren()
{
    if (_outputPosition < _outputSize)
    {
        if (!writeOutput())
            return true;

        if (_outputPosition < _outputSize)
            return true;

        _outputSize = 0;
        _outputPosition = 0;
    }

    if (_sirenElapsedMs >= _sirenDurationMs)
    {
        stop();
        return false;
    }

    constexpr size_t FRAMES = 512;

    int16_t* output =
        reinterpret_cast<int16_t*>(
            _outputBuffer
        );

    uint32_t remainingMs =
        _sirenDurationMs -
        _sirenElapsedMs;

    size_t remainingFrames =
        static_cast<size_t>(
            (
                static_cast<uint64_t>(remainingMs) *
                DEFAULT_SAMPLE_RATE
            ) / 1000ULL
        );

    size_t frames =
        std::min(
            FRAMES,
            remainingFrames
        );

    if (frames == 0)
    {
        stop();
        return false;
    }

    for (size_t i = 0; i < frames; ++i)
    {
        float progress =
            static_cast<float>(_sirenElapsedMs) /
            static_cast<float>(
                std::max<uint32_t>(
                    _sirenDurationMs,
                    1
                )
            );

        // 600 Hz -> 1400 Hz
        float frequency =
            600.0f +
            800.0f *
            (0.5f + 0.5f * sinf(
                progress * TWO_PI * 2.0f
            ));

        float sample =
            sinf(_sirenPhase) * 0.8f;

        int16_t pcm =
            static_cast<int16_t>(
                sample * 32767.0f
            );

        pcm = applyVolume(pcm);

        output[i * 2]     = pcm;
        output[i * 2 + 1] = pcm;

        _sirenPhase +=
            TWO_PI *
            frequency /
            static_cast<float>(
                DEFAULT_SAMPLE_RATE
            );

        if (_sirenPhase >= TWO_PI)
            _sirenPhase -= TWO_PI;

        uint32_t frameElapsed =
            static_cast<uint32_t>(
                (
                    static_cast<uint64_t>(i + 1) *
                    1000ULL
                ) /
                DEFAULT_SAMPLE_RATE
            );

        if (frameElapsed == 0)
            frameElapsed = 1;

        (void)frameElapsed;
    }

    _sirenElapsedMs +=
        static_cast<uint32_t>(
            (
                static_cast<uint64_t>(frames) *
                1000ULL
            ) /
            DEFAULT_SAMPLE_RATE
        );

    _outputSize =
        frames *
        sizeof(int16_t) *
        2;

    _outputPosition = 0;

    return true;
}

// ============================================================
// WRITE OUTPUT
// ============================================================

bool SoundManager::writeOutput()
{
    if (_outputPosition >= _outputSize)
        return true;

    size_t bytesWritten = 0;

    esp_err_t result =
        i2s_write(
            I2S_PORT,
            _outputBuffer + _outputPosition,
            _outputSize - _outputPosition,
            &bytesWritten,
            0
        );

    if (result != ESP_OK)
        return false;

    if (bytesWritten == 0)
        return false;

    _outputPosition += bytesWritten;

    return true;
}

// ============================================================
// VOLUME
// ============================================================

int16_t SoundManager::applyVolume(
    int16_t sample
) const
{
    uint8_t targetVolume =
        getTargetVolume();

    if (targetVolume == 0)
        return 0;

    float fade =
        getFadeVolume() / 100.0f;

    float volume =
        static_cast<float>(targetVolume) /
        100.0f;

    float result =
        static_cast<float>(sample) *
        volume *
        fade;

    // --------------------------------------------------------
    // Saturation
    // --------------------------------------------------------

    if (result > 32767.0f)
        result = 32767.0f;

    if (result < -32768.0f)
        result = -32768.0f;

    return static_cast<int16_t>(result);
}

// ============================================================
// FADE VOLUME
// ============================================================

uint8_t SoundManager::getFadeVolume() const
{
    return static_cast<uint8_t>(
        std::round(
            getFadeFactor() * 100.0f
        )
    );
}

// ============================================================
// FADE FACTOR
// ============================================================

float SoundManager::getFadeFactor() const
{
    uint32_t elapsedMs = 0;

    // --------------------------------------------------------
    // Current playback position
    // --------------------------------------------------------

    if (_soundType == SoundType::WAV)
    {
        if (_blockAlign != 0 &&
            _sampleRate != 0)
        {
            uint32_t frames =
                _dataPosition /
                _blockAlign;

            elapsedMs =
                static_cast<uint32_t>(
                    (
                        static_cast<uint64_t>(frames) *
                        1000ULL
                    ) /
                    _sampleRate
                );
        }
    }
    else if (_soundType == SoundType::TONE)
    {
        elapsedMs = _toneElapsedMs;
    }
    else if (_soundType == SoundType::SIREN)
    {
        elapsedMs = _sirenElapsedMs;
    }

    // --------------------------------------------------------
    // Manual fade-out
    // --------------------------------------------------------

    if (_manualFadeOut)
    {
        if (elapsedMs <= _manualFadeOutStartMs)
            return 1.0f;

        uint32_t passed =
            elapsedMs -
            _manualFadeOutStartMs;

        if (passed >= _manualFadeOutDurationMs)
            return 0.0f;

        float progress =
            static_cast<float>(passed) /
            static_cast<float>(
                _manualFadeOutDurationMs
            );

        float curved =
            applyFadeCurve(
                progress
            );

        return 1.0f - curved;
    }

    // --------------------------------------------------------
    // Fade in
    // --------------------------------------------------------

    float factor = 1.0f;

    if (_fadeInMs > 0)
    {
        if (elapsedMs < _fadeInMs)
        {
            float progress =
                static_cast<float>(elapsedMs) /
                static_cast<float>(_fadeInMs);

            factor =
                applyFadeCurve(progress);
        }
    }

    // --------------------------------------------------------
    // Fade out
    // --------------------------------------------------------

    if (_fadeOutMs > 0 &&
        _soundDurationMs > 0)
    {
        uint32_t fadeStart =
            _soundDurationMs >
            _fadeOutMs
                ? _soundDurationMs - _fadeOutMs
                : 0;

        if (elapsedMs >= fadeStart)
        {
            float progress;

            if (_fadeOutMs == 0)
            {
                progress = 1.0f;
            }
            else
            {
                progress =
                    static_cast<float>(
                        elapsedMs - fadeStart
                    ) /
                    static_cast<float>(
                        _fadeOutMs
                    );
            }

            progress =
                std::min(
                    std::max(progress, 0.0f),
                    1.0f
                );

            float fadeOut =
                applyFadeCurve(progress);

            factor =
                std::min(
                    factor,
                    1.0f - fadeOut
                );
        }
    }

    return std::min(
        std::max(factor, 0.0f),
        1.0f
    );
}

// ============================================================
// FADE CURVE
// ============================================================

float SoundManager::applyFadeCurve(
    float progress
) const
{
    progress =
        std::min(
            std::max(progress, 0.0f),
            1.0f
        );

    FadeCurve curve =
        _manualFadeOut
            ? _manualFadeOutCurve
            : _fadeCurve;

    switch (curve)
    {
        case FadeCurve::Linear:
            return progress;

        case FadeCurve::Exponential:
        {
            // Slow beginning,
            // stronger increase near the end.
            //
            // 0%
            // 1%
            // 6%
            // 16%
            // 32%
            // 56%
            // 100%
            //
            return powf(progress, 2.2f);
        }

        case FadeCurve::Logarithmic:
        {
            // Fast beginning,
            // slower near the end.
            return log10f(
                1.0f +
                9.0f * progress
            );
        }

        default:
            return progress;
    }
}

// ============================================================
// STOP
// ============================================================

void SoundManager::stop()
{
    if (!_ready)
        return;

    i2s_stop(I2S_PORT);

    i2s_zero_dma_buffer(I2S_PORT);

    if (_file)
        _file.close();

    resetPlaybackState();
}

// ============================================================
// PAUSE
// ============================================================

void SoundManager::pause()
{
    if (!_ready)
        return;

    if (!_playing)
        return;

    if (_paused)
        return;

    // --------------------------------------------------------
    // Immediately stop I2S DMA.
    // --------------------------------------------------------

    i2s_stop(I2S_PORT);

    i2s_zero_dma_buffer(I2S_PORT);

    // --------------------------------------------------------
    // Do NOT close file.
    // Do NOT reset buffers.
    // Do NOT reset position.
    // --------------------------------------------------------

    _paused = true;
}

// ============================================================
// RESUME
// ============================================================

void SoundManager::resume()
{
    if (!_ready)
        return;

    if (!_playing)
        return;

    if (!_paused)
        return;

    i2s_start(I2S_PORT);

    _paused = false;
}

// ============================================================
// MANUAL FADE OUT
// ============================================================

void SoundManager::fadeOut(
    uint32_t durationMs,
    FadeCurve curve
)
{
    if (!_playing)
        return;

    if (durationMs == 0)
    {
        stop();
        return;
    }

    uint32_t elapsedMs = 0;

    if (_soundType == SoundType::WAV)
    {
        if (_blockAlign != 0 &&
            _sampleRate != 0)
        {
            uint32_t frames =
                _dataPosition /
                _blockAlign;

            elapsedMs =
                static_cast<uint32_t>(
                    (
                        static_cast<uint64_t>(frames) *
                        1000ULL
                    ) /
                    _sampleRate
                );
        }
    }
    else if (_soundType == SoundType::TONE)
    {
        elapsedMs = _toneElapsedMs;
    }
    else if (_soundType == SoundType::SIREN)
    {
        elapsedMs = _sirenElapsedMs;
    }

    _manualFadeOut = true;

    _manualFadeOutStartMs =
        elapsedMs;

    _manualFadeOutDurationMs =
        durationMs;

    _manualFadeOutCurve =
        curve;
}

// ============================================================
// FADE SETTINGS
// ============================================================

void SoundManager::setFadeIn(
    uint32_t durationMs
)
{
    _fadeInMs = durationMs;
}

void SoundManager::setFadeOut(
    uint32_t durationMs
)
{
    _fadeOutMs = durationMs;
}

void SoundManager::setFadeCurve(
    FadeCurve curve
)
{
    _fadeCurve = curve;
}

uint32_t SoundManager::getFadeIn() const
{
    return _fadeInMs;
}

uint32_t SoundManager::getFadeOut() const
{
    return _fadeOutMs;
}

SoundManager::FadeCurve SoundManager::getFadeCurve() const
{
    return _fadeCurve;
}

// ============================================================
// WAV PARSER
// ============================================================

bool SoundManager::parseWav()
{
    if (!_file)
        return false;

    uint32_t riff;
    uint32_t riffSize;
    uint32_t wave;

    if (_file.read(
        reinterpret_cast<uint8_t*>(&riff),
        4
    ) != 4)
    {
        return false;
    }

    if (riff != RIFF_ID)
        return false;

    if (_file.read(
        reinterpret_cast<uint8_t*>(&riffSize),
        4
    ) != 4)
    {
        return false;
    }

    (void)riffSize;

    if (_file.read(
        reinterpret_cast<uint8_t*>(&wave),
        4
    ) != 4)
    {
        return false;
    }

    if (wave != WAVE_ID)
        return false;

    if (!findFmtChunk())
        return false;

    if (!findDataChunk())
        return false;

    // --------------------------------------------------------
    // Supported format:
    // PCM / 16 bit / mono or stereo
    // --------------------------------------------------------

    if (_audioFormat != WAV_PCM)
        return false;

    if (_bitsPerSample != 16)
        return false;

    if (_channels != 1 &&
        _channels != 2)
    {
        return false;
    }

    if (_sampleRate < MIN_SAMPLE_RATE ||
        _sampleRate > MAX_SAMPLE_RATE)
    {
        return false;
    }

    if (_blockAlign == 0)
        return false;

    return true;
}

// ============================================================
// FIND FMT
// ============================================================

bool SoundManager::findFmtChunk()
{
    while (_file.available())
    {
        uint32_t id;
        uint32_t size;

        if (!readChunkHeader(id, size))
            return false;

        if (id == FMT_ID)
        {
            if (size < 16)
                return false;

            if (_file.read(
                reinterpret_cast<uint8_t*>(
                    &_audioFormat
                ),
                2
            ) != 2)
            {
                return false;
            }

            if (_file.read(
                reinterpret_cast<uint8_t*>(
                    &_channels
                ),
                2
            ) != 2)
            {
                return false;
            }

            if (_file.read(
                reinterpret_cast<uint8_t*>(
                    &_sampleRate
                ),
                4
            ) != 4)
            {
                return false;
            }

            if (_file.read(
                reinterpret_cast<uint8_t*>(
                    &_byteRate
                ),
                4
            ) != 4)
            {
                return false;
            }

            if (_file.read(
                reinterpret_cast<uint8_t*>(
                    &_blockAlign
                ),
                2
            ) != 2)
            {
                return false;
            }

            if (_file.read(
                reinterpret_cast<uint8_t*>(
                    &_bitsPerSample
                ),
                2
            ) != 2)
            {
                return false;
            }

            if (size > 16)
            {
                if (!skipBytes(size - 16))
                    return false;
            }

            return true;
        }

        if (!skipBytes(size))
            return false;
    }

    return false;
}

// ============================================================
// FIND DATA
// ============================================================

bool SoundManager::findDataChunk()
{
    while (_file.available())
    {
        uint32_t id;
        uint32_t size;

        if (!readChunkHeader(id, size))
            return false;

        if (id == DATA_ID)
        {
            _dataSize = size;

            _dataPosition =
                static_cast<uint32_t>(
                    _file.position()
                );

            return true;
        }

        if (!skipBytes(size))
            return false;
    }

    return false;
}

// ============================================================
// READ CHUNK HEADER
// ============================================================

bool SoundManager::readChunkHeader(
    uint32_t& id,
    uint32_t& size
)
{
    if (_file.read(
        reinterpret_cast<uint8_t*>(&id),
        4
    ) != 4)
    {
        return false;
    }

    if (_file.read(
        reinterpret_cast<uint8_t*>(&size),
        4
    ) != 4)
    {
        return false;
    }

    return true;
}

// ============================================================
// SKIP BYTES
// ============================================================

bool SoundManager::skipBytes(
    uint32_t bytes
)
{
    return _file.seek(
        _file.position() + bytes
    );
}

// ============================================================
// I2S INIT
// ============================================================

bool SoundManager::initI2S()
{
    i2s_config_t config = {};

    config.mode =
        static_cast<i2s_mode_t>(
            I2S_MODE_MASTER |
            I2S_MODE_TX
        );

    config.sample_rate =
        DEFAULT_SAMPLE_RATE;

    config.bits_per_sample =
        I2S_BITS_PER_SAMPLE_16BIT;

    config.channel_format =
        I2S_CHANNEL_FMT_RIGHT_LEFT;

    config.communication_format =
        I2S_COMM_FORMAT_STAND_I2S;

    config.intr_alloc_flags =
        ESP_INTR_FLAG_LEVEL1;

    config.dma_buf_count = 8;

    config.dma_buf_len = 256;

    config.use_apll = false;

    config.tx_desc_auto_clear = true;

    config.fixed_mclk = 0;

    esp_err_t result =
        i2s_driver_install(
            I2S_PORT,
            &config,
            0,
            nullptr
        );

    if (result != ESP_OK)
        return false;

    i2s_pin_config_t pins = {};

    pins.bck_io_num =
        I2S_BCLK;

    pins.ws_io_num =
        I2S_LRC;

    pins.data_out_num =
        I2S_DOUT;

    pins.data_in_num =
        I2S_PIN_NO_CHANGE;

    result =
        i2s_set_pin(
            I2S_PORT,
            &pins
        );

    if (result != ESP_OK)
    {
        i2s_driver_uninstall(I2S_PORT);
        return false;
    }

    i2s_zero_dma_buffer(I2S_PORT);

    return true;
}

// ============================================================
// CONFIGURE I2S
// ============================================================

bool SoundManager::configureI2S(
    uint32_t sampleRate,
    uint16_t channels
)
{
    if (channels != 1 &&
        channels != 2)
    {
        return false;
    }

    esp_err_t result =
        i2s_set_clk(
            I2S_PORT,
            sampleRate,
            I2S_BITS_PER_SAMPLE_16BIT,
            I2S_CHANNEL_STEREO
        );

    if (result != ESP_OK)
        return false;

    i2s_zero_dma_buffer(I2S_PORT);

    return true;
}

// ============================================================
// CLEAR I2S
// ============================================================

void SoundManager::clearI2S()
{
    i2s_stop(I2S_PORT);

    i2s_zero_dma_buffer(I2S_PORT);
}

// ============================================================
// RESET PLAYBACK
// ============================================================

void SoundManager::resetPlaybackState()
{
    _playing = false;

    _paused = false;

    _soundType = SoundType::NONE;

    _inputSize = 0;

    _outputSize = 0;

    _outputPosition = 0;

    _dataPosition = 0;

    _soundDurationMs = 0;

    _toneElapsedMs = 0;

    _sirenElapsedMs = 0;

    _manualFadeOut = false;

    _phase = 0.0f;

    _sirenPhase = 0.0f;
}

// ============================================================
// INFORMATION
// ============================================================

uint32_t SoundManager::getSampleRate() const
{
    return _sampleRate;
}

uint16_t SoundManager::getChannels() const
{
    return _channels;
}

uint16_t SoundManager::getBitsPerSample() const
{
    return _bitsPerSample;
}

uint32_t SoundManager::getDataSize() const
{
    return _dataSize;
}

uint32_t SoundManager::getDataPosition() const
{
    return _dataPosition;
}

uint32_t SoundManager::getDurationMs() const
{
    return _soundDurationMs;
}
